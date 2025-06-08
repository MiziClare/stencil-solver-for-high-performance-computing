import matplotlib.pyplot as plt
import numpy as np # numpy 通常与 matplotlib 一起使用，尤其是在处理数值数据时

# --- GCC 数据 (从您提供的表格中获取) ---
gcc_threads = np.array([1, 2, 4, 8, 16, 32])
gcc_speedup = np.array([1.000, 0.842, 1.343, 2.208, 3.744, 6.563])

# --- ICC 数据 (从您提供的表格中获取) ---
icc_threads = np.array([1, 2, 4, 8, 16, 32])
icc_speedup = np.array([1.000, 1.500, 2.350, 3.727, 5.228, 5.215])

# --- 绘图函数 ---
def plot_speedup_curve(threads, speedup_data, compiler_name, filename_suffix):
    """
    绘制加速比曲线图。

    参数:
        threads (np.array): 线程数数组。
        speedup_data (np.array): 对应的加速比数组。
        compiler_name (str): 编译器名称 (例如 "GCC", "ICC")。
        filename_suffix (str): 保存图片时的文件名后缀 (例如 "gcc", "icc")。
    """
    plt.figure(figsize=(10, 6)) # 设置图片大小
    
    # 绘制实际的加速比曲线
    plt.plot(threads, speedup_data, marker='o', linestyle='-', label=f'{compiler_name} Actual Speedup')
    
    # 绘制理想的线性加速比曲线 (作为参考)
    plt.plot(threads, threads, marker='x', linestyle='--', color='gray', label='Ideal Speedup')
    
    plt.title(f'{compiler_name} OpenMP Speedup Curve')
    plt.xlabel('Number of OpenMP Threads')
    plt.ylabel('Speedup')
    
    # 设置 x 轴刻度为实际的线程数，避免出现非整数的线程数
    plt.xticks(threads)
    # 可以根据需要调整 y 轴的范围，例如：
    # plt.ylim(0, max(np.max(speedup_data), np.max(threads)) + 1)
    plt.grid(True, linestyle=':', alpha=0.7) # 添加网格线
    plt.legend() # 显示图例
    
    # 保存图表
    output_filename = f'speedup_curve_{filename_suffix}.png'
    plt.savefig(output_filename)
    print(f"图表已保存为: {output_filename}")
    
    plt.show() # 显示图表

# --- 为 GCC 绘制图表 ---
plot_speedup_curve(gcc_threads, gcc_speedup, "GCC", "gcc")

# --- 为 ICC 绘制图表 ---
plot_speedup_curve(icc_threads, icc_speedup, "ICC", "icc")

# --- 如果您想将 GCC 和 ICC 的结果绘制在同一个图表中进行比较 ---
def plot_combined_speedup_curve(gcc_threads, gcc_speedup, icc_threads, icc_speedup, filename_suffix="combined"):
    """
    在同一个图表中绘制 GCC 和 ICC 的加速比曲线。
    """
    plt.figure(figsize=(10, 6))
    
    # GCC
    plt.plot(gcc_threads, gcc_speedup, marker='o', linestyle='-', label='GCC Actual Speedup')
    # ICC
    plt.plot(icc_threads, icc_speedup, marker='s', linestyle='-', label='ICC Actual Speedup')
    
    # 理想加速比 (以最长的线程数序列为准，这里假设它们相同)
    plt.plot(gcc_threads, gcc_threads, marker='x', linestyle='--', color='gray', label='Ideal Speedup')
    
    plt.title('GCC vs ICC OpenMP Speedup Curve')
    plt.xlabel('Number of OpenMP Threads')
    plt.ylabel('Speedup')
    
    # 确保所有线程数都显示在 x 轴上
    all_threads = np.unique(np.concatenate((gcc_threads, icc_threads)))
    plt.xticks(all_threads)
    
    plt.grid(True, linestyle=':', alpha=0.7)
    plt.legend()
    
    output_filename = f'speedup_curve_{filename_suffix}.png'
    plt.savefig(output_filename)
    print(f"组合图表已保存为: {output_filename}")
    
    plt.show()

# 调用组合绘图函数 (可选)
plot_combined_speedup_curve(gcc_threads, gcc_speedup, icc_threads, icc_speedup)

print("\n绘图完成。请检查生成的 .png 文件。")
