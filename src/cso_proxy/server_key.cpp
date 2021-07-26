#include "cso_proxy/server_key.h"

ServerKey::ServerKey() noexcept
  : gKey{},
    nKey{},
    pubKey{} {}

ServerKey::ServerKey(ServerKey&& other) noexcept
  : gKey{ std::move(other.gKey) },
    nKey{ std::move(other.nKey) },
    pubKey{ std::move(other.pubKey) }{}

ServerKey::ServerKey(BigInt&& gKey, BigInt&& nKey, BigInt&& pubKey) noexcept
  : gKey{ std::forward<BigInt>(gKey) },
    nKey{ std::forward<BigInt>(nKey) },
    pubKey{ std::forward<BigInt>(pubKey) }{}

ServerKey::~ServerKey() noexcept {}

ServerKey& ServerKey::operator=(ServerKey&& other) noexcept {
    this->gKey = std::move(other.gKey);
    this->nKey = std::move(other.nKey);
    this->pubKey = std::move(other.pubKey);
    return *this;
}