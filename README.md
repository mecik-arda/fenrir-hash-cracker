# 🐺 Fenrir Hash Cracker v1.1.2

**GPU-accelerated password hash auditing tool** — C++17 · OpenCL · AVX2/AVX-512 · TUI · CLI

Fenrir is a high-performance password strength auditing tool featuring a live-updating terminal interface (TUI), GPU computing via OpenCL, and CPU SIMD (AVX2/AVX-512). It supports 14 hash algorithms, 6 attack modes, automatic `/etc/shadow` and macOS `.plist` parsing, potfile-based resume, built-in benchmarking, and online leak-database API lookups.

---

## ⚠️ Responsible Use Disclaimer

**This tool is intended solely for authorized security testing and password strength auditing.**

You may only use Fenrir on:
- Your own systems and accounts
- Systems for which you have **explicit written authorization** to perform security testing
- Password hash datasets you have legal permission to audit

**Unauthorized use against systems you do not own or have permission to test is illegal.** The authors assume no liability for misuse of this software.

---

## Live TUI (Text User Interface)

Fenrir now features a cross-platform, live-updating terminal interface:

```
+------------------------------------------------------------------+
| 🐺 Fenrir MD5 · Dictionary · CPU+SIMD                            |
+------------------------------------------------------------------+
| Progress: 35.2% ########....................................    |
| Tested: 1,500,000 | Cracked: 3 | Remaining: 2                    |
+------------------------------------------------------------------+
| Speed: 1.50 GH/s | ETA: 5m 30s | Elapsed: 2m 15s                |
+------------------------------------------------------------------+
```

- **Progress bar** shows live keyspace completion percentage
- **Fixed panels** for stats — no scrolling log clutter
- **Cross-platform**: ANSI escape codes on Linux/macOS, Virtual Terminal on Windows 10+
- **Console title bar** shows current algorithm and attack mode
- **Disable with `--no-tui`** to revert to plain text logging

---

## Supported Hash Types

| Algorithm | CPU Engine | SIMD (AVX2) | GPU Kernel | Salt | Speed |
|-----------|-----------|-------------|------------|------|-------|
| MD5 | ✅ Full RFC 1321 | ✅ 8-way AVX2 | ✅ optimized | ✅ | Fast |
| SHA1 | ✅ Full FIPS 180-4 | — | ✅ optimized | ✅ | Fast |
| SHA256 | ✅ Full FIPS 180-4 | ✅ 8-way AVX2 | ✅ optimized | ✅ | Fast |
| SHA384 | ✅ Full FIPS 180-4 | — | ✅ | ✅ | Fast |
| SHA512 | ✅ Full FIPS 180-4 | — | ✅ | ✅ | Fast |
| NTLM | ✅ MD4 + UTF-16LE | — | ✅ optimized | — | Fast |
| SHA-3 (256) | ✅ Keccak-f[1600] | — | ✅ | ✅ | Fast |
| Blake2b | ✅ RFC 7693 | — | — | ✅ | Fast |
| HMAC-MD5 | ✅ RFC 2104 | — | — | ✅ | Fast |
| HMAC-SHA256 | ✅ RFC 2104 | — | — | ✅ | Fast |
| bcrypt | ✅ **libbcrypt** | — | ✅ | ✅ | Slow |
| scrypt | ✅ Custom | — | ✅ | ✅ | Slow |
| PBKDF2-HMAC-SHA256 | ✅ RFC 2898 | — | ✅ | ✅ | Slow |
| Argon2id | ✅ **phc-winner-argon2** | — | ✅ | ✅ | Slow |

> **bcrypt** and **Argon2id** use industry-standard reference libraries for guaranteed hash compatibility.
> **BLAKE2b** and **HMAC** engines are pure software implementations per RFC 7693 and RFC 2104.

### Auto-Detection from System Files

Fenrir can automatically parse hash formats from:

