import os
import warnings
import random
import re
import json
import datetime
from threading import Lock
warnings.filterwarnings("ignore")  

# 显存优化配置
os.environ["PYTORCH_CUDA_ALLOC_CONF"] = "expandable_segments:True"
os.environ["CUDA_VISIBLE_DEVICES"] = "0" 
os.environ["OMP_NUM_THREADS"] = "4"
from fastapi import FastAPI, HTTPException
from pydantic import BaseModel
import pymysql.cursors
from transformers import AutoTokenizer, AutoModelForCausalLM, BitsAndBytesConfig
import torch
from functools import lru_cache
import threading
import time
# MySQL连接配置
MYSQL_CONFIG = {
    "host": "localhost",
    "port": 3306,
    "user": "root",
    "password": "pwd",
    "database": "flight_ticket_db",
    "charset": "utf8mb4",
    "cursorclass": pymysql.cursors.DictCursor
}
#。。。。。。。。。。。。。。。。。。。。。。。。。。。。
# 初始化FastAPI
app = FastAPI()

# 定义请求体
class QueryRequest(BaseModel):
    question: str


# 日志配置
LOG_DIR = "./flight_query_logs"
os.makedirs(LOG_DIR, exist_ok=True)
LOG_LOCK = Lock()

# 城市列表
COMMON_CITIES = [
    "北京", "上海", "广州", "深圳", "成都", "杭州", "西安", "重庆",
    "武汉", "南京", "青岛", "厦门", "长沙", "郑州", "昆明", "大连",
    "常州", "海南", "苏州", "桂林", "三亚", "哈尔滨", "乌鲁木齐"
]

# 日志写入函数
def write_query_log(user_input: str, ai_output: str, keywords: dict):
    try:
        log_data = {
            "timestamp": datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f")[:-3],
            "user_input": user_input.strip(),
            "ai_output": ai_output.strip(),
            "extracted_keywords": keywords
        }
        
        date_str = datetime.datetime.now().strftime("%Y%m%d")
        log_file_path = os.path.join(LOG_DIR, f"query_log_{date_str}.json")
        
        with LOG_LOCK:
            if not os.path.exists(log_file_path):
                with open(log_file_path, "w", encoding="utf-8") as f:
                    json.dump([log_data], f, ensure_ascii=False, indent=2)
            else:
                with open(log_file_path, "r", encoding="utf-8") as f:
                    existing_logs = json.load(f)
                existing_logs.append(log_data)
                with open(log_file_path, "w", encoding="utf-8") as f:
                    json.dump(existing_logs, f, ensure_ascii=False, indent=2)
        
        print(f"日志已写入：{log_file_path}")
    
    except Exception as e:
        print(f"日志写入失败：{str(e)}")

def get_time_condition(time_type: str, specific_date: str) -> str:
    """
    根据AI输出的时间类型，生成SQL的时间查询条件
    :param time_type: AI输出的"时间类型"字段
    :param specific_date: AI输出的"具体日期"字段
    :return: SQL中的时间WHERE子句（如"DATE(depart_time) = '2025-12-05'"）
    """
    today = datetime.date.today()
    now = datetime.datetime.now()
    
    if time_type == "具体日期" and specific_date:
        # 具体日期：精确匹配日期
        return f"DATE(depart_time) = '{specific_date}'"
    elif time_type == "今天":
        # 今天：日期等于今天
        return f"DATE(depart_time) = '{today.strftime('%Y-%m-%d')}'"
    elif time_type == "明天":
        # 明天：日期等于明天
        tomorrow = today + datetime.timedelta(days=1)
        return f"DATE(depart_time) = '{tomorrow.strftime('%Y-%m-%d')}'"
    elif time_type == "本周":
        # 本周：从周一到周日（默认周一为一周第一天）
        monday = today - datetime.timedelta(days=today.weekday())
        sunday = monday + datetime.timedelta(days=6)
        return f"DATE(depart_time) BETWEEN '{monday.strftime('%Y-%m-%d')}' AND '{sunday.strftime('%Y-%m-%d')}'"
    elif time_type == "下周":
        # 下周：从下周一到下周日
        next_monday = today + datetime.timedelta(days=(7 - today.weekday()))
        next_sunday = next_monday + datetime.timedelta(days=6)
        return f"DATE(depart_time) BETWEEN '{next_monday.strftime('%Y-%m-%d')}' AND '{next_sunday.strftime('%Y-%m-%d')}'"
    elif time_type == "本月":
        # 本月：从月初到月末
        month_start = today.replace(day=1)
        # 计算月末（下个月第一天减1天）
        if today.month == 12:
            month_end = today.replace(year=today.year+1, month=1, day=1) - datetime.timedelta(days=1)
        else:
            month_end = today.replace(month=today.month+1, day=1) - datetime.timedelta(days=1)
        return f"DATE(depart_time) BETWEEN '{month_start.strftime('%Y-%m-%d')}' AND '{month_end.strftime('%Y-%m-%d')}'"
    elif time_type == "最近":
        # 最近：未来7天内的航班（可自定义天数，如3天、10天）
        future_7days = today + datetime.timedelta(days=7)
        return f"DATE(depart_time) BETWEEN '{now.strftime('%Y-%m-%d')}' AND '{future_7days.strftime('%Y-%m-%d')}'"
    else:
        # 无时间类型：仅筛选未过期航班（原有逻辑）
        return "depart_time >= NOW()"

