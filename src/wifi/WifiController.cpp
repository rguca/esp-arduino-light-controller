#define TAG "Wifi"
#include "WifiController.h"

WifiController::WifiController() {
	this->setDebugOutput(false);
}

void WifiController::setup(Settings* settings, std::function<void()> on_save_callback) {
	for (WiFiManagerParameter* p : settings->parameters) {
		this->addParameter(p);
	}
	this->setSaveParamsCallback(on_save_callback);

	bool is_connected = this->autoConnect(settings->ap_ssid, settings->ap_password);
	if (is_connected) {
		this->startWebPortal();
		LOG("Web portal started")
	}
}

void WifiController::connect() {	
	LOG("Connecting WiFi...")
	WiFi.mode(WIFI_STA);
	WiFi.begin();
}

bool WifiController::isConnected() {
	return WiFi.status() == WL_CONNECTED;
}

bool WifiController::awaitConnect(unsigned long timeout) {
	if (WiFi.waitForConnectResult(timeout) == WL_CONNECTED) {
		LOG("Connected as %s", WiFi.localIP().toString().c_str());
		return true;
	}
	LOGE("Connect failed")
	return false;
}
