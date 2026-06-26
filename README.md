# 🐺 Fenrir Hash Cracker v1.0.0

**GPU-accelerated password hash auditing tool** — C++17 · OpenCL · AVX2/AVX-512 · CLI

Fenrir is a high-performance password strength auditing tool that leverages GPU computing (via OpenCL) and CPU SIMD (AVX2/AVX-512) to crack or verify password hashes. It supports dictionary attacks, rule-based mutations, brute-force/mask attacks, hybrid combinations, and online leak-database API lookups.

---

## ⚠️ Responsible Use Disclaimer

**This tool is intended solely for authorized security testing and password strength auditing.**

You may only use Fenrir on:
- Your own systems and accounts
- Systems for which you have **explicit written authorization** to perform security testing
- Password hash datasets you have legal permission to audit

**Unauthorized use against systems you do not own or have permission to test is illegal.** The authors assume no liability for misuse of this software.

---

## Supported Hash Types

| Algorithm | CPU Engine | SIMD (AVX2) | GPU Kernel | Salt | Speed |
|-----------|-----------|-------------|------------|------|-------|
| MD5 | ✅ Full RFC 1321 | ✅ 8-way AVX2 | ✅ | ✅ | Fast |
| SHA1 | ✅ Full FIPS 180-4 | — | ✅ | ✅ | Fast |
| SHA256 | ✅ Full FIPS 180-4 | ✅ 8-way AVX2 | ✅ | ✅ | Fast |
| SHA512 | ✅ Full FIPS 180-4 | — | ✅ | ✅ | Fast |
| NTLM | ✅ MD4 + UTF-16LE | — | ✅ | — | Fast |
| SHA-3 (256) | ✅ Keccak-f[1600] | — | ✅ | ✅ | Fast |
| bcrypt | ✅ **libbcrypt** | — | ✅ | ✅ | Slow |
| scrypt | ✅ Custom | — | ✅ | ✅ | Slow |
| PBKDF2-HMAC-SHA256 | ✅ RFC 2898 | — | ✅ | ✅ | Slow |
| Argon2id | ✅ **phc-winner-argon2** | — | ✅ | ✅ | Slow |

> **bcrypt** and **Argon2id** use industry-standard reference libraries (`libbcrypt` and `phc-winner-argon2`) for guaranteed hash compatibility.

---

## Attack Modes

| Mode | Flag | Description |
|------|------|-------------|
| **Dictionary** | `-a dict` | Reads a wordlist file line-by-line, hashes each word, and compares |
| **Rule-Based** | `-a rule` | Applies 32 hashcat-compatible mutation rules to each dictionary word |
| **Mask / Brute-Force** | `-a mask` | Generates all combinations matching a mask pattern (`?l?l?l?d?d`) |
| **Hybrid** | `-a hybrid` | Dictionary word + brute-force suffix/prefix combinations |
| **API Lookup** | `-a api` | Queries online leak databases (LeakLookup, HashKiller) |

---

## Performance Features

| Feature | Description | Status |
|---------|-------------|--------|
| **AVX2 SIMD** | 8-way parallel MD5/SHA256 on CPU (auto-detected) | ✅ |
| **AVX-512 Detection** | Full XCR0-bits check for AVX-512 readiness | ✅ |
| **GPU (OpenCL)** | 13 kernel files, async double-buffered pipeline | ✅ |
| **Async GPU Pipeline** | Double-buffered OpenCL transfers — GPU computes while CPU prepares | ✅ |
| **Kernel Cache** | Compiled kernels cached in memory with keyed lookup | ✅ |
| **Checkpoint/Resume** | Binary checkpoint with CRC32 integrity — survive interruptions | ✅ |
| **Signal Handling** | Safe SIGINT/SIGTERM with atomic flags — clean shutdown | ✅ |
| **Thread-Safe I/O** | Mutex-guarded result writing with count tracking | ✅ |

---

## Quick Start

### Prerequisites

| Dependency | Required | Notes |
|-----------|----------|-------|
| CMake 3.20+ | ✅ | Build system |
| C++17 Compiler | ✅ | GCC 8+, Clang 7+, MSVC 2019+ |
| OpenCL SDK | Optional | For GPU acceleration |
| libcurl | Optional | For API attack mode |
| Git | ✅ | For FetchContent dependencies |

### Build

```sh
git clone https://github.com/your-org/fenrir-hash-cracker.git
cd fenrir-hash-cracker

# Standard build (CPU + GPU + API)
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release --parallel

# CPU-only build
cmake .. -DFENRIR_USE_OPENCL=OFF -DCMAKE_BUILD_TYPE=Release

# With unit tests
cmake .. -DFENRIR_BUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Release

# With AddressSanitizer (debug)
cmake .. -DFENRIR_ENABLE_ASAN=ON -DCMAKE_BUILD_TYPE=Debug
```

