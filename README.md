# Out of Tree NAU7802 Zephyr Driver

This repository serves as an example of how to use an "out of tree" driver with Zephyr's sensor API. The full write-up [can be found here](https://www.ubiqueiot.com/posts/out-of-tree-nau7802-zephyr-driver). The NAU7802 driver code is borrowed from [this repository by TinNotTim](https://github.com/TinNotTim/nau7802_loadcell_zephyr_driver).

<img width="1024" height="768" alt="hardware" src="https://github.com/user-attachments/assets/1fa9d8d7-9833-4b30-bb83-eb04f5597c6d" />

## Building this project for an ESP32 Feather
First, activate virtual environment:
```
source ~/zephyrproject/.venv/bin/activate
```

⚠️ Make sure to fetch the espressif blobs:
```
west blobs fetch hal_espressif
```

Then, build:
```
west build -p -b adafruit_feather_esp32/esp32/procpu
```

To flash:
```
west flash
```
To monitor:
```
west espressif monitor -p /dev/tty.wchusbserial575E0524461
```

The `adafruit_feather_esp32` board is only available in a very recent (at the time of publishing) Zephyr version.
