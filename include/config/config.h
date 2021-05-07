#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <memory>
#include "interface.h"

class Config : public IConfig {
private:
    String projectID;
    String projectToken;
    String connectionName;
    String csoPubKey;
    String csoAddress;

public:
    static std::shared_ptr<IConfig> build(
        const char* projectID,
        const char* projectToken,
        const char* connectionName,
        const char* csoPubKey,
        const char* csoAddress
    );

    static std::shared_ptr<IConfig> build(const char* filePath);

private:
    Config() = default;
    Config(
        const char* projectID,
        const char* projectToken,
        const char* connectionName,
        const char* csoPubKey,
        const char* csoAddress
    );

public:
    Config(Config&& other) = delete;
    Config(const Config& other) = delete;
    virtual ~Config() noexcept;

    const String& getProjectID() noexcept;
	const String& getProjectToken() noexcept;
	const String& getConnectionName() noexcept;
	const String& getCSOPublicKey() noexcept;
	const String& getCSOAddress() noexcept;
};

#endif //_CONFIG_H_