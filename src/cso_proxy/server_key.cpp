#include "cso_proxy/server_key.h"

ServerKey::ServerKey() noexcept
    : gKey(), 
      nKey(), 
      pubKey() {}

ServerKey::ServerKey(BigNum&& gKey, BigNum&& nKey, BigNum&& pubKey) noexcept
    : gKey(std::forward<BigNum>(gKey)), 
      nKey(std::forward<BigNum>(nKey)), 
      pubKey(std::forward<BigNum>(pubKey)) {}

ServerKey::ServerKey(ServerKey&& other) noexcept {
    std::swap(this->nKey, other.nKey);
    std::swap(this->gKey, other.gKey);
    std::swap(this->pubKey, other.pubKey);
}