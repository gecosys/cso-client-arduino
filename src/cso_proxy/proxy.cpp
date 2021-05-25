#include <cstdio>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
extern "C" {
    #include <crypto/base64.h>
}
#include "cso_proxy/proxy.h"
#include "message/ticket.h"
#include "utils/utils_dh.h"
#include "utils/utils_rsa.h"
#include "utils/utils_aes.h"

std::shared_ptr<IProxy> Proxy::build(std::shared_ptr<IConfig> config) {
    IProxy* obj = Safe::new_obj<Proxy>(config);
    if (obj == nullptr) {        
        throw std::runtime_error("[cso_proxy/Proxy::build(std::shared_ptr<IConfig>)]Not enough memory to create object");
    }
    return std::shared_ptr<IProxy>(obj);
}

Proxy::Proxy(std::shared_ptr<IConfig>& config) {
    this->config.swap(config);
}

Proxy::~Proxy() {}

std::pair<Error::Code, ServerKey> Proxy::exchangeKey() {
    // "data" is a reference to "doc".
    // "deserializeJson" from HTTP response to "doc" copied strings, 
    // if put "doc" and "data" into block and extract neccessary fields into variables,
    // we will copy strings one more time.
    // Because all keys in JSON are used, we should keep "doc".
    // Keeping "doc" will waste a some bytes for unnecessary fields and JsonDocument's data structure
    DynamicJsonDocument doc(0);
    JsonObject data;
    {
        // Build request body
        std::unique_ptr<byte> buffer(nullptr);
        uint16_t lenBuffer;
        {
            StaticJsonDocument<JSON_OBJECT_SIZE(2)> doc;
            JsonObject obj = doc.to<JsonObject>();
            obj["project_id"] = this->config->getProjectID().c_str();
            obj["unique_name"] = this->config->getConnectionName().c_str();

            lenBuffer = 34 + 
                        this->config->getProjectID().length() + 
                        this->config->getConnectionName().length();
            buffer.reset(Safe::new_arr<byte>(lenBuffer + 1));
            if (buffer.get() == nullptr) {
                return std::pair<Error::Code, ServerKey>(Error::NotEnoughMem, ServerKey());
            }
            serializeJson(doc, buffer.get(), lenBuffer + 1);
        }

        // Build URL
        std::unique_ptr<char> url(Safe::new_arr<char>(this->config->getCSOAddress().length() + 14));
        if (url.get() == nullptr) {
            return std::pair<Error::Code, ServerKey>(Error::NotEnoughMem, ServerKey());
        }
        std::sprintf(url.get(), "%s/exchange-key", this->config->getCSOAddress().c_str());

        // Send request and receive response
        std::pair<Error::Code, std::string> resp = sendPOST(url.get(), buffer.get(), lenBuffer);
        if (resp.first != Error::Nil) {
            return std::pair<Error::Code, ServerKey>(resp.first, ServerKey());
        }
        if (resp.second.empty()) {
            return std::pair<Error::Code, ServerKey>(Error::RespEmpty, ServerKey());
        }

        // "DynamicJsonDocument" is allocated on heap, so we need to free unsued memory
        // to allocate new memory.
        delete[] url.release();
        delete[] buffer.release();

        // Parse response
        // JSON string has 7 keys <=> 7 JSON_OBJECT
        doc = DynamicJsonDocument(JSON_OBJECT_SIZE(7) + resp.second.length());
        if (deserializeJson(doc, resp.second)) {
            return std::pair<Error::Code, ServerKey>(Error::Other, ServerKey());
        }

        if ((int16_t)doc["returncode"] != 1) {
            return std::pair<Error::Code, ServerKey>(Error::Other, ServerKey());
        }
        data = doc["data"];
    }
    
    // "JsonObject[key]" will returns a no type reference, 
    // so we need to cast type or extract explicit by "JsonObject[key].as<type>()".
    // However, "verifyDHKeys" function defined type params, extracting implicit will be called.
    Error::Code err = verifyDHKeys(data["g_key"], data["n_key"], data["pub_key"], data["sign"]);
    if (err != Error::Nil) {
        return std::pair<Error::Code, ServerKey>(err, ServerKey());
    }
    return std::pair<Error::Code, ServerKey>(Error::Nil, ServerKey(data["g_key"], data["n_key"], data["pub_key"]));
}

