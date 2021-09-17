#include <cstdio>
#include <cstdlib>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "cso_proxy/proxy.h"
#include "message/ticket.h"
#include "utils/utils_dh.h"
#include "utils/utils_rsa.h"
#include "utils/utils_aes.h"
#include "utils/utils_base64.h"
#include "utils/utils_general.hpp"
#include "error/thirdparty.h"

std::unique_ptr<IProxy> Proxy::build(std::unique_ptr<IConfig>&& config) {
    return std::unique_ptr<IProxy>(new Proxy(std::forward<std::unique_ptr<IConfig>>(config)));
}

Proxy::Proxy(std::unique_ptr<IConfig>&& config) noexcept 
    : config{ std::forward<std::unique_ptr<IConfig>>(config) } {}

std::tuple<Error, ServerKey> Proxy::exchangeKey() {
    // "obj" is a reference to "doc".
    // "deserializeJson" from HTTP response to "doc" copied strings, 
    // if put "doc" and "data" into block and extract neccessary fields into variables,
    // we will copy strings one more time.
    // Because all keys in JSON are used, we should keep "doc".
    // Keeping "doc" will waste a some bytes for unnecessary fields and JsonDocument's data structure
    DynamicJsonDocument doc(0);
    JsonObject obj_json;
    Error err;
    {
        {
            // Build http request body
            Array<uint8_t> httpBody;
            {
                StaticJsonDocument<JSON_OBJECT_SIZE(2)> doc;
                obj_json = doc.to<JsonObject>();
                obj_json["project_id"] = this->config->getProjectID().c_str();
                obj_json["unique_name"] = this->config->getConnectionName().c_str();

                size_t lenHttpBody = 34 + 
                                    this->config->getProjectID().length() + 
                                    this->config->getConnectionName().length();
                httpBody.reset(lenHttpBody, new uint8_t[lenHttpBody + 1]);
                serializeJson(doc, httpBody.get(), lenHttpBody + 1);
            }

            // Build URL
            // std::unique_ptr<char> url{ new char[this->config->getCSOAddress().length() + 14] };
            // sprintf(url.get(), "%s/exchange-key", this->config->getCSOAddress().c_str());
            // Build URL
            std::string url{ format("%s/exchange-key", this->config->getCSOAddress().c_str()) };

            // Send request and receive response
            std::string resp;
            std::tie(err, resp) = post(url, httpBody);
            if (!err.nil()) {
                return std::make_tuple(std::move(err), ServerKey{});
            }

            if (resp.empty()) {
                return std::make_tuple(Error{ GET_FUNC_NAME(), "[HTTP] Response is empty" }, ServerKey{});
            }
            
            // JSON string has 7 keys <=> 7 JSON_OBJECT
            doc = DynamicJsonDocument(JSON_OBJECT_SIZE(7) + resp.length());
            auto jsonError = deserializeJson(doc, resp);
            if (jsonError) {
                return std::make_tuple(
                    Error{ GET_FUNC_NAME(), Thirdparty::getAruduinojsonError(jsonError.code()) }, 
                    ServerKey{}
                );
            }
        }

        // Parse response
        auto serverCode = (int32_t)doc["returncode"];
        if (serverCode != 1) {
            return std::make_tuple(
                Error{ GET_FUNC_NAME(), format("[Server] (%d)%s", serverCode, (const char*)doc["data"]) }, 
                ServerKey{}
            );
        }
        obj_json = doc["data"];
    }
    
    // "JsonObject[key]" will returns a no type reference, 
    // so we need to cast type or extract explicit by "JsonObject[key].as<type>()".
    // However, "verifyDHKeys" function defined type params, extracting implicit will be called.
    bool valid;
    std::tie(err, valid) = verifyDHKeys(
        obj_json["g_key"], 
        obj_json["n_key"], 
        obj_json["pub_key"], 
        obj_json["sign"]
    );
    if (!err.nil()) {
        return std::make_tuple(std::move(err), ServerKey{});
    }

    if (!valid) {
        return std::make_tuple(Error{ GET_FUNC_NAME(), "DH keys authentication failed" }, ServerKey{});
    }

    BigInt gKey;
    BigInt nKey;
    BigInt pubKey;

    err = gKey.setString(obj_json["g_key"]);
    if (!err.nil()) {
        std::make_tuple(std::move(err), ServerKey{});
    }

    err = nKey.setString(obj_json["n_key"]);
    if (!err.nil()) {
        std::make_tuple(std::move(err), ServerKey{});
    }

    err = pubKey.setString(obj_json["pub_key"]);
    if (!err.nil()) {
        std::make_tuple(std::move(err), ServerKey{});
    }
    return std::make_tuple(Error{}, ServerKey{ std::move(gKey), std::move(nKey), std::move(pubKey) });
}

