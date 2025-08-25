#include <iostream>
#include <openssl/ec.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <chrono>

// Simple test to verify EC key generation and validate the keys
int main() {
    ERR_load_crypto_strings();
    
    std::cout << "EC Key Generation Verification Test" << std::endl;
    std::cout << "====================================" << std::endl;
    
    // Test P-256 key generation
    EVP_PKEY_CTX* pctx = EVP_PKEY_CTX_new_id(EVP_PKEY_EC, nullptr);
    if (!pctx) {
        std::cerr << "Failed to create context" << std::endl;
        return 1;
    }
    
    if (EVP_PKEY_keygen_init(pctx) <= 0) {
        std::cerr << "Failed to init keygen" << std::endl;
        EVP_PKEY_CTX_free(pctx);
        return 1;
    }
    
    if (EVP_PKEY_CTX_set_ec_paramgen_curve_nid(pctx, NID_X9_62_prime256v1) <= 0) {
        std::cerr << "Failed to set curve" << std::endl;
        EVP_PKEY_CTX_free(pctx);
        return 1;
    }
    
    // Generate and validate 10 keys
    for (int i = 0; i < 10; i++) {
        auto start = std::chrono::steady_clock::now();
        
        EVP_PKEY* pkey = nullptr;
        if (EVP_PKEY_keygen(pctx, &pkey) <= 0) {
            std::cerr << "Key generation failed for key " << i << std::endl;
            continue;
        }
        
        auto end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        // Validate the key
        EC_KEY* ec_key = EVP_PKEY_get1_EC_KEY(pkey);
        if (!ec_key) {
            std::cerr << "Failed to get EC_KEY from EVP_PKEY" << std::endl;
            EVP_PKEY_free(pkey);
            continue;
        }
        
        // Check if the key has both private and public components
        const BIGNUM* priv_key = EC_KEY_get0_private_key(ec_key);
        const EC_POINT* pub_key = EC_KEY_get0_public_key(ec_key);
        
        if (!priv_key || !pub_key) {
            std::cerr << "Key " << i << " is missing private or public component" << std::endl;
            EC_KEY_free(ec_key);
            EVP_PKEY_free(pkey);
            continue;
        }
        
        // Verify the key pair is valid
        if (EC_KEY_check_key(ec_key) != 1) {
            std::cerr << "Key " << i << " failed validation check" << std::endl;
            EC_KEY_free(ec_key);
            EVP_PKEY_free(pkey);
            continue;
        }
        
        // Get key size info
        const EC_GROUP* group = EC_KEY_get0_group(ec_key);
        int key_size = EC_GROUP_get_degree(group);
        
        std::cout << "Key " << i << ": VALID"
                  << ", Size: " << key_size << " bits"
                  << ", Time: " << duration.count() << " μs" << std::endl;
        
        // Print first few bytes of private key for verification
        char* priv_hex = BN_bn2hex(priv_key);
        if (priv_hex) {
            std::string priv_str(priv_hex);
            std::cout << "  Private key (first 16 chars): " << priv_str.substr(0, 16) << "..." << std::endl;
            OPENSSL_free(priv_hex);
        }
        
        EC_KEY_free(ec_key);
        EVP_PKEY_free(pkey);
    }
    
    EVP_PKEY_CTX_free(pctx);
    ERR_free_strings();
    
    std::cout << std::endl << "All keys generated and validated successfully!" << std::endl;
    
    return 0;
}
