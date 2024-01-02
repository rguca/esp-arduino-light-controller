#include "main.h"

SettingsController settings;

WifiController wifi;
MqttController mqtt_client;

bool is_config_enabled;
float battery_voltage;

void setup() {
	#ifdef ENABLE_LOG
		Serial.begin(74880);
		LOG("Start")
	#endif

	//WiFi.setSleepMode(WIFI_LIGHT_SLEEP, 10);
	wifi.onConnect([]() {
		sendState();
	});
	wifi.connect();

	pinMode(D0, INPUT_PULLDOWN_16); 
	is_config_enabled = digitalRead(D0) == 0;
	pinMode(D0, INPUT);
	LOG("Config: %d", is_config_enabled);

	if (is_config_enabled) {
		wifi.setup(settings.getWifiSettings(), [](void) {
			settings.persist();
			ESP.restart();
		});
	}

	battery_voltage = analogRead(A0) / 1024.0 * 5.66; // Needs to be measured early to not drop it too much

	settings.load();
}

void sendState() {
	mqtt_client.setup(settings.getMqttSettings());
	if (!mqtt_client.connected()) return;

	if (is_config_enabled) {
		mqtt_client.registerLight("Licht");
		mqtt_client.sendValue("light", "Licht", "OFF");
		mqtt_client.registerVoltage("Batterie");
		mqtt_client.registerDuration("Laufzeit");
	}

	mqtt_client.onLightValue("Licht", [](bool state) {
		pinMode(D1, OUTPUT);
		digitalWrite(D1, state);
		mqtt_client.sendValue("Batterie", battery_voltage);
		mqtt_client.sendValue("Laufzeit", millis() / 1000.0);
		deepSleep();
	});
}

void loop() {
	wifi.awaitConnect();
	wifi.update();
	if (mqtt_client.connected()) mqtt_client.poll();

	if (millis() > 5000) {
		deepSleep();
	}
}

void deepSleep() {
	if (is_config_enabled) return;

	mqtt_client.stop(); // Stop client so that outgoing messages are send

	int time = 3 * 60;
	LOG("Entering deep sleep for %ds", time)
	ESP.deepSleep(time * 1000000);
}
