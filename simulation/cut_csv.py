import csv
import argparse
import os

def extract_packets(input_filename, output_prefix="packet"):
    # 包头和包尾定义
    header_1 = ['DE', '3A', '09', '66', '33']
    footer = ['CE', 'FF']

    # 收集所有 data 字段中的十六进制数据
    raw_data = []

    with open(input_filename, 'r', newline='') as csvfile:
        reader = csv.DictReader(csvfile)
        for row in reader:
            data_str = row['data']
            hex_values = data_str.strip().split()
            raw_data.extend(hex_values)

    # 提取数据包
    i = 0
    packet_count = 0

    while i < len(raw_data):
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
                        out_file.write(' '.join(packet) + ' ')
                    i = end
                    break
                i += 1
        else:
            i += 1

    print(f"[✓] 提取完毕，共提取 {packet_count} 个数据段。")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="从 CSV 文件中提取指定包格式的数据段")
    parser.add_argument('--file', '-f', type=str, required=True, help='输入 CSV 文件路径')
    parser.add_argument('--prefix', '-p', type=str, default='packet', help='输出文件名前缀（默认: packet）')
    args = parser.parse_args()

    if not os.path.exists(args.file):
        print(f"[✗] 错误：文件 '{args.file}' 不存在！")
    else:
        extract_packets(args.file, args.prefix)

