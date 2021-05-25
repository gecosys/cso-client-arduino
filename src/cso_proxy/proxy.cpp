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
    // Make request body
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

    std::string gKey;
    std::string nKey;
    std::string pubKey;
    std::string encodeSign;
    {
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
        delete[] url.release();
        delete[] buffer.release();

        // Parse response
        if (resp.second.empty()) {
            return std::pair<Error::Code, ServerKey>(Error::RespEmpty, ServerKey());
        }
        DynamicJsonDocument doc(2048);
        deserializeJson(doc, resp.second);
        JsonObject obj = doc.as<JsonObject>();
        if (obj["returncode"].as<String>().toInt() != 1) {
            return std::pair<Error::Code, ServerKey>(Error::Other, ServerKey());
        }
        // "data" is JSON std::string which stores "exchange key" info
        deserializeJson(doc, obj["data"].as<std::string>());

        // Get info
        gKey = obj["g_key"].as<std::string>();
        nKey = obj["n_key"].as<std::string>();
        pubKey = obj["pub_key"].as<std::string>();
        encodeSign = obj["sign"].as<std::string>();
    }

    // Verify data
    Error::Code err = verifyDHKeys(gKey, nKey, pubKey, encodeSign);
    if (err != Error::Nil) {
        return std::pair<Error::Code, ServerKey>(err, ServerKey());
    }
    return std::pair<Error::Code, ServerKey>(Error::Nil, ServerKey(gKey.c_str(), nKey.c_str(), pubKey.c_str()));
}

std::pair<Error::Code, ServerTicket> Proxy::registerConnection(const ServerKey& serverKey) {
    // Build client secret key
    std::unique_ptr<byte> secretKey(Safe::new_arr<byte>(32));
    if (secretKey.get() == nullptr) {
        return std::pair<Error::Code, ServerTicket>(Error::NotEnoughMem, ServerTicket());
    }
    // Because "BigNumber.tostd::string()" returned pointer to dynamic memory,
    // uses "std::unique_ptr" for simplicity
    BigNumber clientPrivKey = UtilsDH::generatePrivateKey();
    std::unique_ptr<char> clientPubKey(UtilsDH::calcPublicKey(serverKey.gKey, serverKey.nKey, clientPrivKey).toString());
    UtilsDH::calcSecretKey(serverKey.nKey, clientPrivKey, serverKey.pubKey, secretKey.get());

    // Decode client token
    size_t lenToken;
    uint16_t length;
    std::unique_ptr<byte> decodedToken(nullptr);
    {
        length = this->config->getProjectToken().length();
        decodedToken.reset(Safe::new_arr<byte>(length));
        if (decodedToken.get() == nullptr) {
            return std::pair<Error::Code, ServerTicket>(Error::NotEnoughMem, ServerTicket());
        }
        memcpy(decodedToken.get(), this->config->getProjectToken().c_str(), length);
        decodedToken.reset(base64_decode(decodedToken.get(), length, &lenToken));
        if (decodedToken == nullptr || lenToken <= 0) {
            return std::pair<Error::Code, ServerTicket>(Error::Encrypt, ServerTicket());
        }
    }

    // Build client aad
    std::unique_ptr<byte> aad(nullptr);
    {
        uint16_t lenPubKey = strlen(clientPubKey.get());
        uint16_t lenPID = this->config->getProjectID().length();
        uint16_t lenCName = this->config->getConnectionName().length();
        length = lenPID + lenCName + lenPubKey;
        aad.reset(Safe::new_arr<byte>(length));
        if (aad.get() == nullptr) {
            return std::pair<Error::Code, ServerTicket>(Error::NotEnoughMem, ServerTicket());
        }
        memcpy(aad.get(), this->config->getProjectID().c_str(), lenPID);
        memcpy(aad.get() + lenPID, this->config->getConnectionName().c_str(), lenCName);
        memcpy(aad.get() + (lenPID + lenCName), clientPubKey.get(), lenPubKey);
    }

    // Buil encrypt client token
    std::unique_ptr<byte> iv(Safe::new_arr<byte>(LENGTH_IV));
    if (iv.get() == nullptr) {
        return std::pair<Error::Code, ServerTicket>(Error::NotEnoughMem, ServerTicket());
    }

    std::unique_ptr<byte> authenTag(Safe::new_arr<byte>(LENGTH_AUTHEN_TAG));
    if (authenTag.get() == nullptr) {
        return std::pair<Error::Code, ServerTicket>(Error::NotEnoughMem, ServerTicket());
    }

    std::unique_ptr<byte> token(Safe::new_arr<byte>(LENGTH_OUTPUT));
    if (token.get() == nullptr) {
        return std::pair<Error::Code, ServerTicket>(Error::NotEnoughMem, ServerTicket());
    }
    if (UtilsAES::encrypt(
        secretKey.get(), 
        decodedToken.get(), 
        lenToken, 
        aad.get(), 
        length, 
        iv.get(), 
        authenTag.get(),
        token.get()
    ) != SUCCESS) {
        return std::pair<Error::Code, ServerTicket>(Error::Encrypt, ServerTicket());
    }

    // Build request body
    std::unique_ptr<byte> buffer(nullptr);
    {
        size_t lenIV = 0;
        size_t lenPToken = 0;
        size_t lenAuthenTag = 0;
        StaticJsonDocument<JSON_OBJECT_SIZE(2)> doc;
        JsonObject obj = doc.as<JsonObject>();
        obj["project_id"] = this->config->getProjectID().c_str();
        obj["project_token"] = (char*)base64_decode(token.get(), LENGTH_IV, &lenPToken);
        obj["unique_name"] = this->config->getConnectionName().c_str();
        obj["public_key"] = clientPubKey.get();
        obj["iv"] = (char*)base64_decode(iv.get(), LENGTH_OUTPUT, &lenIV);
        obj["authen_tag"] = (char*)base64_decode(authenTag.get(), LENGTH_AUTHEN_TAG, &lenAuthenTag);

        length = 93 + 
                 lenIV +
                 lenPToken +
                 lenAuthenTag +
                 strlen(clientPubKey.get()) +
                 this->config->getProjectID().length() + 
                 this->config->getConnectionName().length();
        buffer.reset(Safe::new_arr<byte>(length + 1));
        if (buffer.get() == nullptr) {
            return std::pair<Error::Code, ServerTicket>(Error::NotEnoughMem, ServerTicket());
        }
        serializeJson(doc, buffer.get(), length + 1);
    }

    // Invoke API
    uint16_t ticketID;
    std::string hubAddress;
    std::string serverPubKey;
    std::unique_ptr<byte> serverTicketToken(nullptr);
    {
        // Build URL
        std::unique_ptr<char> url(Safe::new_arr<char>(this->config->getCSOAddress().length() + 21));
        if (url.get() == nullptr) {
            return std::pair<Error::Code, ServerTicket>(Error::NotEnoughMem, ServerTicket());
        }
        std::sprintf(url.get(), "%s/register-connection", this->config->getCSOAddress().c_str());

        // Send request and receive response
        std::pair<Error::Code, std::string> resp = sendPOST(url.get(), buffer.get(), length);
        if (resp.first != Error::Nil) {
            return std::pair<Error::Code, ServerTicket>(resp.first, ServerTicket());
        }
        delete[] url.release();
        delete[] buffer.release();

        // Parse response
        if (resp.second.empty()) {
            return std::pair<Error::Code, ServerTicket>(Error::RespEmpty, ServerTicket());
        }
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, resp.second);
        JsonObject obj = doc.as<JsonObject>();
        if (obj["returncode"].as<String>().toInt() != 1) {
            return std::pair<Error::Code, ServerTicket>(Error::Other, ServerTicket());
        }
        // "data" is JSON std::string which stores "register connection" info
        deserializeJson(doc, obj["data"].as<std::string>());

        // Get info
        ticketID = (uint16_t)obj["ticket_id"].as<String>().toInt();
        serverPubKey = obj["pub_key"].as<std::string>();
        hubAddress = obj["hub_address"].as<std::string>();

        std::string str(obj["iv"].as<std::string>());
        memcpy(iv.get(), str.c_str(), LENGTH_IV);

        str = obj["auth_tag"].as<std::string>();
        memcpy(authenTag.get(), str.c_str(), LENGTH_AUTHEN_TAG);

        lenToken = str.length();
        serverTicketToken.reset(Safe::new_arr<byte>(lenToken));
        if (serverTicketToken.get() == nullptr) {
            return std::pair<Error::Code, ServerTicket>(Error::NotEnoughMem, ServerTicket());
        }
        str = obj["ticket_token"].as<std::string>();
        memcpy(serverTicketToken.get(), str.c_str(), lenToken);
    }

    // Build server aad
    length = 2 + hubAddress.length() + serverPubKey.length();
    aad.reset(Safe::new_arr<byte>(length));
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
        lenToken, 
        aad.get(),
        length,
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

