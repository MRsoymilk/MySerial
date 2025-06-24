import pandas as pd
import matplotlib.pyplot as plt
import matplotlib
import glob
import os
from scipy.signal import find_peaks

matplotlib.use('QtAgg')  # 或 "TkAgg"

# 设置参数
min_distance = 3
min_prominence = 0.02

# 遍历所有符合条件的 CSV 文件
for filepath in sorted(glob.glob("smoothed_*.csv")):
    filename = os.path.splitext(os.path.basename(filepath))[0]  # e.g. smoothed_1
    df = pd.read_csv(filepath)

    index = df['index'].values
    original = df['original'].values
    smoothed = df['smoothed'].values

    # 查找峰值（基于 smoothed）
    peaks, properties = find_peaks(smoothed, distance=min_distance, prominence=min_prominence)
    peak_x = index[peaks]
    peak_y = original[peaks]  # 使用原始数据做为峰值

    # 创建图像
    plt.figure(figsize=(10, 5))
    plt.plot(index, original, label='Original', linewidth=2)
    plt.plot(index, smoothed, label='Smoothed', linewidth=1)
    plt.plot(peak_x, peak_y, 'ro', label='Peaks')  # 红色标出峰值
    for i, (x, y) in enumerate(zip(peak_x, peak_y)):
        plt.text(x, y, f'{int(x)}', fontsize=8, ha='center', va='bottom')

    plt.title(f'{filename}: Original vs Smoothed Signal')
    plt.xlabel('Index')
    plt.ylabel('Value')
    plt.legend()
    plt.grid(True)
    plt.tight_layout()

    # 保存图像
    plt.savefig(f"{filename}.png")
    print(f"Saved: {filename}.png")
    plt.close()

    # 导出峰值数据
    peak_df = pd.DataFrame({
        'index': peak_x,
        'value': peak_y
    })
    peak_df.to_csv(f"peaks_{filename.split('_')[-1]}.csv", index=False)
    print(f"Saved: peaks_{filename.split('_')[-1]}.csv")