| Source | Format | Auto-Detection |
|--------|--------|---------------|
| **`/etc/shadow`** (Linux) | `user:$6$salt$hash:...` | ✅ Username + hash type (`$1$`, `$5$`, `$6$`, `$y$`) |
| **macOS `.plist`** | PBKDF2-SHA512 in XML plist | ✅ Base64 entropy + salt extraction |
| **Standard hash files** | `hash:password` or `hash` per line | ✅ Hex length or `$id$` prefix |

Simply pass the file via `-H` and Fenrir detects the format automatically — no manual preprocessing needed.

---

## Attack Modes

| Mode | Flag | Description |
|------|------|-------------|
| **Dictionary** | `-a dict` | Reads a wordlist file line-by-line, hashes each word, and compares |
| **Rule-Based** | `-a rule` | Applies 32 hashcat-compatible mutation rules to each dictionary word |
| **Mask / Brute-Force** | `-a mask` | Generates all combinations matching a mask pattern (`?l?l?l?d?d`) |
| **Hybrid** | `-a hybrid` | Dictionary word + brute-force suffix/prefix combinations |
| **Combinator** | `-a combinator` | Combines every word from two wordlists (`word1 + word2`) |
| **API Lookup** | `-a api` | Queries online leak databases (4 providers) |

---

## Performance Features

| Feature | Description | Status |
|---------|-------------|--------|
| **Live TUI** | Cross-platform live-updating terminal with progress bar | ✅ |
| **AVX2 SIMD** | 8-way parallel MD5/SHA256 on CPU (auto-detected) | ✅ |
| **AVX-512 Detection** | Full XCR0-bits check (bits 1,2,5,6,7) | ✅ |
| **GPU (OpenCL)** | 13 kernel files, async double-buffered pipeline | ✅ |
| **Async GPU Pipeline** | Double-buffered OpenCL transfers | ✅ |
| **Kernel Cache** | Compiled kernels cached in memory | ✅ |
| **Built-in Benchmark** | `-b` tests all 14 algorithms on CPU & GPU | ✅ |
| **Shadow Parser** | Auto-parse `/etc/shadow` with username + hash type | ✅ |
| **Plist Parser** | Auto-parse macOS `.plist` PBKDF2 hashes | ✅ |
| **Potfile** | `fenrir.pot` — hashcat-compatible, auto-load/save | ✅ |
| **Checkpoint/Resume** | Binary checkpoint with CRC32 integrity | ✅ |
| **Signal Handling** | Safe SIGINT/SIGTERM with atomic flags | ✅ |
| **Thread-Safe I/O** | Mutex-guarded result writing | ✅ |

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

# Standard build (CPU + GPU + API + TUI)
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release --parallel

# CPU-only build
cmake .. -DFENRIR_USE_OPENCL=OFF -DCMAKE_BUILD_TYPE=Release

# With unit tests
cmake .. -DFENRIR_BUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Release
```

### Usage

```sh
# ── Dictionary Attacks ──────────────────────────────────
fenrir -m md5      -a dict -w rockyou.txt -H hashes.txt
fenrir -m sha256   -a dict -w words.txt   -H hashes.txt
fenrir -m bcrypt   -a dict -w words.txt   -H bcrypt_hashes.txt

# ── Auto-detect from /etc/shadow ────────────────────────
fenrir -m sha512 -a dict -w rockyou.txt -H /etc/shadow
# Output: "CRACKED: root -> password123" (with username!)

# ── Auto-detect from macOS .plist ───────────────────────
fenrir -m pbkdf2 -a dict -w words.txt -H /var/db/dslocal/nodes/Default/users/john.plist

# ── Rule-Based Attack ───────────────────────────────────
fenrir -m sha1 -a rule -w words.txt -r rules/OneRuleToRuleThemAll.rule -H hashes.txt

# ── Mask / Brute-Force ──────────────────────────────────
fenrir -m ntlm    -a mask -p "?l?l?l?l?l?l?d?d" -H hash.txt
fenrir -m sha256  -a mask --charset "abc123" --min-len 4 --max-len 8 -H hashes.txt