def direct_sql_query(departure: str, destination: str, time_type: str, specific_date: str):
    """
    根据AI提取的字段，精准查询航班，返回前10条优质结果
    :param departure: 出发地
    :param destination: 目的地
    :param time_type: 时间类型（AI输出）
    :param specific_date: 具体日期（AI输出）
    :return: 格式化的航班信息
    """
    connection = None
    try:
        connection = pymysql.connect(**MYSQL_CONFIG)
        cursor = connection.cursor()
        
        where_clauses = []
        params = []
        
        # 1. 出发地条件（原有逻辑）
        if departure and departure in COMMON_CITIES:
            where_clauses.append("departure = %s")
            params.append(departure)
        # 2. 目的地条件（原有逻辑）
        if destination and destination in COMMON_CITIES:
            where_clauses.append("destination = %s")
            params.append(destination)
        # 3. 时间条件（新增：调用工具函数生成）
        time_condition = get_time_condition(time_type, specific_date)
        if time_condition:
            where_clauses.append(time_condition)
        
        # 拼接WHERE子句
        where_sql = "WHERE " + " AND ".join(where_clauses) if where_clauses else ""
        
        # 优化排序：模糊时间查询时，"最近"优先按出发时间升序（离现在越近越靠前）
        order_by = "ORDER BY seat_count DESC, price ASC"
        if time_type == "最近":
            order_by = "ORDER BY depart_time ASC, seat_count DESC"  # 最近的航班排前面
        
        # 精准查询：保留前10条（可调整）
        query_sql = f"""
            SELECT flight_num, departure, destination,
                   depart_time, arrive_time,
                   seat_count, price
            FROM flights 
            {where_sql}
            {order_by}
            LIMIT 10;
        """
        print(f"执行精准SQL：{query_sql}")
        print(f"SQL参数：{params}")
        cursor.execute(query_sql, params)
        rows = cursor.fetchall()
        
        if not rows:
            return f"没有找到匹配的航班信息（{time_type if time_type != '无' else '未指定时间'}，{departure if departure else '未指定出发地'}→{destination if destination else '未指定目的地'}）"
        
        # 格式化结果
        flight_info = []
        for idx, row in enumerate(rows, 1):
            info = (
                f"{idx}. 航班号：{row['flight_num']}，出发地：{row['departure']}，目的地：{row['destination']}，"
                f"出发时间：{row['depart_time']}，到达时间：{row['arrive_time']}，"
                f"余票：{row['seat_count']}张，价格：{row['price']}元"
            )
            flight_info.append(info)
        
        return "\n".join(flight_info)
    
    except Exception as e:
        print(f"查询失败：{str(e)}")
        return "查询失败"
    finally:
        if connection:
            connection.close()

def extract_json_from_ai_output(ai_output: str):
    """提取AI输出中的JSON部分，兼容前后有多余文字的情况"""
    # 用正则匹配JSON格式（{}包裹的内容）
    json_pattern = r"\{.*?\}"
    match = re.search(json_pattern, ai_output, re.DOTALL)
    if not match:
        return None
    try:
        return json.loads(match.group())
    except json.JSONDecodeError:
        return None

# 模型加载（不变）
bnb_config = BitsAndBytesConfig(
    load_in_4bit=True,
    bnb_4bit_use_double_quant=True,
    bnb_4bit_quant_type="nf4",
    bnb_4bit_compute_dtype=torch.bfloat16 
)

model_path = r"D:\LLM_scratch\Flight_ticket\model_4B"
tokenizer = AutoTokenizer.from_pretrained(
    model_path,
    use_fast=True,  
    padding_side="right" 
)

if tokenizer.pad_token_id is None:
    tokenizer.pad_token_id = tokenizer.eos_token_id

