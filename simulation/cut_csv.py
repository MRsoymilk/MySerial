import csv
import argparse
import os

def extract_packets(input_filename, output_prefix="packet"):
    header_1 = ['DE', '3A', '09', '66', '33']
    footer = ['CE', 'FF']

    raw_data = []

    with open(input_filename, 'r', newline='') as csvfile:
        reader = csv.DictReader(csvfile)
        for row in reader:
            data_str = row['data']
            hex_values = data_str.strip().split()
            raw_data.extend(hex_values)

    i = 0
    packet_count = 0

    while i < len(raw_data):
        if raw_data[i:i+5] == header_1:
            start = i
            i += 5
            while i < len(raw_data) - 1:
                if raw_data[i:i+2] == footer:
                    end = i + 2
                    packet = raw_data[start:end]
                    packet_count += 1
                    output_filename = f"{output_prefix}{packet_count}.txt"
                    with open(output_filename, 'w') as out_file:
                        out_file.write(' '.join(packet) + ' ')
                    i = end
                    break
                i += 1
        else:
            i += 1

    print(f"[✓] Extraction completed. A total of {packet_count} data segments were extracted.")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Extracts data segments in a specified packet format from a CSV file")
    parser.add_argument('--file', '-f', type=str, required=True, help='Enter CSV path')
    parser.add_argument('--prefix', '-p', type=str, default='packet', help='File pattern for spectrum packet files')
    args = parser.parse_args()

    if not os.path.exists(args.file):
        print(f"[✗] Error: file '{args.file}' not exist!")
    else:
        extract_packets(args.file, args.prefix)