# ── Combinator Attack ───────────────────────────────────
fenrir -m md5 -a combinator -w prefixes.txt --wordlist2 suffixes.txt -H hashes.txt

# ── New Algorithms ──────────────────────────────────────
fenrir -m sha384      -a dict -w words.txt -H hashes.txt
fenrir -m blake2b     -a dict -w words.txt -H hashes.txt
fenrir -m hmac-sha256 -a dict -w words.txt -H hashes.txt

# ── API Lookup ──────────────────────────────────────────
fenrir -m md5 -a api --provider leaklookup --api-key KEY -H hashes.txt

# ── Benchmark ───────────────────────────────────────────
fenrir --benchmark                  # All 14 algorithms on CPU + GPU
fenrir -b --benchmark-count 50000  # Custom count
fenrir -b --cpu-only               # CPU only

# ── Potfile ─────────────────────────────────────────────
fenrir -m md5 -a dict -w words.txt -H hashes.txt --potfile cracked.pot

# ── TUI & Misc ──────────────────────────────────────────
fenrir -m sha256 -a dict -w words.txt -H hashes.txt           # TUI enabled by default
fenrir -m sha256 -a dict -w words.txt -H hashes.txt --no-tui  # Plain text mode
fenrir -m sha256 -a dict -w words.txt -H hashes.txt --cpu-only --no-simd
```

### Full Options

```
  -h,     --help              Print this help message and exit
  -V,     --version           Display program version information and exit
  -b,     --benchmark         Run benchmark for all hash algorithms
          --benchmark-count UINT [100000]  Candidates per algorithm for benchmark

  -m,     --mode TEXT         Hash mode: md5, sha1, sha256, sha384, sha512, ntlm,
                              bcrypt, scrypt, sha3, pbkdf2, argon2, blake2b,
                              hmac-md5, hmac-sha256
  -a,     --attack TEXT       Attack mode: dict, rule, mask, hybrid, combinator, api

  -H,     --hashes TEXT ...   Target hash file(s) — auto-detects shadow/plist format
          --hash TEXT ...     Inline target hash(es)
  -w,     --wordlist TEXT     Dictionary wordlist file path
          --wordlist2 TEXT    Second wordlist (combinator attack)
  -r,     --rule-file TEXT    Rule file (.rule) path
  -p,     --mask-pattern TEXT Mask pattern (e.g., ?l?l?l?d?d)
          --min-len INT [1]   Minimum password length for brute-force
          --max-len INT [8]   Maximum password length for brute-force
          --charset TEXT      Character set for brute-force
  -1..-4 TEXT                 Custom charset definitions

          --provider TEXT     API provider: leaklookup, hashkiller, hashtoolkit, md5decrypt
          --api-key TEXT      API key (or set FENRIR_API_KEY env var)
          --no-api-fallback   Disable API fallback

          --no-tui            Disable live TUI progress (plain log output)
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
          --potfile TEXT [fenrir.pot]  Potfile for storing/loading cracked hashes
          --log-file TEXT     Log file path
          --log-level TEXT [info]  Log verbosity: trace, debug, info, warn, error
```

---

## Configuration

Configuration merges from multiple sources (later overrides earlier):

1. `config/default_config.json` — bundled default configuration
2. `%APPDATA%/.fenrir/config.json` (Windows) or `~/.fenrir/config.json` (Linux/macOS)
3. `./fenrir.json` — project-local configuration
4. Environment variables (`FENRIR_API_KEY`, `FENRIR_POTFILE`, etc.)
5. CLI arguments (highest priority)

**⚠️ Never commit API keys to version control.**

---

## Architecture

```
CLI (CLI11 v2.5.0) → Config (JSON + env + CLI merge)
                              ↓
                  ┌───────────┴───────────┐
                  ↓                       ↓
          ┌── Benchmark Mode         Pipeline (CPU/GPU dispatch)
          │   (14 algorithms)             ↓
          │                    ┌──────────┴──────────┐
          │                    ↓                     ↓
          │              Auto-Detect            Attack Layer
          │         (/etc/shadow, .plist,    (dict/rule/mask/
          │           standard hashes)        hybrid/combinator/api)
          │                    ↓                     ↓
          │              Hash Engines          Candidate Gen
          │           (14 algos + SIMD)        (batched I/O)
          │                    ↓
          │         ┌──────────┴──────────┐
          │         ↓                     ↓
          │    GPU (OpenCL)          CPU (AVX2)
          │    Async Pipeline         SIMD Engine
          │    Kernel Cache           8-way MD5/SHA256
          │         ↓                     ↓
          │         └──────────┬──────────┘
          │                    ↓
          └──────────→  Compare + ResultWriter
                              ↓
                    Live TUI (ProgressDisplay)
                    + Potfile + Checkpoint (CRC32)