### Usage

```sh
# Dictionary attack — MD5
fenrir -m md5 -a dict -w rockyou.txt -H hashes.txt

# Rule-based attack — SHA1 with rule file
fenrir -m sha1 -a rule -w words.txt -r rules/OneRuleToRuleThemAll.rule -H hashes.txt

# Mask attack — NTLM 8-char lowercase
fenrir -m ntlm -a mask -p "?l?l?l?l?l?l?d?d" -H hash.txt

# Brute-force with custom charset
fenrir -m md5 -a mask --charset "abc123" --min-len 4 --max-len 8 -H hashes.txt

# Hybrid — dictionary + numeric suffix
fenrir -m sha256 -a hybrid -w words.txt --charset "0123456789" --min-len 1 --max-len 3 -H hashes.txt

# SHA-3 cracking
fenrir -m sha3 -a dict -w words.txt -H hashes.txt

# bcrypt cracking (uses libbcrypt)
fenrir -m bcrypt -a dict -w words.txt -H bcrypt_hashes.txt

# Argon2id cracking (uses phc-winner-argon2)
fenrir -m argon2 -a dict -w words.txt -H argon2_hashes.txt

# API lookup — LeakLookup
fenrir -m md5 -a api --provider leaklookup -H hash.txt

# CPU-only mode with no SIMD
fenrir -m sha256 -a dict -w words.txt -H hashes.txt --cpu-only --no-simd

# Resume from checkpoint
fenrir --resume
```

### Full Options

```
  -h,     --help              Print this help message and exit
  -V,     --version           Display program version information and exit
  -m,     --mode TEXT         Hash mode: md5, sha1, sha256, sha512, ntlm, bcrypt, scrypt, sha3, pbkdf2, argon2
  -a,     --attack TEXT       Attack mode: dict, rule, mask, hybrid, api
  -H,     --hashes TEXT ...   Target hash file(s) (can specify multiple)
          --hash TEXT ...     Inline target hash(es)
  -w,     --wordlist TEXT     Dictionary wordlist file path
  -r,     --rule-file TEXT    Rule file (.rule) path
  -p,     --mask-pattern TEXT Mask pattern (e.g., ?l?l?l?d?d)
          --min-len INT [1]   Minimum password length for brute-force
          --max-len INT [8]   Maximum password length for brute-force
          --charset TEXT      Character set for brute-force
  -1..-4 TEXT                 Custom charset definitions (referenced as ?1..?4 in mask)
          --provider TEXT     API provider (leaklookup, hashkiller)
          --api-key TEXT      API key (or set FENRIR_API_KEY env var)
          --no-api-fallback   Disable API fallback when offline exhausted
          --cpu-only          Force CPU-only mode (no GPU)
          --no-simd           Disable SIMD (AVX2/AVX-512) CPU acceleration
          --no-async          Disable async GPU double-buffering
          --async-buffers INT [2]  Number of async GPU buffers (2-4)
  -d,     --device INT [0]    GPU device index
          --batch-size UINT   Candidate batch size (0 = auto)
          --checkpoint-interval INT [30]  Checkpoint save interval in seconds
          --resume            Resume from last checkpoint
  -c,     --config TEXT       Config file path
  -o,     --output TEXT [cracked.txt]  Output file for cracked passwords
          --log-file TEXT     Log file path
          --log-level TEXT [info]  Log verbosity: trace, debug, info, warn, error
```

---

## Configuration

Configuration merges from multiple sources (later overrides earlier):

1. `config/default_config.json` — bundled default configuration
2. `%APPDATA%/.fenrir/config.json` (Windows) or `~/.fenrir/config.json` (Linux/macOS)
3. `./fenrir.json` — project-local configuration
4. Environment variables (`FENRIR_API_KEY`, `FENRIR_BATCH_SIZE`, `FENRIR_CPU_ONLY`, etc.)
5. CLI arguments (highest priority)

### Default Configuration

```json
{
  "batch": { "fast_hash_size": 1000000, "slow_hash_size": 1024, "api_size": 100 },
  "checkpoint": { "interval_seconds": 30, "file": "fenrir.checkpoint" },
  "gpu": {
    "default_device": 0,
    "work_group_auto_tune": true,
    "kernel_cache_dir": ".fenrir_kernel_cache",
    "async_pipeline": true,
    "async_buffers": 2,
    "cpu_only": false
  },
  "cpu": { "enable_simd": true, "cpu_only": false },
  "api": { "timeout_seconds": 10, "max_retries": 3, "rate_limit_buffer": 1.1 },
  "logging": { "level": "info", "console_colors": true, "file": "" },
  "output": { "cracked_file": "cracked.txt", "append_mode": false }
}
```

