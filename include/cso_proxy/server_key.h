#ifndef _CSO_PROXY_SERVER_KEY_H_
#define _CSO_PROXY_SERVER_KEY_H_

#include "utils/bignum.h"

// ServerKey is a group of server keys
class ServerKey {
public:
    BigNum gKey;
    BigNum nKey;
    BigNum pubKey;

public:
    ServerKey() noexcept;
    ServerKey(ServerKey&& other) noexcept;
    ServerKey(const ServerKey& other) = delete;
    ServerKey(BigNum&& gKey, BigNum&& nKey, BigNum&& pubKey) noexcept;
};

#endif // _CSO_PROXY_SERVER_KEY_H_