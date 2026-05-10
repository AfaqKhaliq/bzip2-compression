import pandas as pd
import matplotlib.pyplot as plt
import os

# --- Configuration ---
CSV_FILE = "results.csv"
OUTPUT_DIR = "results/graphs"

def generate_graphs():
    if not os.path.exists(CSV_FILE):
        print(f"[!] {CSV_FILE} not found. Run run_benchmark.py first.")
        return

    if not os.path.exists(OUTPUT_DIR):
        os.makedirs(OUTPUT_DIR)

    df = pd.read_csv(CSV_FILE)
    
    # 1. EXTENSION-WISE COMPARISON
    # Extract extension from filename
    df['Extension'] = df['File'].apply(lambda x: os.path.splitext(x)[1].lower())
    ext_grouped = df.groupby('Extension')[['CompressionRatio', 'OfficialRatio']].mean()

    plt.figure(figsize=(10, 6))
    ext_grouped.plot(kind='bar', color=['skyblue', 'salmon'])
    plt.title('Average Compression Ratio by File Extension (Lower is Better)')
    plt.ylabel('Ratio (%)')
    plt.xlabel('Extension')
    plt.xticks(rotation=0)
    plt.legend(['Our Algo', 'Official BZip2'])
    plt.tight_layout()
    plt.savefig(f"{OUTPUT_DIR}/extension_comparison.png")

    # 2. SIZE-WISE TIME COMPARISON (Log Scale for Time)
    df['Size_MB'] = df['Size'] / (1024 * 1024)
    df_sorted = df.sort_values('Size_MB')
    
    plt.figure(figsize=(10, 6))
    plt.plot(df_sorted['Size_MB'], df_sorted['Time'], marker='o', label='Our Algo', color='green')
    plt.plot(df_sorted['Size_MB'], df_sorted['OfficialTime'], marker='s', label='Official BZip2', color='red')
    plt.title('Execution Time vs. File Size (Comparison)')
    plt.xlabel('File Size (MB)')
    plt.ylabel('Time (Seconds) - Log Scale')
    plt.yscale('log')
    plt.grid(True, linestyle='--', alpha=0.6)
    plt.legend()
    plt.tight_layout()
    plt.savefig(f"{OUTPUT_DIR}/size_time_scalability.png")

    # 3. DIRECT RATIO COMPARISON PER FILE
    plt.figure(figsize=(14, 7))
    x = range(len(df['File']))
    width = 0.35
    plt.bar(x, df['CompressionRatio'], width, label='Our Algo', color='skyblue')
    plt.bar([p + width for p in x], df['OfficialRatio'], width, label='Official BZip2', color='salmon')
    plt.title('Per-File Compression Ratio Comparison')
    plt.ylabel('Ratio (%)')
    plt.xticks([p + width/2 for p in x], df['File'], rotation=45, ha='right')
    plt.legend()
    plt.tight_layout()
    plt.savefig(f"{OUTPUT_DIR}/per_file_ratio.png")

    # 4. SPEED (MB/s) COMPARISON
    # Speed S = Size / Time
    df['Our_Speed'] = df['Size_MB'] / df['Time']
    # Filter out cases where official time might be 0 (if bzip2 not installed)
    df['Official_Speed'] = df['Size_MB'] / df['OfficialTime'].replace(0, 1e-9)

    plt.figure(figsize=(10, 6))
    plt.bar(x, df['Our_Speed'], width, label='Our Algo Speed', color='lightgreen')
    plt.bar([p + width for p in x], df['Official_Speed'], width, label='Official BZip2 Speed', color='orange')
    plt.title('Throughput Comparison (MB/s) - Higher is Better')
    plt.ylabel('Speed (MB/s)')
    plt.yscale('log') # Official bzip2 is usually orders of magnitude faster
    plt.xticks([p + width/2 for p in x], df['File'], rotation=45, ha='right')
    plt.legend()
    plt.tight_layout()
    plt.savefig(f"{OUTPUT_DIR}/throughput_comparison.png")

if __name__ == "__main__":
    generate_graphs()
    print("\n[SUCCESS] All comparison graphs generated in results/graphs/")
