#ifndef ERROR_THIRDPARTY_H
#define ERROR_THIRDPARTY_H

#include <string>

class Thirdparty {
public:
    static std::string getMbedtlsError(int32_t code);
    static std::string getAruduinojsonError(int32_t code);
    static std::string getHttpError(int32_t code);
};

#endif // !ERROR_THIRDPARTY_H