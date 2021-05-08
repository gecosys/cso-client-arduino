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
    // If allocation fails, new_obj_s will return "nullptr"
    return std::shared_ptr<IConfig>(Safe::new_obj<Config>(projectID, 
        projectToken, 
        connectionName,
        csoPubKey,
        csoAddress
    ));
}

std::shared_ptr<IConfig> Config::build(const char* filePath) {
    File file = SD_MMC.open(filePath, FILE_READ);
    if (!file) {
        return std::shared_ptr<IConfig>(Safe::new_obj<Config>());
    }

    // Read all file
    String data(file.readString());
    file.close();

    // Parse data
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, data);
    JsonObject obj = doc.as<JsonObject>();
    // JsonObject[key].as will return reference to value
    return std::shared_ptr<IConfig>(Safe::new_obj<Config>(
        obj["pid"].as<String>().c_str(), 
        obj["ptoken"].as<String>().c_str(),
        obj["cname"].as<String>().c_str(),
        obj["csopubkey"].as<String>().c_str(),
        obj["csoaddr"].as<String>().c_str()
    ));
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