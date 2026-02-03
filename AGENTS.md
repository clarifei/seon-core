# SEON Core - Agent Guide

Internal documentation for AI agents working on this project.

## Project Overview

SEON Core is a proof-of-concept for executable integrity verification. The main executable loads a DLL that verifies the executable's cryptographic hash at runtime.

## Architecture

```
Build Time:
  sigtool.exe  ──computes hash──>  signature_data.h
                                          │
  seon.exe  <─────────────── seon.dll (embedded hashes)

Runtime:
  seon.exe ──LoadLibrary()──> seon.dll
                                    │
                                    ├── Read seon.exe from disk
                                    ├── Compute MD5 + SHA256
                                    └── Compare with embedded values
```

## File Structure

| Path | Purpose |
|------|---------|
| `src/main.cpp` | Application entry, loads seon.dll |
| `protector/include/seon.h` | Public API exports |
| `protector/include/checker.h` | Internal checker interface |
| `protector/src/dllmain.cpp` | DLL implementation |
| `protector/src/checker.cpp` | Hash computation (WinCrypto) |
| `tools/sigtool/src/sigtool.cpp` | Hash generator for build |
| `output/` | Build outputs (exe, dll, headers) |

## Build Flow

1. `meson.build` triggers subdirs in order:
   - `tools/sigtool` → builds sigtool.exe
   - `src/` → builds seon.exe
   - `protector/` → placeholder
   
2. Custom target `signature_data_h`:
   - Input: seon.exe
   - Runs: sigtool.exe on seon.exe
   - Output: signature_data.h (contains expected_md5[], expected_sha256[])

3. Final target `seon_dll`:
   - Compiles protector/src/*.cpp with signature_data.h
   - Defines: `-DSEON_HAS_SIGNATURE`
   - Output: seon.dll

4. Scripts copy all outputs to `output/` folder

## Key Implementation Details

### Hash Computation (WinCrypto)
```cpp
HCRYPTPROV hProv;
CryptAcquireContext(&hProv, nullptr, nullptr, PROV_RSA_AES, CRYPT_VERIFYCONTEXT);
CryptCreateHash(hProv, CALG_SHA_256, 0, 0, &hHash);
CryptHashData(hHash, data, len, 0);
CryptGetHashParam(hHash, HP_HASHVAL, hash, &hashLen, 0);
```

### Constant-Time Comparison
```cpp
volatile uint8_t result = 0;
for (size_t i = 0; i < len; ++i) {
    result |= expected[i] ^ computed[i];
}
return result == 0;  // No early exit = constant time
```

### Export Macro
```cpp
#ifndef SEON_API
#ifdef SEON_BUILD_DLL
#define SEON_API __declspec(dllexport)
#else
#define SEON_API __declspec(dllimport)
#endif
#endif
```

## Build Commands

```batch
# From any terminal (auto-loads VS env)
scripts\rebuild.bat

# Individual steps
scripts\setup.bat    # meson setup build
scripts\build.bat    # ninja build + copy to output/
scripts\run.bat      # run output\seon.exe
scripts\clean.bat    # remove build/ and output/
```

## Modifying Code

### Adding new hash algorithm
1. Update `HashEngine` in `protector/src/checker.cpp`
2. Add field to `seon::signature` namespace
3. Update sigtool to generate the new hash
4. Add check in `Checker::verify()`

### Changing API
1. Update `protector/include/seon.h` (extern "C" block)
2. Implement in `protector/src/dllmain.cpp`
3. Update `src/main.cpp` to use new API
4. Rebuild: `scripts\rebuild.bat`

### Adding build config
- Edit `meson.build` for compile flags
- Use `meson configure` for options
- CPP args: `-DSEON_BUILD_DLL`, `-DSEON_HAS_SIGNATURE`

## Common Issues

| Issue | Cause | Fix |
|-------|-------|-----|
| `dllimport` errors | Missing `SEON_BUILD_DLL` define | Ensure defined in source or build |
| Signature mismatch | Built seon.exe changed after header gen | Rebuild: `rebuild.bat` |
| "Cannot open file" | File locked by running process | `taskkill /f /im seon.exe` |
| CL not found | VS env not loaded | Script auto-loads, or run from Dev Prompt |

## Testing

Run and verify output:
```
SEON Core - Protected Application
[INFO] Initialized.
[RESULT] Status: OK
[RESULT] Message: Integrity verification passed
Integrity verification PASSED
```

## Dependencies

- Windows SDK (for WinCrypt)
- Visual Studio 2022 (C++20)
- Python + meson + ninja
- No external crypto libs (uses Windows CryptoAPI)
