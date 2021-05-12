#include <Arduino.h>
#include <SD_MMC.h>
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
    // Check SD_MMC available to read config from file
    if(!SD_MMC.begin()){
        Serial.println("Failed to mount card");
        return;
    }
    setupDone = true;
    connector = Connector::build(1024, Config::build("./config.json"));

    xTaskCreatePinnedToCore(
        exec,     /* Task function */
        "Task1",  /* name of task */
        7 * 1024, /* Stack size of task */
        NULL,     /* parameter of the task */
        1,        /* priority of the task */
        &Task1,   /* Task handle to keep track of created task */
        0         /* pin task to core 0 */
    );
}

void loop() {
    if (!setupDone) {
        return;
    }
    connector->listen(callback);
    // Call send message
    vTaskDelay(100);
}