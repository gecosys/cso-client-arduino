#include <WiFi.h>
#include <SPIFFS.h>
#include <Arduino.h>
#include "config/config.h"
#include "utils/bignum.h"
#include "cso_connector/connector.h"

bool setupDone = false;
TaskHandle_t loopReconnectTask;
std::unique_ptr<IConnector> connector;

void exec(void *pvParameters) {
    connector->loopReconnect();
}

Error::Code callback(const char* sender, byte* data, uint16_t lenData) {
    // Handle response message
    for (int i = 0; i < lenData; ++i) {
        Serial.printf("%d ", data[i]);
    }
    Serial.println();
    return Error::Nil;
}

void setup() {
    Serial.begin(115200);

    // Check SPIFFS available if users want to read config from file
    if(!SPIFFS.begin()){
        log_e("Failed to mount SPIFFS");
        return;
    }

    // Connect wifi
    // "WiFi" will auto reconnect
    WiFi.begin("Wifi SSID", "Wifi password");
    Serial.print("Connecting...");
    if (WiFi.waitForConnectResult() != WL_CONNECTED) {
        log_e("Can not connect wifi");
        return;
    }
    Serial.printf("\nLocal IP: %s\n", WiFi.localIP().toString().c_str());

    // Check successfull allocation memory for "connector" object
    try {
        connector = Connector::build(1024, Config::build(
            "project-id",
            "project-token",
            "connection-name",
            "cso-public-key",
            "cso-address"
        ));
    }
    catch(const std::runtime_error& e) {
        log_e("%s", e.what());
        return;
    }

    // Assign task for core2
    xTaskCreatePinnedToCore(
        exec,                   /* Task function */
        "Loop_Reconnect_Task",  /* name of task */
        30 * 1024,              /* Stack size of task */
        NULL,                   /* parameter of the task */
        1,                      /* priority of the task */
        &loopReconnectTask,     /* Task handle to keep track of created task */
        0                       /* pin task to core 0 */
    );
    setupDone = true;
}

void loop() {
    if (!setupDone) {
        vTaskDelay(3000 / portTICK_PERIOD_MS);
        return;
    }

    // connector->loopReconnect();
    connector->listen(callback);
    byte data[3] = {65, 66, 67};
    Error::Code errorCode = connector->sendMessage("trung", data, 3, false, false);
    if (errorCode != Error::Nil) {
        log_e("%s", Error::getContent(errorCode));
    }
    vTaskDelay(50 / portTICK_PERIOD_MS);
}