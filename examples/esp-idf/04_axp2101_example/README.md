[English](README.md) | [简体中文](README_ZH.md)

# AXP2101 power management

## What It Does

Initialize and configure the board power rails.

## Hardware Required

ESP32-S3-Touch-LCD-3.5B or ESP32-S3-Touch-LCD-3.5B-C. USB-C data cable.

## How to Run

1. Activate the ESP-IDF version listed in the [repository configuration](../../../.github/product.json).
2. Open a terminal in this example directory.
3. Run the commands below; replace `PORT` with your serial port.

```sh
idf.py set-target esp32s3
idf.py build
idf.py -p PORT flash monitor
```

Serial monitor: 115200 baud. Exit with Ctrl+].

## Expected Result

Initialization/status messages appear in the serial monitor; the display is not used by this example.

## Troubleshooting

If initialization fails, check USB power and the selected target. When changing ESP-IDF versions, run `idf.py fullclean` before rebuilding.
