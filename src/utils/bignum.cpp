#include <memory>
#include <cstring>
#include <esp32-hal-log.h>
#include <mbedtls/error.h>
#include "utils/bignum.h"

#define LENGTH_ERROR 150

BigNum::BigNum() noexcept {
    mbedtls_mpi_init(&this->core);
}

BigNum::BigNum(const char* str, int8_t radix) noexcept {
    mbedtls_mpi_init(&this->core);
    auto errorCode = mbedtls_mpi_read_string(&this->core, radix, str);
    if (errorCode == 0) {
        return;
    }
    char error[LENGTH_ERROR];
    mbedtls_strerror(errorCode, error, LENGTH_ERROR);
    log_e("%s", error);
}

BigNum::BigNum(const int32_t n, int8_t radix) noexcept {
    mbedtls_mpi_init(&this->core);
    // Convert little edian to big edian
    int32_t value = (n >> 24U) | 
                    ((n << 8U) & 0xFF0000U) | 
                    ((n >> 8U) & 0xFF00U) | 
                    (n << 24U);
    auto errorCode = mbedtls_mpi_read_binary(&this->core, (uint8_t*)&value, sizeof(int32_t));
    if (errorCode == 0) {
        return;
    }
    char error[LENGTH_ERROR];
    mbedtls_strerror(errorCode, error, LENGTH_ERROR);
    log_e("%s", error);
}

BigNum::BigNum(const BigNum& rhs) noexcept {
    this->core.p = new (std::nothrow) mbedtls_mpi_uint[rhs.core.n];
    if (this->core.p == nullptr) {
        log_e("[BigNum][CopyConstructor]Not enough memory for allocating");
        return;
    }
    this->core.n = rhs.core.n;
    this->core.s = rhs.core.s;
    memcpy(this->core.p, rhs.core.p, rhs.core.n * sizeof(mbedtls_mpi_uint));
}

BigNum::BigNum(BigNum&& rhs) noexcept {
    mbedtls_mpi_init(&this->core);
    std::swap(this->core, rhs.core);
}

BigNum::~BigNum() noexcept {
    mbedtls_mpi_free(&this->core);
}

BigNum& BigNum::operator=(const BigNum& rhs) noexcept {
    mbedtls_mpi_uint* temp = new (std::nothrow) mbedtls_mpi_uint[rhs.core.n];
    if (temp == nullptr) {
        log_e("[BigNum][Operator=]Not enough memory for allocating");
        return *this;
    }
    mbedtls_mpi_free(&this->core);
    this->core.n = rhs.core.n;
    this->core.s = rhs.core.s;
    this->core.p = temp;
    memcpy(this->core.p, rhs.core.p, rhs.core.n);
    return *this;
}
    
BigNum& BigNum::operator+=(const BigNum& n) noexcept {
    auto errorCode = mbedtls_mpi_add_mpi(&this->core, &this->core, &n.core);
    if (errorCode == 0) {
        return *this;
    }

    char error[LENGTH_ERROR];
    mbedtls_strerror(errorCode, error, LENGTH_ERROR);
    log_e("%s", error);
    return *this;
}

BigNum& BigNum::operator-=(const BigNum& n) noexcept {
    auto errorCode = mbedtls_mpi_sub_mpi(&this->core, &this->core, &n.core);
    if (errorCode == 0) {
        return *this;
    }
    char error[LENGTH_ERROR];
    mbedtls_strerror(errorCode, error, LENGTH_ERROR);
    log_e("%s", error);
    return *this;
}

BigNum& BigNum::operator/=(const BigNum& n) noexcept {
    auto errorCode = mbedtls_mpi_div_mpi(&this->core, nullptr, &this->core, &n.core);
    if (errorCode == 0) {
        return *this;
    }
    char error[LENGTH_ERROR];
    mbedtls_strerror(errorCode, error, LENGTH_ERROR);
    log_e("%s", error);
    return *this;
}

BigNum& BigNum::operator*=(const BigNum& n) noexcept {
    auto errorCode = mbedtls_mpi_mul_mpi(&this->core, &this->core, &n.core);
    if (errorCode == 0) {
        return *this;
    }
    char error[LENGTH_ERROR];    
    mbedtls_strerror(errorCode, error, LENGTH_ERROR);
    log_e("%s", error);
    return *this;
}

