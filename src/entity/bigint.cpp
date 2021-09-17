#include <memory>
#include <cstring>
#include "entity/bigint.h"
#include "utils/utils_define.h"
#include "error/thirdparty.h"

BigInt::BigInt() noexcept {
    mbedtls_mpi_init(&this->core);
}

BigInt::BigInt(BigInt&& rhs) noexcept {
    mbedtls_mpi_init(&this->core);
    std::swap(this->core, rhs.core);
}

BigInt::~BigInt() noexcept {
    mbedtls_mpi_free(&this->core);
}

BigInt& BigInt::operator=(BigInt&& rhs) noexcept {
    mbedtls_mpi_free(&this->core);
    mbedtls_mpi_init(&this->core);
    std::swap(this->core, rhs.core);
    return *this;
}

Error BigInt::copy(const BigInt& other) {
    auto errcode = mbedtls_mpi_copy(&this->core, &other.core);
    if (errcode != 0) {
        return Error{ GET_FUNC_NAME(), Thirdparty::getMbedtlsError(errcode) };
    }
    return Error{};
}

Error BigInt::setNumber(const int32_t n) {
    // Convert little edian to big edian
    int32_t value = (n >> 24U) | 
                    ((n << 8U) & 0xFF0000U) | 
                    ((n >> 8U) & 0xFF00U) | 
                    (n << 24U);

    mbedtls_mpi_free(&this->core);
    mbedtls_mpi_init(&this->core);

    auto errcode = mbedtls_mpi_read_binary(&this->core, (uint8_t*)&value, sizeof(int32_t));
    if (errcode != 0) {
        return Error{ GET_FUNC_NAME(), Thirdparty::getMbedtlsError(errcode) };
    }
    return Error{};
}

Error BigInt::setString(const std::string& str, int8_t radix) {
    mbedtls_mpi_free(&this->core);
    mbedtls_mpi_init(&this->core);

    auto errcode = mbedtls_mpi_read_string(&this->core, radix, str.c_str());
    if (errcode != 0) {
        return Error{ GET_FUNC_NAME(), Thirdparty::getMbedtlsError(errcode) };
    }
    return Error{};
}

std::tuple<Error, BigInt> BigInt::powMod(const BigInt& power, const BigInt& modulus) const {
    BigInt result{};
    auto errcode = mbedtls_mpi_exp_mod(&result.core, &this->core, &power.core, &modulus.core, nullptr);
    if (errcode == 0) {
        return std::make_tuple(Error{}, std::move(result));
    }

    return std::make_tuple(
        Error{ GET_FUNC_NAME(), Thirdparty::getMbedtlsError(errcode) }, 
        BigInt{}
    );
}

std::tuple<Error, std::string> BigInt::toString(int8_t radix) const {
    // Get estimating length
    size_t lenBuffer = 0;
    mbedtls_mpi_write_string(&this->core, radix, nullptr, 0, &lenBuffer);
    if (lenBuffer == 0) {
        return std::make_tuple(Error{}, "");
    }

    std::unique_ptr<char> buffer{ new char[lenBuffer] };
    auto errcode = mbedtls_mpi_write_string(&this->core, radix, buffer.get(), lenBuffer, &lenBuffer);
    if (errcode == 0) {
        return std::make_tuple(Error{}, std::string{ buffer.get() });
    }
    return std::make_tuple(Error{ GET_FUNC_NAME(), Thirdparty::getMbedtlsError(errcode) }, "");
}