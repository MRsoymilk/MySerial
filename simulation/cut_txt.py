import os
import argparse

def extract_packets(input_filename):
    output_prefix = "packet"         # 输出文件前缀，如 packet1.txt、packet2.txt

    # 读取文件内容
    with open(input_filename, 'r') as f:
        raw_data = f.read().split()

    # 包头和包尾定义
    header = ['DE', '3A', '09', '66', '31']
    header_1 = ['DE', '3A', '09', '66', '33']
    footer = ['CE', 'FF']

    i = 0
    packet_count = 0

    while i < len(raw_data):
        # 查找包头
        if raw_data[i:i+5] == header or raw_data[i:i+5] == header_1:
            # 找到开头，开始收集
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
                    i = end  # 继续搜索下一个包
                    break
                i += 1
        else:
            i += 1

    print(f"提取完毕，共提取 {packet_count} 个数据段。")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="从输入文件中提取数据包")
    parser.add_argument("--file", help="原始输入文件路径")
    args = parser.parse_args()

    extract_packets(args.file)