```

---

## Project Structure

```
fenrir-hash-cracker/
├── CMakeLists.txt
├── README.md
├── config/default_config.json
├── kernels/                       # 13 OpenCL .cl files
├── rules/                         # 5 hashcat rule files
├── cmake/FindOpenCL.cmake
├── src/
│   ├── main.cpp
│   ├── core/                      # 14 hash engines, Pipeline, Benchmark, Config
│   ├── gpu/                       # OpenCL context, kernel, hasher, cache, stats
│   ├── attack/                    # 6 attack modes
│   ├── rules/                     # 32 opcode rule engine
│   ├── api/                       # 4 API providers
│   ├── ui/                        # TUI: Terminal + ProgressDisplay
│   ├── cli/                       # CLI parser wrapper
│   ├── cpu_simd/                  # AVX2 SIMD detectors and engines
│   └── utils/                     # Logger, FileReader, HashParser, ShadowParser,
│                                  #   PlistParser, Checkpoint, SignalHandler, Timer
└── tests/                         # 9 test files, 24 test cases
```

---

## Dependencies

| Library | Version | Source | Purpose |
|---------|---------|--------|---------|
| CLI11 | v2.5.0 | FetchContent | CLI argument parsing |
| nlohmann/json | v3.11.3 | FetchContent | JSON config parsing |
| spdlog | v1.15.3 | FetchContent | Logging |
| Catch2 | v3.7.1 | FetchContent | Unit testing |
| **libbcrypt** | master | FetchContent | bcrypt hash computation |
| **phc-winner-argon2** | master | FetchContent | Argon2id hash verification |
| OpenCL | System | find_package | GPU acceleration |
| libcurl | System | find_package | HTTP API requests |

---

## Testing

```sh
cmake .. -DFENRIR_BUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
ctest -C Release                           # 24 tests
./tests/Release/fenrir_tests.exe -s        # Verbose output
```

---

## Changelog

### v1.1.2
- 🆕 **Live TUI**: Cross-platform terminal interface with progress bar, fixed stat panels
- 🆕 **`/etc/shadow` parser**: Auto-detect username + hash type from Linux shadow files
- 🆕 **macOS `.plist` parser**: Extract PBKDF2-SHA512 hashes from macOS user plists
- 🆕 **Username tracking**: Cracked output shows `username -> password` for shadow/plist sources
- 🆕 **`--no-tui` flag**: Disable TUI for scripting/headless use

### v1.1.0
- 🆕 4 new hash algorithms: SHA-384, Blake2b, HMAC-MD5, HMAC-SHA256
- 🆕 Combinator attack, Benchmark mode, Potfile support
- 🆕 2 new API providers: HashToolkit, Md5Decrypt
- 🚀 GPU kernel optimizations for MD5, SHA1, SHA256, NTLM

### v1.0.0
- Initial release: 10 hash algorithms, 5 attack modes, GPU OpenCL, AVX2 SIMD

---

## License

MIT License — see [LICENSE](LICENSE) file for details.

---

**Remember:** With great power comes great responsibility. Use Fenrir ethically and legally.

*Fenrir Hash Cracker v1.1.2 — Built with C++17, OpenCL, AVX2, TUI, and ❤️*
