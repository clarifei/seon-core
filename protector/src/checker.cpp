#include "checker.h"
#include "signature_data.h"
#include <windows.h>
#include <wincrypt.h>
#include <cstdio>

#pragma comment(lib, "advapi32.lib")

namespace seon {

static bool read_file(const std::wstring& filepath, std::vector<uint8_t>& data) {
    HANDLE hFile = CreateFileW(filepath.c_str(), GENERIC_READ, FILE_SHARE_READ,
                                nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile == INVALID_HANDLE_VALUE) {
        return false;
    }

    LARGE_INTEGER fileSize;
    if (!GetFileSizeEx(hFile, &fileSize)) {
        CloseHandle(hFile);
        return false;
    }

    data.resize(static_cast<size_t>(fileSize.QuadPart));
    DWORD bytesRead = 0;
    BOOL result = ReadFile(hFile, data.data(), static_cast<DWORD>(data.size()), &bytesRead, nullptr);
    CloseHandle(hFile);

    return result && (bytesRead == data.size());
}

static std::vector<uint8_t> compute_hash(const std::vector<uint8_t>& data, ALG_ID alg_id, DWORD hash_size) {
    std::vector<uint8_t> hash(hash_size);
    HCRYPTPROV hProv = 0;
    HCRYPTHASH hHash = 0;
    DWORD hashLen = hash_size;
    
    DWORD prov_type = (alg_id == CALG_SHA_256) ? PROV_RSA_AES : PROV_RSA_FULL;
    
    if (CryptAcquireContext(&hProv, nullptr, nullptr, prov_type, CRYPT_VERIFYCONTEXT)) {
        if (CryptCreateHash(hProv, alg_id, 0, 0, &hHash)) {
            CryptHashData(hHash, data.data(), static_cast<DWORD>(data.size()), 0);
            CryptGetHashParam(hHash, HP_HASHVAL, hash.data(), &hashLen, 0);
            CryptDestroyHash(hHash);
        }
        CryptReleaseContext(hProv, 0);
    }
    return hash;
}

HashEngine::HashResult HashEngine::compute_file(const std::wstring& filepath) {
    HashResult result;
    std::vector<uint8_t> data;
    
    if (!read_file(filepath, data)) {
        return result;
    }

    result.md5 = compute_hash(data, CALG_MD5, 16);
    result.sha256 = compute_hash(data, CALG_SHA_256, 32);
    result.valid = true;
    return result;
}

HashEngine::HashResult HashEngine::compute_module(HMODULE hModule) {
    wchar_t path[MAX_PATH];
    if (!GetModuleFileNameW(hModule, path, MAX_PATH)) {
        return HashResult{};
    }
    return compute_file(path);
}

Checker::Checker() = default;
Checker::~Checker() = default;

static bool constant_time_compare(const uint8_t* a, const uint8_t* b, size_t len) {
    volatile uint8_t result = 0;
    for (size_t i = 0; i < len; ++i) {
        result |= a[i] ^ b[i];
    }
    return result == 0;
}

static void format_hash_hex(const uint8_t* hash, size_t len, char* out) {
    for (size_t i = 0; i < len; ++i) {
        sprintf_s(out + (i * 2), 3, "%02x", hash[i]);
    }
    out[len * 2] = '\0';
}

static Result verify_single_hash(const char* name, size_t hash_len,
                                  const uint8_t* expected, const uint8_t* computed,
                                  char* msg_buffer, size_t msg_size) {
    if (!constant_time_compare(expected, computed, hash_len)) {
        char exp_str[65], cmp_str[65];
        format_hash_hex(expected, hash_len, exp_str);
        format_hash_hex(computed, hash_len, cmp_str);
        snprintf(msg_buffer, msg_size, "%s mismatch! Expected: %s, Got: %s", name, exp_str, cmp_str);
        return Result::INVALID_HASH;
    }
    return Result::OK;
}

Result Checker::verify(const std::wstring& target_path, const Options& opts, Context& ctx) {
    memset(&ctx, 0, sizeof(ctx));

    #ifndef SEON_HAS_SIGNATURE
    strncpy_s(ctx.message, "Signature data not embedded", sizeof(ctx.message) - 1);
    return Result::SIGNATURE_MISSING;
    #endif

    auto hashes = HashEngine::compute_file(target_path);
    if (!hashes.valid) {
        strncpy_s(ctx.message, "Failed to compute file hash", sizeof(ctx.message) - 1);
        return Result::FILE_NOT_FOUND;
    }

    memcpy(ctx.computed_md5, hashes.md5.data(), 16);
    memcpy(ctx.computed_sha256, hashes.sha256.data(), 32);

    if (opts.check_md5) {
        Result r = verify_single_hash("MD5", 16, signature::expected_md5, ctx.computed_md5,
                                       ctx.message, sizeof(ctx.message));
        if (r != Result::OK) return r;
    }

    if (opts.check_sha256) {
        Result r = verify_single_hash("SHA256", 32, signature::expected_sha256, ctx.computed_sha256,
                                       ctx.message, sizeof(ctx.message));
        if (r != Result::OK) return r;
    }

    strncpy_s(ctx.message, "Integrity verification passed", sizeof(ctx.message) - 1);
    return Result::OK;
}

Result Checker::verify_self(const Options& opts, Context& ctx) {
    wchar_t path[MAX_PATH];
    if (!GetModuleFileNameW(GetModuleHandleW(nullptr), path, MAX_PATH)) {
        strncpy_s(ctx.message, "Failed to get module filename", sizeof(ctx.message) - 1);
        return Result::MODULE_NOT_FOUND;
    }
    return verify(path, opts, ctx);
}

}
