# Fenrir Hash Cracker

**GPU-accelerated password hash auditing tool** — C++17 · OpenCL · AVX2 · CLI

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

| Algorithm | GPU Kernel | SIMD (AVX2) | Salt Support | Speed |
|-----------|-----------|-------------|--------------|-------|
| MD5 | ✅ | ✅ | ✅ | Fast |
| SHA1 | ✅ | — | ✅ | Fast |
| SHA256 | ✅ | ✅ | ✅ | Fast |
| SHA512 | ✅ | — | ✅ | Fast |
| NTLM | ✅ | — | — | Fast |
| bcrypt | ✅ | — | ✅ | Slow |
| scrypt | ✅ | — | ✅ | Slow |
| **SHA-3 (256)** | ✅ | — | ✅ | Fast |
| **PBKDF2-HMAC-SHA256** | ✅ | — | ✅ | Slow |
| **Argon2id** | ✅ | — | ✅ | Slow |

## Attack Modes

| Mode | Flag | Description |
|------|------|-------------|
| Dictionary | `-a dict` | Reads a wordlist, hashes each word, compares |
| Rule-Based | `-a rule` | Applies mutation rules to dictionary words |
| Mask / Brute-Force | `-a mask` | Generates all combinations matching a mask pattern |
| Hybrid | `-a hybrid` | Dictionary word + brute-force suffix/prefix |
| API Lookup | `-a api` | Queries online leak databases (LeakLookup, HashKiller) |

## Performance Features

| Feature | Description |
|---------|-------------|
| **AVX2 SIMD** | 8-way parallel MD5/SHA256 on CPU (auto-detected) |
| **Async GPU Pipeline** | Double-buffered OpenCL transfers — GPU computes while CPU prepares next batch |
| **Optimized Kernels** | `#pragma unroll`, local memory, atomic operations, auto-tuned work-group sizes |
| **Checkpoint/Resume** | Binary checkpoint with CRC32 — survive interruptions on long jobs |

---

## Quick Start

### Prerequisites
- CMake 3.20+
- C++17 compiler (GCC 8+, Clang 7+, MSVC 2019+)
- OpenCL SDK (for GPU acceleration)
- libcurl (optional, for API mode)

### Build

```sh
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

### Usage

```sh
fenrir -m md5 -a dict -w rockyou.txt -h hashes.txt

fenrir -m ntlm -a mask -p "?l?l?l?l?l?l?d?d" -h hash.txt

fenrir -m sha1 -a rule -w words.txt -r rules/OneRuleToRuleThemAll.rule -h hashes.txt

fenrir -m sha3 -a dict -w words.txt -h hashes.txt

fenrir -m argon2 -a dict -w words.txt -h hashes.txt

fenrir -m md5 -a api --provider leaklookup -h hash.txt

fenrir --resume

fenrir -m sha256 -a dict -w words.txt -h hashes.txt --no-simd --no-async
```

### Full Options

```
  -m, --mode          Hash type (md5, sha1, sha256, sha512, ntlm, bcrypt, scrypt,
                      sha3, pbkdf2, argon2)
  -a, --attack        Attack mode (dict, rule, mask, hybrid, api)
  -h, --hashes        Target hash file(s)
  -w, --wordlist      Dictionary file
  -r, --rule-file     Rule file (.rule)
  -p, --mask-pattern  Mask pattern (?l, ?u, ?d, ?s, ?a, ?h, ?H, ?b)
  --min-len, --max-len  Length range for brute-force
  --charset           Character set for brute-force
  -1..-4              Custom charset definitions
  --provider          API provider (leaklookup, hashkiller)
  --api-key           API key (or FENRIR_API_KEY env var)
  --cpu-only          Force CPU-only mode
  --no-simd           Disable SIMD (AVX2/AVX-512) CPU acceleration
  --no-async          Disable async GPU double-buffering
  --async-buffers     Number of async GPU buffers (2-4, default: 2)
  -d, --device        GPU device index (default: 0)
  --batch-size        Candidates per batch (0 = auto)
  --checkpoint-interval  Seconds between checkpoints (default: 30)
  --resume            Resume from last checkpoint
  -c, --config        Config file path
  -o, --output        Output file (default: cracked.txt)
  --log-file          Log file path
  --log-level         Log verbosity (trace, debug, info, warn, error)
  -V, --version       Print version
```

---

## Configuration

Configuration is loaded from multiple sources (later overrides earlier):

1. `config/default_config.json` (bundled with the application)
2. `~/.fenrir/config.json` (user-level)
3. `./fenrir.json` (project-local, in current directory)
4. Environment variables (`FENRIR_API_KEY`, `FENRIR_BATCH_SIZE`, etc.)
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
    "async_buffers": 2
  },
  "cpu": { "enable_simd": true },
  "api": { "timeout_seconds": 10, "max_retries": 3, "rate_limit_buffer": 1.1 },
  "logging": { "level": "info", "console_colors": true, "file": "" },
  "output": { "cracked_file": "cracked.txt", "append_mode": false }
}
```

