#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include "config.h"
#include "mqtt.h"
static char MAC[18];
static byte ar[6];
void APsetup();

unsigned char *wifiGetMACbin(void) {
	delay(10);
	WiFi.macAddress(ar);
	return (unsigned char*) ar;
}
char *wifiGetMAC(void) {
	WiFi.macAddress(ar);
	sprintf(MAC, "%2X%2X%2X%2X%2X%2X", ar[0], ar[1], ar[2], ar[3], ar[4], ar[5]);
	return MAC;
}
String wifiGetIP(void) {
	return WiFi.localIP().toString();
}
static void setup_ota() {
	ArduinoOTA.onStart([]() {
		Serial.println("Start");
	});
	ArduinoOTA.onEnd([]() {
		Serial.println("\nEnd");
	});
	ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
		Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
	});
	ArduinoOTA.onError([](ota_error_t error) {
		Serial.printf("Error[%u]: ", error);
		if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
		else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
		else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
		else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
		else if (error == OTA_END_ERROR) Serial.println("End Failed");
	});
	ArduinoOTA.begin();
}

/* Set these to your desired credentials. */
static const char *ssid = "mkDeviceSetup";
static const char *password = "mk";

ESP8266WebServer server(80);
bool APMode = false;

static void handleRoot() {
	APMode = false; //stop reboot timer if someone has used the webserver
	server.send(200, "text/html", "<h1>Hello From ESP8266</h1>");
}
static void handleUP() {
	APMode = false;
	configFromJSON(server.arg("plain").c_str());
	mqtt.publish("http", server.arg("plain").c_str());
	server.send(200, "text/html", "done");

}
static void handleDOWN() {
	APMode = false;
	mqtt.publish("http", server.arg("plain").c_str());
	server.send(200, "text/html", configToJSON(false));

}

static void HTTPsetup() {
	server.on("/", handleRoot);
	server.on("/UP", handleUP);
	server.on("/DOWN", handleDOWN);
	server.begin();
}

void wifiSetup(char *WLAN, char *WLAN_PW) {
	// We start by connecting to a WiFi network
	WiFi.macAddress(ar);
	sprintf(MAC, "%2X:%2X:%2X:%2X:%2X:%2X", ar[0], ar[1], ar[2], ar[3], ar[4], ar[5]);

	int retries = 60;
	WiFi.persistent(false);
	WiFi.begin(WLAN, WLAN_PW);
	while (WiFi.status() != WL_CONNECTED && retries > 0) {
		delay(500);
		Serial.print(WiFi.status());
		--retries;
	}
	if (retries == 0) {
		WiFi.softAP(ssid, password);
		APMode = true;
	}
	setup_ota();
	HTTPsetup();
	Serial.println("");
	Serial.println("WiFi connected");
	Serial.println("IP address: ");
	Serial.println(WiFi.localIP());
}

void wifiLoop() {
	ArduinoOTA.handle();

	if (APMode) {
		// TODO 15 minute timer to reboot and recheck if WLAN is back online
		//
	}
	server.handleClient();
}
