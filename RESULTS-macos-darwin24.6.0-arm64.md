# OpenSSL Benchmarks – macOS (Darwin 24.6.0, arm64)

Date: 2025-08-27

## Environment
- OS: Darwin 24.6.0
- Architecture: arm64
- CPU: Unknown
- CPU Cores: 10
- Crypto CPU Features: unavailable
- OpenSSL Version: OpenSSL 3.5.2 (5 Aug 2025)

## Notes
- Executables built with Homebrew OpenSSL@3 and rpath set to its lib directory.
- Benchmark binary supports CLI flags: `--iter N`, `--rsa BITS`, `--curve P256|P384|P521`.

## Results

### 1) 200 iterations — RSA-PSS(2048) vs ECDSA P-256
- Key generation:
  - RSA-2048: 52 ms
  - EC P-256: 0.065 ms
  - Ratio (RSA/EC): 800x
- Signing (total, per signature):
  - RSA-PSS-2048: 116865 μs (584 μs/sig)
  - ECDSA-P-256: 4173 μs (20 μs/sig)
  - Ratio (RSA/EC): 28.005x
- Verification (total, per verify):
  - RSA-PSS-2048: 3747 μs (18 μs/verify)
  - ECDSA-P-256: 3105 μs (15 μs/verify)
  - Ratio (RSA/EC): 1.20676x
- Sign/Verify ratios:
  - RSA: 31.189x
  - ECDSA: 1.34396x

### 2) 200 iterations — RSA-PSS(3072) vs ECDSA P-384
- Key generation:
  - RSA-3072: 110 ms
  - EC P-384: 0.151 ms
  - Ratio (RSA/EC): 728.477x
- Signing (total, per signature):
  - RSA-PSS-3072: 334705 μs (1673 μs/sig)
  - ECDSA-P-384: 29948 μs (149 μs/sig)
  - Ratio (RSA/EC): 11.1762x
- Verification (total, per verify):
  - RSA-PSS-3072: 7122 μs (35 μs/verify)
  - ECDSA-P-384: 18668 μs (93 μs/verify)
  - Ratio (RSA/EC): 0.381508x
- Sign/Verify ratios:
  - RSA: 46.9959x
  - ECDSA: 1.60424x

### 3) 100 iterations — RSA-PSS(4096) vs ECDSA P-521
- Key generation:
  - RSA-4096: 174 ms
  - EC P-521: 0.161 ms
  - Ratio (RSA/EC): 1080.75x
- Signing (total, per signature):
  - RSA-PSS-4096: 367670 μs (3676 μs/sig)
  - ECDSA-P-521: 22147 μs (221 μs/sig)
  - Ratio (RSA/EC): 16.6013x
- Verification (total, per verify):
  - RSA-PSS-4096: 5702 μs (57 μs/verify)
  - ECDSA-P-521: 24310 μs (243 μs/verify)
  - Ratio (RSA/EC): 0.234554x
- Sign/Verify ratios:
  - RSA: 64.4809x
  - ECDSA: 0.911024x

## Commands used
```bash
# 200 iterations, RSA-2048 vs P-256
./crypto_benchmark --iter 200 --rsa 2048 --curve P256

# 200 iterations, RSA-3072 vs P-384
./crypto_benchmark --iter 200 --rsa 3072 --curve P384

# 100 iterations, RSA-4096 vs P-521
./crypto_benchmark --iter 100 --rsa 4096 --curve P521
```
