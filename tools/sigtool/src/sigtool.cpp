#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <string>

#include <windows.h>
#include <wincrypt.h>
#pragma comment(lib, "advapi32.lib")

namespace sigtool {

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

static void print_hash(const std::vector<uint8_t>& hash, const char* label) {
    std::cout << "[sigtool] " << label << ": ";
    for (auto b : hash) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(b);
    }
    std::cout << std::endl;
}

static std::string to_upper(const std::string& str) {
    std::string result = str;
    for (auto& c : result) {
        c = std::toupper(c);
    }
    return result;
}

static bool generate_header(const std::vector<uint8_t>& md5, const std::vector<uint8_t>& sha256,
                            const std::string& output_path, const std::string& target_name) {
    std::ofstream out(output_path);
    if (!out) {
        std::cerr << "[ERROR] Cannot create output file: " << output_path << std::endl;
        return false;
    }

    std::string guard = "SEON_SIGNATURE_" + to_upper(target_name) + "_H";
    
    out << "// Auto-generated signature header for " << target_name << "\n";
    out << "// DO NOT EDIT\n\n";
    out << "#ifndef " << guard << "\n";
    out << "#define " << guard << "\n\n";
    out << "#include <cstdint>\n\n";
    out << "namespace seon {\n";
    out << "namespace signature {\n\n";
    
    auto write_hash_array = [&out](const char* name, const std::vector<uint8_t>& hash, size_t expected_size) {
        out << "constexpr uint8_t " << name << "[" << expected_size << "] = {\n    ";
        for (size_t i = 0; i < hash.size() && i < expected_size; ++i) {
            out << "0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
            if (i < hash.size() - 1) out << ", ";
            if ((i + 1) % 8 == 0 && i < hash.size() - 1) out << "\n    ";
        }
        out << std::dec;  // Reset to decimal
        out << "\n};\n\n";
    };
    
    write_hash_array("expected_md5", md5, 16);
    write_hash_array("expected_sha256", sha256, 32);
    
    out << "constexpr size_t md5_len = 16;\n";
    out << "constexpr size_t sha256_len = 32;\n\n";
    out << "} // namespace signature\n";
    out << "} // namespace seon\n\n";
    out << "#endif // " << guard << "\n";
    
    out.close();
    return true;
}

}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cout << "Usage: " << argv[0] << " <input_exe> <output_header> [target_name]\n";
        return 1;
    }

    std::string input_path = argv[1];
    std::string output_path = argv[2];
    std::string target_name = (argc > 3) ? argv[3] : "target";

    std::cout << "[sigtool] Computing hashes for: " << input_path << std::endl;

    std::ifstream file(input_path, std::ios::binary);
    if (!file) {
        std::cerr << "[ERROR] Cannot open file: " << input_path << std::endl;
        return 1;
    }

    std::vector<uint8_t> data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    auto md5 = sigtool::compute_hash(data, CALG_MD5, 16);
    auto sha256 = sigtool::compute_hash(data, CALG_SHA_256, 32);
    
    if (md5.empty() || sha256.empty()) {
        std::cerr << "[ERROR] Failed to compute hashes!" << std::endl;
        return 1;
    }

    sigtool::print_hash(md5, "MD5");
    sigtool::print_hash(sha256, "SHA256");

    if (sigtool::generate_header(md5, sha256, output_path, target_name)) {
        std::cout << "[sigtool] Signature header generated: " << output_path << std::endl;
        return 0;
    }

    return 1;
}
