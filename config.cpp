#include <FS.h>
#include "config.h"
#include "wifi.h"
#include "base66.hpp"
#include <ArduinoJson.h>
#include "mqtt.h"

static bool changed = false;
static bool reconnect = false;

char* configToJSON(bool includepw) {
	static char out[1000];
	StaticJsonBuffer<1000> jsonBuffer;

	JsonObject& root = jsonBuffer.createObject();
	int valid;
	root["valid"] = config.valid;
	root["size"] = config.size;
	root["counter"] = config.counter;
	root["wlan_sid"] = config.wlan_sid;
	if (includepw)
		root["wlan_key"] = config.wlan_key;
	root["mqtt_user"] = config.mqtt_user;
	if (includepw)
		root["mqtt_pw"] = config.mqtt_pw;
	root["mqtt_host"] = config.mqtt_host;
	root["mqtt_port"] = config.mqtt_port;
	root["mqtt_cid"] = config.mqtt_cid;
	JsonObject& pubstat = root.createNestedObject("pubstat");
	pubstat["base"] = config.pubstat.base;
	pubstat["info"] = config.pubstat.info;
	JsonObject& substat = root.createNestedObject("substat");
	substat["config"] = config.substat.config;
	JsonObject& sub = root.createNestedObject("sub");
	sub["cmd"] = config.sub.cmd;
	sub["led1"] = config.sub.led1;
	JsonObject& pub = root.createNestedObject("pub");
	pub["cmd"] = config.pub.cmd;
	pub["key1"] = config.pub.key1;
	pub["battery"] = config.pub.battery;
	pub["temperature1"] = config.pub.temperature1;
	root["baselen"] = config.baselen;

	root.prettyPrintTo(out, sizeof(out));
	out[sizeof(out)] = 0;
	return out;
}

static void configPub(char *dest, const char*b) {
	if (b && strcmp(dest, b)) {
		strncpy(dest, b, TOPIC_LENGTH);
		dest[TOPIC_LENGTH] = 0;
		changed = true;
	}
}
static void configSub(char *dest, const char*b) {
	if (b && strcmp(dest, b)) {
		strncpy(dest, b, TOPIC_LENGTH);
		dest[TOPIC_LENGTH] = 0;
		changed = true;
		reconnect = true;
	}
}
bool configFromJSON(const char *data) {
	StaticJsonBuffer<1000> jsonBuffer;
	JsonObject& root = jsonBuffer.parseObject(data);
	configPub(config.wlan_sid, root["wlan_sid"]);
	configPub(config.wlan_key, root["wlan_key"]);
	configPub(config.mqtt_user, root["mqtt_user"]);
	configPub(config.mqtt_pw, root["mqtt_pw"]);
	configPub(config.mqtt_host, root["mqtt_host"]);
	if ((const char*) root["mqtt_port"]) {
		uint16_t newport = root["mqtt_port"];
		if (newport != config.mqtt_port) {
			config.mqtt_port = newport;
			changed = true;
		}
	}
	configSub(config.sub.cmd, root["sub"]["cmd"]);
	configSub(config.sub.led1, root["sub"]["led1"]);
	configPub(config.pub.cmd, root["pub"]["cmd"]);
	configPub(config.pub.key1, root["pub"]["key1"]);
	configPub(config.pub.battery, root["pub"]["battery"]);
	configPub(config.pub.temperature1, root["pub"]["temperature1"]);
	return true;
}

static void configLoad(void) { // TODO auf JSON statt binär umstellen
	memset(&config, 0, sizeof(config));
	SPIFFS.begin();
	File configFile = SPIFFS.open("/config.bin", "r");
	if (configFile) {
		configFile.read((uint8_t*) &config, sizeof(sct_config));
		if (config.size != sizeof(sct_config)) {
			memset(&config, 0, sizeof(config));
			Serial.println("WRONG CONFIG SIZE");
		} else {
			Serial.println("LOAD SUCCESSFULL");
		}
		configFile.close();
	} else {
		Serial.println("CONFIG FILE NOT FOUND");
	}
}

static void configSave(void) {
	File configFile = SPIFFS.open("/config.bin", "w");
	if (configFile) {
		config.valid = 1;
		config.counter += 1;
		config.size = sizeof(sct_config);
		configFile.write((uint8_t*) &config, sizeof(sct_config));
	}
	configFile.close();
	File configFile2 = SPIFFS.open("/config.json", "w");
	if (configFile2) {
		char *c = configToJSON();
		size_t l = strlen(c);
		configFile2.write((uint8_t*) &c, l);
	}
	configFile2.close();
}
static void staticInit() {
	encode_base66((unsigned char *) wifiGetMACbin(), 6, (unsigned char*) config.mqtt_cid);
	snprintf(config.pubstat.base, TOPIC_LENGTH, "dev/%s", config.mqtt_cid);
	snprintf(config.pubstat.info, TOPIC_LENGTH, "dev/%s/info", config.mqtt_cid);
	snprintf(config.substat.config, TOPIC_LENGTH, "dev/%s/config", config.mqtt_cid);
	config.baselen = strlen(config.pubstat.base);

}
/*
 * config file is loaded, parameters are not used
 * if the file load fails, parameters are used and config file is written
 *
 */
void configLoadOrSetup(char *host, uint16_t port, char*user, char*pw, char *sid, char *key) {
	configLoad();
	staticInit();
	if (config.valid == 0) {
		config.mqtt_port = port;
		snprintf(config.sub.cmd, TOPIC_LENGTH, "dev/%s/stdin", config.mqtt_cid);
		snprintf(config.pub.cmd, TOPIC_LENGTH, "dev/%s/stdout", config.mqtt_cid);
		strcpy(config.wlan_sid, sid);
		strcpy(config.wlan_key, key);
		strncpy(config.mqtt_host, host, 33);
		strncpy(config.mqtt_pw, pw, 33);
		strncpy(config.mqtt_user, user, 33);
		configSave();
	}
}
void configLoop(void) {
	if (changed) {
		configSave();
		changed = false;
	}
	if (reconnect) {
		mqtt.disconnect();
		reconnect = false;
	}
}
