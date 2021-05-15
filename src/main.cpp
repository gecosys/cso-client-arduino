#include <Arduino.h>
#include <SPIFFS.h>
#include <WiFi.h>
// #include <utils/utils_rsa.h>
// extern "C" {
//     #include <crypto/base64.h>
// }
#include "config/config.h"
#include "cso_connector/connector.h"

bool setupDone = false;
TaskHandle_t Task1;
std::shared_ptr<IConnector> connector;

void exec(void *pvParameters) {
    connector->loopReconnect();
}

Error::Code callback(const char* sender, byte* data, uint16_t lenData) {
    // Handle response message
    return Error::Nil;
}

void setup() {
    Serial.begin(115200);

    // Check SPIFFS available if users want to read config from file
    if(!SPIFFS.begin()){
        Serial.println("Failed to mount SPIFFS");
        return;
    }

    // Connect wifi
    // "WiFi" will auto reconnect
    WiFi.begin("GE-Dev", "1234567891011");
    Serial.print("Connecting...");
    if (WiFi.waitForConnectResult() != WL_CONNECTED) {
        Serial.println("Can not connect wifi");
        return;
    }
    Serial.printf("\nLocal IP: %s\n", WiFi.localIP().toString().c_str());

    // Check successfull allocation memory for "connector" object
    try {
        connector = Connector::build(1024, Config::build(
            "75d383f4-594a-11ea-8b79-0242ac11000200010000",
            "JhsNVLDPVO3fIRDmt+iP76AUjeZXkFl9MjJdD5hgp6tjiuU5fmEByGflKplb9aA+MLx8FjjumctJJ51jKPATiQ==",
            "trung",
            "-----BEGIN PUBLIC KEY-----\nMIICIjANBgkqhkiG9w0BAQEFAAOCAg8AMIICCgKCAgEA2ys1SaOk+0ewDSssfCuL\nX0AdvRvcz2+VtrNMggF4VGxRcphB3DUL1HDWqal1yY4LfYN6v7rJ3J3kP24uhQB/\noonwXgaXTFcBODGIekKMsMtUFLgXeukMl6fb3wzFcPvJpIxeHyx1EizJdkEQTWBH\nA53s+KdeDCHgXhx/iyuu/2JoL91ziGtom9yy8Ll80AX/B5jBIGoXn1vvK9gcsw2S\n0mJdDSYgAOaH0loqzRJTW/AJkPKMXx+R5gTNrlJXN4kPRUPbSvK3tABEsC64xNiX\nrhuxTkH+GvCY+348O85XIWVWOP/CjhNyooZL4CaLYwo8Ic0ALu7hDr8CEp8Q9dYy\nQ23WsUVxhY1IuSg7/bwPBe/56cpxh3WUpY3dxOMPasdinGWneJ4ohZYjHQd7KAwC\n48EVEYDkhp46bkt9VzgQ2OX2YWfJL+9Et17UIWdss+5Yeql9I6uyi1RpsKgeR5w/\nhvivZhBS7/AzsvR0ejdcRaDsOtGuVYHOhlA0LpTzEpJuQdPdOf+TuefRB3LqO0Ki\nCr7gBHFFwlqy/tZ2zEucccx9clKAPH1MY3+R+yEDcJnzibYe68IFQS4fp4pv70Iw\niZFDtiGGFBilTQcbqW/vfQWqnI+4rsYEWEu4xN+AJpHvcRAXXD9lBta9+qnR+5fx\noBRT+o2UD8fKVs13QP+Sbq8CAwEAAQ==\n-----END PUBLIC KEY-----\n",
            "http://csoservice.goldeneyetech.com.vn/gateway/proxy"
        ));
    }
    catch(const std::runtime_error& e) {
        log_e("%s", e.what());
        return;
    }

    // Assign task for core2
    xTaskCreatePinnedToCore(
        exec,     /* Task function */
        "Task1",  /* name of task */
        7 * 1024, /* Stack size of task */
        NULL,     /* parameter of the task */
        1,        /* priority of the task */
        &Task1,   /* Task handle to keep track of created task */
        0         /* pin task to core 0 */
    );
    setupDone = true;
}