std::pair<Error::Code, ServerTicket> Proxy::registerConnection(const ServerKey& serverKey) {
    // Build client secret key
    std::unique_ptr<byte> secretKey(Safe::new_arr<byte>(32));
    if (secretKey.get() == nullptr) {
        return std::pair<Error::Code, ServerTicket>(Error::NotEnoughMem, ServerTicket());
    }
    // Because "BigNumber.toString()" returned pointer to dynamic memory,
    // uses "std::unique_ptr" for simplicity
    BigNumber clientPrivKey = UtilsDH::generatePrivateKey();
    std::unique_ptr<char> clientPubKey(UtilsDH::calcPublicKey(serverKey.gKey, serverKey.nKey, clientPrivKey).toString());
    UtilsDH::calcSecretKey(serverKey.nKey, clientPrivKey, serverKey.pubKey, secretKey.get());

    // Build encrypt token
    std::unique_ptr<byte> iv(nullptr);
    std::unique_ptr<byte> token(nullptr);
    std::unique_ptr<byte> authenTag(nullptr);
    auto errorCode = buildEncyptToken(clientPubKey.get(), secretKey.get(), iv, authenTag, token);
    if (errorCode != Error::Nil) {
        return std::pair<Error::Code, ServerTicket>(errorCode, ServerTicket());
    }

    // Invoke API
    uint16_t ticketID;
    uint16_t lenTicketToken;
    std::string hubAddress;
    std::string serverPubKey;
    std::unique_ptr<byte> serverTicketToken(nullptr);
    {
        // Build request body
        uint16_t lenBuffer;
        std::unique_ptr<byte> buffer(nullptr);
        {
            size_t lenIV = 0;
            size_t lenPToken = 0;
            size_t lenAuthenTag = 0;
            StaticJsonDocument<JSON_OBJECT_SIZE(6)> doc;
            JsonObject obj = doc.as<JsonObject>();
            obj["project_id"] = this->config->getProjectID().c_str();
            obj["project_token"] = (char*)base64_decode(token.get(), LENGTH_IV, &lenPToken);
            obj["unique_name"] = this->config->getConnectionName().c_str();
            obj["public_key"] = clientPubKey.get();
            obj["iv"] = (char*)base64_decode(iv.get(), LENGTH_OUTPUT, &lenIV);
            obj["authen_tag"] = (char*)base64_decode(authenTag.get(), LENGTH_AUTHEN_TAG, &lenAuthenTag);

            lenBuffer = 93 + 
                        lenIV +
                        lenPToken +
                        lenAuthenTag +
                        strlen(clientPubKey.get()) +
                        this->config->getProjectID().length() + 
                        this->config->getConnectionName().length();
            buffer.reset(Safe::new_arr<byte>(lenBuffer + 1));
            if (buffer.get() == nullptr) {
                return std::pair<Error::Code, ServerTicket>(Error::NotEnoughMem, ServerTicket());
            }
            serializeJson(doc, buffer.get(), lenBuffer + 1);
        }
        
        DynamicJsonDocument doc(0);
        {
            // Build URL
            std::unique_ptr<char> url(Safe::new_arr<char>(this->config->getCSOAddress().length() + 21));
            if (url.get() == nullptr) {
                return std::pair<Error::Code, ServerTicket>(Error::NotEnoughMem, ServerTicket());
            }
            std::sprintf(url.get(), "%s/register-connection", this->config->getCSOAddress().c_str());

            // Send request and receive response
            std::pair<Error::Code, std::string> resp = sendPOST(url.get(), buffer.get(), lenBuffer);
            if (resp.first != Error::Nil) {
                return std::pair<Error::Code, ServerTicket>(resp.first, ServerTicket());
            }
            if (resp.second.empty()) {
                return std::pair<Error::Code, ServerTicket>(Error::RespEmpty, ServerTicket());
            }

            // "DynamicJsonDocument" is allocated on heap, so we need to free unsued memory
            // to allocate new memory
            delete[] url.release();
            delete[] buffer.release();

            // Parse response
            doc = DynamicJsonDocument(JSON_OBJECT_SIZE(9) + resp.second.length());
            if (deserializeJson(doc, resp.second)) {
                return std::pair<Error::Code, ServerTicket>(Error::Other, ServerTicket());
            }
            if ((int16_t)doc["returncode"] != 1) {
                return std::pair<Error::Code, ServerTicket>(Error::Other, ServerTicket());
            }
        }

        // Get info
        JsonObject data = doc["data"];
        lenTicketToken = strlen(data["ticket_token"]);
        serverTicketToken.reset(Safe::new_arr<byte>(lenTicketToken));
        if (serverTicketToken.get() == nullptr) {
            return std::pair<Error::Code, ServerTicket>(Error::NotEnoughMem, ServerTicket());
        }
        
        ticketID = (uint16_t)data["ticket_id"];
        serverPubKey = (const char*)data["pub_key"];
        hubAddress = (const char*)data["hub_address"];

        memcpy(iv.get(), data["iv"], LENGTH_IV);
        memcpy(authenTag.get(), data["auth_tag"], LENGTH_AUTHEN_TAG);
        memcpy(serverTicketToken.get(), data["ticket_token"], lenTicketToken);
    }

    // Build server aad
    uint16_t lenAad = 2 + 
                      hubAddress.length() + 
                      serverPubKey.length();
    std::unique_ptr<byte> aad(Safe::new_arr<byte>(lenAad));
    if (aad.get() == nullptr) {
        return std::pair<Error::Code, ServerTicket>(Error::NotEnoughMem, ServerTicket());
    }
    memcpy(aad.get(), &ticketID, 2);
    memcpy(aad.get() + 2, hubAddress.c_str(), hubAddress.length());
    memcpy(aad.get() + (2 + hubAddress.length()), serverPubKey.c_str(), serverPubKey.length());

    // Build server secret key
    UtilsDH::calcSecretKey(serverKey.nKey, clientPrivKey, BigNumber(serverPubKey.c_str()), secretKey.get());

    // Decrypt server ticket token
    if (UtilsAES::decrypt(
        secretKey.get(), 
        serverTicketToken.get(), 
        lenTicketToken, 
        aad.get(),
        lenAad,
        iv.get(), 
        authenTag.get(),
        token.get()
    ) != SUCCESS) {
        return std::pair<Error::Code, ServerTicket>(Error::Decrypt, ServerTicket());
    }

    // Parse server ticket token to bytes
    Result<byte*> res = Ticket::buildBytes(ticketID, token.get());
    if (res.errorCode != SUCCESS) {
        return std::pair<Error::Code, ServerTicket>(Error::Build, ServerTicket());
    }

    // Done
    return std::pair<Error::Code, ServerTicket>(
        Error::Nil, 
        ServerTicket(
            hubAddress.c_str(), 
            ticketID,
            res.data,
            secretKey.release() // Don't use "get()" because after function done
                                // "std::unique_ptr's destructor" will delete memory
        )
    );
}

