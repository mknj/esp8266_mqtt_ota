#define TOPIC_LENGTH 100
typedef struct {
	int valid;
	int size;
	int counter;
	char wlan_sid[80];
	char wlan_key[80];
	char mqtt_user[33];
	char mqtt_pw[33];
	char mqtt_host[33];
	uint16_t mqtt_port;
	char mqtt_cid[33];
	struct {
		char base[TOPIC_LENGTH + 1]; // basisverzeichnis für z.B. status und info
		char info[TOPIC_LENGTH + 1];
	} pubstat;
	struct {
		char config[TOPIC_LENGTH + 1]; // send config json here, it is also possible to only send parts of config.json
	} substat;
	struct {
		char cmd[TOPIC_LENGTH + 1]; // hier können Kommandos wie z.b. sleep hingesendet werden
		char led1[TOPIC_LENGTH + 1]; // wenn hier "0..." gepostet geht die led aus, bei anderen Werten an
	} sub;
	struct {
		char cmd[TOPIC_LENGTH + 1];     // Ausgaben des Kommandos landen hier
		char key1[TOPIC_LENGTH + 1]; // hier wird der status von key1 (gpio2?) gepostet entweder 1 oder 0
		char battery[TOPIC_LENGTH + 1]; // hier wird der batteriestatus gepostet
		char temperature1[TOPIC_LENGTH + 1];
	} pub;
	int baselen;
	char dummy[61];
} sct_config;

#define SUBSCRIBERS 3
void configLoop(void);

extern sct_config config;
bool configFromJSON(const char *data);
char* configToJSON(bool includepw = false);
void configLoadOrSetup(char *host, uint16_t port, char*user, char*pw, char *sid, char *key);
