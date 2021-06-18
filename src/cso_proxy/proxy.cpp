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

std::unique_ptr<IProxy> Proxy::build(std::shared_ptr<IConfig> config) {
    return std::unique_ptr<IProxy>(new Proxy(config));
}

Proxy::Proxy(std::shared_ptr<IConfig>& config) {
    this->config.swap(config);
}

Proxy::~Proxy() {}

Result<ServerKey> Proxy::exchangeKey() {
    // "obj" is a reference to "doc".
    // "deserializeJson" from HTTP response to "doc" copied strings, 
    // if put "doc" and "data" into block and extract neccessary fields into variables,
    // we will copy strings one more time.
    // Because all keys in JSON are used, we should keep "doc".
    // Keeping "doc" will waste a some bytes for unnecessary fields and JsonDocument's data structure
    DynamicJsonDocument doc(0);
    JsonObject obj_json;
    {
        // Build http request body
        Array<uint8_t> httpBody;
        {
            StaticJsonDocument<JSON_OBJECT_SIZE(2)> doc;
            obj_json = doc.to<JsonObject>();
            obj_json["project_id"] = this->config->getProjectID().c_str();
            obj_json["unique_name"] = this->config->getConnectionName().c_str();

            httpBody.length = 34 + 
                              this->config->getProjectID().length() + 
                              this->config->getConnectionName().length();
            httpBody.buffer.reset(new (std::nothrow) uint8_t[httpBody.length + 1]);
            if (httpBody.buffer == nullptr) {
                return make_result(Error::NotEnoughMemory, ServerKey());
            }
            serializeJson(doc, httpBody.buffer.get(), httpBody.length + 1);
        }

        // Build URL
        std::unique_ptr<char> url(new (std::nothrow) char[this->config->getCSOAddress().length() + 14]);
        if (url == nullptr) {
            return make_result(Error::NotEnoughMemory, ServerKey());
        }
        sprintf(url.get(), "%s/exchange-key", this->config->getCSOAddress().c_str());

        // Send request and receive response
        Result<std::string> resp = sendPOST(url.get(), httpBody.buffer.get(), httpBody.length);
        if (resp.errorCode != Error::Nil) {
            return make_result(resp.errorCode, ServerKey());
        }
        if (resp.data.empty()) {
            return make_result(Error::CSOProxy_ResponseEmpty, ServerKey());
        }

        // "DynamicJsonDocument" is allocated on heap, so we need to free unsued memory
        // to allocate new memory.
        delete[] url.release();
        delete[] httpBody.buffer.release();

        // Parse response
        // JSON string has 7 keys <=> 7 JSON_OBJECT
        doc = DynamicJsonDocument(JSON_OBJECT_SIZE(7) + resp.data.length());
        auto jsonError = deserializeJson(doc, resp.data);
        if (jsonError) {
            return make_result(
                Error::adaptExternalCode(ExternalTag::ArduinoJSON, jsonError.code()), 
                ServerKey()
            );
        }
        auto serverErrorCode = (int32_t)doc["returncode"];
        if (serverErrorCode != 1) {
            return make_result(
                Error::adaptExternalCode(ExternalTag::Server, serverErrorCode), 
                ServerKey()
            );
        }
        obj_json = doc["data"];
    }
    
    // "JsonObject[key]" will returns a no type reference, 
    // so we need to cast type or extract explicit by "JsonObject[key].as<type>()".
    // However, "verifyDHKeys" function defined type params, extracting implicit will be called.
    auto errorCode = verifyDHKeys(
        obj_json["g_key"], 
        obj_json["n_key"], 
        obj_json["pub_key"], 
        obj_json["sign"]
    );
    if (errorCode != Error::Nil) {
        return make_result(errorCode, ServerKey());
    }

    BigNum gKey;
    errorCode = gKey.setString(obj_json["g_key"]);
    if (errorCode != Error::Nil) {
        return make_result(errorCode, ServerKey());
    }

    BigNum nKey;
    errorCode = nKey.setString(obj_json["n_key"]);
    if (errorCode != Error::Nil) {
        return make_result(errorCode, ServerKey());
    }

    BigNum pubKey;
    errorCode = pubKey.setString(obj_json["pub_key"]);
    if (errorCode != Error::Nil) {
        return make_result(errorCode, ServerKey());
    }

    return make_result(
        Error::Nil, 
        ServerKey(std::move(gKey), std::move(nKey), std::move(pubKey))
    );
}

