import csv
import os

input_filename = "./5.26_1.csv"  # CSV 文件名
output_prefix = "packet"         # 输出文件前缀

# 包头和包尾定义
#header = ['DE', '3A', '09', '66', '31']
header_1 = ['DE', '3A', '09', '66', '33']
footer = ['CE', 'FF']

# 收集所有 data 字段中的十六进制数据
raw_data = []

with open(input_filename, 'r', newline='') as csvfile:
    reader = csv.DictReader(csvfile)
    for row in reader:
        # 提取并拆分 data 字段为十六进制字符串列表
        data_str = row['data']
        hex_values = data_str.strip().split()
        raw_data.extend(hex_values)

# 进行包提取
i = 0
packet_count = 0

while i < len(raw_data):
    #if raw_data[i:i+5] == header or raw_data[i:i+5] == header_1:
    if raw_data[i:i+5] == header_1:
        start = i
        i += 5
        while i < len(raw_data) - 1:
            if raw_data[i:i+2] == footer:
                end = i + 2
                packet = raw_data[start:end]
                packet_count += 1
                output_filename = f"{output_prefix}{packet_count}.txt"
                with open(output_filename, 'w') as out_file:
                    out_file.write(' '.join(packet))
                    out_file.write(' ')
                i = end
                break
            i += 1
    else:
        i += 1

print(f"[✓] 提取完毕，共提取 {packet_count} 个数据段。")

