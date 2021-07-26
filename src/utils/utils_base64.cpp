extern "C" {
    #include <libb64/cdecode.h>
    #include <libb64/cencode.h>
}
#include <cstring>
#include "utils/utils_base64.h"

std::string UtilsBase64::encode(const Array<uint8_t>& data) {
    size_t lenBuffer = base64_encode_expected_len(data.length()) + 1;
    char* buffer = new (std::nothrow) char[lenBuffer];
    if(buffer == nullptr) {
        return "";
    }

    base64_encodestate state;
    base64_init_encodestate(&state);
    auto seek = base64_encode_block((const char*)data.get(), data.length(), buffer, &state);
    base64_encode_blockend(buffer + seek, &state);
    std::string encodeData(buffer);
    delete[] buffer;
    return encodeData;
}

Array<uint8_t> UtilsBase64::decode(const std::string& data) {
    size_t lenBuffer = base64_decode_expected_len(data.length()) + 1;
    uint8_t* buffer = new (std::nothrow) uint8_t[lenBuffer];
    if(buffer == nullptr) {
        return Array<uint8_t>();
    }

    base64_decodestate state;
    base64_init_decodestate(&state);
    auto lenExpected = base64_decode_block(data.c_str(), data.length(), (char*)buffer, &state);
    return Array<uint8_t>(buffer, lenExpected);
}