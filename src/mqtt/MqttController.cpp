#define TAG "MQTT"
#include "MqttController.h"

MqttController::MqttController() : MqttClient(wifi_client) {
}

void MqttController::setup(Settings* settings) {
    this->device.ids = strdup(String(ESP.getChipId(), HEX).c_str());
    this->device.name = settings->device_name;
    this->device.mf = "espressif";
    this->setTxPayloadSize(512);
    
    this->setUsernamePassword(settings->user, settings->password);
    if (this->connect(settings->host)) {
        LOG("Connected to %s", settings->host)
    } else {
        LOGE("Connection to %s failed! Error code = %d", settings->host, this->connectError())
    }
}

void MqttController::registerTemperature(const char* name) {
    StaticJsonDocument<512> doc;
    doc["dev_cla"] = "temperature";
    doc["unit_of_meas"] = "°C";
    doc["suggested_display_precision"] = 1;
    this->registerMeasurement(name, &doc);
    LOG("Temperature %s registered", name)
}

void MqttController::registerVoltage(const char* name) {
    StaticJsonDocument<512> doc;
    doc["dev_cla"] = "voltage";
    doc["unit_of_meas"] = "V";
    this->registerMeasurement(name, &doc);
    LOG("Voltage %s registered", name)
}

void MqttController::registerDuration(const char* name) {
    StaticJsonDocument<512> doc;
    doc["dev_cla"] = "duration";
    doc["unit_of_meas"] = "s";
    this->registerMeasurement(name, &doc);
    LOG("Duration %s registered", name)
}

void MqttController::registerMeasurement(const char* name, JsonDocument* doc) {
    String device_name = this->convertName(this->device.name);
    String name_ = this->convertName(name);
    String topic = String(device_name + "/sensor/" + name_ + "/state");

    (*doc)["stat_cla"] = "measurement";
    (*doc)["name"] = name;
    (*doc)["stat_t"] = topic;
    (*doc)["uniq_id"] = String(device_name + "-" + name_);

    JsonObject dev = doc->createNestedObject("dev");
    dev["ids"] = this->device.ids;
    dev["name"] = this->device.name;
    if (this->device.sw) dev["sw"] = this->device.sw;
    if (this->device.mdl) dev["mdl"] = this->device.mdl;
    if (this->device.mf) dev["mf"] = this->device.mf;
    if (this->device.sa) dev["sa"] = this->device.sa;

    this->beginMessage(String("homeassistant/sensor/" + device_name + "/" + name_ + "/config"), true);
    serializeJson(*doc, *this);
    this->endMessage();
}

void MqttController::sendValue(const char* name, float value) {
    String device_name = this->convertName(this->device.name);
    String name_ = this->convertName(name);
    String topic = String(device_name + "/sensor/" + name_ + "/state");
    this->beginMessage(topic, true);
    this->print(value);
    this->endMessage();
    LOG("Published %s: %f", name, value)
}

String MqttController::convertName(const char* name) {
    String name_ = String(name);
    name_.toLowerCase();
    name_.replace(' ', '-');
    return name_;
}