model = AutoModelForCausalLM.from_pretrained(
    model_path,
    quantization_config=bnb_config,
    low_cpu_mem_usage=True,
    offload_state_dict=True,
    max_memory={0: "5GB"},  
    device_map="cuda", 
    dtype=torch.bfloat16,
    ignore_mismatched_sizes=True, 
    trust_remote_code=True
)
model.config.use_cache = True
model.eval()  
for param in model.parameters():
    param.requires_grad = False  

def generate_answer(question: str):
    try:
        # 步骤1：AI轻量判断+提取字段（Prompt极简，速度快）
        prompt = [
    {"role": "system", "content": f"""你的任务：
1. 若用户问题不是航班查询：直接用自然语言回答用户问题，无需其他格式；
2. 如果用户问题是航班查询：输出JSON格式，包含4个字段：
   - "出发地"：必须是{COMMON_CITIES}中的值，无则留空字符串；
   - "目的地"：必须是{COMMON_CITIES}中的值，无则留空字符串；
   - "时间类型"：可选值："具体日期"、"最近"、"今天"、"明天"、"本周"、"下周"、"本月"，无时间信息则填"无"；
   - "具体日期"：仅当"时间类型"为"具体日期"时填充（格式：YYYY-MM-DD），其他情况留空字符串；
3. 严格遵守：航班查询仅输出JSON，非航班查询仅输出自然语言，不要多余字符，不要注释。
"""},
    {"role": "user", "content": f"问题：{question}"}
]
        
        # AI生成（max_new_tokens设小，加快速度）
        inputs = tokenizer.apply_chat_template(
            prompt,
            add_generation_prompt=True,
            return_tensors="pt",
            truncation=True,
            max_length=512  # 精简Prompt长度
        ).to(model.device)
        
        with torch.no_grad():
            outputs = model.generate(
                inputs,
                max_new_tokens=200,  # 仅需输出JSON或短回答，速度极快
                do_sample=True, 
                eos_token_id=tokenizer.eos_token_id,
                pad_token_id=tokenizer.pad_token_id,
                use_cache=True,
                num_beams=1
            )
        
        ai_raw_output = tokenizer.decode(
            outputs[0][len(inputs[0]):],
            skip_special_tokens=True,
            clean_up_tokenization_spaces=True
        )
        print(f"AI原始输出：{ai_raw_output}")
        
        # 步骤2：解析AI输出，判断是否为航班查询
        flight_keywords = extract_json_from_ai_output(ai_raw_output)
        if flight_keywords:
            # 提取字段（新增：时间类型和具体日期）
            departure = flight_keywords.get("出发地", "").strip()
            destination = flight_keywords.get("目的地", "").strip()
            time_type = flight_keywords.get("时间类型", "无").strip()  # 新增
            specific_date = flight_keywords.get("具体日期", "").strip()  # 新增
            
            # 记录日志（新增时间类型）
            log_keywords = {
                "cities": [departure, destination] if departure and destination else [],
                "time_type": time_type,
                "specific_date": specific_date,
                "is_flight_query": True
            }
            
            # 精准SQL查询（传递4个参数，新增时间相关）
            final_answer = direct_sql_query(departure, destination, time_type, specific_date)
            write_query_log(user_input=question, ai_output=final_answer, keywords=log_keywords)
            return final_answer
        else:
            # 非航班查询：原有逻辑不变
            log_keywords = {
                "cities": [],
                "time_type": "无",
                "specific_date": "",
                "is_flight_query": False
            }
            write_query_log(user_input=question, ai_output=ai_raw_output, keywords=log_keywords)
            return ai_raw_output
        
    except Exception as e:
        import traceback
        print(f"generate_answer 异常：{str(e)}")
        print(traceback.format_exc())
        write_query_log(user_input=question, ai_output="查询失败", keywords={"is_flight_query": False})
        return "查询失败"

# API接口（不变）
@app.post("/query_flight")
async def query_flight(request: QueryRequest):
    try:
        answer = generate_answer(request.question)
        return {"answer": answer}
    except Exception as e:
        import traceback
        print(f"API接口异常：{str(e)}")
        print(traceback.format_exc())
        write_query_log(user_input=request.question, ai_output=f"查询失败：{str(e)}", keywords={"is_flight_query": False})
        raise HTTPException(status_code=500, detail=f"查询失败：{str(e)}")

# 启动后端
if __name__ == "__main__":
    import uvicorn
    # 仅修复uvloop依赖问题：使用默认asyncio循环，其他配置不变
    uvicorn.run(app, host="0.0.0.0", port=8000)