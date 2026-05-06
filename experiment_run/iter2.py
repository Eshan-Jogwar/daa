import time
import math
import hashlib
import numpy as np

# --- SHARED HASH UTILITY ---
def generate_hashes(item, k, total_memory_size):
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

# --- REAL-TIME PARTITIONED CBF (Time-Driven) ---
class RealTimePartitionedCBF:
    def __init__(self, blocks, counters_per_block, num_hashes, delta_t_seconds):
        self.D = blocks
        self.m = counters_per_block
        self.k = num_hashes
        self.delta_t = delta_t_seconds
        self.filter_memory = np.zeros(self.D * self.m, dtype=np.uint8)
        
        # Track actual system time
        self.start_time = time.time()
        self.last_cycle = 0

    def _update_time_window(self):
        """Automatically calculates elapsed time and wipes stale blocks."""
        elapsed = time.time() - self.start_time
        current_cycle = int(elapsed // self.delta_t)
        
        # If the time interval formula dictates we've moved to a new block
        if current_cycle > self.last_cycle:
            # If the system slept longer than the entire window W, wipe everything
            if current_cycle - self.last_cycle >= self.D:
                self.filter_memory.fill(0)
            else:
                # Otherwise, zero-out exactly the blocks we are rotating over
                for cycle in range(self.last_cycle + 1, current_cycle + 1):
                    block_to_clear = cycle % self.D
                    start_idx = block_to_clear * self.m
                    end_idx = start_idx + self.m
                    self.filter_memory[start_idx:end_idx] = 0
            self.last_cycle = current_cycle

        # Return the current active block index
        return current_cycle % self.D

    def get_memory_index(self, block_idx, hash_idx):
        return (block_idx * self.m) + hash_idx

    def insert(self, item):
        active_block = self._update_time_window()
        for h in generate_hashes(item, self.k, self.m):
            idx = self.get_memory_index(active_block, h)
            if self.filter_memory[idx] < 255:
                self.filter_memory[idx] += 1

    def estimate_frequency(self, item):
        self._update_time_window() # Ensure memory is wiped if time passed before querying
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
# REAL-TIME EXECUTION SIMULATION
# ==========================================
if __name__ == "__main__":
    print("Initializing Real-Time Ad Pipeline Simulation...")
    
    # Fast-paced test: 5 blocks, 1.5 seconds per block. Total Window (W) = 7.5 seconds.
    delta_t = 1.5 
    tp_cbf = RealTimePartitionedCBF(blocks=5, counters_per_block=10000, num_hashes=3, delta_t_seconds=delta_t)
    std_bf = StandardBloomFilter(total_size=50000, num_hashes=3)
    
    test_queries = 2000
    
    def measure_fpr(tag):
        std_fp = sum(1 for i in range(test_queries) if std_bf.contains(f"unseen_{tag}_{i}"))
        tp_fp = sum(1 for i in range(test_queries) if tp_cbf.estimate_frequency(f"unseen_{tag}_{i}") > 0)
        print(f"   -> Standard BF FPR: {(std_fp / test_queries) * 100:.1f}% | Time-Partitioned CBF FPR: {(tp_fp / test_queries) * 100:.1f}%")

    # 1. Baseline Traffic
    print("\n[Time: 0.0s] Inserting normal baseline traffic...")
    for i in range(2000):
        std_bf.insert(f"ad_normal_{i}")
        tp_cbf.insert(f"ad_normal_{i}")
    measure_fpr("base")

    # 2. Simulate a massive burst 
    print(f"\n[Time: 1.0s] BURST DETECTED! Inserting 25,000 ad impressions instantly...")
    for i in range(25000):
        std_bf.insert(f"ad_burst_{i}")
        tp_cbf.insert(f"ad_burst_{i}")
    measure_fpr("burst")
    
    # 3. Wait for the active block to rotate out (Waiting out the delta_t interval)
    print(f"\n[Sleeping...] Waiting {delta_t} seconds for the active block to shift...")
    time.sleep(delta_t)
    
    print(f"\n[Time: {delta_t + 1.0}s] Active block shifted. Block saturated with burst data is now dormant.")
    measure_fpr("shifted")

    # 4. Wait for the FULL window to expire to trigger the wipe
    sleep_time = (5 * delta_t) - delta_t
    print(f"\n[Sleeping...] Waiting {sleep_time} seconds for the full window (W) to rotate...")
    time.sleep(sleep_time)
    
    print(f"\n[Time: {5 * delta_t + 1.0}s] BULK ERASURE TRIGGERED. The saturated burst block was mathematically overwritten.")
    measure_fpr("healed")
    
    print("\nSimulation Complete.")
