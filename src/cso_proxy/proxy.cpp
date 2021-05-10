#include <cstdio>
#include <BigNumber.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <crypto/base64.h>
#include "cso_proxy/proxy.h"
#include "config/config.h"
#include "message/ticket.h"
#include "utils/utils_dh.h"
#include "utils/utils_rsa.h"
#include "utils/utils_aes.h"

std::shared_ptr<IProxy> Proxy::build(std::shared_ptr<IConfig> config) {
    return std::shared_ptr<IProxy>(Safe::new_obj<Proxy>(config));
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
        JsonObject obj = doc.as<JsonObject>();
        obj["project_id"] = this->config->getProjectID();
        obj["unique_name"] = this->config->getConnectionName();

        lenBuffer = 34 + 
                    this->config->getProjectID().length() + 
                    this->config->getConnectionName().length();
        buffer.reset(Safe::new_arr<byte>(lenBuffer));
        if (buffer.get() == nullptr) {
            return std::pair<Error::Code, ServerKey>(Error::NotEnoughMem, ServerKey());
        }
        serializeJson(doc, buffer.get(), lenBuffer);
    }

    String gKey;
    String nKey;
    String pubKey;
    String sign;
    {
        // Send request and receive response
        char url[50];
        std::sprintf(url, "%s/exchange-key", this->config->getCSOAddress().c_str());
        std::pair<Error::Code, String> resp = sendPOST(url, buffer.get(), lenBuffer);
        if (resp.first != Error::Nil) {
            return std::pair<Error::Code, ServerKey>(resp.first, ServerKey());
        }
        delete[] buffer.release();

        DynamicJsonDocument doc(1024);
        deserializeJson(doc, resp.second);
        JsonObject obj = doc.as<JsonObject>();
        if (obj["returncode"].as<String>().toInt() != 1) {
            return std::pair<Error::Code, ServerKey>(Error::Other, ServerKey());
        }
        // "data" is JSON string which stores "exchange key" info
        deserializeJson(doc, obj["data"].as<String>());

        // Get info
        gKey = obj["g_key"].as<String>();
        nKey = obj["n_key"].as<String>();
        pubKey = obj["pub_key"].as<String>();
        sign = obj["sign"].as<String>();
    }

    // Verify data
    Error::Code err = verifyDHKeys(gKey, nKey, pubKey, sign);
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
    // Because "BigNumber.toString()" returned pointer to dynamic memory,
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
        byte* buffer = Safe::new_arr<byte>(length);
        if (buffer == nullptr) {
            return std::pair<Error::Code, ServerTicket>(Error::NotEnoughMem, ServerTicket());
        }
        this->config->getProjectToken().getBytes(buffer, length);
        decodedToken.reset(base64_decode(buffer, length, &lenToken));
        delete[] buffer;
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
        this->config->getProjectID().getBytes(aad.get(), lenPID);
        this->config->getConnectionName().getBytes(aad.get(), lenCName, lenPID);
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
        obj["project_id"] = this->config->getProjectID();
        obj["project_token"] = String((char*)base64_decode(token.get(), LENGTH_IV, &lenPToken));
        obj["unique_name"] = this->config->getConnectionName();
        obj["public_key"] = String(clientPubKey.get());
        obj["iv"] = String((char*)base64_decode(iv.get(), LENGTH_OUTPUT, &lenIV));
        obj["authen_tag"] = String((char*)base64_decode(authenTag.get(), LENGTH_AUTHEN_TAG, &lenAuthenTag));

        length = 93 + 
                 lenIV +
                 lenPToken +
                 lenAuthenTag +
                 strlen(clientPubKey.get()) +
                 this->config->getProjectID().length() + 
                 this->config->getConnectionName().length();
        buffer.reset(Safe::new_arr<byte>(length));
        if (buffer.get() == nullptr) {
            return std::pair<Error::Code, ServerTicket>(Error::NotEnoughMem, ServerTicket());
        }
        serializeJson(doc, buffer.get(), length);
    }

    // Invoke API
    uint16_t ticketID;
    String hubAddress;
    String serverPubKey;
    std::unique_ptr<byte> serverTicketToken(nullptr);
    {
        // Send request and receive response
        char url[60];
        std::sprintf(url, "%s/register-connection", this->config->getCSOAddress().c_str());
        std::pair<Error::Code, String> resp = sendPOST(url, buffer.get(), length);
        if (resp.first != Error::Nil) {
            return std::pair<Error::Code, ServerTicket>(resp.first, ServerTicket());
        }
        delete[] buffer.release();

        DynamicJsonDocument doc(1024);
        deserializeJson(doc, resp.second);
        JsonObject obj = doc.as<JsonObject>();
        if (obj["returncode"].as<String>().toInt() != 1) {
            return std::pair<Error::Code, ServerTicket>(Error::Other, ServerTicket());
        }
        // "data" is JSON string which stores "register connection" info
        deserializeJson(doc, obj["data"].as<String>());

        // Get info
        ticketID = (uint16_t)obj["ticket_id"].as<String>().toInt();
        serverPubKey = obj["pub_key"].as<String>();
        hubAddress = obj["hub_address"].as<String>();

        String str(obj["iv"].as<String>());
        str.getBytes(iv.get(), LENGTH_IV);

        str = obj["auth_tag"].as<String>();
        str.getBytes(authenTag.get(), LENGTH_AUTHEN_TAG);

        str = obj["ticket_token"].as<String>();
        lenToken = str.length();
        serverTicketToken.reset((byte*)std::malloc(lenToken * sizeof(byte)));
        if (!serverTicketToken.get()) {
            return std::pair<Error::Code, ServerTicket>(Error::NotEnoughMem, ServerTicket());
        }
        str.getBytes(serverTicketToken.get(), lenToken);
    }

    // Build server aad
    length = 2 + hubAddress.length();
    aad.reset(Safe::new_arr<byte>(length + serverPubKey.length()));
    if (aad.get() == nullptr) {
        return std::pair<Error::Code, ServerTicket>(Error::NotEnoughMem, ServerTicket());
    }
    memcpy(aad.get(), &ticketID, 2);
    hubAddress.getBytes(aad.get(), hubAddress.length(), 2);
    serverPubKey.getBytes(aad.get(), serverPubKey.length(), length);

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
            res.data,
            secretKey.release(), // Don't use "get()" because after function done
                                 // "std::unique_ptr's destructor" will delete memory
            ticketID
        )
    );
}