void loop() {
    if (!setupDone) {
        vTaskDelay(3000);
        return;
    }
    connector->listen(callback);
    if (connector->sendMessage("trung", new byte[3]{65, 66, 67}, 3, false, false) != Error::Nil) {
        Serial.println("Send message failed");
    }
    connector->loopReconnect();

    //// Check verify by RSA
    // char* cso_pub_key = "-----BEGIN PUBLIC KEY-----\nMIICIjANBgkqhkiG9w0BAQEFAAOCAg8AMIICCgKCAgEA2ys1SaOk+0ewDSssfCuL\nX0AdvRvcz2+VtrNMggF4VGxRcphB3DUL1HDWqal1yY4LfYN6v7rJ3J3kP24uhQB/\noonwXgaXTFcBODGIekKMsMtUFLgXeukMl6fb3wzFcPvJpIxeHyx1EizJdkEQTWBH\nA53s+KdeDCHgXhx/iyuu/2JoL91ziGtom9yy8Ll80AX/B5jBIGoXn1vvK9gcsw2S\n0mJdDSYgAOaH0loqzRJTW/AJkPKMXx+R5gTNrlJXN4kPRUPbSvK3tABEsC64xNiX\nrhuxTkH+GvCY+348O85XIWVWOP/CjhNyooZL4CaLYwo8Ic0ALu7hDr8CEp8Q9dYy\nQ23WsUVxhY1IuSg7/bwPBe/56cpxh3WUpY3dxOMPasdinGWneJ4ohZYjHQd7KAwC\n48EVEYDkhp46bkt9VzgQ2OX2YWfJL+9Et17UIWdss+5Yeql9I6uyi1RpsKgeR5w/\nhvivZhBS7/AzsvR0ejdcRaDsOtGuVYHOhlA0LpTzEpJuQdPdOf+TuefRB3LqO0Ki\nCr7gBHFFwlqy/tZ2zEucccx9clKAPH1MY3+R+yEDcJnzibYe68IFQS4fp4pv70Iw\niZFDtiGGFBilTQcbqW/vfQWqnI+4rsYEWEu4xN+AJpHvcRAXXD9lBta9+qnR+5fx\noBRT+o2UD8fKVs13QP+Sbq8CAwEAAQ==\n-----END PUBLIC KEY-----\n";
    // char* encodeSign = "YqJyNzN5qeYM/VxfMf8jxBlKYOZSbJ2ZCaNrifPsgZKOEkqC/7y31Ja4MT4b1qnRikWSJ5RMVmMRGniujCqP82upjKv5PQt0AgFPz/pjwGLgqmbFSAIvNDlhSG489E9qycGF34/uORpd2/2VgO98sAiLPCjxkj9tK/tu4zhALch5qFdqJy5Hl0FKrfVxRJdimZCefhDimwvJO0OhIvjRuOSMHVu7+Y2OxepULJAIw0srIJ50dM0XL+ZHChpLB7BxUZnKNJjW3xAtHC+++GG0Qr5ihPTeW+Un3FR9WADWVR37rqiFGw01+XrLyKztKGyk7wJAKgxjC4Bs08LnTlZepFGMQreU9B6ZjUwGjnzAQn23CBGmNrKxnTIGCgLCKQDAfQy/nJ9/AES8wzNL+DuPNcB02ju85bukofoJBWp87Z3PdGTI69EqA/wOOjV7UJWsghFZBRuQW84eiQy3MVOV0ukSMdft5WmshEdOdTbKdgKwjAQw5EXrHoqyFqkcDC4wfruumcQEJ8/XdDSDAEEqtauqGUGfSoL93aqPjvELQumq1yUVdbSu6gCZNIN0IDr0VRPRW++8kp7uHfvd0wYSJ7azu1kXiGAAY8I7EJkWaeHZiYgNAYcKHCz7OlOyqi+SwwVur60lVJgQfJr4CnS7nr8abseeyXAVaqC6U9gKs+A=";
    // size_t out_len = 0;
    // byte* sign = base64_decode((const byte*)encodeSign, strlen(encodeSign), &out_len);
    
    // char* gKey = "1748697639570378812467537465246230886691752414070090804620647019426003112251785454944648907222729854077019909002235037631967546539551768132243327043167002";
    // char* nKey = "8068134016695843152209603306715779224913639827587768266801407120575119088182884327515338722086486249237258531202684459788755043786868589584094381942349636";
    // char* serverPubKeyBytes = "7764585463098188841618901637090201967640358210735683371369237059034278139773281311687925916322273171540523591316328720142850093965758804322736593430939620";
    // uint16_t lenGKey = strlen(gKey);
	// uint16_t lenGNKey = lenGKey + strlen(nKey);
	// uint16_t lenBuffer = lenGNKey + strlen(serverPubKeyBytes);
    // byte* buffer = new byte[lenBuffer];
    // memcpy(buffer, gKey, lenGKey);
    // memcpy(buffer + lenGKey, nKey, lenGNKey);
    // memcpy(buffer + lenGNKey, serverPubKeyBytes, strlen(serverPubKeyBytes));

    // byte* tt = new byte[strlen(cso_pub_key)];
    // memcpy(tt, cso_pub_key, strlen(cso_pub_key));

    // auto code = UtilsRSA::verifySignature(tt, sign, buffer, lenBuffer);
    // if (code != SUCCESS) {
    //     Serial.println("Bytes signature");
    //     for (uint16_t i = 0; i < out_len; ++i) {
    //         Serial.printf("%d", sign[i]);
    //     }
    //     Serial.println();
        
    //     Serial.println("Bytes data");
    //     Serial.printf("%d\n", lenBuffer);
    //     for (uint16_t i = 0; i < lenBuffer; ++i) {
    //         Serial.printf("%d", buffer[i]);
    //     }
    //     Serial.println();
    // }
    // delete[] sign;
    // delete[] buffer;
    // delete[] tt;
    vTaskDelay(1000);
}