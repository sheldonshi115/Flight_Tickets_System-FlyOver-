import os
import pandas as pd
from datasets import Dataset, DatasetDict
from transformers import (
    AutoModelForCausalLM,
    AutoTokenizer,
    BitsAndBytesConfig,
    TrainingArguments,
    pipeline
)
from peft import LoraConfig, get_peft_model, prepare_model_for_kbit_training
from trl import SFTTrainer
import torch

# ====================== 1. 配置参数（根据你的环境修改）======================
LOCAL_MODEL_PATH = "./model_4B"  # 本地Qwen 4B模型路径（之前下载的目录）
CSV_DIR = "./data/filtered_city_data"  # 筛选后的CSV文件目录（含各城市景点数据）
OUTPUT_DIR = "./qwen-4b-tourism-lora"  # LoRA适配器保存路径
DEVICE = "cuda:0"  # 自动识别GPU/CPU
MAX_SEQ_LENGTH = 1024  # 最大序列长度（根据显存调整，4B模型建议≤1024）
BATCH_SIZE = 2  # 批次大小（显存8GB用2，16GB用4）
LEARNING_RATE = 2e-4  # LoRA学习率（推荐1e-4~5e-4）
TRAIN_EPOCHS = 3  # 训练轮次（景点数据建议2~3轮，避免过拟合）

# 需提取的训练字段（与CSV文件列名一致，若列名不同请修改此处）
TRAIN_FIELDS = [
    "名字", "地址", "介绍", "开放时间", "建议游玩时间", "建议季节", "门票"
]

# ====================== 2. 数据预处理：读取CSV并构建训练样本 ======================
def load_and_format_data(csv_dir, train_fields):
    """遍历CSV文件，提取字段并构建指令式训练样本"""
    all_samples = []
    
    # 遍历所有CSV文件
    for csv_file in os.listdir(csv_dir):
        if not csv_file.endswith(".csv"):
            continue
        csv_path = os.path.join(csv_dir, csv_file)
        
        # 读取CSV（处理编码问题，根据实际情况调整encoding）
        try:
            df = pd.read_csv(csv_path, encoding="utf-8")
        except:
            df = pd.read_csv(csv_path, encoding="gbk")
        
        # 过滤有效字段（只保留需要的列，缺失列填充空值）
        df = df[train_fields].fillna("无")
        
        # 构建训练样本（指令+响应格式，适配Qwen对话习惯）
        for _, row in df.iterrows():
            # 指令：明确要求模型返回景点信息
            prompt = f"""请详细介绍景点《{row['名字']}》的相关信息，包含以下内容：
1. 地址：{row['地址']}
2. 开放时间：{row['开放时间']}
3. 建议游玩时间：{row['建议游玩时间']}
4. 建议季节：{row['建议季节']}
5. 门票信息：{row['门票']}
6. 景点介绍：{row['介绍']}

请用清晰、简洁的语言整理上述信息，方便用户快速了解景点。"""
            
            # 响应：结构化整理后的景点信息（模型需要学习的目标输出）
            response = f"""景点名称：{row['名字']}
地址：{row['地址']}
开放时间：{row['开放时间']}
建议游玩时间：{row['建议游玩时间']}
建议季节：{row['建议季节']}
门票信息：{row['门票']}
景点介绍：{row['介绍']}"""
            
            # 按Qwen要求的格式拼接（prompt + response，用tokenizer的eos_token分隔）
            all_samples.append({
                "text": f"用户：{prompt}\n助手：{response}" + tokenizer.eos_token
            })
    
    dataset = Dataset.from_list(all_samples)
    dataset_split = dataset.train_test_split(test_size=0.1, seed=42)
    # 关键修改：兼容旧版datasets（将'test'键改为'validation'）
    if "test" in dataset_split and "validation" not in dataset_split:
        dataset_split["validation"] = dataset_split.pop("test")
    return dataset_split

# ====================== 3. 加载Tokenizer和Qwen模型 ======================
# 加载Tokenizer（Qwen专用，需trust_remote_code=True）
tokenizer = AutoTokenizer.from_pretrained(
    LOCAL_MODEL_PATH,
    trust_remote_code=True,
    padding_side="right",  # 右填充（避免生成时警告）
    truncation_side="left"  # 长文本左截断（保留尾部响应信息）
)
tokenizer.pad_token = tokenizer.eos_token  # 填充token设为结束token（Qwen默认无pad_token）

# 配置4bit量化（显存优化关键，8GB GPU必备）
bnb_config = BitsAndBytesConfig(
    load_in_4bit=True,  # 4bit量化（显存占用≈6GB）
    bnb_4bit_use_double_quant=True,  # 双量化（进一步节省显存）
    bnb_4bit_quant_type="nf4",  # 量化类型（适合LLM）
    bnb_4bit_compute_dtype=torch.float16  # 计算精度
)

# 加载本地Qwen 4B模型
model = AutoModelForCausalLM.from_pretrained(
    LOCAL_MODEL_PATH,
    trust_remote_code=True,
    quantization_config=bnb_config,
    device_map={"": DEVICE},
    torch_dtype=torch.float16,
    low_cpu_mem_usage=True  # 降低CPU内存占用
)

# 准备模型用于LoRA训练（冻结原始模型权重，只训练LoRA适配器）
model = prepare_model_for_kbit_training(model)

