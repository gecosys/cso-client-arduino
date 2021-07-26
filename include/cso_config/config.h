#ifndef CSO_CONFIG_H
#define CSO_CONFIG_H

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
    static std::unique_ptr<IConfig> build(const std::string& filePath);
    static std::unique_ptr<IConfig> build(
        std::string&& projectID,
        std::string&& projectToken,
        std::string&& connectionName,
        std::string&& csoPubKey,
        std::string&& csoAddress) noexcept;

private:
    Config(
        std::string&& projectID,
        std::string&& projectToken,
        std::string&& connectionName,
        std::string&& csoPubKey,
        std::string&& csoAddress) noexcept;

public:
    Config() = delete;
    Config(Config&& other) = delete;
    Config(const Config& other) = delete;
    ~Config() noexcept = default;

    Config& operator=(Config&& other) = delete;
    Config& operator=(const Config& other) = delete;

    const std::string& getProjectID() noexcept;
	const std::string& getProjectToken() noexcept;
	const std::string& getConnectionName() noexcept;
	const std::string& getCSOPublicKey() noexcept;
	const std::string& getCSOAddress() noexcept;
};

#endif // !CSO_CONFIG_H