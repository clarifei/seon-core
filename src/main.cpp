#include <windows.h>
#include <iostream>
#include <cstdio>

typedef BOOL (*seon_init_t)(void);
typedef const char* (*seon_result_str_t)(uint32_t);

typedef struct {
    uint32_t result;
    char message[256];
    uint8_t computed_md5[16];
    uint8_t computed_sha256[32];
} seon_context_t;

typedef uint32_t (*seon_verify_t)(void* options, seon_context_t* context);

static void print_hash(const uint8_t* hash, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        printf("%02x", hash[i]);
    }
}

int main(int argc, char* argv[]) {
    std::cout << "SEON Core - Protected Application" << std::endl << std::endl;

    HMODULE hSeon = LoadLibraryA("seon.dll");
    if (!hSeon) {
        std::cerr << "[ERROR] Failed to load seon.dll" << std::endl;
        return 1;
    }

    auto seon_init = (seon_init_t)GetProcAddress(hSeon, "seon_init");
    auto seon_verify = (seon_verify_t)GetProcAddress(hSeon, "seon_verify");
    auto seon_result_str = (seon_result_str_t)GetProcAddress(hSeon, "seon_result_str");

    if (!seon_init || !seon_verify || !seon_result_str) {
        std::cerr << "[ERROR] Failed to get required functions from DLL." << std::endl;
        FreeLibrary(hSeon);
        return 1;
    }

    if (!seon_init()) {
        std::cerr << "[ERROR] Failed to initialize." << std::endl;
        FreeLibrary(hSeon);
        return 1;
    }

    std::cout << "[INFO] Initialized." << std::endl;
    std::cout << std::endl << "[INFO] Performing integrity check..." << std::endl;
    
    seon_context_t ctx = {};
    uint32_t result = seon_verify(nullptr, &ctx);

    std::cout << "[RESULT] Status: " << seon_result_str(result) << std::endl;
    std::cout << "[RESULT] Message: " << ctx.message << std::endl;

    std::cout << std::endl << "[INFO] Computed MD5: ";
    print_hash(ctx.computed_md5, 16);
    std::cout << std::endl;

    std::cout << "[INFO] Computed SHA256: ";
    print_hash(ctx.computed_sha256, 32);
    std::cout << std::endl;

    std::cout << std::endl << "Integrity verification " << (result == 0 ? "PASSED" : "FAILED") << std::endl;
    std::cout << std::endl << "[INFO] Application running..." << std::endl;
    std::cout << "[INFO] Press Enter to exit." << std::endl;
    std::cin.get();

    FreeLibrary(hSeon);
    return result == 0 ? 0 : 1;
}