**Environment variables:** `FENRIR_API_KEY`, `FENRIR_BATCH_SIZE`, `FENRIR_GPU_DEVICE`, `FENRIR_CPU_ONLY`, `FENRIR_API_PROVIDER`, `FENRIR_API_TIMEOUT`, `FENRIR_LOG_LEVEL`, `FENRIR_LOG_FILE`, `FENRIR_OUTPUT_FILE`

**⚠️ Never commit API keys to version control.**

---

## Rule Engine

Fenrir supports 32 hashcat-compatible mutation rules:

| Category | Opcodes | Description |
|----------|---------|-------------|
| Case | `l` `u` `c` `C` `t` `T` | Lowercase, uppercase, capitalize, invert, toggle, toggle-at |
| Insert/Delete | `$X` `^X` `iNX` `D` `oNX` | Append, prepend, insert, delete, overwrite |
| Substitution | `sXY` `@X` | Replace and purge |
| Duplication | `d` `pN` `f` | Double, repeat-N, reflect |
| Rotation | `{` `}` `k` `K` | Left, right, swap-first-two, swap-last-two |
| Extraction | `xNM` `ONM` `'N` | Extract, omit, truncate |
| Memory | `M` `4` `6` | Memorize, append-memory, prepend-memory |
| Rejection | `<N` `>N` `!X` `/X` | Reject len>N, len<N, contains-X, not-contains-X |
| Increment | `+N` `-N` | Increment/decrement char at position N |

**Bundled rule files:**

| File | Rules | Description |
|------|-------|-------------|
| `rules/best64.rule` | 79 | Top 64 most effective mutation rules |
| `rules/leetspeak.rule` | 15 | Leetspeak substitutions (`a→@`, `e→3`, `s→$`...) |
| `rules/toggle.rule` | 15 | Case toggling variations |
| `rules/combos.rule` | 29 | Common digit/symbol character combinations |
| `rules/OneRuleToRuleThemAll.rule` | 127 | Comprehensive 90+ rule set |

---

## Mask Attack Syntax

| Placeholder | Character Set | Size |
|-------------|--------------|------|
| `?l` | a–z | 26 |
| `?u` | A–Z | 26 |
| `?d` | 0–9 | 10 |
| `?s` | Special chars | 33 |
| `?a` | `?l` + `?u` + `?d` + `?s` | 95 |
| `?h` | 0–9, a–f | 16 |
| `?H` | 0–9, A–F | 16 |
| `?b` | All 256 bytes | 256 |

Custom charsets via `-1` through `-4` are referenced as `?1`, `?2`, `?3`, `?4`.

Characters are frequency-ordered (common letters first) to maximize early-hit probability during cracking.

---

## Architecture

```
CLI (CLI11 v2.5.0) → Config (JSON + env + CLI merge)
                              ↓
                        Pipeline (CPU/GPU dispatch)
                              ↓
              ┌───────────────┴───────────────┐
              ↓                               ↓
         Attack Layer                    Hash Engines
    (dict/rule/mask/hybrid/api)    (10 algorithms + SIMD)
              ↓                               ↓
    ┌─────────┴─────────┐           ┌─────────┴──────────┐
    ↓                   ↓           ↓                    ↓
  GPU (OpenCL)    CPU (AVX2)    Standard Libs      Scalar CPU
  Async Pipeline  SIMD Engine   (libbcrypt,        Fallback
  Kernel Cache    8-way MD5     phc-winner-
  13 .cl kernels  8-way SHA256  argon2)
         ↓                   ↓           ↓                    ↓
         └───────────────────┴───────────┴────────────────────┘
                                    ↓
                      Compare → ResultWriter → cracked.txt
                                    ↓
                      Progress (H/s, ETA) + Checkpoint (CRC32)
```

### Key Design Patterns

- **Strategy Pattern** — 10 hash algorithms, 5 attack modes, 2 API providers
- **Factory Pattern** — `HashEngineFactory`, `AttackRegistry`, `ApiManager::createProvider()`
- **RAII** — OpenCL buffer guards, `std::unique_ptr` throughout, exception-safe resource management
- **FetchContent** — CLI11, nlohmann/json, spdlog, Catch2, libbcrypt, phc-winner-argon2 auto-downloaded
- **Idempotent Init** — Logger supports re-initialization, SignalHandler supports re-install
- **Atomic Signal Handling** — Signal handlers only set atomic flags; callbacks polled at safe points

---

## Project Structure

