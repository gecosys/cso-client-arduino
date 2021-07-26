#ifndef CSO_PROXY_INTERFACE_H
#define CSO_PROXY_INTERFACE_H

#include <tuple>
#include <memory>
#include "server_key.h"
#include "server_ticket.h"
#include "error/error.h"

class IProxy {
public:
    virtual std::tuple<Error::Code, ServerKey> exchangeKey() = 0;
    virtual std::tuple<Error::Code, ServerTicket> registerConnection(const ServerKey& serverKey) = 0;
};

#endif // !CSO_PROXY_INTERFACE_H