#define TAG "Settings"
#include "SettingsController.h"

SettingsController::SettingsController() :
	rtc_memory("/settings")
{
}

void SettingsController::load() {
	if (this->rtc_data != nullptr) return;
	if (LittleFS.begin()) {
		LOG("Filesystem initialized")
	} else {
		LOGE("Filesystem initialization failed")
	}
	this->rtc_memory.begin();
	this->rtc_data = this->rtc_memory.getData();
	LOG("Loaded")
}

void SettingsController::save() {
	if (this->rtc_memory.save()) {
		LOG("Saved");
	} else {
		LOGE("Save failed")
	}
}

void SettingsController::persist() {
	RtcData* data = this->rtc_data;
	strcpy(data->mqtt_host, this->mqtt_settings->host);
	strcpy(data->mqtt_user, this->mqtt_settings->user);
	strcpy(data->mqtt_password, this->mqtt_settings->password);
	data->is_rebooted = false;
	if (this->rtc_memory.persist()) {
		LOG("Persisted");
	} else {
		LOGE("Persist failed")
	}
}

WifiController::Settings* SettingsController::getWifiSettings() {
	if (this->wifi_settings) return this->wifi_settings;

	this->load();
	RtcData* data = this->rtc_data;
	this->wifi_settings = new WifiController::Settings();
	this->wifi_settings->ap_ssid = DEVICE_NAME;
	this->wifi_settings->ap_password = DEVICE_PASSWORD;
	this->wifi_settings->parameters.push_back(new WiFiManagerParameter("mqtt_host", "MQTT host", data->mqtt_host, sizeof(data->mqtt_host) - 1));
	this->wifi_settings->parameters.push_back(new WiFiManagerParameter("mqtt_user", "MQTT user", data->mqtt_user, sizeof(data->mqtt_user) - 1));
	this->wifi_settings->parameters.push_back(new WiFiManagerParameter("mqtt_password", "MQTT password", data->mqtt_password, sizeof(data->mqtt_password) - 1));
	return this->wifi_settings;
}

MqttController::Settings* SettingsController::getMqttSettings() {
	if (this->mqtt_settings) return this->mqtt_settings;

	getWifiSettings();
	this->mqtt_settings = new MqttController::Settings();
	this->mqtt_settings->device_name = DEVICE_NAME;
	this->mqtt_settings->host = this->wifi_settings->parameters.at(0)->getValue();
	this->mqtt_settings->user = this->wifi_settings->parameters.at(1)->getValue();
	this->mqtt_settings->password = this->wifi_settings->parameters.at(2)->getValue();
	return this->mqtt_settings;
}

bool SettingsController::isRebooted() {
	return this->rtc_data->is_rebooted;
}

void SettingsController::setRebooted(bool value) {
	this->rtc_data->is_rebooted = value;
}

SettingsController::~SettingsController() {
	if (this->wifi_settings) {
		for (WiFiManagerParameter* p : this->wifi_settings->parameters) delete p;
	}
	delete this->wifi_settings;
	delete this->mqtt_settings;
}
