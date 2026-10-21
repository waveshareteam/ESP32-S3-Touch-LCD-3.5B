# Factory application suite

[English](README.md) | [简体中文](README_ZH.md)

## What It Does

Provides a portrait 320 x 480 desktop with 11 applications: Settings, AIChats, Touch Test, Photos, Weather, Music, Sensor, 2048, Camera, Mic Test and Button Test. Sensor displays hardware RTC time and QMI8658 accelerometer/gyroscope data.

## Hardware Required

Use ESP32-S3-Touch-LCD-3.5B or the enclosed ESP32-S3-Touch-LCD-3.5B-C and a USB-C data cable. Camera, audio and SD-card functions require a compatible camera, speaker and FAT32 microSD card respectively; the enclosed version includes an OV5640 camera.

## How to Run

To restore the supplied factory software, follow the [firmware instructions](../../firmware/README.md). To build this project, configure the ESP-IDF version listed in [Development Environment](../../README.md#development-environment), then run `idf.py set-target esp32s3`, `idf.py build` and `idf.py -p PORT flash monitor` in this directory, replacing `PORT` with the device's serial port.

## Expected Result

The screen lights up when the desktop is ready, with Settings first and AIChats second. Tap an icon to open an application; Settings offers brightness, volume, Wi-Fi scanning and setup, and SD-card information.

Short press PWR once to power on; press and hold PWR for 5 seconds to power off. Use short presses when checking PWR in Button Test.

For application controls, Wi-Fi setup and SD-card folders, see the [user guide](USER_GUIDE.md).

## Troubleshooting

Weather and AIChats require Internet access; follow the on-screen instructions to activate AIChats. If a speech model is missing, flash the complete factory firmware or use a full `idf.py flash` rather than flashing only the application; see the [user guide](USER_GUIDE.md#common-questions) for other questions.