//========
// PRIVATE
//========
std::pair<Error::Code, std::string> Proxy::sendPOST(const char* url, byte* buffer, uint16_t length) {
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
    http.setTimeout(20000);
    if (!http.begin(url)) {
        // secureClient.stop();
        return std::pair<Error::Code, std::string>(Error::NotConnectServer, "");
    }

    http.addHeader("Content-Type", "application/json");
    if (http.POST(buffer, length) != 200) {
        return std::pair<Error::Code, std::string>(Error::HttpError, "");
    }

    std::string resp(http.getString().c_str());
    http.end();
    // secureClient.stop();
    return std::pair<Error::Code, std::string>(Error::Nil, resp);
}

Error::Code Proxy::verifyDHKeys(const char* gKey, const char* nKey, const char* pubKey, const char* encodeSign) {
    // Build data
    uint16_t lenGKey = strlen(gKey);
    uint16_t lenNKey = strlen(nKey);
    uint16_t lenPubKey = strlen(pubKey);
    std::unique_ptr<byte> data(Safe::new_arr<byte>(lenGKey + lenNKey + lenPubKey));
    if (data.get() == nullptr) {
        return Error::NotEnoughMem;
    }

    memcpy(data.get(), gKey, lenGKey);
    memcpy(data.get() + lenGKey, nKey, lenNKey);
    memcpy(data.get() + (lenGKey + lenNKey), pubKey, lenPubKey);

    // // Convert public key to bytes
    // memcpy(pubKeyBytes.get(), this->config->getCSOPublicKey().c_str(), length);

    // Decode sign to bytes
    size_t out_len = 0;
    std::unique_ptr<byte> sign(base64_decode((const byte*)encodeSign, strlen(encodeSign), &out_len));
    
    size_t sizeSign = 0;
    std::unique_ptr<byte> sign(base64_decode((const byte*)encodeSign.c_str(), encodeSign.length(), &sizeSign));
    
    // Verify
    if (UtilsRSA::verifySignature(
        (uint8_t *)this->config->getCSOPublicKey().c_str(), 
        sign.get(),
        sizeSign,
        data.get(), 
        lenGKey + lenNKey + lenPubKey
    ) != 0) {
        return Error::Verify;
    }
    return Error::Nil;
}

