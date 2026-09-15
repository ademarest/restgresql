#include "utils.h"
#include <boost/regex.hpp>
#include <algorithm>
#include <cctype>

namespace Utils {
    std::string toLowerCamelCase(std::string s) {
        if (s.empty()) return s;

        // If it contains delimiters, it's snake_case or similar; transform it.
        if (s.find('_') != std::string::npos || s.find('-') != std::string::npos) {
            std::transform(s.begin(), s.end(), s.begin(),
                          [](unsigned char c){ return std::tolower(c); });

            boost::regex re("([_-]+[a-z])");
            s = boost::regex_replace(s, re, [](const boost::smatch& sm){
                return std::string(1, std::toupper(static_cast<unsigned char>(sm[1].str().back())));
            });
        }

        // Always ensure the first character is lowercase for lowerCamelCase
        s[0] = std::tolower(static_cast<unsigned char>(s[0]));
        
        return s;
    }

    std::string detectImageContentType(const std::string &data) {
        auto startsWith = [&](std::string_view sig) {
            return data.size() >= sig.size() && data.compare(0, sig.size(), sig) == 0;
        };
        if (startsWith("\x89PNG\r\n\x1a\n")) return "image/png";
        if (startsWith("\xFF\xD8\xFF")) return "image/jpeg";
        if (startsWith("GIF87a") || startsWith("GIF89a")) return "image/gif";
        if (startsWith("BM")) return "image/bmp";
        return "application/octet-stream";
    }
}
