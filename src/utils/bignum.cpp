#include <memory>
#include <cstring>
#include "utils/bignum.h"

BigNum::BigNum() noexcept {
    mbedtls_mpi_init(&this->core);
}

BigNum::BigNum(BigNum&& rhs) noexcept {
    mbedtls_mpi_init(&this->core);
    std::swap(this->core, rhs.core);
}

BigNum::~BigNum() noexcept {
    mbedtls_mpi_free(&this->core);
}

//===========
// Initilaize
//===========
Error::Code BigNum::setNumber(const int32_t n) noexcept {
    mbedtls_mpi_free(&this->core);
    // Convert little edian to big edian
    int32_t value = (n >> 24U) | 
                    ((n << 8U) & 0xFF0000U) | 
                    ((n >> 8U) & 0xFF00U) | 
                    (n << 24U);
    return BigNum::initFromBinary(&this->core, (uint8_t*)&value, sizeof(int32_t));
}

Error::Code BigNum::setString(const char* str, int8_t radix) noexcept {
    mbedtls_mpi_free(&this->core);
    return BigNum::initFromString(&this->core, str, radix);
}

//=======
// Assign
//=======
BigNum& BigNum::operator=(BigNum&& other) noexcept {
    this->core.n = other.core.n;
    this->core.p = other.core.p;
    this->core.s = other.core.s;
    other.core.p = nullptr;
    return *this;
}

Error::Code BigNum::assign(const BigNum& rhs) noexcept {
    mbedtls_mpi_uint* temp = new (std::nothrow) mbedtls_mpi_uint[rhs.core.n];
    if (temp == nullptr) {
        return Error::NotEnoughMemory;
    }
    mbedtls_mpi_free(&this->core);
    this->core.n = rhs.core.n;
    this->core.s = rhs.core.s;
    this->core.p = temp;
    memcpy(this->core.p, rhs.core.p, rhs.core.n);
    return Error::Nil;
}
    
//=======================
// Calculating and assign
//=======================
Error::Code BigNum::addAndAssign(const BigNum& n) noexcept {
    auto errorCode = mbedtls_mpi_add_mpi(&this->core, &this->core, &n.core);
    if (errorCode == 0) {
        return Error::Nil;
    }
    return Error::adaptExternalCode(Location::Utils_BigNum, ExternalTag::MbedTLS, errorCode);
}

Error::Code BigNum::subAndAssign(const BigNum& n) noexcept {
    auto errorCode = mbedtls_mpi_sub_mpi(&this->core, &this->core, &n.core);
    if (errorCode == 0) {
        return Error::Nil;
    }
    return Error::adaptExternalCode(Location::Utils_BigNum, ExternalTag::MbedTLS, errorCode);
}

Error::Code BigNum::mulAndAssign(const BigNum& n) noexcept {
    auto errorCode = mbedtls_mpi_mul_mpi(&this->core, &this->core, &n.core);
    if (errorCode == 0) {
        return Error::Nil;
    }
    return Error::adaptExternalCode(Location::Utils_BigNum, ExternalTag::MbedTLS, errorCode);
}

Error::Code BigNum::divAndAssign(const BigNum& n) noexcept {
    auto errorCode = mbedtls_mpi_div_mpi(&this->core, nullptr, &this->core, &n.core);
    if (errorCode == 0) {
        return Error::Nil;
    }
    return Error::adaptExternalCode(Location::Utils_BigNum, ExternalTag::MbedTLS, errorCode);
}

Error::Code BigNum::modAndAssign(const BigNum& n) noexcept {
    auto errorCode = mbedtls_mpi_mod_mpi(&this->core, &this->core, &n.core);
    if (errorCode == 0) {
        return Error::Nil;
    }
    return Error::adaptExternalCode(Location::Utils_BigNum, ExternalTag::MbedTLS, errorCode);
}

Error::Code BigNum::addAndAssign(int32_t n) noexcept {
    auto errorCode = mbedtls_mpi_add_int(&this->core, &this->core, n);
    if (errorCode == 0) {
        return Error::Nil;
    }
    return Error::adaptExternalCode(Location::Utils_BigNum, ExternalTag::MbedTLS, errorCode);
}

