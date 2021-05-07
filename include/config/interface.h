#ifndef _CONFIG_INTERFACE_H_
#define _CONFIG_INTERFACE_H_

#include <Arduino.h>

class IConfig {
public:
    virtual const String& getProjectID() noexcept = 0;
	virtual const String& getProjectToken() noexcept = 0;
	virtual const String& getConnectionName() noexcept = 0;
	virtual const String& getCSOPublicKey() noexcept = 0;
	virtual const String& getCSOAddress() noexcept = 0;
};

#endif //_CONFIG_INTERFACE_H_