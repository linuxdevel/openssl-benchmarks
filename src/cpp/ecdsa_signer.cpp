#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
#include <chrono>
#include <mutex>
#include <iomanip>
#include <algorithm>
#include <string>
#include <map>
#include <random>
#include <openssl/ec.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/rand.h>

class ECDSASigner {
private:
    uint64_t total_signatures_generated{0};
    uint64_t total_time_microseconds{0};
    uint64_t min_time_microseconds{0};
    uint64_t max_time_microseconds{0};
    bool first_signature_generated{false};
    std::mutex stats_mutex;
    std::chrono::steady_clock::time_point start_time;
    
    // Mapping of curve names to OpenSSL NID constants
    std::map<std::string, int> curve_map = {
        {"P256", NID_X9_62_prime256v1},
        {"P384", NID_secp384r1},
        {"P521", NID_secp521r1}
    };
    
public:
    ECDSASigner() {
        start_time = std::chrono::steady_clock::now();
        min_time_microseconds = 0;
        max_time_microseconds = 0;
        first_signature_generated = false;
    }
    
    EVP_PKEY* createECKey(const std::string& curve_name) {
        auto it = curve_map.find(curve_name);
        if (it == curve_map.end()) {
            std::cerr << "Unsupported curve: " << curve_name << std::endl;
            return nullptr;
        }
        
        int curve_nid = it->second;
        
        EVP_PKEY_CTX* pctx = EVP_PKEY_CTX_new_id(EVP_PKEY_EC, nullptr);
        if (!pctx) {
            return nullptr;
        }
        
        if (EVP_PKEY_keygen_init(pctx) <= 0) {
            EVP_PKEY_CTX_free(pctx);
            return nullptr;
        }
        
        if (EVP_PKEY_CTX_set_ec_paramgen_curve_nid(pctx, curve_nid) <= 0) {
            EVP_PKEY_CTX_free(pctx);
            return nullptr;
        }
        
        EVP_PKEY* pkey = nullptr;
        if (EVP_PKEY_keygen(pctx, &pkey) <= 0) {
            EVP_PKEY_CTX_free(pctx);
            return nullptr;
        }
        
        EVP_PKEY_CTX_free(pctx);
        return pkey;
    }
    
    void updateStats(uint64_t time_microseconds) {
        // Use mutex to protect all statistics for consistency
        std::lock_guard<std::mutex> lock(stats_mutex);
        
        // Sanity check: reject clearly invalid timing values
        // EC signing should never take more than 1 second (1,000,000 microseconds)
        if (time_microseconds == 0 || time_microseconds > 1000000ULL) {
            return; // Skip this invalid measurement
        }
        
        total_signatures_generated++;
        total_time_microseconds += time_microseconds;
        
        if (!first_signature_generated) {
            min_time_microseconds = time_microseconds;
            max_time_microseconds = time_microseconds;
            first_signature_generated = true;
        } else {
            if (time_microseconds < min_time_microseconds) {
                min_time_microseconds = time_microseconds;
            }
            if (time_microseconds > max_time_microseconds) {
                max_time_microseconds = time_microseconds;
            }
        }
    }
    
    void workerThread(const std::string& curve_name, int num_loops) {
        // Create one EC key per thread before the loop starts
        EVP_PKEY* ec_key = createECKey(curve_name);
        if (!ec_key) {
            std::cerr << "Failed to create EC key for thread" << std::endl;
            return;
        }
        
        // Create signing context
        EVP_MD_CTX* md_ctx = EVP_MD_CTX_new();
        if (!md_ctx) {
            EVP_PKEY_free(ec_key);
            std::cerr << "Failed to create MD context for thread" << std::endl;
            return;
        }
        
        // Initialize for signing with SHA-256
        if (EVP_DigestSignInit(md_ctx, nullptr, EVP_sha256(), nullptr, ec_key) <= 0) {
            EVP_MD_CTX_free(md_ctx);
            EVP_PKEY_free(ec_key);
            std::cerr << "Failed to initialize signing context for thread" << std::endl;
            return;
        }
        
        // Pre-allocate variables outside the loop for better performance
        unsigned char data[32];
        unsigned char* signature = nullptr;
        size_t signature_len = 0;
        auto start_time = std::chrono::steady_clock::now();
        auto end_time = std::chrono::steady_clock::now();
        
        // Random number generator for data
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<unsigned char> dis(0, 255);
        
        // Perform signing operations in the loop
        for (int i = 0; i < num_loops; i++) {
            // Fill buffer with random data
            for (int j = 0; j < 32; j++) {
                data[j] = dis(gen);
            }
            
            start_time = std::chrono::steady_clock::now();
            
            // Reset the context for each signature
            if (EVP_DigestSignInit(md_ctx, nullptr, EVP_sha256(), nullptr, ec_key) > 0) {
                // Update with data to be signed
                if (EVP_DigestSignUpdate(md_ctx, data, 32) > 0) {
                    // Get signature length
                    if (EVP_DigestSignFinal(md_ctx, nullptr, &signature_len) > 0) {
                        // Allocate signature buffer
                        signature = (unsigned char*)OPENSSL_malloc(signature_len);
                        if (signature) {
                            // Generate actual signature
                            if (EVP_DigestSignFinal(md_ctx, signature, &signature_len) > 0) {
                                end_time = std::chrono::steady_clock::now();
                                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
                                
                                // Ensure we got a valid positive duration
                                if (duration.count() > 0) {
                                    updateStats(static_cast<uint64_t>(duration.count()));
                                }
                            }
                            
                            // Clean up signature buffer
                            OPENSSL_free(signature);
                            signature = nullptr;
                        }
                    }
                }
            }
        }
        
        // Clean up
        EVP_MD_CTX_free(md_ctx);
        EVP_PKEY_free(ec_key);
    }
    
