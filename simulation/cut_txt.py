import argparse
import re

def extract_packets(input_filename, headers, footer):
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

    footer_list = footer

    i = 0
    packet_count = 0

    while i <= len(raw_data) - len(headers[0]):
        matched_header = None

        # 匹配多个 header
        for header in headers:
            if raw_data[i:i+len(header)] == header:
                matched_header = header
                break

        if matched_header:
            start = i
            i += len(matched_header)

            # 找尾
            while i <= len(raw_data) - len(footer_list):
                if raw_data[i:i+len(footer_list)] == footer_list:
                    end = i + len(footer_list)
                    packet = raw_data[start:end]
                    packet_count += 1
                    output_filename = f"{output_prefix}{packet_count}.txt"

                    with open(output_filename, 'w') as out_file:
                        out_file.write(' '.join(packet) + ' ')

                    # 启用粘包模式，让下一次搜索从 header 后面继续
                    i = start + 1
                    break

                i += 1
        else:
            i += 1

    print(f"提取完毕，共提取 {packet_count} 个数据段。")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="从输入文件中提取数据包（支持粘包和无空格格式）")

    parser.add_argument("--file", required=True, help="原始输入文件路径")

    # 可多次指定 --header，单次传入一个序列
    parser.add_argument("--header", action="append", required=True,
                        help='包头，如 "DE 3A 17 73 31"，可多次使用 --header 添加多个')

    parser.add_argument("--footer", required=True,
                        help='包尾，如 "CE FF"')

    args = parser.parse_args()

    # 解析 header/footer 为 [['DE','3A'...], [...]]
    header_lists = [h.strip().upper().split() for h in args.header]
    footer_list = args.footer.strip().upper().split()

    extract_packets(args.file, header_lists, footer_list)

"""
python cut_txt.py \
    --file raw.txt \
    --header "DE 3A 17 73 31" \
    --footer "CE FF"
"""