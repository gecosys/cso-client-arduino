#include <new>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include "config/config.h"

std::shared_ptr<IConfig> Config::build(const char* projectID, const char* projectToken, const char* connectionName, const char* csoPubKey, const char* csoAddress) {
    // If allocation fails, "UtilsSafe::new_obj" will return "nullptr"
    IConfig* obj = new (std::nothrow) Config(
        projectID, 
        projectToken, 
        connectionName, 
        csoPubKey, 
        csoAddress
    );
    if (obj == nullptr) {
        throw "[cso_config/Config::build(...)]Not enough memory to create object";
    }
    return std::shared_ptr<IConfig>(obj);
}

std::shared_ptr<IConfig> Config::build(const char* filePath) {
    File file = SPIFFS.open(filePath, FILE_READ);
    if (!file) {
        return std::shared_ptr<IConfig>(new (std::nothrow) Config("", "", "", "", ""));
    }

    // Read all file
    String data(file.readString());
    file.close();

    // Parse data
    // See more at: "https://arduinojson.org/v6/how-to/reuse-a-json-document/"
    DynamicJsonDocument doc(JSON_OBJECT_SIZE(5) + data.length());
    deserializeJson(doc, data);
    IConfig* obj = new (std::nothrow) Config(doc["pid"], doc["ptoken"],doc["cname"],doc["csopubkey"],doc["csoaddr"]);
    if (obj == nullptr) {
        throw "[cso_config/Config::build(...)]Not enough memory to create object";
    }
    return std::shared_ptr<IConfig>(obj);
}

Config::Config(
    const char* projectID, 
    const char* projectToken, 
    const char* connectionName, 
    const char* csoPubKey, 
    const char* csoAddress
) noexcept
  : projectID(projectID),
    projectToken(projectToken),
    connectionName(connectionName),
    csoPubKey(csoPubKey),
    csoAddress(csoAddress) {}

Config::~Config() noexcept {}

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