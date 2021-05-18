#ifndef _CSO_PROXY_H_
#define _CSO_PROXY_H_

#include <string>
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

    std::pair<Error::Code, std::string> sendPOST(
        const char* url, 
        byte* buffer, 
        uint16_t length
    );
    Error::Code verifyDHKeys(
        const char* gKey, 
        const char* nKey, 
        const char* pubKey, 
        const char* encodeSign
    );
    Error::Code buildEncyptToken(
        const char* clientPubKey, 
        const byte* secretKey, 
        std::unique_ptr<byte>& iv, 
        std::unique_ptr<byte>& authenTag, 
        std::unique_ptr<byte>& token
    );

public:
    Proxy(Proxy&& other) = delete;
    Proxy(const Proxy& other) = delete;
    virtual ~Proxy() noexcept;

    std::pair<Error::Code, ServerKey> exchangeKey();
    std::pair<Error::Code, ServerTicket> registerConnection(const ServerKey& serverKey);
};

#endif //_CSO_PROXY_H_