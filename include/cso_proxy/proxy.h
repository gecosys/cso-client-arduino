#ifndef _CSO_PROXY_H_
#define _CSO_PROXY_H_

#include "interface.h"
#include "config/config.h"
#include "utils/utils_safe.h"

class Proxy : public IProxy {
private:
    // The author wants use "std::unique_ptr" for private properties.
    // However, "Config" class returns "std::share_ptr" and there is
    // no way yet can convert safely "std::share_ptr" to "std::unique_ptr"
    // (not delete memory when "std::share_ptr's destructor".
    std::shared_ptr<IConfig> config;

public:
    static std::shared_ptr<IProxy> build(std::shared_ptr<IConfig> config);

private:
    friend class Safe;
    Proxy() = default;
    Proxy(std::shared_ptr<IConfig>& config);

    std::pair<Error::Code, String> sendPOST(const char* url, byte* buffer, uint16_t length);
    Error::Code verifyDHKeys(const String& gKey, const String& nKey, const String& pubKey, const String& sign);

public:
    Proxy(Proxy&& other) = delete;
    Proxy(const Proxy& other) = delete;
    virtual ~Proxy() noexcept;

    std::pair<Error::Code, ServerKey> exchangeKey();
    std::pair<Error::Code, ServerTicket> registerConnection(const ServerKey& serverKey);
};

#endif //_CSO_PROXY_H_