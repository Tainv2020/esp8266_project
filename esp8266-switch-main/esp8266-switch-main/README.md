#SWITCH V1.0.0

1/Clone SDK 
git clone https://github.com/espressif/ESP8266_RTOS_SDK.git
git checkout release/v3.3

2/Download FW
git clone https://gitlab.com/truonggiangbk/esp8266-switch.git

3/ Flash client ID
esptool.py --port /dev/ttyUSB0 write_flash 0x10000 main/clientinfo.txt

4/ Build
make flash monitor