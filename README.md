# SEON Core

A security research project demonstrating executable integrity verification through DLL-based hash validation.

## Overview

SEON Core implements a self-protection mechanism where the main executable loads a companion DLL at startup. The DLL contains embedded cryptographic hashes of the original executable and verifies the runtime integrity before allowing execution to continue.

## Architecture

```
seon.exe ──LoadLibrary()──> seon.dll
                               │
                               ├── Compute MD5/SHA256 of seon.exe
                               ├── Compare with embedded expected hashes
                               └── Return OK / INVALID_HASH
```

## How It Works

**Build-Time**
1. Compile `sigtool.exe` - Hash generator utility
2. Compile `seon.exe` - Target application
3. Run sigtool to compute MD5/SHA256 of seon.exe → generate `signature_data.h`
4. Compile `seon.dll` with embedded hashes

**Runtime**
1. seon.exe loads seon.dll
2. seon.dll computes hashes of seon.exe
3. Compares against embedded expected values
4. Returns result: OK (0) or INVALID_HASH (1)

## Quick Start

### Requirements
- Windows with Visual Studio 2022 (C++20)
- Python with PVM: `python -m pip install meson ninja`

### Build

```batch
# From any terminal (auto-detects VS environment)
scripts\rebuild.bat

# Or step by step
scripts\setup.bat
scripts\build.bat
```

### Run

```batch
scripts\run.bat
```

## Output

All build outputs are placed in `output/`:

| File | Description |
|------|-------------|
| `seon.exe` | Protected application |
| `seon.dll` | Integrity verifier DLL |
| `seon.lib` | Import library |
| `signature_data.h` | Auto-generated hash header |
| `sigtool.exe` | Signature generator tool |

## API

```cpp
#include <windows.h>

typedef struct {
    uint32_t result;
    char message[256];
    uint8_t computed_md5[16];
    uint8_t computed_sha256[32];
} seon_context_t;

// Load DLL
HMODULE hMod = LoadLibraryA("seon.dll");

// Get functions
auto seon_verify = (uint32_t (*)(void*, seon_context_t*))
    GetProcAddress(hMod, "seon_verify");
auto seon_result_str = (const char* (*)(uint32_t))
    GetProcAddress(hMod, "seon_result_str");

// Verify integrity
seon_context_t ctx;
uint32_t result = seon_verify(nullptr, &ctx);

printf("Status: %s\n", seon_result_str(result));
printf("MD5: "); for (int i = 0; i < 16; i++) printf("%02x", ctx.computed_md5[i]);
```

## Security Features

| Feature | Implementation |
|---------|---------------|
| Dual Hash | MD5 (16 bytes) + SHA256 (32 bytes) |
| Constant-Time Compare | `volatile uint8_t result \|= a[i] ^ b[i]` prevents timing attacks |
| Embedded Signature | Hashes compiled into DLL `.rdata` section |
| Runtime Verification | Fresh hash computation on every launch |

## Testing

### Normal Execution
```
SEON Core - Protected Application

[INFO] Initialized.
[INFO] Performing integrity check...
[RESULT] Status: OK
[RESULT] Message: Integrity verification passed
[INFO] Computed MD5: f1b98dcf683ed3e453e9c30a63bdac79
[INFO] Computed SHA256: 6792a6a700c2e76627a2842c0396c4d951fa879dea1237bb67cb262829ebed36

Integrity verification PASSED
```

### Tampered Binary
Modify seon.exe with a hex editor, then run:
```
[RESULT] Status: Invalid Hash
[RESULT] Message: MD5 mismatch! Expected: f1b9..., Got: 8c2a...

Integrity verification FAILED
```

## Project Structure

```
seon-core/
├── output/                   # Build outputs
│   ├── seon.exe
│   ├── seon.dll
│   ├── seon.lib
│   ├── sigtool.exe
│   └── signature_data.h
├── src/
│   └── main.cpp
├── protector/
│   ├── include/seon.h
│   ├── include/checker.h
│   ├── src/dllmain.cpp
│   └── src/checker.cpp
├── tools/sigtool/
│   └── src/sigtool.cpp
├── scripts/
│   ├── setup.bat
│   ├── build.bat
│   ├── rebuild.bat
│   ├── run.bat
│   └── clean.bat
└── meson.build
```

## Build Scripts

| Script | Purpose |
|--------|---------|
| `setup.bat` | Configure meson build directory |
| `build.bat` | Compile project |
| `rebuild.bat` | Clean + setup + build |
| `run.bat` | Run seon.exe from output folder |
| `clean.bat` | Remove build and output directories |

## Limitations & Future Work

- **Static hashing**: Entire file is hashed; could use section-specific hashing
- **No anti-debug**: No debugger/VM detection
- **Simple response**: Process exit on failure; could implement more sophisticated responses

Potential enhancements:
- Code obfuscation
- Anti-debugging techniques
- Encrypted signature storage
- Hardware-backed attestation

## License

Security Research Project - Educational Use Only
