#include <iostream>
#include <chrono>
#include <cmath>
#include <cstring>
#include <fstream>
#include <sstream>
#include <vector>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/ec.h>
#include <openssl/err.h>
#include <openssl/opensslv.h>
#include <sys/utsname.h>
#include <unistd.h>

#include <openssl/opensslv.h>
#include <sys/utsname.h>
#include <unistd.h>

std::string get_cpu_info() {
    std::ifstream cpuinfo("/proc/cpuinfo");
    std::string line;
    std::string cpu_model = "Unknown";
    
    while (std::getline(cpuinfo, line)) {
        if (line.find("model name") != std::string::npos) {
            size_t colon = line.find(":");
            if (colon != std::string::npos) {
                cpu_model = line.substr(colon + 2);
                break;
            }
        }
    }
    return cpu_model;
}

std::string get_cpu_flags() {
    std::ifstream cpuinfo("/proc/cpuinfo");
    std::string line;
    std::string flags = "";
    
    while (std::getline(cpuinfo, line)) {
        if (line.find("flags") != std::string::npos) {
            size_t colon = line.find(":");
            if (colon != std::string::npos) {
                std::string all_flags = line.substr(colon + 2);
                std::istringstream iss(all_flags);
                std::string flag;
                std::string crypto_flags;
                
                // Look for crypto-relevant flags
                while (iss >> flag) {
                    if (flag == "aes" || flag == "sha_ni" || flag == "avx" || 
                        flag == "avx2" || flag == "sse4_1" || flag == "sse4_2" ||
                        flag == "pclmulqdq" || flag == "rdrand" || flag == "rdseed") {
                        if (!crypto_flags.empty()) crypto_flags += ", ";
                        crypto_flags += flag;
                    }
                }
                return crypto_flags.empty() ? "none detected" : crypto_flags;
            }
        }
    }
    return "unavailable";
}

int get_cpu_cores() {
    return sysconf(_SC_NPROCESSORS_ONLN);
}

void print_system_info() {
    struct utsname sys_info;
    uname(&sys_info);
    
    std::cout << "System Information:" << std::endl;
    std::cout << "===================" << std::endl;
    std::cout << "OS: " << sys_info.sysname << " " << sys_info.release << std::endl;
    std::cout << "Architecture: " << sys_info.machine << std::endl;
    std::cout << "CPU: " << get_cpu_info() << std::endl;
    std::cout << "CPU Cores: " << get_cpu_cores() << std::endl;
    std::cout << "Crypto CPU Features: " << get_cpu_flags() << std::endl;
    std::cout << "OpenSSL Version: " << OPENSSL_VERSION_TEXT << std::endl;
    std::cout << std::endl;
}

