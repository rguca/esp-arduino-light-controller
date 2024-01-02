#define TAG "Wifi"
#include "WifiController.h"

WifiController::WifiController() {
	this->on_connect_handler = WiFi.onStationModeGotIP([this](const WiFiEventStationModeGotIP) {
		LOG("Connected as %s", WiFi.localIP().toString().c_str());
		this->status = ON_CONNECT;
	});
}

void WifiController::setup(Settings* settings, std::function<void()> on_save_callback) {
	this->wifi_manager = new WiFiManager();
	this->wifi_manager->setDebugOutput(false);
	for (WiFiManagerParameter* p : settings->parameters) {
		this->wifi_manager->addParameter(p);
	}
	this->wifi_manager->setSaveParamsCallback(on_save_callback);

	bool is_connected = this->wifi_manager->autoConnect(settings->ap_ssid, settings->ap_password);
	if (is_connected) {
		this->wifi_manager->startWebPortal();
		LOG("Web portal started")
	}
}

bool WifiController::connect() {	
	LOG("Connecting WiFi...")
	WiFi.mode(WIFI_STA);
	if (WiFi.begin() == WL_CONNECT_FAILED) {
		LOGE("Connect failed");
		return false;
	}
	this->status = CONNECTING;
	return true;
}

bool WifiController::isConnected() {
	return WiFi.status() == WL_CONNECTED;
}

bool WifiController::awaitConnect(unsigned long timeout) {
	if (this->isConnected()) return true;
	int result = WiFi.waitForConnectResult(timeout);
	if (result == WL_CONNECTED) return true;
	if (result == -1) {
		LOGE("Connect timeout");
		return false;
	}
	LOGE("Connect failed with status: %d", result)
	return false;
}

void WifiController::onConnect(std::function<void()> callback) {
	this->on_connect_callback = callback;
}

void WifiController::update() {
	switch (this->status) {
	case DISCONNECTED:
		break;
	case ON_CONNECT:
		if (this->on_connect_callback) {
			on_connect_callback();
		}
		this->status = CONNECTED;
		break;
	case CONNECTED:
		if (!this->isConnected()) {
			this->status = DISCONNECTED;
			LOG("Disconnected")
		}
		break;
	default:
		break;
	}

	if (this->wifi_manager) {
		this->wifi_manager->process();
	}
}

WifiController::~WifiController() {
	delete this->wifi_manager;
}
