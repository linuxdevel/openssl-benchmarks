#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
#include <chrono>
#include <mutex>
#include <iomanip>
#include <algorithm>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/evp.h>

class RSAGenerator {
private:
    uint64_t total_keys_generated{0};
    uint64_t total_time_microseconds{0};
    uint64_t min_time_microseconds{0};
    uint64_t max_time_microseconds{0};
    bool first_key_generated{false};
    std::mutex stats_mutex;
    std::chrono::steady_clock::time_point start_time;
    
public:
    RSAGenerator() {
        start_time = std::chrono::steady_clock::now();
        min_time_microseconds = 0;
        max_time_microseconds = 0;
        first_key_generated = false;
    }
    
    EVP_PKEY_CTX* createKeygenContext(int keysize) {
        EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr);
        if (!ctx) {
            return nullptr;
        }
        
        if (EVP_PKEY_keygen_init(ctx) <= 0) {
            EVP_PKEY_CTX_free(ctx);
            return nullptr;
        }
        
        if (EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, keysize) <= 0) {
            EVP_PKEY_CTX_free(ctx);
            return nullptr;
        }
        
        return ctx;
    }
    
    void updateStats(uint64_t time_microseconds) {
        // Use mutex to protect all statistics for consistency
        std::lock_guard<std::mutex> lock(stats_mutex);
        
        // Sanity check: reject clearly invalid timing values
        // RSA key generation should never take more than 1 hour (3,600,000,000 microseconds)
        // or be negative/zero due to timing issues
        if (time_microseconds == 0 || time_microseconds > 3600000000ULL) {
            return; // Skip this invalid measurement
        }
        
        total_keys_generated++;
        total_time_microseconds += time_microseconds;
        
        if (!first_key_generated) {
            min_time_microseconds = time_microseconds;
            max_time_microseconds = time_microseconds;
            first_key_generated = true;
        } else {
            if (time_microseconds < min_time_microseconds) {
                min_time_microseconds = time_microseconds;
            }
            if (time_microseconds > max_time_microseconds) {
                max_time_microseconds = time_microseconds;
            }
        }
    }
    
    void workerThread(int keysize, int num_loops) {
        // Create the key generation context once per thread
        EVP_PKEY_CTX* ctx = createKeygenContext(keysize);
        if (!ctx) {
            std::cerr << "Failed to create key generation context for thread" << std::endl;
            return;
        }
        
        // Pre-allocate variables outside the loop for better performance
        EVP_PKEY* pkey = nullptr;
        auto start_time = std::chrono::steady_clock::now();
        auto end_time = std::chrono::steady_clock::now();
        
        // Generate keys using the reused context
        for (int i = 0; i < num_loops; i++) {
            start_time = std::chrono::steady_clock::now();
            
            if (EVP_PKEY_keygen(ctx, &pkey) > 0) {
                end_time = std::chrono::steady_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
                
                // Ensure we got a valid positive duration
                if (duration.count() > 0) {
                    updateStats(static_cast<uint64_t>(duration.count()));
                }
                
                // Clean up the key
                EVP_PKEY_free(pkey);
                pkey = nullptr;
            }
        }
        
        // Clean up the context when thread is done
        EVP_PKEY_CTX_free(ctx);
    }
    
    void printStats() {
        auto current_time = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(current_time - start_time);
        
        // Get all statistics safely under the same mutex
        uint64_t total_keys, total_time, min_time, max_time;
        bool keys_generated;
        {
            std::lock_guard<std::mutex> lock(stats_mutex);
            total_keys = total_keys_generated;
            total_time = total_time_microseconds;
            min_time = min_time_microseconds;
            max_time = max_time_microseconds;
            keys_generated = first_key_generated;
        }
        
        if (total_keys == 0) {
            std::cout << "\rKeys: 0, Throughput: 0.00 keys/s, Avg: 0.00ms, Min: 0.00ms, Max: 0.00ms" << std::flush;
            return;
        }
        
        double throughput = (elapsed.count() > 0) ? static_cast<double>(total_keys) / elapsed.count() : 0.0;
        double avg_time_ms = static_cast<double>(total_time) / total_keys / 1000.0;
        
        // Convert to milliseconds with proper bounds checking
        double min_time_ms = static_cast<double>(min_time) / 1000.0;
        double max_time_ms = static_cast<double>(max_time) / 1000.0;
        
        // If we haven't generated any keys yet or values are uninitialized, show 0
        if (!keys_generated || min_time == 0 || max_time == 0) {
            min_time_ms = 0.0;
            max_time_ms = 0.0;
        }
        
        std::cout << "\rKeys: " << std::setw(6) << total_keys 
                  << ", Throughput: " << std::fixed << std::setprecision(2) << std::setw(6) << throughput << " keys/s"
                  << ", Avg: " << std::setw(7) << avg_time_ms << "ms"
                  << ", Min: " << std::setw(7) << min_time_ms << "ms"
                  << ", Max: " << std::setw(7) << max_time_ms << "ms" 
                  << std::flush;
    }
    
    void run(int keysize, int num_threads, int num_loops) {
        std::cout << "Starting RSA key generation with:" << std::endl;
        std::cout << "Key size: " << keysize << " bits" << std::endl;
        std::cout << "Threads: " << num_threads << std::endl;
        std::cout << "Loops per thread: " << num_loops << std::endl;
        std::cout << "Total keys to generate: " << (num_threads * num_loops) << std::endl;
        std::cout << std::endl;
        
        std::vector<std::thread> threads;
        
        // Start worker threads
        for (int i = 0; i < num_threads; i++) {
            threads.emplace_back(&RSAGenerator::workerThread, this, keysize, num_loops);
        }
        
        // Stats printing thread
        std::atomic<bool> done{false};
        std::thread stats_thread([this, &done]() {
            while (!done.load()) {
                printStats();
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            }
        });
        
        // Wait for all worker threads to complete
        for (auto& t : threads) {
            t.join();
        }
        
        done = true;
        stats_thread.join();
        
        // Print final statistics
        std::cout << std::endl << std::endl;
        std::cout << "Final Statistics:" << std::endl;
        printStats();
        std::cout << std::endl;
    }
};

void printUsage(const char* program_name) {
    std::cout << "Usage: " << program_name << " <keysize> <num_threads> <num_loops>" << std::endl;
    std::cout << "  keysize     - RSA key size in bits (e.g., 1024, 2048, 4096)" << std::endl;
    std::cout << "  num_threads - Number of worker threads" << std::endl;
    std::cout << "  num_loops   - Number of key pairs to generate per thread" << std::endl;
    std::cout << std::endl;
    std::cout << "Example: " << program_name << " 2048 4 100" << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        printUsage(argv[0]);
        return 1;
    }
    
    int keysize = std::atoi(argv[1]);
    int num_threads = std::atoi(argv[2]);
    int num_loops = std::atoi(argv[3]);
    
    if (keysize < 512 || keysize > 8192) {
        std::cerr << "Error: Key size must be between 512 and 8192 bits" << std::endl;
        return 1;
    }
    
    if (num_threads < 1 || num_threads > 100) {
        std::cerr << "Error: Number of threads must be between 1 and 100" << std::endl;
        return 1;
    }
    
    if (num_loops < 1) {
        std::cerr << "Error: Number of loops must be at least 1" << std::endl;
        return 1;
    }
    
    // Initialize OpenSSL
    ERR_load_crypto_strings();
    
    RSAGenerator generator;
    generator.run(keysize, num_threads, num_loops);
    
    // Cleanup OpenSSL
    ERR_free_strings();
    
    return 0;
}
