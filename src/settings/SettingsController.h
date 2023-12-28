#include <RTCMemory.h>
#include "log/log.h"
#include "wifi/WifiController.h"
#include "mqtt/MqttController.h"

class SettingsController {
public:
	const char* DEVICE_NAME = "Licht Controller";
	const char* DEVICE_PASSWORD = "temp12345";

	SettingsController();

	void load();
	void save();
	void persist();
	
	WifiController::Settings* getWifiSettings();
	MqttController::Settings* getMqttSettings();
	bool isRebooted();
	void setRebooted(bool value = true);

	~SettingsController();

protected:
	#pragma pack(push, 1)
	struct RtcData {
		bool is_rebooted;

		char mqtt_host[21];
		char mqtt_user[21];
		char mqtt_password[21];
	};
	#pragma pack(pop)
	// char (*__kaboom)[sizeof( RtcSettings )] = 1; // Prints the size in error

	RTCMemory<RtcData> rtc_memory;
	RtcData* rtc_data;
	WifiController::Settings* wifi_settings;
	MqttController::Settings* mqtt_settings;
};
