# EC Key Generator

A high-performance multithreaded C++ application that uses OpenSSL to generate Elliptic Curve (EC) keypairs with real-time performance statistics.

## Supported Curves

- **P-256** (NIST P-256, secp256r1, prime256v1) - 256-bit curve
- **P-384** (NIST P-384, secp384r1) - 384-bit curve  
- **P-521** (NIST P-521, secp521r1) - 521-bit curve

## Features

- Multithreaded EC key generation using OpenSSL
- Support for NIST standard curves (P-256, P-384, P-521)
- Real-time statistics display showing:
  - Total keys generated
  - Throughput (keys per second)
  - Average time per key pair
  - Minimum time per key pair
  - Maximum time per key pair
- Configurable curve type, thread count, and loop count
- Thread-safe statistics collection
- Performance optimizations:
  - Context reuse per thread (avoids repeated context creation/destruction)
  - Pre-allocated variables to reduce allocation overhead
  - Optimized timing measurements using steady_clock
  - Bounds checking to prevent invalid timing values

## Performance Characteristics

EC key generation is significantly faster than RSA:

| Curve | Key Generation Time | Relative Performance |
|-------|-------------------|---------------------|
| P-256 | ~0.03ms avg      | ~6000x faster than RSA-2048 |
| P-384 | ~1.3ms avg       | ~150x faster than RSA-2048 |
| P-521 | ~0.3ms avg       | ~600x faster than RSA-2048 |

*Performance will vary based on hardware and system load*

## Building

```bash
make ec_generator
# or build both generators
make all
```

## Usage

```bash
./ec_generator <curve> <num_threads> <num_loops>
```

### Parameters

- `curve`: EC curve name (P256, P384, P521) - case insensitive
- `num_threads`: Number of worker threads (1-100)
- `num_loops`: Number of key pairs to generate per thread

### Examples

Generate 1000 P-256 keys using 8 threads (125 keys per thread):
```bash
./ec_generator P256 8 125
```

Generate 400 P-384 keys using 4 threads (100 keys per thread):
```bash
./ec_generator P384 4 100
```

Generate 100 P-521 keys using 2 threads (50 keys per thread):
```bash
./ec_generator P521 2 50
```

### List Supported Curves

```bash
./ec_generator --curves
```

## Performance Testing

Run comprehensive tests across all curves:
```bash
make test-ec
```

## Comparison with RSA

EC provides several advantages over RSA:

1. **Performance**: EC key generation is 100-6000x faster than RSA
2. **Key Size**: Smaller keys for equivalent security (P-256 ≈ RSA-3072)
3. **Bandwidth**: Smaller signatures and certificates
4. **Battery Life**: Lower computational requirements for mobile devices

| Security Level | RSA Key Size | EC Curve | Performance Ratio |
|---------------|-------------|----------|------------------|
| 128-bit       | 3072 bits   | P-256    | ~6000x faster    |
| 192-bit       | 7680 bits   | P-384    | ~5000x faster    |
| 256-bit       | 15360 bits  | P-521    | ~4000x faster    |

## Output Example

```
Starting EC key generation with:
Curve: P256
Threads: 8
Loops per thread: 500
Total keys to generate: 4000

Keys:   4000, Throughput:  4000.00 keys/s, Avg:   0.03ms, Min:   0.02ms, Max:   3.09ms

Final Statistics:
Keys:   4000, Throughput:  4000.00 keys/s, Avg:   0.03ms, Min:   0.02ms, Max:   3.09ms
```

## Security Notes

All curves are NIST-approved standards:
- P-256: Widely supported, excellent performance, suitable for most applications
- P-384: Higher security margin, good performance, recommended for sensitive applications
- P-521: Highest security level, moderate performance, for maximum security requirements
