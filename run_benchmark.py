import os
import subprocess
import time
import csv
import re
from pathlib import Path
import bz2


# --- Configuration ---
BENCHMARK_DIR = "benchmarks"
RESULTS_DIR = "results"
EXE = "./bzip2_stage1.exe"
RESULTS_CSV = "results.csv"
CONFIG_FILE = "config.ini"

def get_block_size():
    """Extracts block_size from config.ini for the CSV report."""
    try:
        with open(CONFIG_FILE, 'r') as f:
            content = f.read()
            match = re.search(r'block_size\s*=\s*(\d+)', content)
            if match:
                return match.group(1)
    except:
        pass
    return "500000" # Default if not found

def setup_results():
    if not os.path.exists(RESULTS_DIR):
        os.makedirs(RESULTS_DIR)
    if os.path.exists(RESULTS_CSV):
        os.remove(RESULTS_CSV)

def run_test(file_path, block_size):
    filename = file_path.name
    orig_size = os.path.getsize(file_path)
    if orig_size == 0: return None

    compressed_path = Path(RESULTS_DIR) / (filename + ".bz")
    official_path = Path(RESULTS_DIR) / (filename + ".official.bz")
    
    print(f"[*] Testing: {filename} ({orig_size / 1024 / 1024:.2f} MB)")
    
    # 1. CUSTOM ALGO ENCODING
    start_c = time.perf_counter()
    try:
        # Command: ./bzip2_stage1 encode <input> <output>
        subprocess.run([EXE, "encode", str(file_path), str(compressed_path)], 
                       check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        end_c = time.perf_counter()
        custom_time = round(end_c - start_c, 4)
        custom_size = os.path.getsize(compressed_path)
        custom_ratio = round((custom_size / orig_size) * 100, 2)
    except Exception as e:
        print(f"  [!] Custom Algo Failed: {e}")
        return None

    # 2. OFFICIAL BZIP2 ENCODING
    official_ratio = 0.0
    official_time = 0.0
    try:
        start_o = time.perf_counter()
        with open(file_path, 'rb') as f_in:
            data = f_in.read()
        compressed = bz2.compress(data)
        with open(official_path, 'wb') as f_out:
            f_out.write(compressed)
        end_o = time.perf_counter()
        official_time = round(end_o - start_o, 4)
        official_size = os.path.getsize(official_path)
        official_ratio = round((official_size / orig_size) * 100, 2)
    except Exception as e:
        print(f"  [!] Official bzip2 failed: {e}")
    return {
        "File": filename,
        "Size": orig_size,
        "BlockSize": block_size,
        "CompressionRatio": custom_ratio,
        "OfficialRatio": official_ratio, # Added for graph comparison
        "Time": custom_time,
        "OfficialTime": official_time,   # Added for graph comparison
        "Memory": "Approx. 4MB" 
    }

def main():
    block_size = get_block_size()
    setup_results()
    
    all_results = []
    # Find all files in benchmarks folder
    files = sorted(list(Path(BENCHMARK_DIR).glob("*.*")), key=os.path.getsize)

    if not files:
        print(f"[!] No files found in '{BENCHMARK_DIR}' folder.")
        return

    print(f"--- Starting Evaluation on {len(files)} files ---\n")

    for f_path in files:
        res = run_test(f_path, block_size)
        if res:
            all_results.append(res)
            print(f"  -> Ratio: {res['CompressionRatio']}% | Time: {res['Time']}s")
    
    # Writing CSV with EXACT columns from Point 7.3
    # Extra columns (OfficialRatio, OfficialTime) are added for the graph script
    keys = ["File", "Size", "BlockSize", "CompressionRatio", "OfficialRatio", "Time", "OfficialTime", "Memory"]
    with open(RESULTS_CSV, 'w', newline='') as f:
        writer = csv.DictWriter(f, fieldnames=keys)
        writer.writeheader()
        writer.writerows(all_results)
    
    print(f"\n[DONE] Results saved to {RESULTS_CSV}")

if __name__ == "__main__":
    main()