void benchmark_rsa_vs_ecdsa() {
    const int iterations = 100;
    unsigned char data[32];
    memset(data, 0xAA, 32); // Simple test data
    
    std::cout << "Cryptographic Operation Performance Comparison" << std::endl;
    std::cout << "=============================================" << std::endl;
    std::cout << "Iterations: " << iterations << std::endl;
    std::cout << "RSA Algorithm: RSA-PSS with SHA-256 and MGF1-SHA256" << std::endl;
    std::cout << "ECDSA Algorithm: ECDSA with SHA-256" << std::endl;
    std::cout << std::endl;
    
    // RSA 3072 Key Generation and Signing (RSA-PSS)
    auto start = std::chrono::high_resolution_clock::now();
    
    EVP_PKEY_CTX* rsa_ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr);
    EVP_PKEY_keygen_init(rsa_ctx);
    EVP_PKEY_CTX_set_rsa_keygen_bits(rsa_ctx, 3072);
    EVP_PKEY* rsa_key = nullptr;
    EVP_PKEY_keygen(rsa_ctx, &rsa_key);
    
    auto rsa_keygen_time = std::chrono::high_resolution_clock::now();
    
    // RSA-PSS Signing with SHA-256 and MGF1-SHA256
    EVP_MD_CTX* rsa_md_ctx = EVP_MD_CTX_new();
    EVP_PKEY_CTX* rsa_sign_ctx = nullptr;
    EVP_DigestSignInit(rsa_md_ctx, &rsa_sign_ctx, EVP_sha256(), nullptr, rsa_key);
    
    // Configure RSA-PSS parameters
    EVP_PKEY_CTX_set_rsa_padding(rsa_sign_ctx, RSA_PKCS1_PSS_PADDING);
    EVP_PKEY_CTX_set_rsa_pss_saltlen(rsa_sign_ctx, RSA_PSS_SALTLEN_DIGEST);
    EVP_PKEY_CTX_set_rsa_mgf1_md(rsa_sign_ctx, EVP_sha256());
    
    // Store signatures for verification
    std::vector<std::vector<unsigned char>> rsa_signatures(iterations);
    
    auto rsa_sign_start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; i++) {
        EVP_DigestSignInit(rsa_md_ctx, &rsa_sign_ctx, EVP_sha256(), nullptr, rsa_key);
        
        // Re-configure RSA-PSS parameters for each signature
        EVP_PKEY_CTX_set_rsa_padding(rsa_sign_ctx, RSA_PKCS1_PSS_PADDING);
        EVP_PKEY_CTX_set_rsa_pss_saltlen(rsa_sign_ctx, RSA_PSS_SALTLEN_DIGEST);
        EVP_PKEY_CTX_set_rsa_mgf1_md(rsa_sign_ctx, EVP_sha256());
        
        EVP_DigestSignUpdate(rsa_md_ctx, data, 32);
        
        size_t sig_len = 0;
        EVP_DigestSignFinal(rsa_md_ctx, nullptr, &sig_len);
        rsa_signatures[i].resize(sig_len);
        EVP_DigestSignFinal(rsa_md_ctx, rsa_signatures[i].data(), &sig_len);
    }
    auto rsa_sign_end = std::chrono::high_resolution_clock::now();
    
    // EC P-256 Key Generation and Signing
    auto ec_start = std::chrono::high_resolution_clock::now();
    
    EVP_PKEY_CTX* ec_ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_EC, nullptr);
    EVP_PKEY_keygen_init(ec_ctx);
    EVP_PKEY_CTX_set_ec_paramgen_curve_nid(ec_ctx, NID_X9_62_prime256v1);
    EVP_PKEY* ec_key = nullptr;
    EVP_PKEY_keygen(ec_ctx, &ec_key);
    
    auto ec_keygen_time = std::chrono::high_resolution_clock::now();
    
    // ECDSA Signing
    EVP_MD_CTX* ec_md_ctx = EVP_MD_CTX_new();
    EVP_DigestSignInit(ec_md_ctx, nullptr, EVP_sha256(), nullptr, ec_key);
    
    // Store signatures for verification
    std::vector<std::vector<unsigned char>> ec_signatures(iterations);
    
    auto ec_sign_start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; i++) {
        EVP_DigestSignInit(ec_md_ctx, nullptr, EVP_sha256(), nullptr, ec_key);
        EVP_DigestSignUpdate(ec_md_ctx, data, 32);
        
        size_t sig_len = 0;
        EVP_DigestSignFinal(ec_md_ctx, nullptr, &sig_len);
        ec_signatures[i].resize(sig_len);
        EVP_DigestSignFinal(ec_md_ctx, ec_signatures[i].data(), &sig_len);
    }
    auto ec_sign_end = std::chrono::high_resolution_clock::now();
    
    // RSA-PSS Verification
    EVP_MD_CTX* rsa_verify_ctx = EVP_MD_CTX_new();
    auto rsa_verify_start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; i++) {
        EVP_PKEY_CTX* rsa_verify_pkey_ctx = nullptr;
        EVP_DigestVerifyInit(rsa_verify_ctx, &rsa_verify_pkey_ctx, EVP_sha256(), nullptr, rsa_key);
        
        // Configure RSA-PSS parameters for verification
        EVP_PKEY_CTX_set_rsa_padding(rsa_verify_pkey_ctx, RSA_PKCS1_PSS_PADDING);
        EVP_PKEY_CTX_set_rsa_pss_saltlen(rsa_verify_pkey_ctx, RSA_PSS_SALTLEN_DIGEST);
        EVP_PKEY_CTX_set_rsa_mgf1_md(rsa_verify_pkey_ctx, EVP_sha256());
        
        EVP_DigestVerifyUpdate(rsa_verify_ctx, data, 32);
        EVP_DigestVerifyFinal(rsa_verify_ctx, rsa_signatures[i].data(), rsa_signatures[i].size());
    }
    auto rsa_verify_end = std::chrono::high_resolution_clock::now();
    
    // ECDSA Verification
    EVP_MD_CTX* ec_verify_ctx = EVP_MD_CTX_new();
    auto ec_verify_start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; i++) {
        EVP_DigestVerifyInit(ec_verify_ctx, nullptr, EVP_sha256(), nullptr, ec_key);
        EVP_DigestVerifyUpdate(ec_verify_ctx, data, 32);
        EVP_DigestVerifyFinal(ec_verify_ctx, ec_signatures[i].data(), ec_signatures[i].size());
    }
    auto ec_verify_end = std::chrono::high_resolution_clock::now();
    
    // Calculate timings
    auto rsa_keygen_ms = std::chrono::duration_cast<std::chrono::milliseconds>(rsa_keygen_time - start).count();
    auto rsa_sign_ms = std::chrono::duration_cast<std::chrono::microseconds>(rsa_sign_end - rsa_sign_start).count();
    auto rsa_verify_ms = std::chrono::duration_cast<std::chrono::microseconds>(rsa_verify_end - rsa_verify_start).count();
    
    auto ec_keygen_ms = std::chrono::duration_cast<std::chrono::microseconds>(ec_keygen_time - ec_start).count();
    auto ec_sign_ms = std::chrono::duration_cast<std::chrono::microseconds>(ec_sign_end - ec_sign_start).count();
    auto ec_verify_ms = std::chrono::duration_cast<std::chrono::microseconds>(ec_verify_end - ec_verify_start).count();
    
    std::cout << "Key Generation Performance:" << std::endl;
    std::cout << "  RSA-3072:  " << rsa_keygen_ms << " ms" << std::endl;
    std::cout << "  EC P-256:  " << ec_keygen_ms / 1000.0 << " ms" << std::endl;
    std::cout << "  Speed Ratio: " << (double)rsa_keygen_ms / (ec_keygen_ms / 1000.0) << "x faster" << std::endl;
    std::cout << std::endl;
    
    std::cout << "Signing Performance (" << iterations << " signatures):" << std::endl;
    std::cout << "  RSA-PSS-3072: " << rsa_sign_ms << " μs total (" << rsa_sign_ms / iterations << " μs/sig)" << std::endl;
    std::cout << "  ECDSA-256:    " << ec_sign_ms << " μs total (" << ec_sign_ms / iterations << " μs/sig)" << std::endl;
    std::cout << "  Speed Ratio: " << (double)rsa_sign_ms / ec_sign_ms << "x faster" << std::endl;
    std::cout << std::endl;
    
    std::cout << "Verification Performance (" << iterations << " verifications):" << std::endl;
    std::cout << "  RSA-PSS-3072: " << rsa_verify_ms << " μs total (" << rsa_verify_ms / iterations << " μs/verify)" << std::endl;
    std::cout << "  ECDSA-256:    " << ec_verify_ms << " μs total (" << ec_verify_ms / iterations << " μs/verify)" << std::endl;
    std::cout << "  Speed Ratio: " << (double)rsa_verify_ms / ec_verify_ms << "x faster" << std::endl;
    std::cout << std::endl;
    
    std::cout << "Algorithm Details:" << std::endl;
    std::cout << "  RSA-PSS: PKCS#1 v2.1 with SHA-256, MGF1-SHA256, salt length = digest length" << std::endl;
    std::cout << "  ECDSA: P-256 curve with SHA-256 hash" << std::endl;
    std::cout << std::endl;
    
    std::cout << "Mathematical Complexity Analysis:" << std::endl;
    std::cout << "  RSA-PSS-3072: O(log³ n) with n = 3072 bits" << std::endl;
    std::cout << "  ECDSA-256: O(log n) with n = 256 bits" << std::endl;
    std::cout << "  Theoretical ratio: ~" << (int)pow(3072.0/256.0, 2) << "x difference" << std::endl;
    std::cout << std::endl;
    
    std::cout << "Signature Verification Complexity:" << std::endl;
    std::cout << "  RSA-PSS Verification: O(log e × log n) where e=65537 (public exponent)" << std::endl;
    std::cout << "    - Uses small public exponent vs large private exponent" << std::endl;
    std::cout << "    - Fast modular exponentiation: signature^65537 mod n" << std::endl;
    std::cout << "  RSA-PSS Signing: O(log d × log n) where d is large private exponent" << std::endl;
    std::cout << "    - Private exponent d ≈ 3072 bits, much larger than e" << std::endl;
    std::cout << "    - Slow modular exponentiation: message^d mod n" << std::endl;
    std::cout << "  ECDSA Verification: O(log n) - similar to signing complexity" << std::endl;
    std::cout << "    - Point multiplication: sG + hP (two scalar multiplications)" << std::endl;
    std::cout << "    - Verification complexity similar to signing" << std::endl;
    std::cout << "  ECDSA Signing: O(log n) - scalar multiplication kG" << std::endl;
    std::cout << std::endl;
    
    std::cout << "Performance Ratio Analysis:" << std::endl;
    double rsa_sign_verify_ratio = (double)rsa_sign_ms / rsa_verify_ms;
    double ec_sign_verify_ratio = (double)ec_sign_ms / ec_verify_ms;
    std::cout << "  RSA Sign/Verify ratio: " << rsa_sign_verify_ratio << "x (asymmetric)" << std::endl;
    std::cout << "  ECDSA Sign/Verify ratio: " << ec_sign_verify_ratio << "x (symmetric)" << std::endl;
    std::cout << "  Explanation: RSA uses small public exponent (65537) vs large private key" << std::endl;
    std::cout << "               ECDSA operations have similar computational complexity" << std::endl;
    
    // Cleanup
    EVP_PKEY_free(rsa_key);
    EVP_PKEY_free(ec_key);
    EVP_PKEY_CTX_free(rsa_ctx);
    EVP_PKEY_CTX_free(ec_ctx);
    EVP_MD_CTX_free(rsa_md_ctx);
    EVP_MD_CTX_free(ec_md_ctx);
    EVP_MD_CTX_free(rsa_verify_ctx);
    EVP_MD_CTX_free(ec_verify_ctx);
}

int main() {
    ERR_load_crypto_strings();
    
    print_system_info();
    benchmark_rsa_vs_ecdsa();
    
    ERR_free_strings();
    return 0;
}
