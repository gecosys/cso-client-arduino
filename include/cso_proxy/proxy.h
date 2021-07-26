#ifndef _CSO_PROXY_H_
#define _CSO_PROXY_H_

#include <string>
#include "interface.h"
#include "entity/array.h"
#include "cso_config/config.h"

class Proxy : public IProxy {
private:
    std::unique_ptr<IConfig> config;

public:
    static std::unique_ptr<IProxy> build(std::unique_ptr<IConfig>&& config);

private:
    Proxy(std::unique_ptr<IConfig>&& config) noexcept;

    std::tuple<Error::Code, std::string> post(const std::string& url, const Array<uint8_t>& body);
    std::tuple<Error::Code, bool> verifyDHKeys(const std::string& gKey, const std::string& nKey, const std::string& pubKey, const std::string& encodeSign);
    std::tuple<Error::Code, Array<uint8_t>, Array<uint8_t>, Array<uint8_t>> buildEncyptToken(const std::string& clientPubKey, const Array<uint8_t>& secretKey);

public:
    Proxy() = delete;
    Proxy(Proxy&& other) = delete;
    Proxy(const Proxy& other) = delete;
    ~Proxy() noexcept = default;

    Proxy& operator=(Proxy&& other) = delete;
    Proxy& operator=(const Proxy& other) = delete;

    std::tuple<Error::Code, ServerKey> exchangeKey();
    std::tuple<Error::Code, ServerTicket> registerConnection(const ServerKey& serverKey);
};

#endif //_CSO_PROXY_H_