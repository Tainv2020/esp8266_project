make erase_flash
esptool.py --port /dev/ttyUSB0 write_flash 0x10000 main/clientinfo.txt
make flash monitor
