#ifndef ENTITY_BIGINT_H
#define ENTITY_BIGINT_H

#include <tuple>
#include <string>
#include <mbedtls/bignum.h>
#include "error/error.h"

class BigInt {
private:
    mbedtls_mpi core;

public:
    BigInt() noexcept;
    BigInt(BigInt&& other) noexcept;
    BigInt(const BigInt& other) = delete;
    ~BigInt() noexcept;
    
    BigInt& operator=(const BigInt& rhs) = delete;
    BigInt& operator=(BigInt&& rhs) noexcept;
    
    Error copy(const BigInt& other);
    Error setNumber(const int32_t n);
    Error setString(const std::string& str, int8_t radix = 10);
   
    std::tuple<Error, BigInt> powMod(const BigInt& power, const BigInt& modulus) const;
    std::tuple<Error, std::string> toString(int8_t radix = 10) const;
};

#endif // !ENTITY_BIGINT_H