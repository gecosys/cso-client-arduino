#ifndef _CONFIG_INTERFACE_H_
#define _CONFIG_INTERFACE_H_

#include <string>

class IConfig {
public:
    virtual const std::string& getProjectID() noexcept = 0;
	virtual const std::string& getProjectToken() noexcept = 0;
	virtual const std::string& getConnectionName() noexcept = 0;
	virtual const std::string& getCSOPublicKey() noexcept = 0;
	virtual const std::string& getCSOAddress() noexcept = 0;
};

#endif //_CONFIG_INTERFACE_H_