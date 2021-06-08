#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <memory>
#include "interface.h"

class Config : public IConfig {
private:
    std::string projectID;
    std::string projectToken;
    std::string connectionName;
    std::string csoPubKey;
    std::string csoAddress;

public:
    static std::shared_ptr<IConfig> build(const char* projectID, const char* projectToken, const char* connectionName, const char* csoPubKey, const char* csoAddress);
    static std::shared_ptr<IConfig> build(const char* filePath);

private:
    Config(
        const char* projectID, 
        const char* projectToken, 
        const char* connectionName, 
        const char* csoPubKey, 
        const char* csoAddress
    ) noexcept;

public:
    Config() = delete;
    Config(Config&& other) = delete;
    Config(const Config& other) = delete;
    Config& operator=(const Config& other) = delete;
    
    ~Config() noexcept;

    const std::string& getProjectID() noexcept;
	const std::string& getProjectToken() noexcept;
	const std::string& getConnectionName() noexcept;
	const std::string& getCSOPublicKey() noexcept;
	const std::string& getCSOAddress() noexcept;
};

#endif //_CONFIG_H_