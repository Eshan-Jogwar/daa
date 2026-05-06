import hashlib
import numpy as np

# --- SHARED HASH UTILITY ---
def generate_hashes(item, k, total_memory_size):
    # Using md5 for deterministic, cross-platform hashing
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

# --- RAW TIME-PARTITIONED CBF (Data-Driven Time) ---
class TimePartitionedCBF:
    def __init__(self, blocks, counters_per_block, num_hashes, delta_t):
        self.D = blocks
        self.m = counters_per_block
        self.k = num_hashes
        self.delta_t = delta_t
        self.filter_memory = np.zeros(self.D * self.m, dtype=np.uint8)
        self.last_cycle = 0

    def _update_time_window(self, current_timestamp):
        """Calculates cycles passed based on the data's timestamp and clears stale blocks."""
        current_cycle = int(current_timestamp // self.delta_t)
        
        if current_cycle > self.last_cycle:
            cycles_passed = current_cycle - self.last_cycle
            
            # If the gap in data is larger than the total window W, wipe everything
            if cycles_passed >= self.D:
                self.filter_memory.fill(0)
            else:
                # Otherwise, clear only the specific blocks that aged out
                for i in range(1, cycles_passed + 1):
                    block_to_clear = (self.last_cycle + i) % self.D
                    start_idx = block_to_clear * self.m
                    end_idx = start_idx + self.m
                    self.filter_memory[start_idx:end_idx] = 0
                    
            self.last_cycle = current_cycle

        # Return the active block index for the current timestamp
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
        # Ensure memory is updated to the query's timestamp before checking
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
# RAW DATA PIPELINE SIMULATION
# ==========================================
if __name__ == "__main__":
    # Parameters to ensure near-0 baseline and 100% saturation during burst
    total_memory = 2000000  # 2 million total slots
    hash_count = 3
    num_blocks = 20
    block_size = total_memory // num_blocks
    delta_t = 1.0  # 1 virtual time unit per block

    std_bf = StandardBloomFilter(total_memory, hash_count)
    tp_cbf = TimePartitionedCBF(num_blocks, block_size, hash_count, delta_t)

    test_queries = 2000
    total_steps = 35
    ad_id_counter = 0

    print(f"{'Step':<6} | {'Traffic':<10} | {'Standard BF FPR (%)':<20} | {'TPCBF FPR (%)':<15}")
    print("-" * 62)

    for step in range(1, total_steps + 1):
        virtual_timestamp = float(step)
        
        # 1. Determine Traffic Volume
        traffic_this_step = 2500 
        
        if step == 10:
            traffic_this_step = 250000  # Massive burst
            print("-" * 62)
            print(f">>> BURST TRAFFIC INJECTED ({traffic_this_step} items)")
            print("-" * 62)
            
        if step == 30:
            print("-" * 62)
            print(f">>> BULK ERASURE TRIGGERED: BURST DATA WIPED (W={num_blocks} blocks)")
            print("-" * 62)

        # 2. Insert Data Stream
        for _ in range(traffic_this_step):
            ad = f"ad_event_{ad_id_counter}"
            ad_id_counter += 1
            std_bf.insert(ad)
            tp_cbf.insert(ad, virtual_timestamp)

        # 3. Query False Positive Rate
        std_fp = 0
        tp_fp = 0
        for i in range(test_queries):
            unseen_ad = f"unseen_query_step_{step}_{i}"
            if std_bf.contains(unseen_ad):
                std_fp += 1
            if tp_cbf.estimate_frequency(unseen_ad, virtual_timestamp) > 0:
                tp_fp += 1

        std_fpr_percent = (std_fp / test_queries) * 100.0
        tp_fpr_percent = (tp_fp / test_queries) * 100.0

        print(f"{step:<6} | {traffic_this_step:<10} | {std_fpr_percent:<20.2f} | {tp_fpr_percent:<15.2f}")