Error::Code Proxy::buildEncyptToken(const char* clientPubKey, const byte* secretKey, std::unique_ptr<byte>& iv, std::unique_ptr<byte>& authenTag, std::unique_ptr<byte>& token) {
    // Decode client token
    size_t lenDecodeToken;
    std::unique_ptr<byte> decodedToken(nullptr);
    {
        uint16_t lenEncodeToken = this->config->getProjectToken().length();
        decodedToken.reset(Safe::new_arr<byte>(lenEncodeToken));
        if (decodedToken.get() == nullptr) {
            return Error::NotEnoughMem;
        }

        memcpy(decodedToken.get(), this->config->getProjectToken().c_str(), lenEncodeToken);
        decodedToken.reset(base64_decode(decodedToken.get(), lenEncodeToken, &lenDecodeToken));
        if (decodedToken == nullptr || lenDecodeToken <= 0) {
            return Error::Decrypt;
        }
    }

    // Build client aad
    uint16_t lenAad;
    std::unique_ptr<byte> aad(nullptr);
    {
        uint16_t lenPubKey = strlen(clientPubKey);
        uint16_t lenPID = this->config->getProjectID().length();
        uint16_t lenCName = this->config->getConnectionName().length();
        lenAad = lenPID + lenCName + lenPubKey;
        aad.reset(Safe::new_arr<byte>(lenAad));
        if (aad.get() == nullptr) {
            return Error::NotEnoughMem;
        }
        memcpy(aad.get(), this->config->getProjectID().c_str(), lenPID);
        memcpy(aad.get() + lenPID, this->config->getConnectionName().c_str(), lenCName);
        memcpy(aad.get() + (lenPID + lenCName), clientPubKey, lenPubKey);
    }

    // Buil encrypt client token
    iv.reset(Safe::new_arr<byte>(LENGTH_IV));
    if (iv.get() == nullptr) {
        return Error::NotEnoughMem;
    }

    authenTag.reset(Safe::new_arr<byte>(LENGTH_AUTHEN_TAG));
    if (authenTag.get() == nullptr) {
        return Error::NotEnoughMem;
    }

    token.reset(Safe::new_arr<byte>(LENGTH_OUTPUT));
    if (token.get() == nullptr) {
        return Error::NotEnoughMem;
    }
    if (UtilsAES::encrypt(
        secretKey, 
        decodedToken.get(), 
        lenDecodeToken, 
        aad.get(), 
        lenAad, 
        iv.get(), 
        authenTag.get(),
        token.get()
    ) != SUCCESS) {
        return Error::Encrypt;
    }
    return Error::Nil;
}