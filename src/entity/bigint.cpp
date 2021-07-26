#include <memory>
#include <cstring>
#include "entity/bigint.h"
#include "error/external.h"
#include "error/package/entity_err.h"

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

Error::Code BigInt::copy(const BigInt& other) {
    auto errcode = mbedtls_mpi_copy(&this->core, &other.core);
    if (errcode == 0) {
        return Error::Code::Nil;
    }

    return Error::buildCode(
        EntityErr::ID,
        EntityErr::Func::BigInt_Copy,
        errcode,
        External::ID::MbedTLS
    );
}

Error::Code BigInt::setNumber(const int32_t n) {
    // Convert little edian to big edian
    int32_t value = (n >> 24U) | 
                    ((n << 8U) & 0xFF0000U) | 
                    ((n >> 8U) & 0xFF00U) | 
                    (n << 24U);

    mbedtls_mpi_free(&this->core);
    mbedtls_mpi_init(&this->core);

    auto errcode = mbedtls_mpi_read_binary(&this->core, (uint8_t*)&value, sizeof(int32_t));
    if (errcode == 0) {
        return Error::Code::Nil;
    }

    return Error::buildCode(
        EntityErr::ID,
        EntityErr::Func::BigInt_SetNumber,
        errcode,
        External::ID::MbedTLS
    );
}

Error::Code BigInt::setString(const std::string& str, int8_t radix) {
    mbedtls_mpi_free(&this->core);
    mbedtls_mpi_init(&this->core);

    auto errcode = mbedtls_mpi_read_string(&this->core, radix, str.c_str());
    if (errcode == 0) {
        return Error::Code::Nil;
    }

    return Error::buildCode(
        EntityErr::ID,
        EntityErr::Func::BigInt_SetString,
        errcode,
        External::ID::MbedTLS
    );
}

std::tuple<Error::Code, BigInt> BigInt::powMod(const BigInt& power, const BigInt& modulus) const {
    BigInt result{};
    auto errcode = mbedtls_mpi_exp_mod(&result.core, &this->core, &power.core, &modulus.core, nullptr);
    if (errcode == 0) {
        return std::make_tuple(Error::Code::Nil, std::move(result));
    }

    return std::make_tuple(
        Error::buildCode(
            EntityErr::ID,
            EntityErr::Func::BigInt_Powmod,
            errcode,
            External::ID::MbedTLS
        ), 
        BigInt{}
    );
}

std::tuple<Error::Code, std::string> BigInt::toString(int8_t radix) const {
    // Get estimating length
    size_t lenBuffer = 0;
    mbedtls_mpi_write_string(&this->core, radix, nullptr, 0, &lenBuffer);
    if (lenBuffer == 0) {
        return std::make_tuple(Error::Code::Nil, "");
    }

    std::unique_ptr<char> buffer{ new char[lenBuffer] };
    auto errcode = mbedtls_mpi_write_string(&this->core, radix, buffer.get(), lenBuffer, &lenBuffer);
    if (errcode == 0) {
        return std::make_tuple(Error::Code::Nil, std::string{ buffer.get() });
    }

    return std::make_tuple(
        Error::buildCode(
            EntityErr::ID,
            EntityErr::Func::BigInt_ToString,
            errcode,
            External::ID::MbedTLS
        ), 
        ""
    );
}