import pandas as pd
import matplotlib.pyplot as plt

# 读取 CSV 文件
df = pd.read_csv("distance.csv")

# 提取 index 和 distance 列
index = df["index"]
distance = df["distance"]

# 计算常见帧距
expected = distance.mode()[0]

# 绘图
plt.figure(figsize=(12, 6))
plt.plot(index, distance, label='frame distance', marker='o', linewidth=1)

# 标出异常帧距
abnormal = df[distance != expected]
if not abnormal.empty:
    plt.plot(abnormal["index"], abnormal["distance"], 'rx', label='abnormal', markersize=8)

# 添加图形信息
plt.xlabel("frame idx")
plt.ylabel("frame distance")
plt.title("DE 3A 09 66 31 distance")
#plt.title("CE FF distance")
plt.grid(True)
plt.legend()
plt.tight_layout()

# 显示或保存图像
plt.savefig("distance_plot.png", dpi=300)  # 可选保存
plt.show()

