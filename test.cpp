#define ARDUINO_ARCH_ESP8266
#include "mqtt.h"
#include "test.h"

static int32_t acker(int32_t i) {
	if(i<1)
		return 1;
	else
		return acker(i-1)+acker(i-2);
}

static void acker_cb(char *topic, int32_t value) {
	snprintf(mqttSendMsgBuffer,75,"acker2(%d)=%d\n", value,acker(value));
	mqtt.publish("outTopic", mqttSendMsgBuffer);
}
static void float_cb(char *topic, int32_t value) {
	double d=42.42;
	float f=43.43;
	d=d+4200.0;
	//float printing not supported by ESP8266 snprintf
	snprintf(mqttSendMsgBuffer,75,"topic=%s d=%d f=%d",topic, (int)(d*100),(int)(f*100));
	mqtt.publish("outTopic", mqttSendMsgBuffer);
}

void *p=0;

static void stacker(int32_t i) {
	uint32_t x[10];
	x[i%10]=i;
	p=&x;
	if(i>1) stacker(i-1);
}

static void stack_cb(char *topic, int32_t value) {

	void *op=&op;
	stacker(value);
	snprintf(mqttSendMsgBuffer,75,"stackdelta3 %d\n", (int32_t)p-(int32_t)op);
	mqtt.publish("stdout", mqttSendMsgBuffer);
}
static void busy_cb(char *topic, int32_t value) {

	int32_t a,b;
	int32_t c=0;
	snprintf(mqttSendMsgBuffer,75,"pretime %d input %d\n", millis(),value);
	mqtt.publish("stdout", mqttSendMsgBuffer);

	for(a=0;a<value;++a)
	{
		for(b=0;b<a;++b)
		{
			c=c+1;
		}
	}
	snprintf(mqttSendMsgBuffer,75,"posttime %d output %d\n", millis(),c);
	mqtt.publish("stdout", mqttSendMsgBuffer);
}
char topics[90][90];
static void registermany_cb(char *topic, int32_t value) {
	int i=0;
	for(i=0;i<value;++i)
	{
		snprintf(topics[i],20,"float%i",i);
		mqttSubscribeInteger(topics[i], float_cb);
	}
	mqtt.publish("outTopic", "registered many");

}
void test_setup() {
/*	// stack tests stack usage, currently the complete stack size is 4k
	mqttSubscribeInteger("stack", stack_cb);
	// tests recursion with the ackermann function
	mqttSubscribeInteger("acker", acker_cb);
	// tests the watchdog timeout
	mqttSubscribeInteger("busy", busy_cb);
	mqttSubscribeInteger("registermany", registermany_cb);
*/
}
void test_loop()
{

}