**API keys** should be set via the `FENRIR_API_KEY` environment variable or in a local config file. **Never commit API keys to version control.**

---

## Rule Engine

Fenrir supports 50+ hashcat-compatible mutation rules:

| Category | Examples |
|----------|----------|
| Case | `l` (lowercase), `u` (uppercase), `c` (capitalize), `t` (toggle), `TN` (toggle at N) |
| Insert/Delete | `$X` (append), `^X` (prepend), `iNX` (insert), `DN` (delete), `oNX` (overwrite) |
| Substitution | `sXY` (replace X→Y), `@X` (purge X) |
| Duplication | `d` (double), `pN` (repeat N times), `f` (reflect) |
| Rotation | `{` (left), `}` (right), `k` (swap first two), `K` (swap last two) |
| Extraction | `xNM` (extract), `ONM` (omit), `'N` (truncate) |
| Memory | `M` (memorize), `4` (append memory), `6` (prepend memory) |
| Reject | `<N` (len > N), `>N` (len < N), `!X` (contains X), `/X` (not contains X) |
| Increment | `+N` (increment char at N), `-N` (decrement char at N) |

**Bundled rule files:**

| File | Description |
|------|-------------|
| `rules/best64.rule` | Top 64 most effective rules |
| `rules/leetspeak.rule` | Leetspeak substitutions (s0, si1, se3...) |
| `rules/toggle.rule` | Case toggling variations |
| `rules/combos.rule` | Common digit/symbol combinations |
| `rules/OneRuleToRuleThemAll.rule` | 90+ rule comprehensive set |

---

## Mask Attack Syntax

| Placeholder | Character Set | Size |
|-------------|--------------|------|
| `?l` | a–z | 26 |
| `?u` | A–Z | 26 |
| `?d` | 0–9 | 10 |
| `?s` | Special chars | 33 |
| `?a` | ?l + ?u + ?d + ?s | 95 |
| `?h` | 0–9, a–f | 16 |
| `?H` | 0–9, A–F | 16 |
| `?b` | All bytes | 256 |

Custom charsets via `-1` through `-4` are referenced as `?1`, `?2`, `?3`, `?4`.

---

## Architecture

```
CLI (CLI11) → Config → Pipeline → Attack → Candidates
                                     ↓
                         ┌───────────┴───────────┐
                         ↓                       ↓
                   GPU (OpenCL)            CPU (AVX2 SIMD)
                   Async 2x Buffer         Scalar Fallback
                         ↓                       ↓
                         └───────────┬───────────┘
                                     ↓
                              Compare → ResultWriter
                                     ↓
                              Progress (H/s, ETA) + Checkpoint
```

- **Attack layer is GPU/CPU-agnostic** — works with GPU, AVX2 SIMD, and scalar CPU fallback
- **Strategy pattern** for 10 hash algorithms and 5 attack modes
- **OpenCL kernels** loaded from `.cl` files at runtime (editable without recompilation)
- **SIMD auto-detection** at startup — uses fastest available CPU path (AVX-512 > AVX2 > SSE4.1 > scalar)
- **Async double-buffering** — GPU computes batch N while CPU prepares batch N+1
- **Checkpoint/resume** — binary format with CRC32, auto-saved on interval and SIGINT

---

## Build Options

```sh
cmake .. -DFENRIR_USE_OPENCL=OFF     # CPU-only build (no GPU)
cmake .. -DFENRIR_USE_CURL=OFF       # No API attack mode
cmake .. -DFENRIR_BUILD_TESTS=ON     # Build unit tests with Catch2
cmake .. -DFENRIR_ENABLE_ASAN=ON     # Enable AddressSanitizer (debug)
```

---

## Project Structure

```
fenrir-hash-cracker/
├── CMakeLists.txt              # Build system
├── config/default_config.json  # Default configuration
├── kernels/                    # OpenCL .cl kernel files (11 files)
├── rules/                      # Hashcat-format rule files (5 files)
├── src/
│   ├── main.cpp                # Entry point
│   ├── core/                   # Hash engines, pipeline, config
│   ├── gpu/                    # OpenCL context, kernels, async hasher
│   ├── attack/                 # Dictionary, rule, mask, hybrid, API attacks
│   ├── rules/                  # Rule parser and engine
│   ├── api/                    # REST API providers
│   ├── cli/                    # CLI argument parsing
│   ├── cpu_simd/               # AVX2 SIMD accelerated engines
│   └── utils/                  # Logger, file I/O, checkpoint, timer
└── tests/                      # Catch2 unit tests
```

---

## License

MIT License — see LICENSE file for details.

---

**Remember:** With great power comes great responsibility. Use Fenrir ethically and legally.
