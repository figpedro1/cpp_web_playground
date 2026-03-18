#include <iostream>
#include "utils.hpp"

static const std::string base64_chars = 
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz"
            "0123456789+/";

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

}