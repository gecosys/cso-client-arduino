extern "C" {
    #include <libb64/cdecode.h>
    #include <libb64/cencode.h>
}
#include <cstring>
#include "utils/utils_base64.h"

std::string UtilsBase64::encode(const uint8_t* data, size_t lenData) {
    size_t lenBuffer = base64_encode_expected_len(lenData) + 1;
    char* buffer = new (std::nothrow) char[lenBuffer];
    if(buffer == nullptr) {
        return "";
    }

    base64_encodestate state;
    base64_init_encodestate(&state);
    auto seek = base64_encode_block((const char*)data, lenData, buffer, &state);
    base64_encode_blockend(buffer + seek, &state);
    std::string encodeData(buffer);
    delete[] buffer;
    return encodeData;
}

Array<uint8_t> UtilsBase64::decode(const char* data, size_t lenData) {
    if (lenData == 0) {
        lenData = strlen(data);
    }
    size_t lenBuffer = base64_decode_expected_len(lenData) + 1;
    uint8_t* buffer = new (std::nothrow) uint8_t[lenBuffer];
    if(buffer == nullptr) {
        return Array<uint8_t>();
    }

    base64_decodestate state;
    base64_init_decodestate(&state);
    auto lenExpected = base64_decode_block(data, lenData, (char*)buffer, &state);
    return Array<uint8_t>(buffer, lenExpected);
}