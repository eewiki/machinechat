const char* host     = "ESP-OTA"; // Used for MDNS resolution
const char* ssid     = "DemoWiFi";
const char* password = "BeagleBone";

#include <WebOTA.h>

// the setup function runs once when you press reset or power the board
void setup() {
	Serial.begin(115200);

	init_wifi(ssid, password, host);

	// Defaults to 8080 and "/webota"
	//webota.init(80, "/update");
}

// the loop function runs over and over again forever
void loop() {
	int md = 1000;

	webota.delay(md);

	webota.handle();
}
