import argparse

def extract_packets_from_bin(input_filename):
    output_prefix = "packet"

    with open(input_filename, 'rb') as f:
        raw_bytes = f.read()

    # 帧头和帧尾（用字节定义）
    headers = [
        b'\xDE\x3A\x09\x66\x31',
        b'\xDE\x3A\x09\x66\x33'
    ]
    footer = b'\xCE\xFF'

    i = 0
    packet_count = 0
    data_len = len(raw_bytes)

    while i <= data_len - len(headers[0]):
        matched_header = None
        for header in headers:
            if raw_bytes[i:i + len(header)] == header:
                matched_header = header
                break

        if matched_header:
            start = i
            i += len(matched_header)
            # 找尾
            while i <= data_len - len(footer):
                if raw_bytes[i:i + len(footer)] == footer:
                    end = i + len(footer)
                    packet = raw_bytes[start:end]
                    packet_count += 1
                    with open(f"{output_prefix}{packet_count}.bin", 'wb') as out_file:
                        out_file.write(packet)
                    i = start + 1  # 支持粘包继续处理
                    break
                i += 1
        else:
            i += 1

    print(f"提取完毕，共提取 {packet_count} 个数据段。")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="从二进制文件中提取数据包（支持粘包）")
    parser.add_argument("--file", required=True, help="原始二进制文件路径")
    args = parser.parse_args()

    extract_packets_from_bin(args.file)

