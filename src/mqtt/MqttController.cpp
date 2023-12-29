#define TAG "MQTT"
#include "MqttController.h"

MqttController::MqttController() : MqttClient(wifi_client) {
}

void MqttController::setup(Settings* settings) {
	this->device.ids = strdup(String(ESP.getChipId(), HEX).c_str());
	this->device.name = settings->device_name;
	this->device.mf = "espressif";
	this->setId(String(this->device.mf) + "-" + String(this->device.ids));
	this->setTxPayloadSize(512);
	this->setCleanSession(false); // Don't delete old subscriptions
	
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
	doc["sug_dsp_prc"] = 1;
	this->registerSensor(name, &doc);
	LOG("Temperature %s registered", name)
}

void MqttController::registerVoltage(const char* name) {
	StaticJsonDocument<512> doc;
	doc["dev_cla"] = "voltage";
	doc["unit_of_meas"] = "V";
	this->registerSensor(name, &doc);
	LOG("Voltage %s registered", name)
}

void MqttController::registerDuration(const char* name) {
	StaticJsonDocument<512> doc;
	doc["dev_cla"] = "duration";
	doc["unit_of_meas"] = "s";
	this->registerSensor(name, &doc);
	LOG("Duration %s registered", name)
}

void MqttController::registerSensor(const char* name, JsonDocument* doc) {
	(*doc)["stat_cla"] = "measurement";
	this->registerComponent("sensor", name, doc);
}

void MqttController::registerLight(const char* name, bool default_state) {
	StaticJsonDocument<512> doc;
	doc["retain"] = true;
	doc["cmd_t"] = this->getStateTopic("light", name);
	this->registerComponent("light", name, &doc);
	this->sendValue("light", name, default_state ? "ON" : "OFF");
	LOG("Light %s registered", name)
}

void MqttController::onLightChange(const char* name, std::function<void(bool)> callback) {
	String topic = this->getStateTopic("light", name);
	this->on_message_callbacks[topic] = [callback](String message) { 
		callback(message == "ON"); 
	};
	this->subscribe(topic);
	LOG("Light %s subscribed", name)
}

void MqttController::registerComponent(const char* component, const char* name, JsonDocument* doc) {
	(*doc)["name"] = name;
	(*doc)["stat_t"] = this->getStateTopic(component, name);
	(*doc)["uniq_id"] = String(this->device.mf) + "-" + this->device.ids + "-" + name;

	JsonObject dev = doc->createNestedObject("dev");
	dev["ids"] = this->device.ids;
	dev["name"] = this->device.name;
	if (this->device.sw) dev["sw"] = this->device.sw;
	if (this->device.mdl) dev["mdl"] = this->device.mdl;
	if (this->device.mf) dev["mf"] = this->device.mf;
	if (this->device.sa) dev["sa"] = this->device.sa;

	this->beginMessage(this->getHomeassistantTopic(component, name), true);
	serializeJson(*doc, *this);
	this->endMessage();
}

void MqttController::sendValue(const char* name, float value) {
	this->beginMessage(this->getStateTopic("sensor", name), true);
	this->print(value);
	this->endMessage();
	LOG("Published %s: %f", name, value)
}

 void MqttController::sendValue(const char* component, const char* name, const char* value) {
	this->beginMessage(this->getStateTopic(component, name), true);
	this->print(value);
	this->endMessage();
	LOG("Published %s: %s", name, value)
 }

String MqttController::getTopic(const char* component, const char* name) {
	String device_name = this->convertName(this->device.name);
	String name_ = this->convertName(name);
	return device_name + "/" + component + "/" + name_;
}

String MqttController::getStateTopic(const char* component, const char* name) {
	return this->getTopic(component, name) + "/state";
}

String MqttController::getHomeassistantTopic(const char* component, const char* name) {
	String device_name = this->convertName(this->device.name);
	String name_ = this->convertName(name);
	return String("homeassistant/") + component + "/" + device_name + "/" + name_ + "/config";
}

String MqttController::convertName(const char* name) {
	String name_ = String(name);
	name_.toLowerCase();
	name_.replace(' ', '-');
	return name_;
}

void MqttController::poll() {
	int message_size = this->parseMessage();
	if (message_size) {
		String topic = this->messageTopic();
		String message;
		while (this->available()) {
				message += (char)this->read();
		}
		LOG("Received topic: %s, message: %s", topic.c_str(), message.c_str());
		if (this->on_message_callbacks.count(topic)) {
				this->on_message_callbacks[topic](message);
		}
	}
	MqttClient::poll();
}