std::tuple<Error, ServerTicket> Proxy::registerConnection(const ServerKey& serverKey) {
    // Generate client private key
    Error err;
    BigInt clientPrivKey;

    std::tie(err, clientPrivKey) = UtilsDH::generatePrivateKey();
    if (!err.nil()) {
        return std::make_tuple(std::move(err), ServerTicket{});
    }

    // Calculate client public key
    std::string clientPubKey;
    {
        BigInt pubKey;

        std::tie(err, pubKey) = UtilsDH::calcPublicKey(serverKey.gKey, serverKey.nKey, clientPrivKey);
        if (!err.nil()) {
            return std::make_tuple(std::move(err), ServerTicket{});
        }

        std::tie(err, clientPubKey) = pubKey.toString();
        if (!err.nil()) {
            return std::make_tuple(std::move(err), ServerTicket{});
        }
    }

    Array<uint8_t> iv;
    Array<uint8_t> tag;
    Array<uint8_t> token;
    {
        // Calculate client secret key
        Array<uint8_t> secretKey;

        std::tie(err, secretKey) = UtilsDH::calcSecretKey(serverKey.nKey, clientPrivKey, serverKey.pubKey);
        if (!err.nil()) {
            return std::make_tuple(std::move(err), ServerTicket{});
        }

        // Encrypt client token
        std::tie(err, iv, tag, token) = buildEncyptToken(clientPubKey, secretKey);
        if (!err.nil()) {
            return std::make_tuple(std::move(err), ServerTicket{});
        }
    }

    // Get data
    Array<uint8_t> ticketToken;
    Array<uint8_t> aad;
    BigInt serverPubKey;
    std::string hubIP;
    uint16_t hubPort;
    uint16_t ticketID;
    {
        std::string strServerPubKey;
        std::string hubAddress;
        {
            DynamicJsonDocument doc(0);
            {
                // Build http request body
                Array<uint8_t> httpBody;
                {
                    std::string encodeIV = UtilsBase64::encode(iv);
                    std::string encodeToken = UtilsBase64::encode(token);
                    std::string encodeAuthenTag = UtilsBase64::encode(tag);

                    iv.reset();
                    tag.reset();
                    token.reset();
                
                    StaticJsonDocument<JSON_OBJECT_SIZE(6)> js_doc;
                    JsonObject js_object = js_doc.to<JsonObject>();
                    js_object["project_id"] = this->config->getProjectID().c_str();
                    js_object["project_token"] = encodeToken.c_str();
                    js_object["unique_name"] = this->config->getConnectionName().c_str();
                    js_object["public_key"] = clientPubKey.c_str();
                    js_object["iv"] = encodeIV.c_str();
                    js_object["authen_tag"] = encodeAuthenTag.c_str();

                    size_t lenHttpBody = 93 + 
                                        encodeIV.length() +
                                        encodeToken.length() +
                                        encodeAuthenTag.length() +
                                        clientPubKey.length() +
                                        this->config->getProjectID().length() + 
                                        this->config->getConnectionName().length();
                    httpBody.reset(lenHttpBody, new byte[lenHttpBody + 1]);
                    serializeJson(js_doc, httpBody.get(), lenHttpBody + 1);
                }

                // Build URL
                std::string url{ format("%s/register-connection", this->config->getCSOAddress().c_str()) };
                std::string resp;

                // Send request and receive response
                std::tie(err, resp) = post(url, httpBody);
                if (!err.nil()) {
                    return std::make_tuple(std::move(err), ServerTicket{});
                }

                if (resp.empty()) {
                    return std::make_tuple(Error{ GET_FUNC_NAME(), "[HTTP] Response is empty" }, ServerTicket{});
                }

                // Parse response
                doc = DynamicJsonDocument(JSON_OBJECT_SIZE(9) + resp.length());
                auto jsonError = deserializeJson(doc, resp);
                if (jsonError) {
                    return std::make_tuple(
                        Error{ GET_FUNC_NAME(), Thirdparty::getAruduinojsonError(jsonError.code()) }, 
                        ServerTicket{}
                    );
                }
            }

            auto serverCode = (int32_t)doc["returncode"];
            if (serverCode != 1) {
                return std::make_tuple(
                    Error{ GET_FUNC_NAME(), format("[Server] (%d)%s", serverCode, (const char*)doc["data"]) }, 
                    ServerTicket{}
                );
            }

            JsonObject js_object = doc["data"];
            strServerPubKey = (const char*)js_object["pub_key"];
            hubAddress = (const char*)js_object["hub_address"];
            
            // Get ticket_id
            ticketID = (uint16_t)js_object["ticket_id"];

            // Decode base64 data
            iv = UtilsBase64::decode((const char*)js_object["iv"]);
            tag = UtilsBase64::decode((const char*)js_object["auth_tag"]);
            ticketToken = UtilsBase64::decode((const char*)js_object["ticket_token"]);
        }

        // Build server aad
        {
            size_t lenHubAddress = hubAddress.length();
            size_t lenServerPubKey = strServerPubKey.length();

            aad.reset(2 + lenHubAddress + lenServerPubKey);
            memcpy(aad.get(), &ticketID, 2);
            memcpy(aad.get() + 2, hubAddress.c_str(), lenHubAddress);
            memcpy(aad.get() + (2 + lenHubAddress), strServerPubKey.c_str(), lenServerPubKey);
        }

        // Parse hub_address string to hub_ip + hub_port
        {
            int8_t index = -1;
            for (int8_t idx = hubAddress.length() - 1; idx >= 0; --idx) {
                if (hubAddress[idx] == ':') {
                    index = idx;
                    break;
                }
            }

            if (index == -1) {
                return std::make_tuple(Error{ GET_FUNC_NAME(), "Hub address is invalid" }, ServerTicket{});
            }

            hubIP.assign(hubAddress, 0, index);
            hubPort = atoi(hubAddress.c_str() + index + 1);
        }

        // Build server public key
        err = serverPubKey.setString(strServerPubKey);
        if (!err.nil()) {
            return std::make_tuple(std::move(err), ServerTicket{});
        }
    }

    // Build server secret key
    Array<uint8_t> serverSecretKey;
    std::tie(err, serverSecretKey) = UtilsDH::calcSecretKey(serverKey.nKey, clientPrivKey, serverPubKey);
    if (!err.nil()) {
        return std::make_tuple(std::move(err), ServerTicket{});
    }

    // Decrypt server token
    std::tie(err, token) = UtilsAES::decrypt(serverSecretKey, ticketToken, aad, iv, tag);
    if (!err.nil()) {
        return std::make_tuple(std::move(err), ServerTicket{});
    }

    // Parse server ticket token to bytes
    Array<uint8_t> ticket;
    std::tie(err, ticket) = Ticket::buildBytes(ticketID, token);
    if (!err.nil()) {
        return std::make_tuple(std::move(err), ServerTicket{});
    }

    // Done
    return std::make_tuple(
        Error{},
        ServerTicket{
            std::move(hubIP),
            hubPort,
            ticketID,
            std::move(ticket),
            std::move(serverSecretKey)
        }
    );
}

