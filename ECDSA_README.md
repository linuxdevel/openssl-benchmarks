# ECDSA Signer Performance Tool

A high-performance multithreaded C++ application that benchmarks ECDSA (Elliptic Curve Digital Signature Algorithm) signing performance using OpenSSL with real-time statistics.

## Overview

This tool creates one EC key pair per thread and then performs repeated ECDSA signing operations on random 32-byte data buffers. It's designed to measure the performance characteristics of ECDSA signing operations across different curves and thread counts.

## Features

- **Multithreaded ECDSA signing** using OpenSSL
- **Support for NIST standard curves**: P-256, P-384, P-521
- **Key reuse per thread**: Each thread creates one EC key and reuses it for all signatures
- **Random data generation**: Each signature signs a unique 32-byte random buffer
- **SHA-256 hashing**: Uses SHA-256 as the message digest algorithm
- **Real-time statistics display** showing:
  - Total signatures generated
  - Throughput (signatures per second)
  - Average time per signature
  - Minimum time per signature
  - Maximum time per signature
- **Thread-safe statistics collection**
- **Performance optimizations**:
  - Context reuse per thread
  - Pre-allocated variables
  - Optimized timing measurements
  - Bounds checking for reliable statistics

## Architecture

```
Per Thread:
1. Create EC Key Pair (once)
2. Initialize MD Context for SHA-256 + ECDSA
3. Loop:
   a. Generate 32 bytes random data
   b. Sign data with ECDSA
   c. Record timing statistics
   d. Free signature buffer
4. Cleanup EC Key and Context
```

## Performance Characteristics

ECDSA signing performance by curve:

| Curve | Signature Time | Throughput | Security Level |
|-------|---------------|------------|----------------|
| P-256 | ~0.05ms avg   | ~8000 sigs/s | 128-bit |
| P-384 | ~1.1ms avg    | ~800 sigs/s  | 192-bit |
| P-521 | ~0.4ms avg    | ~2500 sigs/s | 256-bit |

*Performance will vary based on hardware and system load*

## Building

```bash
make ecdsa_signer
# or build all tools
make all
```

## Usage

```bash
./ecdsa_signer <curve> <num_threads> <num_loops>
```

### Parameters

- `curve`: EC curve name (P256, P384, P521) - case insensitive
- `num_threads`: Number of worker threads (1-100)
- `num_loops`: Number of signatures to generate per thread

### Examples

Generate 4000 P-256 signatures using 8 threads (500 signatures per thread):
```bash
./ecdsa_signer P256 8 500
```

Generate 2000 P-384 signatures using 4 threads (500 signatures per thread):
```bash
./ecdsa_signer P384 4 500
```

Generate 1000 P-521 signatures using 4 threads (250 signatures per thread):
```bash
./ecdsa_signer P521 4 250
```

### List Supported Curves

```bash
./ecdsa_signer --curves
```

## Performance Testing

Run comprehensive ECDSA signing tests across all curves:
```bash
make test-ecdsa
```

## Output Example

```
Starting EC-DSA signing performance test with:
Curve: P256
Threads: 8
Loops per thread: 1000
Total signatures to generate: 8000
Data size: 32 bytes (random data per signature)
Hash algorithm: SHA-256

Sigs:   8000, Throughput:  8000.00 sigs/s, Avg:   0.05ms, Min:   0.03ms, Max:   1.04ms

Final Statistics:
Sigs:   8000, Throughput:  8000.00 sigs/s, Avg:   0.05ms, Min:   0.03ms, Max:   1.04ms
```

## Technical Details

### Cryptographic Setup
- **Algorithm**: ECDSA (Elliptic Curve Digital Signature Algorithm)
- **Hash Function**: SHA-256
- **Data Size**: 32 bytes per signature (randomized)
- **Key Reuse**: Each thread creates one key pair and reuses it for all signatures

### Thread Safety
- Each thread operates on its own EC key and signing context
- Statistics collection is protected by mutex
- No shared cryptographic state between threads

### Signature Process
1. Generate 32 bytes of random data
2. Initialize ECDSA signing context with SHA-256
3. Update context with data to be signed
4. Generate ECDSA signature
5. Measure and record timing
6. Free signature buffer

### Memory Management
- EC keys are created once per thread and reused
- Signature buffers are allocated/freed for each signature
- All OpenSSL resources properly cleaned up

## Comparison with RSA Signing

ECDSA offers significant advantages over RSA for signing:

| Metric | RSA-2048 | EC P-256 | Advantage |
|--------|----------|----------|-----------|
| **Signature Time** | ~5-10ms | ~0.05ms | ~100-200x faster |
| **Signature Size** | 256 bytes | ~64 bytes | ~4x smaller |
| **Key Size** | 2048 bits | 256 bits | ~8x smaller |
| **Security Level** | ~112 bits | ~128 bits | Higher security |

## Use Cases

This tool is valuable for:
- **Performance benchmarking** of ECDSA implementations
- **Capacity planning** for high-throughput signing services
- **Hardware evaluation** for cryptographic workloads
- **Comparing curve performance** for optimal selection
- **Testing threading scalability** for signing operations

## Security Notes

- All curves are NIST-approved standards
- P-256: Widely supported, excellent performance, suitable for most applications
- P-384: Higher security margin, good performance, recommended for sensitive applications  
- P-521: Highest security level, moderate performance, for maximum security requirements
- SHA-256 provides 128-bit security level (matches P-256)
- Keys are properly validated by OpenSSL during creation
