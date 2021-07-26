#ifndef ERROR_H
#define ERROR_H

#include <tuple>
#include <string>
#include <cstdint>

// The exception handling:
//      1. Just throw an exception in constructors and some operators if it is really necessary.
//         Return error codes in other methods.
//      2. Should throw a string instead of a std::exception 
//         because a std:exception can throw another unexpected error especially from its copy constructors.
//      3. The thrown string should include the error location (class, function) and 
//         the error reason for easy tracking
// The reason for doing this is that throwing exceptions is discouraged especially in embedded systems.

class Error {
public:
    // Because OpenSSL use 32 bits for error code, We need 64 bits to adapt it
    // This enum will define general error codes
    // Specific errors will be defined by package in "package"
    enum Code : uint64_t {
        Nil = 0U,
    };

private:
    static std::tuple<uint8_t, uint8_t, uint8_t, uint32_t> parseCode(Error::Code code) noexcept;

public:
    static Error::Code buildCode(uint8_t packID, uint8_t funcID, int32_t errCode, uint8_t extID = 0) noexcept;
    static std::string getString(Error::Code code) noexcept;
};

#endif // !ERROR_H