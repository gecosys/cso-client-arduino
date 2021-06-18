#ifndef _CSO_PROXY_INTERFACE_H_
#define _CSO_PROXY_INTERFACE_H_

#include <memory>
#include "server_key.h"
#include "server_ticket.h"
#include "utils/result.h"
#include "error/error_code.h"

class IProxy {
public:
    virtual Result<ServerKey> exchangeKey() = 0;
    virtual Result<ServerTicket> registerConnection(const ServerKey& serverKey) = 0;
};

#endif //_PROXY_INTERFACE_H_