Error::Code Proxy::verifyDHKeys(const std::string& gKey, const std::string& nKey, const std::string& pubKey, const std::string& encodeSign) {
    // Build data
    uint16_t lenGKey = gKey.length();
    uint16_t lenNKey = nKey.length();
    uint16_t lenPubKey = pubKey.length();
    std::unique_ptr<byte> data(Safe::new_arr<byte>(lenGKey + lenNKey + lenPubKey));
    if (data.get() == nullptr) {
        return Error::NotEnoughMem;
    }

    memcpy(data.get(), gKey.c_str(), lenGKey);
    memcpy(data.get() + lenGKey, nKey.c_str(), lenNKey);
    memcpy(data.get() + (lenGKey + lenNKey), pubKey.c_str(), lenPubKey);

    // Decode sign to bytes
    size_t sizeSign = 0;
    std::unique_ptr<byte> sign(base64_decode((const byte*)encodeSign.c_str(), encodeSign.length(), &sizeSign));
    
    // Verify
    auto code = UtilsRSA::verifySignature(
        (uint8_t *)this->config->getCSOPublicKey().c_str(), 
        sign.get(),
        sizeSign,
        data.get(), 
        lenGKey + lenNKey + lenPubKey
    );

    if (code != SUCCESS) {
        Serial.printf("GKey: %s\n", gKey.c_str());
        Serial.printf("NKey: %s\n", nKey.c_str());
        Serial.printf("PubKey: %s\n", pubKey.c_str());
        Serial.printf("Sign: %s\n", encodeSign.c_str());

        Serial.println("Bytes signature");
        for (uint16_t i = 0; i < sizeSign; ++i) {
            Serial.printf("%d", sign.get()[i]);
        }
        Serial.println();
        
        Serial.println("Bytes data");
        Serial.printf("%d\n", lenGKey + lenNKey + lenPubKey);
        for (uint16_t i = 0; i < lenGKey + lenNKey + lenPubKey; ++i) {
            Serial.printf("%d", data.get()[i]);
        }
        Serial.println();
        char tt[500];
        UtilsRSA::parseError(code, tt, 500);
        Serial.println(tt);
        return Error::Verify;
    }
    return Error::Nil;
}