//========
// PRIVATE
//========
std::tuple<Error, std::string> Proxy::post(const std::string& url, const Array<uint8_t>& body) {
    // WiFiClientSecure secureClient;
    // secureClient.setTimeout(20000);
    // secureClient.setInsecure();

    // log_e("%s", url);
    // log_e("Start connect");
    // if (!secureClient.connect("https://goldeneyetech.com.vn", 443)) {
    //     log_e("Send POST failed");
    //     vTaskDelay(1000);
    //     return std::pair<Error, std::string>(Error::NotConnectServer, "");
    // }
    // log_e("Send POST success");
    
    HTTPClient http;
    http.setTimeout(20000); // 20s
    if (!http.begin(url.c_str())) {
        // secureClient.stop();
        return std::make_tuple(Error{ GET_FUNC_NAME(), "[HTTP] Connection is disconnected" }, "");
    }

    http.addHeader("Content-Type", "application/json");
    auto status = http.POST(body.get(), body.length());
    if (status != 200) {
        http.end();
        return std::make_tuple(Error{ GET_FUNC_NAME(), Thirdparty::getHttpError(status) }, "");
    }

    std::string resp(http.getString().c_str());
    http.end();
    // secureClient.stop();
    return std::make_tuple(Error{}, std::move(resp));
}

std::tuple<Error, bool> Proxy::verifyDHKeys(const std::string& gKey, const std::string& nKey, const std::string& pubKey, const std::string& encodeSign) {
    // Build data
    Array<uint8_t> data;
    {
        size_t lenGKey = gKey.length();
        size_t lenNKey = nKey.length();
        size_t lenPubKey = pubKey.length();

        data.reset(lenGKey + lenNKey + lenPubKey);
        memcpy(data.get(), gKey.c_str(), lenGKey);
        memcpy(data.get() +lenGKey, nKey.c_str(), lenNKey);
        memcpy(data.get() + (lenGKey + lenNKey), pubKey.c_str(), lenPubKey);
    }
    
    // Verify
    return UtilsRSA::verifySignature(
        this->config->getCSOPublicKey(),
        UtilsBase64::decode(encodeSign),
        data
    );
}

std::tuple<Error, Array<uint8_t>, Array<uint8_t>, Array<uint8_t>> Proxy::buildEncyptToken(const std::string& clientPubKey, const Array<uint8_t>& secretKey) {
    // Build client aad
    Array<uint8_t> aad;
    {
        const std::string& pid = this->config->getProjectID();
        const std::string& cname = this->config->getConnectionName();

        size_t lenPid = pid.length();
        size_t lenCname = cname.length();
        size_t lenClientPubKey = clientPubKey.length();

        aad.reset(lenPid + lenCname + lenClientPubKey);
        memcpy(aad.get(), pid.c_str(), lenPid);
        memcpy(aad.get() + lenPid, cname.c_str(), lenCname);
        memcpy(aad.get() + (lenPid + lenCname), clientPubKey.c_str(), lenClientPubKey);
    }

    // AES encrypt token
    return UtilsAES::encrypt(secretKey, UtilsBase64::decode(this->config->getProjectToken()), aad);
}