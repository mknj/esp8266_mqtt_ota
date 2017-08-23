# 
#  makeEspArduino.mk does all the hard work 
#  cd ; git clone https://github.com/plerup/makeEspArduino
#
# e.g. "make ota" will do OTA to ESP_ADDR 
ESP_ADDR=192.168.0.127
ESP_ADDR=192.168.0.167
ESP_ADDR=192.168.0.191
ESP_ADDR=192.168.0.195
ESP_ADDR=192.168.0.199
ESP_ADDR=192.168.0.211
ESP_ADDR=192.168.0.230
ESP_ADDR=192.168.0.231
ESP_ADDR=192.168.1.57
ESP_ADDR=192.168.1.106
ESP_ADDR=192.168.1.126
ESP_ADDR=192.168.1.228
ESP_ADDR=192.168.1.218
ESP_ADDR=192.168.1.232
ESP_ADDR=192.168.1.202

BUILD_DIR=build
FLASH_DEF=1M64
VERBOSE=1

CPP_EXTRA=-DARDUINO_SKETCH=$(PWD) -DMQTT_MAX_PACKET_SIZE=1400
#CPP_EXTRA=-fno-lto -fstack-usage
#C_EXTRA=-fno-lto -fstack-usage

all:


# extract compiler defaults
compilerdefaults: 
	/home/bob/.arduino15/packages/esp8266/tools/xtensa-lx106-elf-gcc/1.20.0-26-gb404fb9-2/bin/xtensa-lx106-elf-g++ -E -P -v -dD # "${INPUTS}"

inc:
	rm -rf /tmp/mkESP build
	make 2>&1|tr " " "\\n"|grep  -- "-I"|sort|uniq

include /home/bob/makeEspArduino/makeEspArduino.mk
