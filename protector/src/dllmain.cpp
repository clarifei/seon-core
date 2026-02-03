#ifndef SEON_BUILD_DLL
#define SEON_BUILD_DLL
#endif

#include "seon.h"
#include "checker.h"
#include <windows.h>
#include <cstdio>

using namespace seon;

static Checker* g_checker = nullptr;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        g_checker = new Checker();
    } else if (reason == DLL_PROCESS_DETACH) {
        delete g_checker;
        g_checker = nullptr;
    }
    return TRUE;
}

SEON_API BOOL __cdecl seon_init(void) {
    if (!g_checker) {
        g_checker = new Checker();
    }
    return g_checker != nullptr;
}

SEON_API void __cdecl seon_cleanup(void) {
    delete g_checker;
    g_checker = nullptr;
}

SEON_API Result __cdecl seon_verify(const Options* options, Context* context) {
    if (!g_checker || !context) {
        return Result::INTERNAL_ERROR;
    }

    Options opts = options ? *options : Options{};
    Context ctx = {};
    
    auto result = g_checker->verify_self(opts, ctx);
    
    *context = ctx;
    context->result = result;
    return result;
}

SEON_API const char* __cdecl seon_result_str(Result result) {
    switch (result) {
        case Result::OK:               return "OK";
        case Result::INVALID_HASH:     return "Invalid Hash";
        case Result::FILE_NOT_FOUND:   return "File Not Found";
        case Result::MODULE_NOT_FOUND: return "Module Not Found";
        case Result::INTERNAL_ERROR:   return "Internal Error";
        case Result::SIGNATURE_MISSING: return "Signature Missing";
        default:                       return "Unknown";
    }
}

SEON_API BOOL __cdecl seon_quick_check(void) {
    if (!g_checker) {
        return FALSE;
    }

    Options opts = { true, true, true, 0xDEAD };
    Context ctx;
    auto result = g_checker->verify_self(opts, ctx);

    if (result != Result::OK && opts.exit_on_fail) {
        char msg[512];
        snprintf(msg, sizeof(msg), "[seon] Integrity check failed: %s\n", ctx.message);
        OutputDebugStringA(msg);
        ExitProcess(opts.exit_code);
    }

    return result == Result::OK;
}
