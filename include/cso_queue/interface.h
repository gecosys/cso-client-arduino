#ifndef _CSO_QUEUE_INTERFACE_H_
#define _CSO_QUEUE_INTERFACE_H_

#include "item.h"

class IQueue { 
public:
    // Method can invoke on many threads
	// This method needs to be invoked before PushMessage method
	virtual bool takeIndex() noexcept = 0;
    virtual void pushMessage(std::shared_ptr<ItemQueue> item) noexcept = 0;
    virtual const std::shared_ptr<ItemQueue>* nextMessage() noexcept = 0;
    virtual void clearMessage(uint64_t msgID) noexcept = 0;
};

#endif //_CSO_QUEUE_INTERFACE_H_