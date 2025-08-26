# RSA and EC Key Generators with Cryptographic Benchmarking

High-performance multithreaded C++ applications that use OpenSSL to generate RSA and Elliptic Curve (EC) keypairs with real-time performance statistics, plus comprehensive cryptographic performance analysis.

Disclaimer: This whole project is generated using copilot. Dont blame me for any errors in text, sourcecode or in the test resulst.

## Applications

### RSA Key Generator (`rsa_generator`)
- Supports RSA key sizes: 512-8192 bits
- Recommended sizes: 1024, 2048, 4096 bits
- Typical performance: 5-20 keys/second for 2048-bit keys

### EC Key Generator (`ec_generator`) 
- Supports NIST curves: P-256, P-384, P-521
- Extremely fast: 1000-10000+ keys/second
- Ideal for high-throughput applications

### Cryptographic Benchmark (`crypto_benchmark`)
- **Performance comparison**: RSA-PSS vs ECDSA algorithms
- **Complete analysis**: Key generation, signing, and verification
- **Hardware profiling**: System information and CPU crypto features
- **Mathematical complexity**: Detailed algorithmic analysis
- **Security equivalence**: RSA-3072 vs ECDSA-256 (both ~128-bit security)

## Performance Comparison

| Key Type | Security Level | Generation Time | Throughput |
|----------|---------------|----------------|------------|
| RSA-2048 | ~112 bits     | ~200ms        | ~5 keys/s  |
| RSA-4096 | ~140 bits     | ~1500ms       | ~0.7 keys/s|
| EC P-256 | ~128 bits     | ~0.03ms       | ~4000 keys/s|
| EC P-384 | ~192 bits     | ~1.3ms        | ~800 keys/s |
| EC P-521 | ~256 bits     | ~0.3ms        | ~3000 keys/s|

## Cryptographic Performance Analysis

The `crypto_benchmark` provides detailed performance comparison between RSA-PSS and ECDSA:

| Operation | RSA-PSS-3072 | ECDSA-256 | Speed Advantage |
|-----------|---------------|-----------|-----------------|
| Key Generation | ~1000ms | ~10ms | **ECDSA 100x faster** |
| Signing | ~2800μs | ~40μs | **ECDSA 70x faster** |
| Verification | ~60μs | ~35μs | **ECDSA 1.7x faster** |

### Key Insights
- **ECDSA dominates** in key generation and signing performance
- **RSA verification** is much faster than RSA signing (45x difference)
- **ECDSA operations** have balanced signing/verification performance
- **Hardware acceleration** (AES-NI, SHA-NI, AVX2) significantly improves performance

## Algorithm Details

### RSA-PSS (Probabilistic Signature Scheme)
- **Standard**: PKCS#1 v2.1 with SHA-256
- **Mask Generation Function**: MGF1 with SHA-256
- **Salt Length**: Equal to digest length (32 bytes)
- **Security**: Provably secure, modern RSA signature scheme
- **Key Size**: 3072 bits (equivalent to ~128-bit security)

### ECDSA (Elliptic Curve Digital Signature Algorithm)
- **Curve**: P-256 (secp256r1 / prime256v1)
- **Hash Function**: SHA-256
- **Security Level**: ~128 bits
- **Key Size**: 256 bits (much smaller than equivalent RSA)
- **Performance**: Balanced signing/verification operations

## Features

- Multithreaded RSA key generation using OpenSSL
- Real-time statistics display showing:
  - Total keys generated
  - Throughput (keys per second)
  - Average time per key pair
  - Minimum time per key pair
  - Maximum time per key pair
- Configurable key size, thread count, and loop count
- Thread-safe statistics collection
- Performance optimizations:
  - Context reuse per thread (avoids repeated context creation/destruction)
  - Pre-allocated variables to reduce allocation overhead
  - Optimized timing measurements using steady_clock
  - Bounds checking to prevent invalid timing values

## Requirements

- C++11 compatible compiler (g++)
- OpenSSL development libraries
- pthread support

## Installation

### Ubuntu/Debian
```bash
sudo apt-get update
sudo apt-get install build-essential libssl-dev
```

### CentOS/RHEL/Fedora
```bash
sudo yum install gcc-c++ openssl-devel
# or for newer versions:
sudo dnf install gcc-c++ openssl-devel
```

## Building

```bash
make
```

Or to install dependencies and build:
```bash
make install-deps
make
```

