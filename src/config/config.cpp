#include <SPIFFS.h>
#include <ArduinoJson.h>
#include "config/config.h"

std::shared_ptr<IConfig> Config::build(
    const char* projectID, 
    const char* projectToken, 
    const char* connectionName, 
    const char* csoPubKey, 
    const char* csoAddress
) {
    // If allocation fails, "Safe::new_obj" will return "nullptr"
    IConfig* obj = Safe::new_obj<Config>(
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
        return std::shared_ptr<IConfig>(Safe::new_obj<Config>());
    }

    // Read all file
    String data(file.readString());
    file.close();

    // Parse data
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, data);
    JsonObject obj_json = doc.as<JsonObject>();
    // "JsonObject["key"].as" will return reference to value
    IConfig* obj = Safe::new_obj<Config>(
        obj_json["pid"].as<String>().c_str(), 
        obj_json["ptoken"].as<String>().c_str(),
        obj_json["cname"].as<String>().c_str(),
        obj_json["csopubkey"].as<String>().c_str(),
        obj_json["csoaddr"].as<String>().c_str()
    );
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
) : projectID(projectID),
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