Result<ServerTicket> Proxy::registerConnection(const ServerKey& serverKey) {
    // Generate client private key
    BigNum clientPrivKey;
    {
        auto result_genPrivKey = UtilsDH::generatePrivateKey();
        if (result_genPrivKey.errorCode != Error::Nil) {
            return make_result(result_genPrivKey.errorCode, ServerTicket());
        }
        std::swap(clientPrivKey, result_genPrivKey.data);
    }

    // Calculate client public key
    std::string clientPubKey;
    {
        auto result_calcPubKey = UtilsDH::calcPublicKey(serverKey.gKey, serverKey.nKey, clientPrivKey);
        if (result_calcPubKey.errorCode != Error::Nil) {
            return make_result(result_calcPubKey.errorCode, ServerTicket());
        }
        auto result_toString = result_calcPubKey.data.toString();
        if (result_toString.errorCode != Error::Nil) {
            return make_result(result_toString.errorCode, ServerTicket());
        }
        std::swap(clientPubKey, result_toString.data);
    }

    // Calculate client secret key
    std::unique_ptr<byte> secretKey(new (std::nothrow) byte[32]);
    if (secretKey == nullptr) {
        return make_result(Error::NotEnoughMemory, ServerTicket());
    }
    auto errorCode = UtilsDH::calcSecretKey(serverKey.nKey, clientPrivKey, serverKey.pubKey, secretKey.get());
    if (errorCode != Error::Nil) {
        return make_result(errorCode, ServerTicket());
    }

    // Build encrypt token
    Array<byte> token;
    std::unique_ptr<byte> iv(nullptr);
    std::unique_ptr<byte> authenTag(nullptr);
    errorCode = buildEncyptToken(clientPubKey.c_str(), secretKey, iv, authenTag, token);
    if (errorCode != Error::Nil) {
        return make_result(errorCode, ServerTicket());
    }

    // Invoke API
    Array<byte> aad;
    uint16_t ticketID;
    uint16_t hubPort;
    std::string hubIP;
    BigNum serverPubKey;
    uint16_t lenServerTicketToken;
    std::unique_ptr<byte> serverTicketToken(nullptr);
    {
        // Build http request body
        Array<byte> httpBody;
        {
            std::string encodeIV = UtilsBase64::encode(iv.get(), LENGTH_IV);
            std::string encodeToken = UtilsBase64::encode(token.buffer.get(), token.length);
            std::string encodeAuthenTag = UtilsBase64::encode(authenTag.get(), LENGTH_AUTHEN_TAG);
           
            StaticJsonDocument<JSON_OBJECT_SIZE(6)> doc;
            JsonObject obj_json = doc.to<JsonObject>();
            obj_json["project_id"] = this->config->getProjectID().c_str();
            obj_json["project_token"] = encodeToken.c_str();
            obj_json["unique_name"] = this->config->getConnectionName().c_str();
            obj_json["public_key"] = clientPubKey.c_str();
            obj_json["iv"] = encodeIV.c_str();
            obj_json["authen_tag"] = encodeAuthenTag.c_str();

            httpBody.length = 93 + 
                              encodeIV.length() +
                              encodeToken.length() +
                              encodeAuthenTag.length() +
                              clientPubKey.length() +
                              this->config->getProjectID().length() + 
                              this->config->getConnectionName().length();
            httpBody.buffer.reset(new (std::nothrow) byte[httpBody.length + 1]);
            if (httpBody.buffer == nullptr) {
                return make_result(Error::NotEnoughMemory, ServerTicket());
            }
            serializeJson(doc, httpBody.buffer.get(), httpBody.length + 1);
        }
        
        DynamicJsonDocument doc(0);
        {
            // Build URL
            std::unique_ptr<char> url(new (std::nothrow) char[this->config->getCSOAddress().length() + 21]);
            if (url.get() == nullptr) {
                return make_result(Error::NotEnoughMemory, ServerTicket());
            }
            sprintf(url.get(), "%s/register-connection", this->config->getCSOAddress().c_str());

            // Send request and receive response
            Result<std::string> resp = sendPOST(url.get(), httpBody.buffer.get(), httpBody.length);
            if (resp.errorCode != Error::Nil) {
                return make_result(resp.errorCode, ServerTicket());
            }
            if (resp.data.empty()) {
                return make_result(Error::CSOProxy_ResponseEmpty, ServerTicket());
            }

            // "DynamicJsonDocument" is allocated on heap, so we need to free unsued memory
            // to allocate new memory
            delete[] url.release();
            delete[] httpBody.buffer.release();

            // Parse response
            doc = DynamicJsonDocument(JSON_OBJECT_SIZE(9) + resp.data.length());
            auto jsonError = deserializeJson(doc, resp.data);
            if (jsonError) {
                return make_result(
                    Error::adaptExternalCode(ExternalTag::ArduinoJSON, jsonError.code()), 
                    ServerTicket()
                );
            }
            auto serverErrorCode = (int32_t)doc["returncode"];
            if (serverErrorCode != 1) {
                return make_result(
                    Error::adaptExternalCode(ExternalTag::Server, serverErrorCode), 
                    ServerTicket()
                );
            }
        }

        JsonObject obj_json = doc["data"];
        const char* serverPublicKey = (const char*)obj_json["pub_key"];
        const char* hubAddress = (const char*)obj_json["hub_address"];
        size_t lenHubAddress = strlen(hubAddress);

        // Get ticket_id
        ticketID = (uint16_t)obj_json["ticket_id"];

        // Decode base64 data
        {
            Array<byte> decodeData = UtilsBase64::decode((const char*)obj_json["ticket_token"]);
            serverTicketToken.reset(new (std::nothrow) byte[decodeData.length]);
            if (serverTicketToken == nullptr) {
                return make_result(Error::NotEnoughMemory, ServerTicket());
            }
            lenServerTicketToken = decodeData.length;
            memcpy(serverTicketToken.get(), decodeData.buffer.get(), lenServerTicketToken);

            decodeData = UtilsBase64::decode((const char*)obj_json["iv"]);
            memcpy(iv.get(), decodeData.buffer.get(), LENGTH_IV);

            decodeData = UtilsBase64::decode((const char*)obj_json["auth_tag"]);
            memcpy(authenTag.get(), decodeData.buffer.get(), LENGTH_AUTHEN_TAG);
        }

        // Build server aad
        {
            size_t lenServerPubKey = strlen(serverPublicKey);
            aad.length = 2 + 
                         lenHubAddress +
                         lenServerPubKey;
            aad.buffer.reset(new (std::nothrow) byte[aad.length]);
            if (aad.buffer == nullptr) {
                return make_result(Error::NotEnoughMemory, ServerTicket());
            }
            memcpy(aad.buffer.get(), &ticketID, 2);
            memcpy(aad.buffer.get() + 2, hubAddress, lenHubAddress);
            memcpy(aad.buffer.get() + (2 + lenHubAddress), serverPublicKey, lenServerPubKey);
        }

        // Parse hub_address string to hub_ip + hub_port
        {
            uint8_t index = -1;
            for (uint8_t idx = lenHubAddress - 1; idx >= 0; --idx) {
                if (hubAddress[idx] == ':') {
                    index = idx;
                    break;
                }
            }
            if (index == -1) {
                return make_result(Error::CSOProxy_InvalidHubAddress, ServerTicket());
            }

            hubIP.assign(hubAddress, index);
            hubPort = atoi(hubAddress + index + 1);
        }

        // Build big number for server_public_key
        errorCode = serverPubKey.setString(serverPublicKey);
        if (errorCode != Error::Nil) {
            return make_result(errorCode, ServerTicket());
        }
    }

    // Build server secret key
    errorCode = UtilsDH::calcSecretKey(
        serverKey.nKey, 
        clientPrivKey, 
        serverPubKey, 
        secretKey.get()
    );
    if (errorCode != Error::Nil) {
        return make_result(errorCode, ServerTicket());
    }

    // Decrypt server ticket token
    errorCode = UtilsAES::decrypt(
        secretKey.get(), 
        serverTicketToken.get(), 
        lenServerTicketToken, 
        aad.buffer.get(),
        aad.length,
        iv.get(), 
        authenTag.get(),
        token.buffer.get()
    );
    if (errorCode != Error::Nil) {
        return make_result(errorCode, ServerTicket());
    }

    // Parse server ticket token to bytes
    Result<byte*> ticket = Ticket::buildBytes(ticketID, token.buffer.get());
    if (ticket.errorCode != Error::Nil) {
        return make_result(ticket.errorCode, ServerTicket());
    }
    
    // Done
    return make_result(
        Error::Nil, 
        ServerTicket(
            std::move(hubIP),
            hubPort,
            ticketID,
            ticket.data,
            secretKey.release() // Don't use "get()" because after function done
                                // "std::unique_ptr's destructor" will delete memory
        )
    );
}

