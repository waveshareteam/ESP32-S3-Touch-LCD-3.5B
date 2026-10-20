[English](README.md) | [简体中文](README_ZH.md)

# Camera web server

## What It Does

Serve a camera preview over a 2.4 GHz Wi-Fi network.

## Hardware Required

ESP32-S3-Touch-LCD-3.5B or ESP32-S3-Touch-LCD-3.5B-C. OV5640 camera and 2.4 GHz Wi-Fi. The enclosure version includes a camera; add one for the bare board.

## How to Run

1. Install the Arduino-ESP32 version and use the board settings from the [root README](../../../../README.md#development-environment).
2. Copy all supplied libraries and the shared `lv_conf.h` from [libraries](../../libraries/) into your Arduino sketchbook's `libraries` folder. Avoid duplicate LVGL installations.
3. Open `11_camera_web_server.ino`. Set `ssid` and `password` to your 2.4 GHz network.
4. Select the board port, upload, then open Serial Monitor at 115200 baud.

## Expected Result

The serial monitor prints a camera URL. Open it on a device connected to the same network.

## Troubleshooting

No URL: check the Wi-Fi credentials. Camera initialization failure: check the camera model, ribbon orientation and PSRAM setting.
