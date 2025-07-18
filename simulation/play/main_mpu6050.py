import serial
import time
import math
import random

SERIAL_PORT = '/dev/pts/2'  # 或 /dev/ttyUSB0, /dev/pts/3
BAUD_RATE = 115200

def simulate_mpu_data():
    """模拟生成 roll/pitch/yaw 数据"""
    t = time.time()
    roll = 30 * math.sin(t) + random.uniform(-1, 1)
    pitch = 15 * math.sin(t / 2) + random.uniform(-1, 1)
    yaw = (t * 10) % 360  # 模拟不断旋转
    return roll, pitch, yaw

def main():
    try:
        ser = serial.Serial(SERIAL_PORT, BAUD_RATE)
        print(f"Opened serial port {SERIAL_PORT} at {BAUD_RATE} baud.")
    except Exception as e:
        print(f"Failed to open port: {e}")
        return

    try:
        while True:
            roll, pitch, yaw = simulate_mpu_data()
            packet = f"<MPU>{roll:.2f},{pitch:.2f},{yaw:.2f}<END>\n"
            ser.write(packet.encode())
            print(f"Sent: {packet.strip()}")
            time.sleep(0.05)  # 20Hz 发送频率
    except KeyboardInterrupt:
        print("\nStopped by user.")
    finally:
        ser.close()

if __name__ == "__main__":
    main()

