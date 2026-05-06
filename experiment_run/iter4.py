import hashlib
import numpy as np
import matplotlib.pyplot as plt

# --- SHARED HASH UTILITY ---
def generate_hashes(item, k, total_memory_size):
    # Deterministic hashing for accurate collision simulation
    base_hash = int(hashlib.md5(item.encode('utf-8')).hexdigest()[:15], 16)
    return [(base_hash + i * 0x9e3779b9) % total_memory_size for i in range(k)]

# --- STANDARD BLOOM FILTER ---
class StandardBloomFilter:
    def __init__(self, total_size, num_hashes):
        self.size = total_size
        self.k = num_hashes
        self.filter_memory = np.zeros(self.size, dtype=bool)
        
    def insert(self, item):
        for h in generate_hashes(item, self.k, self.size):
            self.filter_memory[h] = True
            
    def contains(self, item):
        for h in generate_hashes(item, self.k, self.size):
            if not self.filter_memory[h]: 
                return False
        return True

# --- RAW TIME-PARTITIONED CBF ---
class TimePartitionedCBF:
    def __init__(self, blocks, counters_per_block, num_hashes, delta_t):
        self.D = blocks
        self.m = counters_per_block
        self.k = num_hashes
        self.delta_t = delta_t
        self.filter_memory = np.zeros(self.D * self.m, dtype=np.uint8)
        self.last_cycle = 0

    def _update_time_window(self, current_timestamp):
        current_cycle = int(current_timestamp // self.delta_t)
        if current_cycle > self.last_cycle:
            cycles_passed = current_cycle - self.last_cycle
            if cycles_passed >= self.D:
                self.filter_memory.fill(0)
            else:
                for i in range(1, cycles_passed + 1):
                    block_to_clear = (self.last_cycle + i) % self.D
                    start_idx = block_to_clear * self.m
                    end_idx = start_idx + self.m
                    self.filter_memory[start_idx:end_idx] = 0
            self.last_cycle = current_cycle
        return current_cycle % self.D

    def get_memory_index(self, block_idx, hash_idx):
        return (block_idx * self.m) + hash_idx

    def insert(self, item, current_timestamp):
        active_block = self._update_time_window(current_timestamp)
        for h in generate_hashes(item, self.k, self.m):
            idx = self.get_memory_index(active_block, h)
            if self.filter_memory[idx] < 255:
                self.filter_memory[idx] += 1

    def estimate_frequency(self, item, current_timestamp):
        self._update_time_window(current_timestamp)
        hashes = generate_hashes(item, self.k, self.m)
        total_freq = 0
        for block in range(self.D):
            min_count = 255
            for h in hashes:
                idx = self.get_memory_index(block, h)
                if self.filter_memory[idx] < min_count: 
                    min_count = self.filter_memory[idx]
            total_freq += min_count
        return total_freq

# ==========================================
# SIMULATION EXECUTION
# ==========================================
if __name__ == "__main__":
    print("Running multi-burst simulation... (This may take a few seconds to hash 1M+ items)")

    # Memory parameters optimized to perfectly demonstrate the toggling effect
    total_memory = 1000000  # 1 Million slots
    hash_count = 3
    num_blocks = 20
    block_size = total_memory // num_blocks
    delta_t = 1.0  # 1 step = 1 block rotation

    std_bf = StandardBloomFilter(total_memory, hash_count)
    tp_cbf = TimePartitionedCBF(num_blocks, block_size, hash_count, delta_t)

    test_queries = 2000
    total_steps = 100
    burst_steps = [15, 45, 75] # Spaced by 30 steps so the 20-block window fully clears between hits
    ad_id_counter = 0

    steps_data, std_fprs, tp_fprs = [], [], []

    for step in range(1, total_steps + 1):
        virtual_timestamp = float(step)
        
        # 1. Traffic Volume (Base traffic near 0, Burst traffic massive)
        traffic_this_step = 200 
        if step in burst_steps:
            traffic_this_step = 300000 
            print(f"[*] Step {step}: MASSIVE BURST INJECTED (300,000 items)")

        # 2. Insert Data
        for _ in range(traffic_this_step):
            ad = f"ad_event_{ad_id_counter}"
            ad_id_counter += 1
            std_bf.insert(ad)
            tp_cbf.insert(ad, virtual_timestamp)

        # 3. Measure FPR
        std_fp = 0
        tp_fp = 0
        for i in range(test_queries):
            unseen_ad = f"unseen_query_step_{step}_{i}"
            if std_bf.contains(unseen_ad):
                std_fp += 1
            if tp_cbf.estimate_frequency(unseen_ad, virtual_timestamp) > 0:
                tp_fp += 1

        std_fprs.append((std_fp / test_queries) * 100.0)
        tp_fprs.append((tp_fp / test_queries) * 100.0)
        steps_data.append(step)

        if step % 10 == 0:
            print(f"  -> Step {step}/100 Processed | Std BF FPR: {std_fprs[-1]:.1f}% | TPCBF FPR: {tp_fprs[-1]:.1f}%")

    # ==========================================
    # GENERATE PUBLICATION-READY GRAPH
    # ==========================================
    print("\nGenerating graph...")
    plt.figure(figsize=(12, 6))

    # Plot the exact algorithm behavior
    plt.plot(steps_data, std_fprs, label='Standard Bloom Filter FPR', color='#e74c3c', linewidth=2.5)
    plt.plot(steps_data, tp_fprs, label='Time-Partitioned CBF FPR', color='#2980b9', linewidth=2.5)

    # Highlight the burst zones and erasure points
    for b_step in burst_steps:
        plt.axvline(x=b_step, color='#f39c12', linestyle='--', alpha=0.7, label='Traffic Burst' if b_step == burst_steps[0] else "")
        plt.axvline(x=b_step + num_blocks, color='#27ae60', linestyle='--', alpha=0.7, label='Data Evicted (Self-Heal)' if b_step == burst_steps[0] else "")

    # Formatting
    plt.title('Algorithm Comparison: Monotonic Saturation vs. Deterministic Recovery', fontsize=14, fontweight='bold')
    plt.xlabel('Time Step / Processing Cycles', fontsize=12)
    plt.ylabel('False Positive Rate (%)', fontsize=12)
    plt.xlim(1, 100)
    plt.ylim(0, 105)
    plt.grid(True, linestyle=':', alpha=0.6)
    plt.legend(loc='upper left', framealpha=0.9)
    plt.tight_layout()
    plt.show()
