#ifndef _UTILS_BIGNUM_H_
#define _UTILS_BIGNUM_H_

#include <string>
#include <mbedtls/bignum.h>

class BigNum {
private:
    mbedtls_mpi core;

public:
    BigNum() noexcept;
    BigNum(const char* str, int8_t radix = 10) noexcept;
    BigNum(const int32_t n, int8_t radix = 10) noexcept;
    BigNum(const BigNum& rhs) noexcept;
    BigNum(BigNum&& rhs) noexcept;
    ~BigNum() noexcept;
    
    BigNum& operator=(const BigNum& rhs) noexcept;
    
    BigNum& operator+=(const BigNum& n) noexcept;
    BigNum& operator-=(const BigNum& n) noexcept;
    BigNum& operator/=(const BigNum& n) noexcept;
    BigNum& operator*=(const BigNum& n) noexcept;
    BigNum& operator%=(const BigNum& n) noexcept;

    BigNum operator+(const BigNum& n) const noexcept;
    BigNum operator-(const BigNum& n) const noexcept;
    BigNum operator/(const BigNum& n) const noexcept;
    BigNum operator*(const BigNum& n) const noexcept;
    BigNum operator%(const BigNum& n) const noexcept;
    
    BigNum& operator++() noexcept;
    BigNum& operator--() noexcept;
    
    BigNum operator++(int32_t) noexcept;
    BigNum operator--(int32_t) noexcept;
    
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
    
    BigNum powMod (const BigNum& power, const BigNum& modulus) const noexcept;
    std::string toString(int8_t radix = 10) const noexcept;
};

#endif // _UTILS_BIGNUM_H_