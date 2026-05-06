#include <algorithm>
#include <cstdint>
#include <functional>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

// --- SHARED HASH UTILITY ---
std::vector<size_t> generate_hashes(const std::string &item, size_t k,
                                    size_t total_memory_size) {
  std::vector<size_t> hashes(k);
  std::hash<std::string> hasher;
  size_t base_hash = hasher(item);
  for (size_t i = 0; i < k; ++i) {
    hashes[i] = (base_hash + i * 0x9e3779b9) % total_memory_size;
  }
  return hashes;
}

// --- STANDARD BLOOM FILTER ---
class StandardBloomFilter {
private:
  size_t size;
  size_t k;
  std::vector<bool> filter_memory;

public:
  StandardBloomFilter(size_t total_size, size_t num_hashes)
      : size(total_size), k(num_hashes) {
    filter_memory.assign(size, false);
  }
  void insert(const std::string &item) {
    for (size_t hash_val : generate_hashes(item, k, size))
      filter_memory[hash_val] = true;
  }
  bool contains(const std::string &item) const {
    for (size_t hash_val : generate_hashes(item, k, size)) {
      if (!filter_memory[hash_val])
        return false;
    }
    return true;
  }
};

// --- TIME-PARTITIONED CBF ---
class TimePartitionedCBF {
private:
  size_t D;
  size_t m;
  size_t k;
  size_t active_block;
  std::vector<uint8_t> filter_memory;
  inline size_t get_memory_index(size_t block_idx, size_t hash_idx) const {
    return (block_idx * m) + hash_idx;
  }

public:
  TimePartitionedCBF(size_t blocks, size_t counters_per_block,
                     size_t num_hashes)
      : D(blocks), m(counters_per_block), k(num_hashes), active_block(0) {
    filter_memory.assign(D * m, 0);
  }
  void insert(const std::string &item) {
    for (size_t hash_val : generate_hashes(item, k, m)) {
      size_t idx = get_memory_index(active_block, hash_val);
      if (filter_memory[idx] < 255)
        filter_memory[idx]++;
    }
  }
  int estimate_frequency(const std::string &item) const {
    std::vector<size_t> hashes = generate_hashes(item, k, m);
    int total_frequency = 0;
    for (size_t block = 0; block < D; ++block) {
      uint8_t min_count = 255;
      for (size_t hash_val : hashes) {
        min_count = std::min(min_count,
                             filter_memory[get_memory_index(block, hash_val)]);
      }
      total_frequency += min_count;
    }
    return total_frequency;
  }
  void advance_time_window() {
    active_block = (active_block + 1) % D;
    std::fill(filter_memory.begin() + (active_block * m),
              filter_memory.begin() + (active_block * m) + m, 0);
  }
};

// --- CONTINUOUS BENCHMARKING SIMULATION ---
int main() {
  size_t total_memory = 100000; // 100k slots total
  size_t hash_count = 3;
  size_t num_blocks = 100;                       // 20 time blocks (W = 20 * dt)
  size_t block_size = total_memory / num_blocks; // 5000 slots per block

  StandardBloomFilter std_bf(total_memory, hash_count);
  TimePartitionedCBF tp_cbf(num_blocks, block_size, hash_count);

  int test_queries = 5000; // Number of unseen ads to test FPR against
  int total_steps = 35;    // Total time intervals to simulate

  // --- TERMINAL FORMATTING HEADER ---
  std::cout << std::string(75, '=') << "\n";
  std::cout
      << "   TIME-PARTITIONED vs STANDARD BLOOM FILTER BENCHMARK SIMULATION\n";
  std::cout << std::string(75, '=') << "\n";

  std::cout << std::left << std::setw(10) << "Step" << std::setw(20)
            << "Traffic Inserted" << std::setw(25) << "Standard BF FPR (%)"
            << std::setw(20) << "TPCBF FPR (%)" << "\n";
  std::cout << std::string(75, '-') << "\n";

  int ad_id_counter = 0;

  for (int step = 1; step <= total_steps; ++step) {
    // 1. Determine traffic volume for this time step
    int traffic_this_step = 1500; // Normal background traffic

    // Inject a massive traffic burst at step 10
    if (step == 10) {
      traffic_this_step = 25000;
      std::cout << std::string(75, '-') << "\n";
      std::cout
          << " >>> WARNING: MASSIVE TRAFFIC BURST DETECTED (25,000 Ads) <<<\n";
      std::cout << std::string(75, '-') << "\n";
    }

    // Trigger self-healing visual exactly W (20) steps after the burst
    if (step == 30) {
      std::cout << std::string(75, '-') << "\n";
      std::cout
          << " >>> TPCBF BULK ERASURE TRIGGERED: STALE BURST DATA PURGED <<<\n";
      std::cout << std::string(75, '-') << "\n";
    }

    // 2. Insert traffic into both filters
    for (int i = 0; i < traffic_this_step; ++i) {
      std::string ad = "ad_event_" + std::to_string(ad_id_counter++);
      std_bf.insert(ad);
      tp_cbf.insert(ad);
    }

    // 3. Measure False Positive Rate (FPR)
    int std_fp = 0, tp_fp = 0;
    for (int i = 0; i < test_queries; ++i) {
      // Generate completely unique strings that have never been inserted
      std::string unseen_ad =
          "unseen_query_step_" + std::to_string(step) + "_" + std::to_string(i);
      if (std_bf.contains(unseen_ad))
        std_fp++;
      if (tp_cbf.estimate_frequency(unseen_ad) > 0)
        tp_fp++;
    }

    double std_fpr_percent = (std_fp / (double)test_queries) * 100.0;
    double tp_fpr_percent = (tp_fp / (double)test_queries) * 100.0;

    // 4. Print row with aligned columns
    std::cout << std::left << std::setw(10) << step << std::setw(20)
              << traffic_this_step << std::setw(25) << std::fixed
              << std::setprecision(2) << std_fpr_percent << std::setw(20)
              << tp_fpr_percent << "\n";

    // 5. Advance the time window for the proposed system
    tp_cbf.advance_time_window();
  }

  std::cout << std::string(75, '=') << "\n";
  std::cout << "   SIMULATION COMPLETE\n";
  std::cout << std::string(75, '=') << "\n";

  return 0;
}
