#!/bin/bash

# Key Generator and Signer Test Suite
# This script runs various test configurations for RSA generators, EC generators, and ECDSA signers

echo "Cryptographic Performance Test Suite"
echo "===================================="
echo

# Function to check if executable exists
check_executable() {
    if [ ! -f "./$1" ]; then
        echo "Error: $1 executable not found. Please run 'make' first."
        return 1
    fi
    return 0
}

# RSA Generator Tests
echo "RSA Key Generator Tests"
echo "======================="
echo

if check_executable "rsa_generator"; then
    # Test 1: Quick test with 1024-bit keys
    echo "Test 1: Quick RSA test with 1024-bit keys (2 threads, 5 keys each)"
    echo "-------------------------------------------------------------------"
    ./rsa_generator 1024 2 5
    echo
    echo

    # Test 2: Medium test with 2048-bit keys
    echo "Test 2: Medium RSA test with 2048-bit keys (4 threads, 5 keys each)"
    echo "--------------------------------------------------------------------"
    ./rsa_generator 2048 4 5
    echo
    echo

    # Test 3: High security test with 4096-bit keys
    echo "Test 3: High security RSA test with 4096-bit keys (2 threads, 3 keys each)"
    echo "---------------------------------------------------------------------------"
    ./rsa_generator 4096 2 3
    echo
    echo
else
    echo "Skipping RSA tests - executable not found"
    echo
fi

# EC Generator Tests
echo "EC Key Generator Tests"
echo "====================="
echo

if check_executable "ec_generator"; then
    # Test 4: P-256 curve test
    echo "Test 4: EC P-256 curve test (4 threads, 20 keys each)"
    echo "-----------------------------------------------------"
    ./ec_generator P256 4 20
    echo
    echo

    # Test 5: P-384 curve test
    echo "Test 5: EC P-384 curve test (4 threads, 15 keys each)"
    echo "-----------------------------------------------------"
    ./ec_generator P384 4 15
    echo
    echo

    # Test 6: P-521 curve test
    echo "Test 6: EC P-521 curve test (4 threads, 10 keys each)"
    echo "-----------------------------------------------------"
    ./ec_generator P521 4 10
    echo
    echo
else
    echo "Skipping EC key generation tests - executable not found"
    echo
fi

# ECDSA Signer Tests
echo "ECDSA Signing Tests"
echo "=================="
echo

if check_executable "ecdsa_signer"; then
    # Test 7: P-256 signing test
    echo "Test 7: ECDSA P-256 signing test (4 threads, 100 signatures each)"
    echo "-----------------------------------------------------------------"
    ./ecdsa_signer P256 4 100
    echo
    echo

    # Test 8: P-384 signing test
    echo "Test 8: ECDSA P-384 signing test (4 threads, 75 signatures each)"
    echo "----------------------------------------------------------------"
    ./ecdsa_signer P384 4 75
    echo
    echo

    # Test 9: P-521 signing test
    echo "Test 9: ECDSA P-521 signing test (4 threads, 50 signatures each)"
    echo "----------------------------------------------------------------"
    ./ecdsa_signer P521 4 50
    echo
    echo

    # Test 10: High-throughput signing test
    echo "Test 10: High-throughput ECDSA P-256 signing (8 threads, 250 signatures each)"
    echo "------------------------------------------------------------------------------"
    ./ecdsa_signer P256 8 250
    echo
    echo
else
    echo "Skipping ECDSA signing tests - executable not found"
    echo
fi

echo "All tests completed!"
echo
echo "Performance Summary:"
echo "==================="
echo "RSA Key Generation:   ~5-20 keys/second (depends on key size)"
echo "EC Key Generation:    ~1000-30000 keys/second (extremely fast)"
echo "ECDSA Signing:        ~400-8000 signatures/second (very fast)"
echo
echo "To run custom tests, use:"
echo "  ./rsa_generator <keysize> <num_threads> <num_loops>"
echo "  ./ec_generator <curve> <num_threads> <num_loops>"
echo "  ./ecdsa_signer <curve> <num_threads> <num_loops>"
echo
echo "Examples:"
echo "  ./rsa_generator 2048 4 25    # 100 x RSA 2048-bit keys, 4 threads"
echo "  ./rsa_generator 4096 2 10    # 20 x RSA 4096-bit keys, 2 threads"
echo "  ./ec_generator P256 8 500    # 4000 x EC P-256 keys, 8 threads"
echo "  ./ec_generator P384 4 250    # 1000 x EC P-384 keys, 4 threads"
echo "  ./ec_generator P521 2 150    # 300 x EC P-521 keys, 2 threads"
echo "  ./ecdsa_signer P256 8 1000   # 8000 x P-256 signatures, 8 threads"
echo "  ./ecdsa_signer P384 4 500    # 2000 x P-384 signatures, 4 threads"
echo "  ./ecdsa_signer P521 4 300    # 1200 x P-521 signatures, 4 threads"
echo
echo "To see supported curves:"
echo "  ./ec_generator --curves"
echo "  ./ecdsa_signer --curves"
