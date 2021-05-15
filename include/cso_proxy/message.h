#ifndef _CSO_PROXY_MESSAGE_H_
#define _CSO_PROXY_MESSAGE_H_

#include <memory>
#include <string>
#include <BigNumber.h>

// ServerKey is a group of server keys
struct ServerKey {
    // "BigNumber" has to set default data unless when returning will be crashed
    BigNumber gKey;
    BigNumber nKey;
    BigNumber pubKey;

    ServerKey() 
        : gKey("-1"), nKey("-1"), pubKey("-1") {}
    ServerKey(const char* gKey, const char* nKey, const char* pubKey)
        : gKey(gKey), nKey(nKey), pubKey(pubKey) {}
};

// ServerTicket is an activation ticket from the Hub server
struct ServerTicket {
    std::string hubAddress;
    uint16_t ticketID;
    std::shared_ptr<byte> ticketBytes;
    std::shared_ptr<byte> serverSecretKey;

    ServerTicket() = default;
    ServerTicket(
        const char* hubAddress, 
        uint16_t ticketID,
        byte* ticketBytes,
        byte* serverSecretKey
    ) : hubAddress(hubAddress),
        ticketID(ticketID),
        ticketBytes(ticketBytes),
        serverSecretKey(serverSecretKey) {}
};

#endif // _CSO_PROXY_MESSAGE_H_