    void printStats() {
        auto current_time = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(current_time - start_time);
        
        // Get all statistics safely under the same mutex
        uint64_t total_signatures, total_time, min_time, max_time;
        bool signatures_generated;
        {
            std::lock_guard<std::mutex> lock(stats_mutex);
            total_signatures = total_signatures_generated;
            total_time = total_time_microseconds;
            min_time = min_time_microseconds;
            max_time = max_time_microseconds;
            signatures_generated = first_signature_generated;
        }
        
        if (total_signatures == 0) {
            std::cout << "\rSigs: 0, Throughput: 0.00 sigs/s, Avg: 0.00ms, Min: 0.00ms, Max: 0.00ms" << std::flush;
            return;
        }
        
        double throughput = (elapsed.count() > 0) ? static_cast<double>(total_signatures) / elapsed.count() : 0.0;
        double avg_time_ms = static_cast<double>(total_time) / total_signatures / 1000.0;
        
        // Convert to milliseconds
        double min_time_ms = static_cast<double>(min_time) / 1000.0;
        double max_time_ms = static_cast<double>(max_time) / 1000.0;
        
        // If we haven't generated any signatures yet or values are uninitialized, show 0
        if (!signatures_generated || min_time == 0 || max_time == 0) {
            min_time_ms = 0.0;
            max_time_ms = 0.0;
        }
        
        std::cout << "\rSigs: " << std::setw(6) << total_signatures 
                  << ", Throughput: " << std::fixed << std::setprecision(2) << std::setw(8) << throughput << " sigs/s"
                  << ", Avg: " << std::setw(6) << avg_time_ms << "ms"
                  << ", Min: " << std::setw(6) << min_time_ms << "ms"
                  << ", Max: " << std::setw(6) << max_time_ms << "ms" 
                  << std::flush;
    }
    
    void run(const std::string& curve_name, int num_threads, int num_loops) {
        // Validate curve name
        if (curve_map.find(curve_name) == curve_map.end()) {
            std::cerr << "Error: Unsupported curve '" << curve_name << "'" << std::endl;
            std::cerr << "Supported curves: P256, P384, P521" << std::endl;
            return;
        }
        
        std::cout << "Starting EC-DSA signing performance test with:" << std::endl;
        std::cout << "Curve: " << curve_name << std::endl;
        std::cout << "Threads: " << num_threads << std::endl;
        std::cout << "Loops per thread: " << num_loops << std::endl;
        std::cout << "Total signatures to generate: " << (num_threads * num_loops) << std::endl;
        std::cout << "Data size: 32 bytes (random data per signature)" << std::endl;
        std::cout << "Hash algorithm: SHA-256" << std::endl;
        std::cout << std::endl;
        
        std::vector<std::thread> threads;
        
        // Start worker threads
        for (int i = 0; i < num_threads; i++) {
            threads.emplace_back(&ECDSASigner::workerThread, this, curve_name, num_loops);
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
    
    void listSupportedCurves() {
        std::cout << "Supported EC curves:" << std::endl;
        std::cout << "  P256  - NIST P-256 (secp256r1, prime256v1) - 256-bit" << std::endl;
        std::cout << "  P384  - NIST P-384 (secp384r1) - 384-bit" << std::endl;
        std::cout << "  P521  - NIST P-521 (secp521r1) - 521-bit" << std::endl;
    }
};

void printUsage(const char* program_name) {
    std::cout << "Usage: " << program_name << " <curve> <num_threads> <num_loops>" << std::endl;
    std::cout << "  curve       - EC curve name (P256, P384, P521)" << std::endl;
    std::cout << "  num_threads - Number of worker threads" << std::endl;
    std::cout << "  num_loops   - Number of signatures to generate per thread" << std::endl;
    std::cout << std::endl;
    std::cout << "Examples:" << std::endl;
    std::cout << "  " << program_name << " P256 4 1000  # Generate 4000 P-256 signatures using 4 threads" << std::endl;
    std::cout << "  " << program_name << " P384 8 500   # Generate 4000 P-384 signatures using 8 threads" << std::endl;
    std::cout << "  " << program_name << " P521 2 250   # Generate 500 P-521 signatures using 2 threads" << std::endl;
    std::cout << std::endl;
    std::cout << "Use '" << program_name << " --curves' to list supported curves" << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc == 2 && (std::string(argv[1]) == "--curves" || std::string(argv[1]) == "-c")) {
        ECDSASigner signer;
        signer.listSupportedCurves();
        return 0;
    }
    
    if (argc != 4) {
        printUsage(argv[0]);
        return 1;
    }
    
    std::string curve_name = argv[1];
    int num_threads = std::atoi(argv[2]);
    int num_loops = std::atoi(argv[3]);
    
    // Convert to uppercase for consistency
    std::transform(curve_name.begin(), curve_name.end(), curve_name.begin(), ::toupper);
    
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
    
    ECDSASigner signer;
    signer.run(curve_name, num_threads, num_loops);
    
    // Cleanup OpenSSL
    ERR_free_strings();
    
    return 0;
}
