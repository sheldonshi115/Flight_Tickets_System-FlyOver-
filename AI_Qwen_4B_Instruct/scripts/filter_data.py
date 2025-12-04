import os
import shutil

# 你的目标城市列表（和 COMMON_CITIES 完全一致）
COMMON_CITIES = [
    "北京", "上海", "广州", "深圳", "成都", "杭州", "西安", "重庆",
    "武汉", "南京", "青岛", "厦门", "长沙", "郑州", "昆明", "大连",
    "常州", "海南", "苏州", "桂林", "三亚", "哈尔滨", "乌鲁木齐"
]

# 配置路径（按需修改！）
source_dir = r"D:\LLM_scratch\Flight_ticket\data\citydata"  # 原数据集目录（city_data 的路径）
output_dir = r"D:\LLM_scratch\Flight_ticket\data\filtered_city_data"  # 筛选后的输出目录

# 创建输出目录（如果不存在）
os.makedirs(output_dir, exist_ok=True)

missing_cities = []

# 遍历目标城市，筛选并复制文件
for city in COMMON_CITIES:
    file_name = f"{city}.csv"
    source_path = os.path.join(source_dir, file_name)
    output_path = os.path.join(output_dir, file_name)

    if os.path.exists(source_path):
        # 复制文件到输出目录
        shutil.copy2(source_path, output_path)  # copy2 保留文件元信息
        print(f"✅ 已复制：{file_name}")
    else:
        missing_cities.append(city)
        print(f"❌ 缺失文件：{file_name}")

# 输出汇总报告
print("\n" + "="*30)
print(f"✅ 筛选完成！")
print(f"📊 总目标城市数：{len(COMMON_CITIES)}")
print(f"✅ 成功复制数：{len(COMMON_CITIES) - len(missing_cities)}")
print(f"❌ 缺失城市数：{len(missing_cities)}")
if missing_cities:
    print(f"❌ 缺失城市列表：{', '.join(missing_cities)}")
print(f"📂 筛选后文件路径：{os.path.abspath(output_dir)}")