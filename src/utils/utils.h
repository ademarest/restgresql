#ifndef UTILS_H
#define UTILS_H

#include <string>

namespace Utils {
    std::string toLowerCamelCase(std::string in);

    // Sniffs the file's magic bytes; returns "application/octet-stream" if unrecognized.
    std::string detectImageContentType(const std::string &data);
}

#endif // UTILS_H
