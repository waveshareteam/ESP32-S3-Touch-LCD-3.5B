[English](README.md) | [简体中文](README_ZH.md)

# Factory Applications with Xiaozhi

## What It Does

Run the 320 x 480 desktop with Settings, Xiaozhi voice chat, Touch Test, Photos, Weather, Music, Sensor, 2048, Camera and Mic Test. Sensor displays RTC and QMI8658 data.

## Hardware Required

ESP32-S3-Touch-LCD-3.5B or ESP32-S3-Touch-LCD-3.5B-C, a USB-C data cable and a speaker. Camera functions require an OV5640; photos and music use a FAT32 microSD card. Xiaozhi requires Wi-Fi and service activation.

## How to Run

Use the ESP-IDF version in [product.json](../../.github/product.json), then run in this directory:

```sh
idf.py set-target esp32s3
idf.py build
idf.py -p PORT flash monitor
```

Replace `PORT` with your serial port. Use full `flash` when upgrading from the legacy firmware: it writes the new partition table and WakeNet model as well as the application. Open Settings to configure Wi-Fi, then open Xiaozhi and follow its activation prompt.

## Expected Result

The desktop opens at startup. Open Xiaozhi, complete activation, then say "Ni hao Xiao Zhi" or tap Talk to start a conversation; the camera tool can capture a photo for analysis. Other applications remain available from the launcher.

## Troubleshooting

If the wake word does not work, flash the complete project including the model partition. Keep `CONFIG_CODEC_I2C_BACKWARD_COMPATIBLE` disabled to share the BSP I2C bus. Check Wi-Fi and activation status for connection failures. The [legacy factory example](../esp-idf/01_factory/) remains available separately.