# ====================== 4. 配置LoRA参数（核心）======================
lora_config = LoraConfig(
    r=8,  # LoRA秩（越大效果越好，但显存占用越高，建议8~16）
    lora_alpha=32,  # 缩放因子（通常是r的4倍）
    target_modules=["q_proj", "k_proj", "v_proj", "o_proj", "gate_proj", "up_proj", "down_proj"],  # Qwen的目标模块（必须正确）
    lora_dropout=0.05,  # Dropout比例（防止过拟合）
    bias="none",  # 不训练偏置项
    task_type="CAUSAL_LM"  # 因果语言模型（生成任务）
)

# 给模型添加LoRA适配器（仅新增≈0.1GB参数，无需训练整个模型）
model = get_peft_model(model, lora_config)
model.print_trainable_parameters()  # 打印可训练参数比例（正常应≈0.1%~0.5%）

# ====================== 5. 数据分词（适配模型输入格式）======================
def tokenize_function(examples):
    """将文本样本转换为模型可识别的token"""
    return tokenizer(
        examples["text"],
        truncation=True,
        max_length=MAX_SEQ_LENGTH,
        padding="max_length",
        return_tensors="pt"
    )

# 加载并格式化数据
dataset = load_and_format_data(CSV_DIR, TRAIN_FIELDS)
# 分词（批量处理，提高效率）
tokenized_dataset = dataset.map(
    tokenize_function,
    batched=True,
    remove_columns=dataset["train"].column_names  # 移除原始文本列，只保留tokenized数据
)

# ====================== 6. 配置训练参数 ======================
training_args = TrainingArguments(
    output_dir=OUTPUT_DIR,
    per_device_train_batch_size=BATCH_SIZE,
    per_device_eval_batch_size=BATCH_SIZE,
    learning_rate=LEARNING_RATE,
    num_train_epochs=TRAIN_EPOCHS,
    logging_steps=10,  # 每10步打印一次日志
    eval_strategy="epoch",  # 每轮结束后验证
    save_strategy="epoch",  # 每轮结束后保存模型
    fp16=True,  # 启用混合精度训练（节省显存+加速）
    push_to_hub=False,  # 不推送到Hugging Face Hub（本地训练）
    report_to="none",  # 不使用wandb等日志工具（简化流程）
    gradient_accumulation_steps=4,  # 梯度累积（相当于增大有效批次，提升效果）
    gradient_checkpointing=True,  # 梯度检查点（节省显存）
    optim="paged_adamw_8bit",  # 8bit优化器（进一步节省显存）
    lr_scheduler_type="cosine",  # 余弦学习率调度（稳定训练）
    warmup_ratio=0.05  # 热身比例（避免初始学习率过高）
)

# ====================== 7. 启动LoRA训练 ======================
trainer = SFTTrainer(
    model=model,
    args=training_args,
    train_dataset=tokenized_dataset["train"],
    eval_dataset=tokenized_dataset["validation"],
    peft_config=lora_config
)

# 开始训练
print("="*50)
print("开始LoRA微调训练...")
print(f"训练样本数：{len(tokenized_dataset['train'])}")
print(f"验证样本数：{len(tokenized_dataset['validation'])}")
print("="*50)
trainer.train()

# 保存最终的LoRA适配器（仅几十MB，可复用）
trainer.save_model(os.path.join(OUTPUT_DIR, "final_lora"))
print(f"\n训练完成！LoRA适配器保存至：{os.path.join(OUTPUT_DIR, 'final_lora')}")

# ====================== 8. 推理验证：用微调后的模型回答景点问题 ======================
def test_finetuned_model(lora_path):
    """加载微调后的模型，测试景点查询效果"""
    # 加载原始模型 + LoRA适配器
    model = AutoModelForCausalLM.from_pretrained(
        LOCAL_MODEL_PATH,
        trust_remote_code=True,
        quantization_config=bnb_config,
        device_map=DEVICE,
        torch_dtype=torch.float16
    )
    model = get_peft_model(model, LoraConfig.from_pretrained(lora_path))
    model.eval()  # 推理模式（禁用Dropout）
    
    # 构建推理管道
    generator = pipeline(
        "text-generation",
        model=model,
        tokenizer=tokenizer,
        device_map=DEVICE
    )
    
    # 测试查询（可替换为你的景点名称）
    test_queries = [
        "介绍一下故宫的相关信息",
        "杭州西湖的开放时间和门票是多少？",
        "推荐什么时候去桂林漓江游玩，需要玩多久？"
    ]
    
    print("\n" + "="*50)
    print("微调后模型推理测试：")
    print("="*50)
    for query in test_queries:
        # 按Qwen格式构造prompt
        prompt = f"用户：{query}\n助手："
        # 生成回答
        outputs = generator(
            prompt,
            max_new_tokens=512,  # 最大生成长度
            temperature=0.7,  # 随机性（越低越稳定）
            top_p=0.9,
            repetition_penalty=1.1,  # 避免重复
            do_sample=True
        )
        # 提取并打印回答
        response = outputs[0]["generated_text"].replace(prompt, "").strip()
        print(f"\n用户：{query}")
        print(f"助手：{response}")

# 执行推理测试（训练完成后自动运行）
test_finetuned_model(os.path.join(OUTPUT_DIR, "final_lora"))