#pragma once
#include <WiFiManager.h>
#include "log/log.h"

class WifiController: public WiFiManager {
public:
	struct Settings {
		const char* ap_ssid;
		const char* ap_password;

		std::vector<WiFiManagerParameter*> parameters;
	};

	WifiController();

	void connect();
	bool isConnected();
	bool awaitConnect(unsigned long timeout = 3000);

	void setup(Settings* settings, std::function<void()> on_save_callback);
};
