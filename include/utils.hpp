#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <ctime>

namespace utils{
    std::string base64_encode(const std::string &in);

    std::vector<std::filesystem::directory_entry> scan_dir(std::filesystem::path path, bool recursive = false);

    std::string calculate_sha1(const std::filesystem::path file_path);
    
    std::time_t get_last_modified(std::filesystem::path file_path);
}