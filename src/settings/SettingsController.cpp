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
	if (!this->rtc_data->is_rebooted) {
		this->rtc_data->is_rebooted = true;
		LOG("Set rebooted flag")
	}
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

// void SettingsController::enterLightSleep(uint32 duration) {
// 	LOG("Entering light sleep for %dms", duration)
// 	#ifdef ENABLE_LOG
// 		Serial.flush();
// 	#endif

// 	extern os_timer_t* timer_list;
// 	timer_list = nullptr;  // stop (but don't disable) the 4 OS timers
	
// 	station_config fwconfig;
// 	// memset(fwconfig.bssid, 0xff, 6);
// 	wifi_station_get_config(&fwconfig);

// 	//wifi_station_disconnect(); 
// 	wifi_set_opmode_current(NULL_MODE); // Need to disconnect WiFi, otherwise WDT soft reset error
// 	wifi_fpm_set_sleep_type(LIGHT_SLEEP_T);
// 	wifi_fpm_open();
// 	// wifi_fpm_set_wakeup_cb(callback);
// 	wifi_fpm_do_sleep(duration * 1000); // Sleep range = 10000 ~ 268,435,454 uS (0xFFFFFFE, 2^28-1)
// 	delay(duration + 1);

// 	LOG("Woke from light sleep")
// 	wifi_fpm_do_wakeup();
// 	wifi_fpm_close(); // disable force sleep function

// 	// fwconfig.bssid_set = 1;
// 	// uint8_t bssid[] = { 0xF4,0xEC,0x38,0xAB,0x27,0x10 };
// 	// std::copy(bssid, bssid + 6, fwconfig.bssid);
// 	// wifi_station_set_config_current(&fwconfig);
// 	// wifi_set_channel(1);

// 	wifi_set_opmode_current(STATION_MODE);
// 	wifi_station_connect();
// }

SettingsController::~SettingsController() {
	if (this->wifi_settings) {
		for (WiFiManagerParameter* p : this->wifi_settings->parameters) delete p;
	}
	delete this->wifi_settings;
	delete this->mqtt_settings;
}
