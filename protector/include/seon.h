#ifndef SEON_H
#define SEON_H

#include <windows.h>
#include <cstdint>

#ifndef SEON_API
#ifdef SEON_BUILD_DLL
#define SEON_API __declspec(dllexport)
#else
#define SEON_API __declspec(dllimport)
#endif
#endif

namespace seon {

enum class Result : uint32_t {
    OK = 0,
    INVALID_HASH = 1,
    FILE_NOT_FOUND = 2,
    MODULE_NOT_FOUND = 3,
    INTERNAL_ERROR = 4,
    SIGNATURE_MISSING = 5,
};

struct Options {
    bool check_md5 = true;
    bool check_sha256 = true;
    bool exit_on_fail = true;
    uint32_t exit_code = 0xDEAD;
};

struct Context {
    Result result;
    char message[256];
    uint8_t computed_md5[16];
    uint8_t computed_sha256[32];
};

}

extern "C" {

SEON_API BOOL __cdecl seon_init(void);
SEON_API void __cdecl seon_cleanup(void);
SEON_API seon::Result __cdecl seon_verify(const seon::Options* options, seon::Context* context);
SEON_API const char* __cdecl seon_result_str(seon::Result result);
SEON_API BOOL __cdecl seon_quick_check(void);

}

#endif