### Project Structure
```
openssltest/
├── src/cpp/              # C++ source files
│   ├── rsa_generator.cpp
│   ├── ec_generator.cpp  
│   ├── ecdsa_signer.cpp
│   ├── crypto_benchmark.cpp
│   └── verify_ec_keys.cpp
├── obj/                  # Object files (auto-created)
├── Makefile             # Build configuration
├── README.md            # This file
└── [executables]        # Built applications
```

## Usage

### RSA Generator
```bash
./rsa_generator <keysize> <num_threads> <num_loops>
```

### EC Generator  
```bash
./ec_generator <curve> <num_threads> <num_loops>
```

### Cryptographic Benchmark
```bash
./crypto_benchmark
```
No parameters required - runs automatic performance comparison between RSA-PSS-3072 and ECDSA-256.

### Parameters

**RSA Generator:**
- `keysize`: RSA key size in bits (512-8192, recommended: 1024, 2048, 4096)
- `num_threads`: Number of worker threads (1-100)
- `num_loops`: Number of key pairs to generate per thread

**EC Generator:**
- `curve`: EC curve name (P256, P384, P521) - case insensitive  
- `num_threads`: Number of worker threads (1-100)
- `num_loops`: Number of key pairs to generate per thread

### Examples

**RSA Examples:**
```bash
./rsa_generator 2048 4 25   # 100 x 2048-bit RSA keys, 4 threads
./rsa_generator 4096 2 10   # 20 x 4096-bit RSA keys, 2 threads
```

**EC Examples:**
```bash
./ec_generator P256 8 125   # 1000 x P-256 keys, 8 threads  
./ec_generator P384 4 100   # 400 x P-384 keys, 4 threads
./ec_generator P521 2 50    # 100 x P-521 keys, 2 threads
```

**Cryptographic Benchmark:**
```bash
./crypto_benchmark          # Complete RSA-PSS vs ECDSA performance analysis
```

**List EC curves:**
```bash
./ec_generator --curves
```

**Quick tests:**
```bash
make test        # Test all applications including benchmark
make test-ec     # Test EC generator with all curves
```

## Output

### Key Generators
The applications display real-time statistics in a single line format:
```
Keys:    156, Throughput:   3.25 keys/s, Avg:  285.42ms, Min:  245.33ms, Max:  342.18ms
```

Final statistics are displayed when all threads complete.

### Cryptographic Benchmark
The benchmark provides comprehensive performance analysis including:

**System Information:**
```
OS: Linux 5.15.153.1-microsoft-standard-WSL2
Architecture: x86_64
CPU: 13th Gen Intel(R) Core(TM) i9-13900H
CPU Cores: 20
Crypto CPU Features: pclmulqdq, sse4_1, sse4_2, aes, avx, rdrand, avx2, rdseed, sha_ni
OpenSSL Version: OpenSSL 3.0.13 30 Jan 2024
```

**Performance Results:**
```
Key Generation Performance:
  RSA-3072:  1062 ms
  EC P-256:  9.849 ms
  Speed Ratio: 107.828x faster

Signing Performance (100 signatures):
  RSA-PSS-3072: 279224 μs total (2792 μs/sig)
  ECDSA-256:    3822 μs total (38 μs/sig)
  Speed Ratio: 73.057x faster

Verification Performance (100 verifications):
  RSA-PSS-3072: 5925 μs total (59 μs/verify)
  ECDSA-256:    2983 μs total (29 μs/verify)
  Speed Ratio: 1.98626x faster
```

**Mathematical Complexity Analysis:**
- Detailed algorithmic complexity for each operation
- Explanation of RSA asymmetric performance (fast verify, slow sign)
- ECDSA symmetric performance characteristics
- Performance ratio analysis with mathematical foundations

## Performance Notes

- Larger key sizes (4096+ bits) take significantly longer to generate
- Performance scales well with multiple CPU cores
- Memory usage is minimal as keys are generated and immediately discarded
- OpenSSL's random number generator is thread-safe

## Cleaning Up

```bash
make clean
```

## Troubleshooting

### OpenSSL not found
Make sure OpenSSL development libraries are installed:
```bash
# Ubuntu/Debian
sudo apt-get install libssl-dev

# CentOS/RHEL/Fedora
sudo yum install openssl-devel
```

### Compilation errors
Ensure you have a C++11 compatible compiler:
```bash
g++ --version
```

### Runtime errors
Check that OpenSSL libraries are available:
```bash
ldd ./rsa_generator
```
