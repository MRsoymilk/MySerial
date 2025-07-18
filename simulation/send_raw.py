import serial
import sys
import os

CHUNK_SIZE = 2048  # 每次发送 1024 字节

def print_progress(sent, total):
    percent = sent / total * 100
    bar_len = 40
    filled_len = int(bar_len * sent // total)
    bar = '=' * filled_len + '-' * (bar_len - filled_len)
    print(f"\r[{bar}] {percent:6.2f}% ({sent}/{total} bytes)", end='', flush=True)

def main():
    if len(sys.argv) != 4:
        print("用法: python3 send_hex_serial.py <串口路径> <波特率> <文件路径>")
        print("示例: python3 send_hex_serial.py /dev/pts/5 115200 ./7_9.txt")
        return

    serial_port = sys.argv[1]
    baudrate = int(sys.argv[2])
    filepath = sys.argv[3]

    if not os.path.isfile(filepath):
        print(f"文件不存在: {filepath}")
        return

    # 读取并清理数据
    with open(filepath, 'r') as f:
        hex_str = ''.join(f.read().split())  # 去掉所有空白字符

    # 转换为字节数组
    try:
        hex_bytes = bytearray(int(hex_str[i:i+2], 16) for i in range(0, len(hex_str), 2))
    except ValueError as e:
        print(f"解析出错: {e}")
        return

    total_len = len(hex_bytes)
    print(f"准备发送，总字节数: {total_len}")

    # 打开串口
    try:
        ser = serial.Serial(serial_port, baudrate, timeout=1)
    except Exception as e:
        print(f"打开串口失败: {e}")
        return

    # 发送数据并显示进度
    sent = 0
    for i in range(0, total_len, CHUNK_SIZE):
        chunk = hex_bytes[i:i + CHUNK_SIZE]
        ser.write(chunk)
        sent += len(chunk)
        print_progress(sent, total_len)

    ser.close()
    print("\n发送完成")

if __name__ == '__main__':
    main()