```
fenrir-hash-cracker/
├── CMakeLists.txt                 # Build system (FetchContent deps)
├── README.md                      # This file
├── LICENSE                        # MIT License
├── config/
│   └── default_config.json        # Bundled default configuration
├── kernels/                       # OpenCL .cl kernel files (13 files)
│   ├── common.cl                  # Shared macros (ROT, MD5_F/G/H/I, SHA_CH/MAJ...)
│   ├── md5.cl / md5_optimized.cl
│   ├── sha1.cl / sha256.cl / sha512.cl / sha3.cl
│   ├── ntlm.cl / ntlm_optimized.cl
│   ├── bcrypt.cl / scrypt.cl
│   └── pbkdf2.cl / argon2.cl
├── rules/                         # Hashcat-format rule files (5 files)
│   ├── best64.rule
│   ├── leetspeak.rule
│   ├── toggle.rule
│   ├── combos.rule
│   └── OneRuleToRuleThemAll.rule
├── cmake/
│   └── FindOpenCL.cmake           # Custom OpenCL finder (cross-platform)
├── src/
│   ├── main.cpp                   # Entry point — CLI setup, config merge, pipeline launch
│   ├── core/                      # Hash engines, Config, Pipeline, ResultWriter (16 files)
│   ├── gpu/                       # OpenCL context, kernel, hasher, cache, stats (12 files)
│   ├── attack/                    # Dictionary, Rule, Mask, Hybrid, API attacks (13 files)
│   ├── rules/                     # Rule parser and engine — 32 opcodes (5 files)
│   ├── api/                       # LeakLookup, HashKiller providers, RateLimiter (9 files)
│   ├── cli/                       # CLI argument parsing wrapper (2 files)
│   ├── cpu_simd/                  # AVX2 SIMD detectors and engines (6 files)
│   └── utils/                     # Logger, FileReader, HashParser, Checkpoint, etc. (13 files)
└── tests/                         # Catch2 unit tests (9 test files, 24 test cases)
    ├── test_main.cpp
    ├── test_hash_engines.cpp       # MD5/SHA1/SHA256/SHA512/NTLM RFC vectors
    ├── test_hash_parser.cpp        # Hash type detection and parsing
    ├── test_rule_parser.cpp        # Rule file parsing
    ├── test_rule_engine.cpp        # Rule application and opcodes
    ├── test_mask_generator.cpp     # Mask/brute-force generation
    ├── test_checkpoint.cpp         # Checkpoint save/load round-trip
    ├── test_api_manager.cpp        # API provider creation and query
    └── test_pipeline.cpp           # End-to-end pipeline with known hash
```

---

## Dependencies

| Library | Version | Source | Purpose |
|---------|---------|--------|---------|
| CLI11 | v2.5.0 | FetchContent | CLI argument parsing |
| nlohmann/json | v3.11.3 | FetchContent | JSON config parsing |
| spdlog | v1.15.3 | FetchContent | Logging (console + file) |
| Catch2 | v3.7.1 | FetchContent | Unit testing framework |
| **libbcrypt** | master | FetchContent | bcrypt hash computation |
| **phc-winner-argon2** | master | FetchContent | Argon2id hash verification |
| OpenCL | System | find_package | GPU acceleration |
| libcurl | System | find_package | HTTP API requests |

---

## Testing

```sh
# Build with tests
cmake .. -DFENRIR_BUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release

# Run all 24 tests
ctest -C Release

# Run specific test
./tests/Release/fenrir_tests.exe "MD5 RFC 1321 test vectors"

# Run with verbose output
./tests/Release/fenrir_tests.exe -s
```

Test coverage:
- ✅ **MD5** — RFC 1321 test vectors (5 assertions, including empty, single-char, multi-char)
- ✅ **SHA1** — FIPS 180-4 test vectors (empty, `"abc"`, long string)
- ✅ **SHA256** — FIPS 180-4 test vectors
- ✅ **SHA512** — FIPS 180-4 test vectors
- ✅ **NTLM** — Known test vectors (empty, `"password"`, `"hashcat"`)
- ✅ **Rule Parser** — All 32 opcodes parsed
- ✅ **Rule Engine** — Single and combined rule application
- ✅ **Mask Generator** — 26×26 lowercase, 10×10×10 digits, uniqueness checks
- ✅ **Checkpoint** — Save/load round-trip with CRC32
- ✅ **Rate Limiter** — Token bucket consumption
- ✅ **Pipeline** — End-to-end MD5 crack with dictionary

---

## License

MIT License — see [LICENSE](LICENSE) file for details.

---

## Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

---

**Remember:** With great power comes great responsibility. Use Fenrir ethically and legally.

*Fenrir Hash Cracker v1.0.0 — Built with C++17, OpenCL, AVX2, and ❤️*
