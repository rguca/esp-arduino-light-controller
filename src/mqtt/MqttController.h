#pragma once
#include <map>
#include <WiFiClient.h>
#include <ArduinoMqttClient.h>
#include <ArduinoJson.h>
#include "log/log.h"

class MqttController : public MqttClient {
public:
    struct Settings  {
        const char* device_name;

        const char* host;
        const char* user;
        const char* password;
    };

    MqttController();

    void setup(Settings* settings);

    void registerTemperature(const char* name);
    void registerVoltage(const char* name);
    void registerDuration(const char* name);

    void registerLight(const char* name);

    void sendValue(const char* name, float value);
    void sendValue(const char* component, const char* name, const char* value);

    void poll();
    void onLightValue(const char* name, std::function<void(bool)> callback);

protected:
    struct Device {
        const char* ids;
        const char* name;
        const char* sw;
        const char* mdl;
        const char* mf;
        const char* sa;
    } device;

    WiFiClient wifi_client;
    std::map<String, std::function<void(String)>> on_message_callbacks;

    void registerSensor(const char* name, JsonDocument* doc);
    void registerComponent(const char* component, const char* name, JsonDocument* doc);

    String getTopic(const char* component, const char* name);
    String getStateTopic(const char* component, const char* name);
    String getHomeassistantTopic(const char* component, const char* name);
    String convertName(const char* name);
};
