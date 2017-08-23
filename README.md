
# ESP8266 example that includes 
 - gpio input
 - blue led output
 - MQTT
 - HTTP
 - WiFi ACCESS POINT for initial configuration
 - JSON config file parsing
 - DS18B20 temperature sensor (must not be used together with gpio)
 - MAC based uniq id for each ESP8266 module
 - very simple shell
 - voltage monitor

everything is easily configurable at runtime via JSON.

# example topic tree

  - dev/GP40_bth         "on" or "error" this is the last will topic
  - dev/GP40_bth/info    JSON with ip address and other information
  - dev/GP40_bth/config  post json file with config entries here 
  - dev/GP40_bth/stdin   send shell commands here (i.e. dir, config)
  - dev/GP40_bth/stdout  receive shell output here

examples:

```sh
$ mosquitto_sub -t "dev/GP40_bth/#" 
$ mosquitto_pub -t "dev/GP40_bth/config" -m '{"pub":{"key1":"keystate"}}'
$ mosquitto_pub -t "dev/XA40XSAh/config" -m '{"sub":{"led1":"keystate"}}'
$ mosquitto_pub -t "dev/GP40_bth/stdin" -m 'config'
$ mosquitto_pub -t "dev/GP40_bth/stdin" -m 'dir'
```

* dev/GP40_bth/info
```
 {"ip":"192.168.1.202","sdk":"1.5.3(aec24ac9)","stack":2352,"heap":34432,"vcc":2.551,"time":14361,"buildtime":"Aug 23 2017 22:25:38","project":"mqtt_ota","configcounter":1,"mac":"18FE34FDBB61"}
```

* config
```

{
  "wlan_sid": "ESPTEST",
  "mqtt_user": "alice",
  "mqtt_host": "192.168.1.1",
  "mqtt_port": 1883,
  "mqtt_cid": "GP40_bth",
  "sub": {
    "cmd": "dev/GP40_bth/stdin",
    "led1": "keytopic"
  },
  "pub": {
    "cmd": "dev/GP40_bth/stdout",
    "key1": "keytopic",
    "battery": "currentvoltage",
    "temperature1": "templivingroom"
  }
}
```


# Build dependencies:
- Arduino IDE + ESP8266 SDK (see https://github.com/esp8266/Arduino)
- PubSubClient https://pubsubclient.knolleary.net/ ( set MQTT_MAX_PACKET_SIZE manually to e.g. 1400 if you are not using the makefile)