BigNum& BigNum::operator%=(const BigNum& n) noexcept {
    auto errorCode = mbedtls_mpi_mod_mpi(&this->core, &this->core, &n.core);
    if (errorCode == 0) {
        return *this;
    }
    char error[LENGTH_ERROR];
    mbedtls_strerror(errorCode, error, LENGTH_ERROR);
    log_e("%s", error);
    return *this;
}

BigNum BigNum::operator+(const BigNum& n) const noexcept {
    BigNum temp = *this; 
    temp += n; 
    return temp; 
} 

BigNum BigNum::operator-(const BigNum& n) const noexcept {
    BigNum temp = *this; 
    temp -= n; 
    return temp;
};

BigNum BigNum::operator/(const BigNum& n) const noexcept {
    BigNum temp = *this; 
    temp /= n; 
    return temp;
};

BigNum BigNum::operator*(const BigNum& n) const noexcept {
    BigNum temp = *this; 
    temp *= n; 
    return temp; 
};

BigNum BigNum::operator%(const BigNum& n) const noexcept {
    BigNum temp = *this; 
    temp %= n; 
    return temp;
};

BigNum& BigNum::operator++() noexcept { 
    *this += 1; 
    return *this;
}

BigNum& BigNum::operator--() noexcept {
    *this -= 1; 
    return *this;
}

BigNum BigNum::operator++(int32_t) noexcept {
    BigNum temp = *this; 
    *this += 1; 
    return temp;
}

BigNum BigNum::operator--(int32_t) noexcept {
    BigNum temp = *this; 
    *this -= 1; 
    return temp;
}

bool BigNum::operator<(const BigNum& rhs) const noexcept {
    return mbedtls_mpi_cmp_mpi(&this->core, &rhs.core) == -1;
}

bool BigNum::operator<(const int32_t rhs) const noexcept {
    return mbedtls_mpi_cmp_int(&this->core, rhs) == -1;
}

bool BigNum::operator>(const BigNum& rhs) const noexcept {
    return mbedtls_mpi_cmp_mpi(&this->core, &rhs.core) == 1;
}

bool BigNum::operator>(const int32_t rhs) const noexcept {
    return mbedtls_mpi_cmp_int(&this->core, rhs) == 1;
}

bool BigNum::operator<=(const BigNum& rhs) const noexcept {
    return mbedtls_mpi_cmp_mpi(&this->core, &rhs.core) <= 0;
}

bool BigNum::operator<=(const int32_t rhs) const noexcept {
    return mbedtls_mpi_cmp_int(&this->core, rhs) <= 0;
}

bool BigNum::operator>=(const BigNum& rhs) const noexcept {
    return mbedtls_mpi_cmp_mpi(&this->core, &rhs.core) >= 0;
}

bool BigNum::operator>=(const int32_t rhs) const noexcept {
    return mbedtls_mpi_cmp_int(&this->core, rhs) >= 0;
}

bool BigNum::operator!=(const BigNum& rhs) const noexcept {
    return mbedtls_mpi_cmp_mpi(&this->core, &rhs.core) != 0;
}

bool BigNum::operator!=(const int32_t rhs) const noexcept {
    return mbedtls_mpi_cmp_int(&this->core, rhs) != 0;
}

bool BigNum::operator==(const BigNum& rhs) const noexcept {
    return mbedtls_mpi_cmp_mpi(&this->core, &rhs.core) == 0;
}

bool BigNum::operator==(const int32_t rhs) const noexcept {
    return mbedtls_mpi_cmp_int(&this->core, rhs) == 0;
}

BigNum BigNum::powMod(const BigNum& power, const BigNum& modulus) const  noexcept {
    BigNum ret;
    auto errorCode = mbedtls_mpi_exp_mod(&ret.core, &this->core, &power.core, &modulus.core, nullptr);
    if (errorCode == 0) {
        return ret;
    }
    char error[LENGTH_ERROR];
    mbedtls_strerror(errorCode, error, LENGTH_ERROR);
    log_e("%s", error);
    return ret;
}

std::string BigNum::toString(int8_t radix) const  noexcept {
    // Get estimating length
    size_t lenBuffer = 0;
    mbedtls_mpi_write_string(&this->core, radix, nullptr, 0, &lenBuffer);
    if (lenBuffer == 0) {
        return std::string("");
    }

    std::unique_ptr<char> buffer(new (std::nothrow) char[lenBuffer]);
    if (buffer == nullptr) {
        return std::string("");
    }

    if (mbedtls_mpi_write_string(&this->core, radix, buffer.get(), lenBuffer, &lenBuffer) != 0) {
        return std::string("");
    }
    return std::string(buffer.get());
}