Error::Code BigNum::subAndAssign(int32_t n) noexcept {
    auto errorCode = mbedtls_mpi_sub_int(&this->core, &this->core, n);
    if (errorCode == 0) {
        return Error::Nil;
    }
    return Error::adaptExternalCode(Location::Utils_BigNum, ExternalTag::MbedTLS, errorCode);
}

Error::Code BigNum::mulAndAssign(int32_t n) noexcept {
    auto errorCode = mbedtls_mpi_mul_int(&this->core, &this->core, n);
    if (errorCode != 0) {
        return Error::Nil;
    }
    return Error::adaptExternalCode(Location::Utils_BigNum, ExternalTag::MbedTLS, errorCode);
}

Error::Code BigNum::divAndAssign(int32_t n) noexcept {
    auto errorCode = mbedtls_mpi_div_int(&this->core, nullptr, &this->core, n);
    if (errorCode == 0) {
        return Error::Nil;
    }
    return Error::adaptExternalCode(Location::Utils_BigNum, ExternalTag::MbedTLS, errorCode);
}

Error::Code BigNum::modAndAssign(int32_t n) noexcept {
    uint32_t result;
    auto errorCode = mbedtls_mpi_mod_int(&result, &this->core, n);
    if (errorCode != 0) {
        return Error::adaptExternalCode(Location::Utils_BigNum, ExternalTag::MbedTLS, errorCode);
    }

    mbedtls_mpi_free(&this->core);
    return BigNum::initFromBinary(&this->core, (uint8_t*)&result, sizeof(uint32_t));
}

//============
// Calculating
//============
Result<BigNum> BigNum::add(const BigNum& n) const noexcept {
    BigNum result;
    auto errorCode = mbedtls_mpi_add_mpi(&result.core, &this->core, &n.core);
    if (errorCode == 0) {
        return make_result(Error::Nil, std::move(result));
    }
    return make_result(
        Error::adaptExternalCode(Location::Utils_BigNum, ExternalTag::MbedTLS, errorCode), 
        BigNum()
    );
}

Result<BigNum> BigNum::sub(const BigNum& n) const noexcept {
    BigNum result;
    auto errorCode = mbedtls_mpi_sub_mpi(&result.core, &this->core, &n.core);
    if (errorCode == 0) {
        return make_result(Error::Nil, std::move(result));
    }
    return make_result(
        Error::adaptExternalCode(Location::Utils_BigNum, ExternalTag::MbedTLS, errorCode), 
        BigNum()
    );
}

Result<BigNum> BigNum::mul(const BigNum& n) const noexcept {
    BigNum result;
    auto errorCode = mbedtls_mpi_mul_mpi(&result.core, &this->core, &n.core);
    if (errorCode == 0) {
        return make_result(Error::Nil, std::move(result));
    }
    return make_result(
        Error::adaptExternalCode(Location::Utils_BigNum, ExternalTag::MbedTLS, errorCode), 
        BigNum()
    );
}

Result<BigNum> BigNum::div(const BigNum& n) const noexcept {
    BigNum result;
    auto errorCode = mbedtls_mpi_div_mpi(&result.core, nullptr, &this->core, &n.core);
    if (errorCode == 0) {
        return make_result(Error::Nil, std::move(result));
    }
    return make_result(
        Error::adaptExternalCode(Location::Utils_BigNum, ExternalTag::MbedTLS, errorCode), 
        BigNum()
    );
}

Result<BigNum> BigNum::mod(const BigNum& n) const noexcept {
    BigNum result;
    auto errorCode = mbedtls_mpi_mod_mpi(&result.core, &this->core, &n.core);
    if (errorCode == 0) {
        return make_result(Error::Nil, std::move(result));
    }
    return make_result(
        Error::adaptExternalCode(Location::Utils_BigNum, ExternalTag::MbedTLS, errorCode), 
        BigNum()
    );
}

Result<BigNum> BigNum::add(int32_t n) const noexcept {
    BigNum result;
    auto errorCode = mbedtls_mpi_add_int(&result.core, &this->core, n);
    if (errorCode == 0) {
        return make_result(Error::Nil, std::move(result));
    }
    return make_result(
        Error::adaptExternalCode(Location::Utils_BigNum, ExternalTag::MbedTLS, errorCode), 
        BigNum()
    );
}

