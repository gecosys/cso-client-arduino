#ifndef UTILS_BASE64_H
#define UTILS_BASE64_H

#include <string>
#include "entity/array.h"

class UtilsBase64 {
public:
    static std::string encode(const Array<uint8_t>& data);
    static Array<uint8_t> decode(const std::string& data);
};

#endif // !UTILS_BASE64_H