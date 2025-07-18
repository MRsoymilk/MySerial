import argparse
import csv

def find_ceff_offsets(filename):
    with open(filename, 'rb') as f:
        data = f.read()

#    target = b'\xCE\xFF'
    target = b'\xDE\x3A\x09\x66\x31'
    offset = 0
    count = 0
    offsets = []

    while True:
        idx = data.find(target, offset)
        if idx == -1:
            break

        offsets.append(idx)
        offset = idx + 1  # 继续查找下一个
        count += 1

    if count == 0:
        print("未找到任何 CE FF 序列。")
        return

    print(f"总共找到 {count} 个 CE FF。正在写入 distance.csv...")

    with open("distance.csv", "w", newline='') as csvfile:
        writer = csv.writer(csvfile)
        writer.writerow(["index", "start_offset", "end_offset", "distance", "is_1990"])
        for i in range(1, len(offsets)):
            start = offsets[i - 1]
            end = offsets[i]
            distance = end - start
            writer.writerow([
                i,
                f"0x{start:08X}",
                f"0x{end:08X}",
                distance,
                "YES" if distance == 1990 else "NO"
            ])

    print("写入完成：distance.csv")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="查找二进制文件中所有 CE FF 的偏移量，并输出间距到 CSV")
    parser.add_argument("file", help="要分析的二进制文件路径")
    args = parser.parse_args()

    find_ceff_offsets(args.file)

