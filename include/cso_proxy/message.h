#ifndef _CSO_PROXY_MESSAGE_H_
#define _CSO_PROXY_MESSAGE_H_

#include <memory>
#include <string>
#include <Arduino.h>
#include "utils/bignum.h"
#include "utils/address.h"

// ServerKey is a group of server keys
struct ServerKey {
    BigNum gKey;
    BigNum nKey;
    BigNum pubKey;

    ServerKey() noexcept
      : gKey(), 
        nKey(), 
        pubKey() {}

    ServerKey(const char* gKey, const char* nKey, const char* pubKey)
       : gKey(gKey), 
         nKey(nKey), 
         pubKey(pubKey) {}
};

// ServerTicket is an activation ticket from the Hub server
struct ServerTicket {
    Address hubAddress;
    uint16_t ticketID;
    std::shared_ptr<byte> ticketBytes;
    std::shared_ptr<byte> serverSecretKey;

    ServerTicket() noexcept
      : hubAddress(),
        ticketID(0),
        ticketBytes(nullptr),
        serverSecretKey(nullptr) {}

    ServerTicket(
        Address&& hubAddress, 
        uint16_t ticketID,
        byte* ticketBytes,
        byte* serverSecretKey
    ) : hubAddress(std::forward<Address>(hubAddress)),
        ticketID(ticketID),
        ticketBytes(ticketBytes),
        serverSecretKey(serverSecretKey) {}
};

#endif // _CSO_PROXY_MESSAGE_H_