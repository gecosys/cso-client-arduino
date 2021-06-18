#ifndef _UTILS_BASE64_H_
#define _UTILS_BASE64_H_

#include <string>
#include "utils/array.h"

class UtilsBase64 {
public:
    static std::string encode(const uint8_t* data, size_t lenData);
    static Array<uint8_t> decode(const char* data, size_t lenData = 0);
};

#endif // _UTILS_BASE64_H_