import argparse
import re

def extract_packets(input_filename):
    output_prefix = "packet"

    with open(input_filename, 'r') as f:
        raw_hex = f.read()

    # 清除空格、换行、制表符等
    raw_hex = re.sub(r'\s+', '', raw_hex).upper()

    # 转为两两一组的字节数组
    if len(raw_hex) % 2 != 0:
        print("⚠️ 数据长度不是偶数，可能有损坏！")
        raw_hex = raw_hex[:-1]

    raw_data = [raw_hex[i:i+2] for i in range(0, len(raw_hex), 2)]

    # 包头和尾
    headers = [
        ['DE', '3A', '09', '66', '31'],
        ['DE', '3A', '09', '66', '33']
    ]
    footer = ['CE', 'FF']

    i = 0
    packet_count = 0

    while i <= len(raw_data) - 5:
        matched_header = None

        for header in headers:
            if raw_data[i:i+5] == header:
                matched_header = header
                break

        if matched_header:
            start = i
            i += 5
            # 找尾
            while i <= len(raw_data) - 2:
                if raw_data[i:i+2] == footer:
                    end = i + 2
                    packet = raw_data[start:end]
                    packet_count += 1
                    output_filename = f"{output_prefix}{packet_count}.txt"
                    with open(output_filename, 'w') as out_file:
                        out_file.write(' '.join(packet) + ' ')
                    i = start + 1  # 支持粘包
                    break
                i += 1
        else:
            i += 1

    print(f"提取完毕，共提取 {packet_count} 个数据段。")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="从输入文件中提取数据包（支持粘包和无空格格式）")
    parser.add_argument("--file", help="原始输入文件路径")
    args = parser.parse_args()

    extract_packets(args.file)

