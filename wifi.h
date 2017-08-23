void wifiSetup(char *WLAN,char *WLAN_PW) ;
void wifiLoop();
char *wifiGetMAC(void);
String wifiGetIP(void);
unsigned char *wifiGetMACbin(void);

// workaround for reference bug in cont_get_free_stack(&g_cont) 
extern "C" {
#include <user_interface.h>
#include <cont.h>
  extern cont_t g_cont;
}

