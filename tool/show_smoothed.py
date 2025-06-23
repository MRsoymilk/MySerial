import pandas as pd
import matplotlib.pyplot as plt
import matplotlib
import glob
import os

matplotlib.use('QtAgg')  # 或 "TkAgg" 更通用些

# 遍历所有符合条件的 CSV 文件
for filepath in sorted(glob.glob("smoothed_*.csv")):
    filename = os.path.splitext(os.path.basename(filepath))[0]  # e.g. smooth_1

    # 读取 CSV
    df = pd.read_csv(filepath)

    # 创建图像
    plt.figure(figsize=(10, 5))
    plt.plot(df['index'], df['original'], label='Original', linewidth=2)
    plt.plot(df['index'], df['smoothed'], label='Smoothed', linewidth=1)
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

