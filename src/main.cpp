#include <WiFi.h>
#include <SPIFFS.h>
#include <Arduino.h>
#include "config/config.h"
#include "cso_connector/connector.h"

bool setupDone = false;
TaskHandle_t loopReconnectTask;
std::unique_ptr<IConnector> connector;

void exec(void* pvParameters) {
    connector->loopReconnect();
}

Error::Code callback(const char* sender, uint8_t* data, uint16_t lenData) {
    // Handle response message
    for (int i = 0; i < lenData; ++i) {
        Serial.printf("%d ", data[i]);
    }
    Serial.println();
    return Error::Nil;
}

void setup() {
    Serial.begin(115200);

    // // Check SPIFFS available if users want to read config from file
    // if(!SPIFFS.begin()){
    //     log_e("Failed to mount SPIFFS");
    //     return;
    // }

    // Connect wifi
    // "WiFi" will auto reconnect
    WiFi.begin("GE", "1234567891011");
    Serial.print("Connecting...");
    if (WiFi.waitForConnectResult() != WL_CONNECTED) {
        log_e("Can not connect wifi");
        return;
    }
    Serial.printf("\nLocal IP: %s\n", WiFi.localIP().toString().c_str());

    // Check successfull allocation memory for "connector" object
    try {
        connector = Connector::build(1024, Config::build(
            "75d383f4-594a-11ea-8b79-0242ac11000200010000",
            "JhsNVLDPVO3fIRDmt+iP76AUjeZXkFl9MjJdD5hgp6tjiuU5fmEByGflKplb9aA+MLx8FjjumctJJ51jKPATiQ==",
            "trung3",
            "-----BEGIN PUBLIC KEY-----\nMIICIjANBgkqhkiG9w0BAQEFAAOCAg8AMIICCgKCAgEA2ys1SaOk+0ewDSssfCuL\nX0AdvRvcz2+VtrNMggF4VGxRcphB3DUL1HDWqal1yY4LfYN6v7rJ3J3kP24uhQB/\noonwXgaXTFcBODGIekKMsMtUFLgXeukMl6fb3wzFcPvJpIxeHyx1EizJdkEQTWBH\nA53s+KdeDCHgXhx/iyuu/2JoL91ziGtom9yy8Ll80AX/B5jBIGoXn1vvK9gcsw2S\n0mJdDSYgAOaH0loqzRJTW/AJkPKMXx+R5gTNrlJXN4kPRUPbSvK3tABEsC64xNiX\nrhuxTkH+GvCY+348O85XIWVWOP/CjhNyooZL4CaLYwo8Ic0ALu7hDr8CEp8Q9dYy\nQ23WsUVxhY1IuSg7/bwPBe/56cpxh3WUpY3dxOMPasdinGWneJ4ohZYjHQd7KAwC\n48EVEYDkhp46bkt9VzgQ2OX2YWfJL+9Et17UIWdss+5Yeql9I6uyi1RpsKgeR5w/\nhvivZhBS7/AzsvR0ejdcRaDsOtGuVYHOhlA0LpTzEpJuQdPdOf+TuefRB3LqO0Ki\nCr7gBHFFwlqy/tZ2zEucccx9clKAPH1MY3+R+yEDcJnzibYe68IFQS4fp4pv70Iw\niZFDtiGGFBilTQcbqW/vfQWqnI+4rsYEWEu4xN+AJpHvcRAXXD9lBta9+qnR+5fx\noBRT+o2UD8fKVs13QP+Sbq8CAwEAAQ==\n-----END PUBLIC KEY-----\n",
            "http://csoservice.goldeneyetech.com.vn/gateway/proxy"
        ));
    }
    catch(const char* ex) {
        log_e("%s", ex);
        return;
    }

    // Assign task for core2
    xTaskCreatePinnedToCore(
        exec,                   /* Task function */
        "Loop-Reconnect-Task",  /* name of task */
        30 * 1024,              /* Stack size of task */
        NULL,                   /* parameter of the task */
        1,                      /* priority of the task */
        &loopReconnectTask,     /* Task handle to keep track of created task */
        0                       /* pin task to core 0 */
    );
    setupDone = true;
}

const char* test() {
    return "Hello world";
}

void loop() {
    if (!setupDone) {
        vTaskDelay(3000 / portTICK_PERIOD_MS);
        return;
    }

    // connector->loopReconnect();
    connector->listen(callback);
    byte data[3] = {65, 66, 67};
    Error::Code errorCode = connector->sendMessage("trung3", data, 3, false, false);
    if (errorCode != Error::Nil) {
        log_e("%s", Error::getContent(errorCode));
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        return;
    }
    Serial.println("Send message success");
    vTaskDelay(50 / portTICK_PERIOD_MS);
}