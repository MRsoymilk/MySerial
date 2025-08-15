import pandas as pd

def int_to_24bit_bytes(value: int) -> bytes:
    """将整数转换为 24bit (3字节)"""
    # 假设 val24 是有符号整数范围 -8388608 ~ 8388607
    if value < 0:
        value = (1 << 24) + value  # 转换为补码
    
    return bytes([
        (value >> 16) & 0xFF,
        (value >> 8) & 0xFF,
        value & 0xFF
    ])

def convert_csv_to_txt(csv_file, txt_file):
    df = pd.read_csv(csv_file)

    if "val24" not in df.columns:
        raise ValueError("CSV 文件中缺少 'val24' 列")

    # 包头和包尾
    header = bytes.fromhex("DE3A096631")
    footer = bytes.fromhex("CEFF")

    # 转换所有 val24
    payload = b''.join(int_to_24bit_bytes(int(v)) for v in df["val24"])

    # 拼接完整数据
    full_data = header + payload + footer

    # 保存为 txt（十六进制字符串）
    with open(txt_file, "w") as f:
        f.write(full_data.hex().upper())

    print(f"转换完成，保存到 {txt_file}")

if __name__ == "__main__":
    convert_csv_to_txt("24.csv", "output24.txt")

