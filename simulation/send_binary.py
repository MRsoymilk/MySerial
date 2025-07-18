import serial
import sys
import os

CHUNK_SIZE = 2048  # 每次发送 2048 字节

def print_progress(sent, total):
    percent = sent / total * 100
    bar_len = 40
    filled_len = int(bar_len * sent // total)
    bar = '=' * filled_len + '-' * (bar_len - filled_len)
    print(f"\r[{bar}] {percent:6.2f}% ({sent}/{total} bytes)", end='', flush=True)

def main():
    if len(sys.argv) != 4:
        print("用法: python3 send_bin_serial.py <串口路径> <波特率> <二进制文件路径>")
        print("示例: python3 send_bin_serial.py /dev/pts/5 115200 ./data.bin")
        return

    serial_port = sys.argv[1]
    baudrate = int(sys.argv[2])
    filepath = sys.argv[3]

    if not os.path.isfile(filepath):
        print(f"文件不存在: {filepath}")
        return

    # 读取二进制文件
    try:
        with open(filepath, 'rb') as f:
            binary_data = f.read()
    except Exception as e:
        print(f"读取文件失败: {e}")
        return

    total_len = len(binary_data)
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
        chunk = binary_data[i:i + CHUNK_SIZE]
        ser.write(chunk)
        sent += len(chunk)
        print_progress(sent, total_len)

    ser.close()
    print("\n发送完成")

if __name__ == '__main__':
    main()

