# RSA and EC Key Generators

High-performance multithreaded C++ applications that use OpenSSL to generate RSA and Elliptic Curve (EC) keypairs with real-time performance statistics.

## Applications

### RSA Key Generator (`rsa_generator`)
- Supports RSA key sizes: 512-8192 bits
- Recommended sizes: 1024, 2048, 4096 bits
- Typical performance: 5-20 keys/second for 2048-bit keys

### EC Key Generator (`ec_generator`) 
- Supports NIST curves: P-256, P-384, P-521
- Extremely fast: 1000-10000+ keys/second
- Ideal for high-throughput applications

## Performance Comparison

| Key Type | Security Level | Generation Time | Throughput |
|----------|---------------|----------------|------------|
| RSA-2048 | ~112 bits     | ~200ms        | ~5 keys/s  |
| RSA-4096 | ~140 bits     | ~1500ms       | ~0.7 keys/s|
| EC P-256 | ~128 bits     | ~0.03ms       | ~4000 keys/s|
| EC P-384 | ~192 bits     | ~1.3ms        | ~800 keys/s |
| EC P-521 | ~256 bits     | ~0.3ms        | ~3000 keys/s|

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

## Usage

### RSA Generator
```bash
./rsa_generator <keysize> <num_threads> <num_loops>
```

### EC Generator  
```bash
./ec_generator <curve> <num_threads> <num_loops>
```

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

**List EC curves:**
```bash
./ec_generator --curves
```

**Quick tests:**
```bash
make test        # Test both generators
make test-ec     # Test EC generator with all curves
```

## Output

The application displays real-time statistics in a single line format:
```
Keys:    156, Throughput:   3.25 keys/s, Avg:  285.42ms, Min:  245.33ms, Max:  342.18ms
```

Final statistics are displayed when all threads complete.

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