//========
// PRIVATE
//========
std::pair<Error::Code, String> Proxy::sendPOST(const char* url, byte* buffer, uint16_t length) {
    HTTPClient http;
    if (!http.begin(url)) {
        return std::pair<Error::Code, String>(Error::NotConnectServer, "");
    }

    http.addHeader("Content-Type", "application/json");
    if (http.POST(buffer, length) != 200) {
        return std::pair<Error::Code, String>(Error::HttpError, "");
    }

    String resp = http.getString();
    http.end();
    if (resp.isEmpty()) {
        return std::pair<Error::Code, String>(Error::Empty, "");
    }
    return std::pair<Error::Code, String>(Error::Nil, resp);
}

Error::Code Proxy::verifyDHKeys(const String& gKey, const String& nKey, const String& pubKey, const String& sign) {
    std::unique_ptr<byte> signBytes(Safe::new_arr<byte>(LENGTH_SIGN));
    if (signBytes.get() == nullptr) {
        return Error::NotEnoughMem;
    }

    uint16_t length = this->config->getCSOPublicKey().length();
    std::unique_ptr<byte> pubKeyBytes(Safe::new_arr<byte>(length));
    if (pubKeyBytes.get() == nullptr) {
        return Error::NotEnoughMem;
    }

    // Make data
    uint16_t lenGKey = gKey.length();
    uint16_t lenNKey = nKey.length();
    uint16_t lenPubKey = pubKey.length();
    std::unique_ptr<byte> data(Safe::new_arr<byte>(lenGKey + lenNKey + lenPubKey));
    if (data.get() == nullptr) {
        return Error::NotEnoughMem;
    }
    gKey.getBytes(data.get(), lenGKey);
    nKey.getBytes(data.get(), lenNKey, lenGKey);
    pubKey.getBytes(data.get(), lenPubKey, lenGKey + lenNKey);

    // Convert public key to bytes
    this->config->getCSOPublicKey().getBytes(pubKeyBytes.get(), length);

    // Convert sign to bytes
    sign.getBytes(signBytes.get(), LENGTH_SIGN);
    
    // Verify
    if (UtilsRSA::verifySignature(
        pubKeyBytes.get(), 
        signBytes.get(), 
        data.get(), 
        lenGKey + lenNKey + lenPubKey
    ) != SUCCESS) {
        return Error::Verify;
    }
    return Error::Nil;
}