//========
// PRIVATE
//========
Result<std::string> Proxy::sendPOST(const char* url, uint8_t* content, uint16_t length) {
    // WiFiClientSecure secureClient;
    // secureClient.setTimeout(20000);
    // secureClient.setInsecure();

    // log_e("%s", url);
    // log_e("Start connect");
    // if (!secureClient.connect("https://goldeneyetech.com.vn", 443)) {
    //     log_e("Send POST failed");
    //     vTaskDelay(1000);
    //     return std::pair<Error::Code, std::string>(Error::NotConnectServer, "");
    // }
    // log_e("Send POST success");
    
    HTTPClient http;
    http.setTimeout(20000); // 20s
    if (!http.begin(url)) {
        // secureClient.stop();
        return make_result(Error::CSOProxy_Disconnected, std::string(""));
    }

    http.addHeader("Content-Type", "application/json");
    auto status = http.POST(content, length);
    if (status != 200) {
        http.end();
        return make_result(
            Error::adaptExternalCode(ExternalTag::HTTP, status), 
            std::string("")
        );
    }

    std::string resp(http.getString().c_str());
    http.end();
    // secureClient.stop();
    return make_result(Error::Nil, std::move(resp));
}

Error::Code Proxy::verifyDHKeys(const char* gKey, const char* nKey, const char* pubKey, const char* encodeSign) {
    // Build data
    uint16_t lenGKey = strlen(gKey);
    uint16_t lenNKey = strlen(nKey);
    uint16_t lenPubKey = strlen(pubKey);

    Array<uint8_t> data;
    data.length = lenGKey + lenNKey + lenPubKey;
    data.buffer.reset(new (std::nothrow) uint8_t[data.length]);
    if (data.buffer == nullptr) {
        return Error::NotEnoughMemory;
    }

    memcpy(data.buffer.get(), gKey, lenGKey);
    memcpy(data.buffer.get() + lenGKey, nKey, lenNKey);
    memcpy(data.buffer.get() + (lenGKey + lenNKey), pubKey, lenPubKey);

    // Decode signature to bytes
    Array<uint8_t> signature(UtilsBase64::decode(encodeSign));
    
    // Verify
    return UtilsRSA::verifySignature(
        (uint8_t*)this->config->getCSOPublicKey().c_str(), 
        signature.buffer.get(),
        signature.length,
        data.buffer.get(), 
        data.length
    );
}

