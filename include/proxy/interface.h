#ifndef _PROXY_INTERFACE_H_
#define _PROXY_INTERFACE_H_

#include <memory>
#include <utility>
#include "message.h"
#include "utils/utils_code.h"

class IProxy {
public:
    virtual std::pair<Error::Code, ServerKey> exchangeKey() = 0;
    virtual std::pair<Error::Code, ServerTicket> registerConnection(const ServerKey& serverKey) = 0;
};

#endif //_PROXY_INTERFACE_H_