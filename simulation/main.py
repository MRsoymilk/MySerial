import serial
import time
import glob
import os
import argparse
import sys

def load_spectrum_data(filename):
    try:
        with open(filename, 'r') as f:
            hex_str = f.read().strip()
            hex_parts = hex_str.split()
            return bytes(int(b, 16) for b in hex_parts)
    except Exception as e:
        print(f"Error reading {filename}: {e}")
        return None

def load_all_packets(pattern):
    packet_files = sorted(glob.glob(pattern), key=lambda x: int(''.join(filter(str.isdigit, x))))
    packets = []
    for file in packet_files:
        print(f"[INFO] Loading {file}")
        packet = load_spectrum_data(file)
        if packet:
            packets.append(packet)
        else:
            print(f"[WARN] Skipping invalid packet file: {file}")
    return packets

def open_serial(port, baudrate):
    try:
        ser = serial.Serial(port, baudrate, timeout=1)
        print(f"[INFO] Opened serial port {port} @ {baudrate} baud")
        return ser
    except serial.SerialException as e:
        print(f"[ERROR] Failed to open serial port: {e}")
        sys.exit(1)

def main():
    parser = argparse.ArgumentParser(description="Send spectrum packets over serial port upon receiving a start command.")
    parser.add_argument('--port', type=str, required=True, help='Serial port device (e.g. /dev/pts/4 or COM3)')
    parser.add_argument('--baudrate', type=int, default=115200, help='Serial baudrate (default: 115200)')
    parser.add_argument('--pattern', type=str, default='packet*.txt', help='File pattern for spectrum packet files')
    parser.add_argument('--delay', type=float, default=0.0, help='Delay in seconds between sending each packet')
    args = parser.parse_args()

    spectrum_packets = load_all_packets(args.pattern)
    if not spectrum_packets:
        print("[ERROR] No valid packets found.")
        sys.exit(1)

    ser = open_serial(args.port, args.baudrate)
    cmd_start = bytes([0xDD, 0x3C, 0x00, 0x01, 0x30, 0xCD, 0xFF])

    try:
        print("[INFO] Waiting for start command...")
        while True:
            recv = ser.read(7)
            if recv == cmd_start:
                print("[INFO] Start command received!")
                for i, packet in enumerate(spectrum_packets):
                    print(f"[SEND] Packet {i} ({len(packet)} bytes)")
                    ser.write(packet)
                    if args.delay > 0:
                        time.sleep(args.delay)
                print("[INFO] All packets sent.")
            else:
                if recv:
                    print(f"[RECV] {recv.hex(' ')}")
            time.sleep(0.1)
    except KeyboardInterrupt:
        print("\n[INFO] Interrupted by user.")
    finally:
        ser.close()
        print("[INFO] Serial port closed.")

if __name__ == '__main__':
    main()
