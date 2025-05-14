import serial
import time
import glob
import os

def load_spectrum_data(filename):
    with open(filename, 'r') as f:
        hex_str = f.read().strip()
        hex_parts = hex_str.split()
        return bytes(int(b, 16) for b in hex_parts)

# 读取所有 packet*.txt 文件并按序加载为字节数据列表
def load_all_packets(pattern='packet*.txt'):
    packet_files = sorted(glob.glob(pattern), key=lambda x: int(''.join(filter(str.isdigit, x))))
    packets = []
    for file in packet_files:
        print(f"loading {file}")
        packets.append(load_spectrum_data(file))
    return packets

# 加载所有数据段
spectrum_packets = load_all_packets()

# 启动串口
try:
    ser = serial.Serial('/dev/pts/4', 115200, timeout=1)
except serial.SerialException as e:
    print(f"open port fail: {e}")
    exit(1)

# 启动命令
cmd_start = bytes([0xDD, 0x3C, 0x00, 0x01, 0x30, 0xCD, 0xFF])

try:
    print("begin")
    while True:
        recv = ser.read(7)
        if recv == cmd_start:
#            while True:
            if True:
                print("start command received!")
                for i, packet in enumerate(spectrum_packets):
                    print(f"sending packet_{i}: {len(packet)}")
                    ser.write(packet)
                    time.sleep(0.5)
                print("sending all packets done")
                time.sleep(0.5)
        else:
            ser.write(b'return')
        time.sleep(0.5)
except KeyboardInterrupt:
    print("\nfinish\n")
finally:
    ser.close()