Error::Code Proxy::buildEncyptToken(const char* clientPubKey, const std::unique_ptr<uint8_t>& secretKey, std::unique_ptr<uint8_t>& iv, std::unique_ptr<uint8_t>& authenTag, Array<uint8_t>& token) {
    // Decode client token
    Array<uint8_t> decodedToken;
    {
        const std::string& encodeToken = this->config->getProjectToken();
        decodedToken = UtilsBase64::decode(encodeToken.c_str(), encodeToken.length());
    }

    // Build client aad
    Array<uint8_t> aad;
    {
        uint16_t lenPubKey = strlen(clientPubKey);
        uint16_t lenPID = this->config->getProjectID().length();
        uint16_t lenCName = this->config->getConnectionName().length();
        aad.length = lenPID + lenCName + lenPubKey;
        aad.buffer.reset(new (std::nothrow) uint8_t[aad.length]);
        if (aad.buffer == nullptr) {
            return Error::NotEnoughMemory;
        }
        memcpy(aad.buffer.get(), this->config->getProjectID().c_str(), lenPID);
        memcpy(aad.buffer.get() + lenPID, this->config->getConnectionName().c_str(), lenCName);
        memcpy(aad.buffer.get() + (lenPID + lenCName), clientPubKey, lenPubKey);
    }

    // Buil encrypt client token
    iv.reset(new (std::nothrow) uint8_t[LENGTH_IV]);
    if (iv == nullptr) {
        return Error::NotEnoughMemory;
    }

    authenTag.reset(new (std::nothrow) uint8_t[LENGTH_AUTHEN_TAG]);
    if (authenTag == nullptr) {
        return Error::NotEnoughMemory;
    }

    token.length = decodedToken.length;
    token.buffer.reset(new (std::nothrow) uint8_t[token.length]);
    if (token.buffer == nullptr) {
        return Error::NotEnoughMemory;
    }    

    return UtilsAES::encrypt(
        secretKey.get(), 
        decodedToken.buffer.get(),
        decodedToken.length, 
        aad.buffer.get(), 
        aad.length, 
        iv.get(), 
        authenTag.get(),
        token.buffer.get()
    );
}