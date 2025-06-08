import re
import os

def process_output_file(filename, compiler_name):
    """
    处理输出文件，提取计时数据，计算加速比和并行效率。

    参数:
        filename (str): 输出文件的路径。
        compiler_name (str): 编译器的名称 (例如 "GCC", "ICC")。

    返回:
        str: Markdown 格式的结果表格，如果文件未找到或数据不完整则返回错误信息。
    """
    threads_data = []
    current_threads = None

    if not os.path.exists(filename):
        return f"错误：文件 '{filename}' 未找到。\n请确保文件路径正确，或者文件与脚本在同一目录下。"

    try:
        with open(filename, 'r') as f:
            for line in f:
                # 匹配 "-> Threads = X"
                threads_match = re.search(r'-> Threads = (\d+)', line)
                if threads_match:
                    current_threads = int(threads_match.group(1))

                # 匹配 "STENCIL_TIME: Y.YYYYYY"
                time_match = re.search(r'STENCIL_TIME: (\d+\.\d+)', line)
                if time_match and current_threads is not None:
                    time_val = float(time_match.group(1))
                    threads_data.append({'threads': current_threads, 'time': time_val})
                    current_threads = None # 为下一对数据重置
    except Exception as e:
        return f"处理文件 '{filename}' 时发生错误: {e}"

    if not threads_data:
        return f"在文件 '{filename}' 中没有找到 {compiler_name} 的计时数据。"

    # 按线程数排序，以确保 T1 (单线程时间) 如果不是第一个也能被正确处理
    threads_data.sort(key=lambda x: x['threads'])

    # 查找 T1 (1个线程的运行时间)
    t1_entry = next((item for item in threads_data if item['threads'] == 1), None)
    if not t1_entry:
        return f"错误: 在文件 '{filename}' 中未找到 {compiler_name} 的单线程计时数据。"
    
    t1_time = t1_entry['time']

    results = []
    for data_point in threads_data:
        p = data_point['threads']
        tp = data_point['time']
        
        speedup = t1_time / tp if tp > 0 else 0
        efficiency = speedup / p if p > 0 else 0
        
        results.append({
            'threads': p,
            'time': tp,
            'speedup': speedup,
            'efficiency': efficiency
        })

    # 构建 Markdown 表格
    table_output = f"### {compiler_name} OpenMP Strong Scaling\n\n"
    table_output += "| Threads | Time (s) | Speedup | Efficiency |\n"
    table_output += "|-----------------|--------------------|-----------------|-----------------------|\n"
    
    for res in results:
        table_output += f"| {res['threads']:<15} | {res['time']:<18.6f} | {res['speedup']:<15.3f} | {res['efficiency']:<21.3f} |\n"
        
    return table_output

# --- 脚本主程序 ---

# 定义您的 .out 文件路径
# 请确保这些文件与脚本在同一目录下，或者提供完整路径
gcc_output_filename = 'strong_gcc_52375392.out'
icc_output_filename = 'strong_icc_52375394.out'

# 处理 GCC 结果
gcc_table = process_output_file(gcc_output_filename, "GCC")
print(gcc_table)
print("\n" + "="*50 + "\n") # 分隔符

# 处理 ICC 结果
icc_table = process_output_file(icc_output_filename, "ICC")
print(icc_table)
