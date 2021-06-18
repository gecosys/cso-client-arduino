#ifndef _UTILS_BIGNUM_H_
#define _UTILS_BIGNUM_H_

#include <string>
#include <mbedtls/bignum.h>
#include "utils/result.h"
#include "error/error_code.h"

class BigNum {
private:
    mbedtls_mpi core;

private:
    static Error::Code initFromString(mbedtls_mpi* out, const char* str, int8_t radix) noexcept;
    static Error::Code initFromBinary(mbedtls_mpi* out, const uint8_t* buffer,  size_t bufflen) noexcept;

public:
    BigNum() noexcept;
    BigNum(BigNum&& other) noexcept;
    BigNum(const BigNum& other) = delete;
    ~BigNum() noexcept;
    
    BigNum& operator=(const BigNum& other) = delete;
    BigNum& operator=(BigNum&& other) noexcept;
    
    Error::Code setNumber(const int32_t n) noexcept;
    Error::Code setString(const char* str, int8_t radix = 10) noexcept;
    
    Error::Code assign(const BigNum& other) noexcept;

    Error::Code addAndAssign(const BigNum& n) noexcept;
    Error::Code subAndAssign(const BigNum& n) noexcept;
    Error::Code mulAndAssign(const BigNum& n) noexcept;
    Error::Code divAndAssign(const BigNum& n) noexcept;
    Error::Code modAndAssign(const BigNum& n) noexcept;

    Error::Code addAndAssign(int32_t n) noexcept;
    Error::Code subAndAssign(int32_t n) noexcept;
    Error::Code mulAndAssign(int32_t n) noexcept;
    Error::Code divAndAssign(int32_t n) noexcept;
    Error::Code modAndAssign(int32_t n) noexcept;
    
    Result<BigNum> add(const BigNum& n) const noexcept;
    Result<BigNum> sub(const BigNum& n) const noexcept;
    Result<BigNum> mul(const BigNum& n) const noexcept;
    Result<BigNum> div(const BigNum& n) const noexcept;
    Result<BigNum> mod(const BigNum& n) const noexcept;

    Result<BigNum> add(int32_t n) const noexcept;
    Result<BigNum> sub(int32_t n) const noexcept;
    Result<BigNum> mul(int32_t n) const noexcept;
    Result<BigNum> div(int32_t n) const noexcept;
    Result<BigNum> mod(int32_t n) const noexcept;
    
    bool operator<  (const BigNum& rhs) const noexcept;
    bool operator<  (const int32_t rhs) const noexcept;
    bool operator>  (const BigNum& rhs) const noexcept;
    bool operator>  (const int32_t rhs) const noexcept;
    bool operator<= (const BigNum& rhs) const noexcept;
    bool operator<= (const int32_t rhs) const noexcept;
    bool operator>= (const BigNum& rhs) const noexcept;
    bool operator>= (const int32_t rhs) const noexcept;
    bool operator!= (const BigNum& rhs) const noexcept;
    bool operator!= (const int32_t rhs) const noexcept;
    bool operator== (const BigNum& rhs) const noexcept;
    bool operator== (const int32_t rhs) const noexcept;
    
    Result<BigNum> powMod(const BigNum& power, const BigNum& modulus) const noexcept;
    Result<std::string> toString(int8_t radix = 10) const noexcept;
};

#endif // _UTILS_BIGNUM_H_