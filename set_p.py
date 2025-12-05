import os
import argparse

def create_blog_files(target_dir, file_count=100):
    """
    在指定目录创建指定数量的blog{i}.txt文件
    
    参数:
        target_dir: 目标文件夹路径（相对路径或绝对路径）
        file_count: 要创建的文件数量，默认100个
    """
    try:
        # 1. 检查并创建目标文件夹（如果不存在）
        if not os.path.exists(target_dir):
            os.makedirs(target_dir, exist_ok=True)  # exist_ok=True 避免文件夹已存在时报错
            print(f"已创建目标文件夹: {os.path.abspath(target_dir)}")
        else:
            print(f"使用现有文件夹: {os.path.abspath(target_dir)}")
        
        # 2. 循环创建100个文件
        created_count = 0
        for i in range(1, file_count + 1):
            # 构建文件名和完整路径
            filename = f"blog{i}.txt"
            file_path = os.path.join(target_dir, filename)
            
            # 检查文件是否已存在，避免覆盖
            if os.path.exists(file_path):
                print(f"警告: 文件 {filename} 已存在，跳过创建")
                continue
            
            # 创建空文件（使用with语句自动关闭文件句柄）
            with open(file_path, 'w', encoding='utf-8') as f:
                # 可选：写入初始内容（如需要可取消注释）
                # f.write(f"这是第{i}个博客文件\n")
                pass
            
            created_count += 1
            # 每创建10个文件输出一次进度（可选）
            if created_count % 10 == 0:
                print(f"已创建 {created_count} 个文件...")
        
        # 3. 输出创建结果
        print(f"\n创建完成！共成功创建 {created_count} 个文件")
        print(f"文件路径: {os.path.abspath(target_dir)}")
        print(f"文件格式: blog1.txt ~ blog{file_count}.txt")
    
    except PermissionError:
        print(f"错误: 没有访问 {target_dir} 的权限，请检查文件夹权限")
    except OSError as e:
        print(f"错误: 创建文件时发生系统错误 - {str(e)}")
    except Exception as e:
        print(f"错误: 未知错误 - {str(e)}")

if __name__ == "__main__":
    # 使用命令行参数解析，支持指定目标文件夹
    parser = argparse.ArgumentParser(description="在指定文件夹创建100个blog{i}.txt文件")
    parser.add_argument(
        "folder", 
        nargs="?",  # 可选参数
        default="resources/text",  # 默认文件夹名
        help="目标文件夹路径（默认：当前目录下的blogs文件夹）"
    )
    args = parser.parse_args()
    
    # 调用函数创建文件
    create_blog_files(args.folder)