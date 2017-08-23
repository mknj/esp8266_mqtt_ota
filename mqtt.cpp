// define is some workaroud for something

#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
WiFiClient espClient;
#include "config.h"
#include "mqtt.h"
#include "wifi.h"
#include <FS.h>

PubSubClient mqtt(espClient);
char mqttSendMsgBuffer[200];
#define xstr(s) #s
#define str(s) xstr(s)

static void config_cb(char *t,uint8_t *v,unsigned int len);
static void cmd_cb(char *t,uint8_t *v,unsigned int len);
static void led1_cb(char *t,uint8_t *v,unsigned int len);

void callback(char* topic, byte* payload, unsigned int length) {
	if(strcmp(topic,config.substat.config)==0)
	{
		config_cb(topic,payload,length);
	}
	if(strcmp(topic,config.sub.cmd)==0)
	{
			cmd_cb(topic,payload,length);
	}
	if(strcmp(topic,config.sub.led1)==0)
	{
			led1_cb(topic,payload,length);
	}
}

static void led1_cb(char *topic,uint8_t *payload,unsigned int len)
{
  Serial.println("CONSOLE DISABLED BY LED!");
  pinMode(LED_BUILTIN, OUTPUT); // Initialize the BUILTIN_LED pin as an output, WARNING DISABLES Serial.print
  if ((char) payload[0] == '0') {
    digitalWrite(LED_BUILTIN, HIGH);  // inverted logic
  } else {
    digitalWrite(LED_BUILTIN, LOW);
  }
}


static void config_cb(char *t,uint8_t *v,unsigned int len)
{
  v[len]=0;
  configFromJSON((char*)v);
}

static void cmd_cb(char *t,uint8_t *v,unsigned int len)
{
	Serial.println("CMD:");
	int i;
	v[len]=0;
	  if (strcmp((char*)v,"cstat")==0)
	  {
			Serial.println(config.counter);
			Serial.println(config.valid);
			Serial.println(config.size);
			Serial.println(config.sub.cmd);
			Serial.println(config.sub.led1);
	  }
	  if (strcmp((char*)v,"config")==0)
	  {
		  mqtt.publish(config.pub.cmd,configToJSON());
	  }
	  if (strcmp((char*)v,"restart")==0)
	  {
		  	  ESP.restart();
	  }
	  if (strcmp((char*)v,"writefile")==0)
	  {
		  	 File f=SPIFFS.open("/data","a+");
		  	 f.print("x");
		  	 f.close();
	  }
	  if (strcmp((char*)v,"dir")==0)
	  {
		  char info[1000];
		  StaticJsonBuffer<250> jsonBuffer;
		  JsonObject& root = jsonBuffer.createObject();
		  Dir dir = SPIFFS.openDir("/");
		  while (dir.next()) {
		      File f = dir.openFile("r");
			  root[dir.fileName()] = f.size();
			  f.close();
		  }
		  root.prettyPrintTo(info);
		  mqtt.publish(config.pub.cmd,info);
	  }
	  if (strncmp((char*)v,"sleep",5)==0)
	  {
		  unsigned int sec=atol((char*)(v+6));
		  if (sec>3600) sec=3600;
		  if (sec<1) sec=60;

	      ESP.deepSleep(sec * 1000000); // sleep sec seconds, then restart (if Reset is connected to deepsleep pin(GPIO16))
	  }
}

static void send_info()
{
  char info[1000];
  StaticJsonBuffer<250> jsonBuffer;

  JsonObject& root = jsonBuffer.createObject();
  root["ip"] = wifiGetIP();
  root["sdk"] = system_get_sdk_version();
  root["stack"] = cont_get_free_stack(&g_cont);
  root["heap"] = system_get_free_heap_size();
  root["vcc"] = ESP.getVcc()/1000.0;
  root["time"] = millis();
  root["buildtime"] = String(__DATE__)+" "+String(__TIME__);
  root["project"] = str(ARDUINO_SKETCH);
  root["configcounter"] = config.counter;
  root["mac"] = wifiGetMAC();
  root.printTo(info);
  info[1000]=0;
  mqtt.publish(config.pubstat.info, info, true);
}

void mqttSetup(char *host,uint16_t port,char*user,char*pw)
{
	mqtt.setServer(config.mqtt_host, config.mqtt_port);
	mqtt.setCallback(callback);
}
void mqttLoop(void) {
	static unsigned long lastInfo=millis()-60*60*1000;
	static int subscribed=0;
	static bool pleaseReconnect=false;
	static long lastTry = 0;
	if (!mqtt.connected()) {
		pleaseReconnect=false;
		if (millis() - lastTry > 1000 * 5) {
			lastTry=millis();
			if (mqtt.connect(config.mqtt_cid, config.mqtt_user, config.mqtt_pw, config.pubstat.base, 0, true, "error")) {
  				mqtt.publish(config.pubstat.base, "on", true);
  				subscribed=0;
			} else {
				Serial.print("failed, rc=");
				Serial.print(mqtt.state());
				Serial.println(" try again in 5 seconds");
			}
		}
	} else
	{
		if(millis()-lastInfo>60*60*1000) 
		{
			send_info();
			lastInfo=millis();
		}
		if(subscribed<SUBSCRIBERS)
		{
			if (subscribed==0 && config.substat.config[0]!=0) { mqtt.subscribe(config.substat.config);};
			if (subscribed==1 && config.sub.cmd[0]!=0) { mqtt.subscribe(config.sub.cmd);};
			if (subscribed==2 && config.sub.led1[0]!=0) { mqtt.subscribe(config.sub.led1);};
			delay(10); // needed when subscribing to many topics at once
			++subscribed;
		}
		if(pleaseReconnect)
		{
			mqtt.disconnect();
		}
	};
	mqtt.loop();
}

