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
    IProxy* obj = new (std::nothrow) Proxy(config);
    if (obj == nullptr) {        
        throw std::runtime_error("[cso_proxy/Proxy::build(std::shared_ptr<IConfig>)]Not enough memory to create object");
    }
    return std::unique_ptr<IProxy>(obj);
}

Proxy::Proxy(std::shared_ptr<IConfig>& config) {
    this->config.swap(config);
}

Proxy::~Proxy() {}

std::pair<Error::Code, ServerKey> Proxy::exchangeKey() {
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
        Array<byte> httpBody;
        {
            StaticJsonDocument<JSON_OBJECT_SIZE(2)> doc;
            obj_json = doc.to<JsonObject>();
            obj_json["project_id"] = this->config->getProjectID().c_str();
            obj_json["unique_name"] = this->config->getConnectionName().c_str();

            httpBody.length = 34 + 
                              this->config->getProjectID().length() + 
                              this->config->getConnectionName().length();
            httpBody.buffer.reset(new (std::nothrow) byte[httpBody.length + 1]);
            if (httpBody.buffer == nullptr) {
                return std::make_pair(Error::NotEnoughMemory, ServerKey());
            }
            serializeJson(doc, httpBody.buffer.get(), httpBody.length + 1);
        }

        // Build URL
        std::unique_ptr<char> url(new (std::nothrow) char[this->config->getCSOAddress().length() + 14]);
        if (url == nullptr) {
            return std::make_pair(Error::NotEnoughMemory, ServerKey());
        }
        sprintf(url.get(), "%s/exchange-key", this->config->getCSOAddress().c_str());

        // Send request and receive response
        std::pair<Error::Code, std::string> resp = sendPOST(url.get(), httpBody.buffer.get(), httpBody.length);
        if (resp.first != Error::Nil) {
            return std::make_pair(resp.first, ServerKey());
        }
        if (resp.second.empty()) {
            return std::make_pair(Error::CSOProxy_ResponseEmpty, ServerKey());
        }

        // "DynamicJsonDocument" is allocated on heap, so we need to free unsued memory
        // to allocate new memory.
        delete[] url.release();
        delete[] httpBody.buffer.release();

        // Parse response
        // JSON string has 7 keys <=> 7 JSON_OBJECT
        doc = DynamicJsonDocument(JSON_OBJECT_SIZE(7) + resp.second.length());
        auto jsonError = deserializeJson(doc, resp.second);
        if (jsonError) {
            return std::make_pair(
                Error::adaptExternalCode(ExtTag::ArduinoJSON, jsonError.code()), 
                ServerKey()
            );
        }
        auto serverErrorCode = (int32_t)doc["returncode"];
        if (serverErrorCode != 1) {
            return std::make_pair(
                Error::adaptExternalCode(ExtTag::Server, serverErrorCode), 
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
        return std::make_pair(errorCode, ServerKey());
    }
    return std::make_pair(
        Error::Nil, 
        ServerKey(
            obj_json["g_key"], 
            obj_json["n_key"], 
            obj_json["pub_key"]
        )
    );
}

std::pair<Error::Code, ServerTicket> Proxy::registerConnection(const ServerKey& serverKey) {
    // Build client secret key
    std::unique_ptr<byte> secretKey(new (std::nothrow) byte[32]);
    if (secretKey == nullptr) {
        return std::make_pair(Error::NotEnoughMemory, ServerTicket());
    }

    BigNum clientPrivKey = UtilsDH::generatePrivateKey();
    std::string clientPubKey = UtilsDH::calcPublicKey(serverKey.gKey, serverKey.nKey, clientPrivKey).toString();
    UtilsDH::calcSecretKey(serverKey.nKey, clientPrivKey, serverKey.pubKey, secretKey.get());

    // Build encrypt token
    Array<byte> token;
    std::unique_ptr<byte> iv(nullptr);
    std::unique_ptr<byte> authenTag(nullptr);
    auto errorCode = buildEncyptToken(clientPubKey.c_str(), secretKey, iv, authenTag, token);
    if (errorCode != Error::Nil) {
        return std::make_pair(errorCode, ServerTicket());
    }

    // Invoke API
    uint16_t ticketID;
    uint16_t lenServerTicketToken;
    std::string hubAddress;
    std::string serverPubKey;
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
                return std::make_pair(Error::NotEnoughMemory, ServerTicket());
            }
            serializeJson(doc, httpBody.buffer.get(), httpBody.length + 1);
        }
        
        DynamicJsonDocument doc(0);
        {
            // Build URL
            std::unique_ptr<char> url(new (std::nothrow) char[this->config->getCSOAddress().length() + 21]);
            if (url.get() == nullptr) {
                return std::make_pair(Error::NotEnoughMemory, ServerTicket());
            }
            sprintf(url.get(), "%s/register-connection", this->config->getCSOAddress().c_str());

            // Send request and receive response
            std::pair<Error::Code, std::string> resp = sendPOST(url.get(), httpBody.buffer.get(), httpBody.length);
            if (resp.first != Error::Nil) {
                return std::make_pair(resp.first, ServerTicket());
            }
            if (resp.second.empty()) {
                return std::make_pair(Error::CSOProxy_ResponseEmpty, ServerTicket());
            }

            // "DynamicJsonDocument" is allocated on heap, so we need to free unsued memory
            // to allocate new memory
            delete[] url.release();
            delete[] httpBody.buffer.release();

            // Parse response
            doc = DynamicJsonDocument(JSON_OBJECT_SIZE(9) + resp.second.length());
            auto jsonError = deserializeJson(doc, resp.second);
            if (jsonError) {
                return std::make_pair(
                    Error::adaptExternalCode(ExtTag::ArduinoJSON, jsonError.code()), 
                    ServerTicket()
                );
            }
            auto serverErrorCode = (int32_t)doc["returncode"];
            if (serverErrorCode != 1) {
                return std::make_pair(
                    Error::adaptExternalCode(ExtTag::Server, serverErrorCode), 
                    ServerTicket()
                );
            }
        }

        // Get info
        JsonObject obj_json = doc["data"];
        ticketID = (uint16_t)obj_json["ticket_id"];
        serverPubKey = (const char*)obj_json["pub_key"];
        hubAddress = (const char*)obj_json["hub_address"];

        const char* encodeData = (const char*)obj_json["ticket_token"];
        Array<byte> decodeData = UtilsBase64::decode(encodeData);
        serverTicketToken.reset(new (std::nothrow) byte[decodeData.length]);
        if (serverTicketToken == nullptr) {
            return std::make_pair(Error::NotEnoughMemory, ServerTicket());
        }
        lenServerTicketToken = decodeData.length;
        memcpy(serverTicketToken.get(), decodeData.buffer.get(), lenServerTicketToken);

        encodeData = (const char*)obj_json["iv"];
        decodeData = UtilsBase64::decode(encodeData);
        memcpy(iv.get(), decodeData.buffer.get(), LENGTH_IV);

        encodeData = (const char*)obj_json["auth_tag"];
        decodeData = UtilsBase64::decode(encodeData);
        memcpy(authenTag.get(), decodeData.buffer.get(), LENGTH_AUTHEN_TAG);        
    }
    
    // Build server aad
    Array<byte> aad;
    aad.length = 2 + 
                 hubAddress.length() + 
                 serverPubKey.length();
    aad.buffer.reset(new (std::nothrow) byte[aad.length]);
    if (aad.buffer == nullptr) {
        return std::pair<Error::Code, ServerTicket>(Error::NotEnoughMemory, ServerTicket());
    }
    memcpy(aad.buffer.get(), &ticketID, 2);
    memcpy(aad.buffer.get() + 2, hubAddress.c_str(), hubAddress.length());
    memcpy(aad.buffer.get() + (2 + hubAddress.length()), serverPubKey.c_str(), serverPubKey.length());

    // Build server secret key
    UtilsDH::calcSecretKey(serverKey.nKey, clientPrivKey, BigNum(serverPubKey.c_str()), secretKey.get());

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
        return std::make_pair(errorCode, ServerTicket());
    }

    // Parse server ticket token to bytes
    Result<byte*> ticket = Ticket::buildBytes(ticketID, token.buffer.get());
    if (ticket.errorCode != Error::Nil) {
        return std::make_pair(ticket.errorCode, ServerTicket());
    }
    
    // Done
    return std::make_pair(
        Error::Nil, 
        ServerTicket(
            Address::parse(hubAddress.c_str()),
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
std::pair<Error::Code, std::string> Proxy::sendPOST(const char* url, byte* content, uint16_t length) {
    HTTPClient http;
    http.setTimeout(20000); // 20s
    if (!http.begin(url)) {
        return std::make_pair(Error::CSOProxy_Disconnected, "");
    }

    http.addHeader("Content-Type", "application/json");
    auto status = http.POST(content, length);
    if (status != 200) {
        http.end();
        return std::make_pair(Error::adaptExternalCode(ExtTag::HTTP, status), "");
    }

    std::string resp(http.getString().c_str());
    http.end();
    return std::make_pair(Error::Nil, resp);
}

Error::Code Proxy::verifyDHKeys(const char* gKey, const char* nKey, const char* pubKey, const char* encodeSign) {
    // Build data
    uint16_t lenGKey = strlen(gKey);
    uint16_t lenNKey = strlen(nKey);
    uint16_t lenPubKey = strlen(pubKey);

    Array<byte> data;
    data.length = lenGKey + lenNKey + lenPubKey;
    data.buffer.reset(new (std::nothrow) byte[data.length]);
    if (data.buffer == nullptr) {
        return Error::NotEnoughMemory;
    }

    memcpy(data.buffer.get(), gKey, lenGKey);
    memcpy(data.buffer.get() + lenGKey, nKey, lenNKey);
    memcpy(data.buffer.get() + (lenGKey + lenNKey), pubKey, lenPubKey);

    // Decode signature to bytes
    Array<byte> signature(UtilsBase64::decode(encodeSign));
    
    // Verify
    return UtilsRSA::verifySignature(
        (byte*)this->config->getCSOPublicKey().c_str(), 
        signature.buffer.get(),
        signature.length,
        data.buffer.get(), 
        data.length
    );
}

Error::Code Proxy::buildEncyptToken(const char* clientPubKey, const std::unique_ptr<byte>& secretKey, std::unique_ptr<byte>& iv, std::unique_ptr<byte>& authenTag, Array<byte>& token) {
    // Decode client token
    Array<byte> decodedToken;
    {
        const std::string& encodeToken = this->config->getProjectToken();
        decodedToken = UtilsBase64::decode(encodeToken.c_str(), encodeToken.length());
    }

    // Build client aad
    Array<byte> aad;
    {
        uint16_t lenPubKey = strlen(clientPubKey);
        uint16_t lenPID = this->config->getProjectID().length();
        uint16_t lenCName = this->config->getConnectionName().length();
        aad.length = lenPID + lenCName + lenPubKey;
        aad.buffer.reset(new (std::nothrow) byte[aad.length]);
        if (aad.buffer == nullptr) {
            return Error::NotEnoughMemory;
        }
        memcpy(aad.buffer.get(), this->config->getProjectID().c_str(), lenPID);
        memcpy(aad.buffer.get() + lenPID, this->config->getConnectionName().c_str(), lenCName);
        memcpy(aad.buffer.get() + (lenPID + lenCName), clientPubKey, lenPubKey);
    }

    // Buil encrypt client token
    iv.reset(new (std::nothrow) byte[LENGTH_IV]);
    if (iv == nullptr) {
        return Error::NotEnoughMemory;
    }

    authenTag.reset(new (std::nothrow) byte[LENGTH_AUTHEN_TAG]);
    if (authenTag == nullptr) {
        return Error::NotEnoughMemory;
    }

    token.length = decodedToken.length;
    token.buffer.reset(new (std::nothrow) byte[token.length]);
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