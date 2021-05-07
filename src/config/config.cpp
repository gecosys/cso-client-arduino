#include <SD_MMC.h>
#include <ArduinoJson.h>
#include "config/config.h"

std::shared_ptr<IConfig> Config::build(
    const char* projectID,
    const char* projectToken,
    const char* connectionName,
    const char* csoPubKey,
    const char* csoAddress
) {
    return std::shared_ptr<IConfig>(new Config(
        projectID, 
        projectToken, 
        connectionName,
        csoPubKey,
        csoAddress
    ));
}

std::shared_ptr<IConfig> Config::build(const char* filePath) {
    File file = SD_MMC.open(filePath, FILE_READ);
    if (!file) {
        return std::shared_ptr<IConfig>(new Config());
    }

    // Read all file
    String data(file.readString());
    file.close();

    // Parse data
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, data);
    JsonObject obj = doc.as<JsonObject>();
    Config* config = new Config();
    config->projectID = obj["pid"].as<String>();
    config->projectToken = obj["ptoken"].as<String>();
    config->connectionName = obj["cname"].as<String>();
    config->csoPubKey = obj["csopubkey"].as<String>();
    config->csoPubKey = obj["csoaddr"].as<String>();
    return std::shared_ptr<IConfig>(config);
}

Config::Config(
    const char* projectID,
    const char* projectToken,
    const char* connectionName,
    const char* csoPubKey,
    const char* csoAddress
)
    : projectID(projectID),
      projectToken(projectToken),
      connectionName(connectionName),
      csoPubKey(csoPubKey),
      csoAddress(csoAddress) {}

Config::~Config() noexcept {}

const String& Config::getProjectID() noexcept {
    return this->projectID;
}

const String& Config::getProjectToken() noexcept {
    return this->projectToken;
}

const String& Config::getConnectionName() noexcept {
    return this->connectionName;
}

const String& Config::getCSOPublicKey() noexcept {
    return this->csoPubKey;
}

const String& Config::getCSOAddress() noexcept {
    return this->csoAddress;
}