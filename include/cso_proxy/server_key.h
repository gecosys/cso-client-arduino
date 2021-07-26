#ifndef CSO_PROXY_SERVER_KEY_H
#define CSO_PROXY_SERVER_KEY_H

#include "entity/bigint.h"

// ServerKey is a group of server keys
struct ServerKey {
    BigInt gKey;
    BigInt nKey;
    BigInt pubKey;

    ServerKey() noexcept;
    ServerKey(ServerKey&& other) noexcept;
    ServerKey(const ServerKey& other) = delete;
    ServerKey(BigInt&& gKey, BigInt&& nKey, BigInt&& pubKey) noexcept;
    ~ServerKey() noexcept;

    ServerKey& operator=(const ServerKey& other) = default;
    ServerKey& operator=(ServerKey&& other) noexcept;
};

#endif // !CSO_PROXY_SERVER_KEY_H