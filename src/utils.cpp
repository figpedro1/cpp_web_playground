#include "utils.hpp"

#include "crow.h"
#include <openssl/evp.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>

static const std::string base64_chars = 
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz"
            "0123456789+/";

namespace fs = std::filesystem;

namespace utils {

    std::string base64_encode(const std::string &in) {
        std::string out;
        out.reserve(((in.size() + 2) / 3) * 4);
        int val = 0, valb = -6;
        for (unsigned char c : in) {
            val = (val << 8) + c;
            valb += 8;
            while (valb >= 0) {
                out.push_back(base64_chars[(val >> valb) & 0x3F]);
                valb -= 6;
            }
        }
        if (valb > -6) {
            out.push_back(base64_chars[((val << 8) >> (valb + 8)) & 0x3F]);
        }
        while (out.size() % 4 != 0) {
            out.push_back('=');
        }
        return out;
    }

    std::vector<fs::directory_entry> scan_dir(fs::path path, bool recursive){
        try {
            if (recursive) {
                return std::vector<fs::directory_entry>(
                    fs::recursive_directory_iterator(path),
                    fs::recursive_directory_iterator{}
                );
            }
            return std::vector<fs::directory_entry>(
                fs::directory_iterator(path),
                fs::directory_iterator{}
            );
        } catch (const fs::filesystem_error& e) {
            CROW_LOG_ERROR << "Filesystem error: " << e.what();
            CROW_LOG_DEBUG << "OS error code: " << e.code().value() << " - " << e.code().message();

            return {};
        }
    }

    std::string calculate_sha1(const fs::path file_path) {
        std::ifstream file(file_path, std::ios::binary);
        if (!file) return "";

        EVP_MD_CTX* ctx = EVP_MD_CTX_new();
        const EVP_MD* md = EVP_sha1();
        EVP_DigestInit_ex(ctx, md, nullptr);

        std::vector<char> buffer(8192);
        while (file.read(buffer.data(), buffer.size()) || file.gcount() > 0) {
            EVP_DigestUpdate(ctx, buffer.data(), file.gcount());
        }

        unsigned char hash[EVP_MAX_MD_SIZE];
        unsigned int length = 0;
        EVP_DigestFinal_ex(ctx, hash, &length);
        EVP_MD_CTX_free(ctx);

        std::stringstream ss;
        for (unsigned int i = 0; i < length; i++) {
            ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
        }
        return ss.str();
    }

    std::time_t get_last_modified(fs::path file_path) {
        auto ftime = fs::last_write_time(file_path);
        return std::chrono::system_clock::to_time_t(
                std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                    ftime - decltype(ftime)::clock::now() + std::chrono::system_clock::now()
                )
            );
    }
}