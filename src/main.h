#include "log/log.h"
#include "settings/SettingsController.h"
#include "wifi/WifiController.h"
#include "mqtt/MqttController.h"

void setup();
void setup_wifi();
void setup_mqtt();
void sendState();
void loop();
void deepSleep();