Result<BigNum> BigNum::sub(int32_t n) const noexcept {
    BigNum result;
    auto errorCode = mbedtls_mpi_sub_int(&result.core, &this->core, n);
    if (errorCode == 0) {
        return make_result(Error::Nil, std::move(result));
    }
    return make_result(
        Error::adaptExternalCode(Location::Utils_BigNum, ExternalTag::MbedTLS, errorCode), 
        BigNum()
    );
}

Result<BigNum> BigNum::mul(int32_t n) const noexcept {
    BigNum result;
    auto errorCode = mbedtls_mpi_mul_int(&result.core, &this->core, n);
    if (errorCode == 0) {
        return make_result(Error::Nil, std::move(result));
    }
    return make_result(
        Error::adaptExternalCode(Location::Utils_BigNum, ExternalTag::MbedTLS, errorCode), 
        BigNum()
    );
}

Result<BigNum> BigNum::div(int32_t n) const noexcept {
    BigNum result;
    auto errorCode = mbedtls_mpi_div_int(&result.core, nullptr, &this->core, n);
    if (errorCode == 0) {
        return make_result(Error::Nil, std::move(result));
    }
    return make_result(
        Error::adaptExternalCode(Location::Utils_BigNum, ExternalTag::MbedTLS, errorCode), 
        BigNum()
    );
}

Result<BigNum> BigNum::mod(int32_t n) const noexcept {
    uint32_t r;
    auto errorCode = mbedtls_mpi_mod_int(&r, &this->core, n);
    if (errorCode != 0) {
        return make_result(Error::adaptExternalCode(Location::Utils_BigNum, ExternalTag::MbedTLS, errorCode), BigNum());
    }

    BigNum result;
    errorCode = BigNum::initFromBinary(&result.core, (uint8_t*)&r, sizeof(uint32_t));
    if (errorCode == Error::Nil) {
        return make_result(Error::Nil, std::move(result));
    }
    return make_result(
        Error::adaptExternalCode(Location::Utils_BigNum, ExternalTag::MbedTLS, errorCode), 
        BigNum()
    );
}

//========
// Compare
//========
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

//======
// Other
//======
Result<BigNum> BigNum::powMod(const BigNum& power, const BigNum& modulus) const  noexcept {
    BigNum result;
    auto errorCode = mbedtls_mpi_exp_mod(&result.core, &this->core, &power.core, &modulus.core, nullptr);
    if (errorCode == 0) {
        return make_result(Error::Nil, std::move(result));
    }
    return make_result(
        Error::adaptExternalCode(Location::Utils_BigNum, ExternalTag::MbedTLS, errorCode), 
        BigNum()
    );
}

Result<std::string> BigNum::toString(int8_t radix) const  noexcept {
    // Get estimating length
    size_t lenBuffer = 0;
    mbedtls_mpi_write_string(&this->core, radix, nullptr, 0, &lenBuffer);
    if (lenBuffer == 0) {
        return make_result(Error::Nil, std::string(""));
    }

    std::unique_ptr<char> buffer(new (std::nothrow) char[lenBuffer]);
    if (buffer == nullptr) {
        return make_result(Error::NotEnoughMemory, std::string(""));
    }

    auto errorCode = mbedtls_mpi_write_string(&this->core, radix, buffer.get(), lenBuffer, &lenBuffer);
    if (errorCode != 0) {
        return make_result(
            Error::adaptExternalCode(Location::Utils_BigNum, ExternalTag::MbedTLS, errorCode), 
            std::string("")
        );
    }
    return make_result(Error::Nil, std::string(buffer.get()));
}

//========
// Private
//========
Error::Code BigNum::initFromString(mbedtls_mpi* out, const char* str, int8_t radix) noexcept {
    mbedtls_mpi_init(out);
    auto errorCode = mbedtls_mpi_read_string(out, radix, str);
    if (errorCode == 0) {
        return Error::Nil;
    }
    return Error::adaptExternalCode(Location::Utils_BigNum, ExternalTag::MbedTLS, errorCode);
}

Error::Code BigNum::initFromBinary(mbedtls_mpi* out, const uint8_t* buffer,  size_t bufflen) noexcept {
    mbedtls_mpi_init(out);
    auto errorCode = mbedtls_mpi_read_binary(out, buffer, bufflen);
    if (errorCode == 0) {
        return Error::Nil;
    }
    return Error::adaptExternalCode(Location::Utils_BigNum, ExternalTag::MbedTLS, errorCode);
}