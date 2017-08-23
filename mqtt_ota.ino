// List all used Arduino-Headerfiles here
// makefile will scan this file and set INCLUDE PATH accordingly

#include <ArduinoJson.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <FS.h>
#include <ESP8266WebServer.h>

// define is some workaroud for something
#define ARDUINO_ARCH_ESP8266
#include <PubSubClient.h>

#define BUTTON 0
#include "wifi.h"
#include "mqtt.h"
#include "test.h"
#include "config.h"

#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 2
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);

sct_config config;

// konfig.inc contains the below defined settings for my setup
#include "/home/bob/konfig.inc"

// #define WLAN      "YOUR WLAN SID"
// #define WLAN_PW   "YOUR WLAN PASSWORD"
// #define MQTT_USER "YOUR MQTT USERNAME"
// #define MQTT_PW   "YOUR MQTT PASSWORD"
// #define MQTT_HOST "YOUR MQTT SERVER"

ADC_MODE(ADC_VCC);
void setup() {
	Serial.begin(115200);
	configLoadOrSetup(MQTT_HOST, 1883, MQTT_USER, MQTT_PW, WLAN, WLAN_PW);
	wifiSetup(config.wlan_sid, config.wlan_key);
	mqttSetup(config.mqtt_host, config.mqtt_port, config.mqtt_user, config.mqtt_pw);
	test_setup();
	DS18B20.begin();
	pinMode(BUTTON, INPUT_PULLUP);
}

// debound a gpio pin
// val is the value of a gpio
// store is a variable that stored the old value and a counter
// if old!=new counter is incremented
// if counter >250, old is set to new, counter is cleared and 2 or 3 is returned once
//
// return values
//  0 -> debounced value is 0 
//  1 -> debounced value is 1 
//  2 -> debounced value changed from 1 to 0 
//  3 -> debounced value changed from 0 to 1 
//

int debounce(int val, int *store) {
	int old = *store & 1;
	if (old != val) {
		*store = *store + 2;
	}
	if (*store > 250) {
		*store = val;
		return val + 2;
	}
	return val;
}

int digitalReadDebounce(int pin, int *store) {
	return debounce(1 - digitalRead(pin), store);
}

void loop() {
	static long now = 0;
	static long lastMsg = 0;
	static int pressed = 0;
	static int buttonState = 0;

	now = millis();
	mqttLoop();
	if (config.pub.key1[0]) {
		pressed = digitalReadDebounce(BUTTON, &buttonState);
		if (pressed > 1) {
			snprintf(mqttSendMsgBuffer, 2, "%d", pressed - 2);
			mqtt.publish(config.pub.key1, mqttSendMsgBuffer);
		}
	}
	if (now - lastMsg > 1000 * 60 * 5) {
		lastMsg = now;
		if (config.pub.temperature1[0] != 0) {
			float tempC;
			for (int i = 0; i < 20; ++i) {
				DS18B20.requestTemperatures();
				tempC = DS18B20.getTempCByIndex(0);
				if (!(tempC == 85.0 || tempC == (-127.0))) {
					Serial.println(tempC);
					mqtt.publish(config.pub.temperature1, String(+tempC).c_str(), true);
					break;
				}
			}
		}
	}
	test_loop();
	wifiLoop();
	configLoop();
}
