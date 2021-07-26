#include <SPIFFS.h>
#include <ArduinoJson.h>
#include "cso_config/config.h"

std::unique_ptr<IConfig> Config::build(const std::string& filePath) {
    File file = SPIFFS.open(filePath.c_str(), FILE_READ);
    if (!file) {
        return std::unique_ptr<IConfig>(new Config("", "", "", "", ""));
    }

    // Read all file
    String data(file.readString());
    file.close();

    // Parse data
    // See more at: "https://arduinojson.org/v6/how-to/reuse-a-json-document/"
    DynamicJsonDocument doc(JSON_OBJECT_SIZE(5) + data.length());
    deserializeJson(doc, data);
    return std::unique_ptr<IConfig>(new Config(
        doc["pid"], 
        doc["ptoken"],
        doc["cname"],
        doc["csopubkey"],
        doc["csoaddr"]
    ));
}

std::unique_ptr<IConfig> Config::build(std::string&& projectID, std::string&& projectToken, std::string&& connectionName, std::string&& csoPubKey, std::string&& csoAddress) noexcept {
    return std::unique_ptr<IConfig>(new Config(
        std::forward<std::string>(projectID),
        std::forward<std::string>(projectToken),
        std::forward<std::string>(connectionName),
        std::forward<std::string>(csoPubKey),
        std::forward<std::string>(csoAddress)
    ));
}

Config::Config(
    std::string&& projectID,
    std::string&& projectToken,
    std::string&& connectionName,
    std::string&& csoPubKey,
    std::string&& csoAddress
) noexcept
  : projectID{ projectID },
    projectToken{ projectToken },
    connectionName{ connectionName },
    csoPubKey{ csoPubKey },
    csoAddress{ csoAddress } {}

const std::string& Config::getProjectID() noexcept {
    return this->projectID;
}

const std::string& Config::getProjectToken() noexcept {
    return this->projectToken;
}

const std::string& Config::getConnectionName() noexcept {
    return this->connectionName;
}

const std::string& Config::getCSOPublicKey() noexcept {
    return this->csoPubKey;
}

const std::string& Config::getCSOAddress() noexcept {
    return this->csoAddress;
}