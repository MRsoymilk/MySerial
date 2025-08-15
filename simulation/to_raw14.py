import pandas as pd

def int14_to_3bytes(value: int) -> bytes:
    """将14bit有符号整数转换为3字节表示"""
    # 限制范围 -8192 ~ 8191
    if value < -8192 or value > 8191:
        raise ValueError(f"数值超出14bit范围: {value}")

    # 转换成 14bit 补码
    if value < 0:
        value = (1 << 14) + value   # 负数转补码
    
    # 打包到3字节（高位清零，低14位存放数据）
    return bytes([
        (value >> 16) & 0xFF,
        (value >> 8) & 0xFF,
        value & 0xFF
    ])

def convert_csv_to_txt(csv_file, txt_file):
    df = pd.read_csv(csv_file)

    if "val14" not in df.columns:
        raise ValueError("CSV 文件缺少 'val14' 列")

    # 包头和包尾
    header = bytes.fromhex("DE3A096633")
    footer = bytes.fromhex("CEFF")

    # 转换所有 val14
    payload = b''.join(int14_to_3bytes(int(v)) for v in df["val14"])

    # 拼接完整数据
    full_data = header + payload + footer

    # 保存为十六进制字符串
    with open(txt_file, "w") as f:
        f.write(full_data.hex().upper())

    print(f"转换完成，输出保存到 {txt_file}")

if __name__ == "__main__":
    convert_csv_to_txt("14.csv", "output14.txt")

