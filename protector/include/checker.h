#ifndef SEON_CHECKER_H
#define SEON_CHECKER_H

#include "seon.h"
#include <vector>
#include <string>

namespace seon {

class HashEngine {
public:
    struct HashResult {
        std::vector<uint8_t> md5;
        std::vector<uint8_t> sha256;
        bool valid = false;
    };

    static HashResult compute_file(const std::wstring& filepath);
    static HashResult compute_module(HMODULE hModule);
};

class Checker {
public:
    Checker();
    ~Checker();

    Result verify(const std::wstring& target_path, const Options& opts, Context& ctx);
    Result verify_self(const Options& opts, Context& ctx);
};

}

#endif
