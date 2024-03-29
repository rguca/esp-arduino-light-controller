#pragma once
#include <WiFiManager.h>
#include "log/log.h"

class WifiController {
public:
	struct Settings {
		const char* ap_ssid;
		const char* ap_password;

		std::vector<WiFiManagerParameter*> parameters;
	};

	WifiController();

	void update();
	bool connect();
	bool isConnected();
	bool awaitConnect(unsigned long timeout = 3000);
	void onConnect(std::function<void()> callback);

	void setup(Settings* settings, std::function<void()> on_save_callback);

	~WifiController();

protected:
	typedef enum {
		DISCONNECTED,
		CONNECTING,
		ON_CONNECT,
		CONNECTED
	} connect_status;

	connect_status status;
	WiFiManager* wifi_manager;
	WiFiEventHandler on_connect_handler;
	std::function<void()> on_connect_callback;
};
