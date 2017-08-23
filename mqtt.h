#define ARDUINO_ARCH_ESP8266
#include <PubSubClient.h>

typedef void (*mqttcb)(char* topic, uint8_t* payload, unsigned int length);
typedef void (*mqttcb_int)(char* topic, int32_t value);
typedef void (*mqttcb_float)(char* topic, float value);

void mqttSetup(char *host,uint16_t port,char*user,char*pw);
void mqttLoop(void);
extern PubSubClient mqtt;
void mqttUnsubscribe(char *topic);
void mqttSubscribeString(char *topic,mqttcb cb);
void mqttSubscribeInteger(char *topic,mqttcb_int cb);
void mqttSubscribeFloat(char *topic,mqttcb_float cb);
extern char mqttSendMsgBuffer[];
