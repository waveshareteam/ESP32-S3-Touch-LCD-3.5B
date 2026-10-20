[English](README.md) | [简体中文](README_ZH.md)

# AXP2101 power management

## What It Does

Configure the PMIC and report its status over serial.

## Hardware Required

ESP32-S3-Touch-LCD-3.5B or ESP32-S3-Touch-LCD-3.5B-C. USB-C cable; battery is optional for battery readings.

## How to Run

1. Install the Arduino-ESP32 version and use the board settings from the [root README](../../../../README.md#development-environment).
2. Copy all supplied libraries and the shared `lv_conf.h` from [libraries](../../libraries/) into your Arduino sketchbook's `libraries` folder. Avoid duplicate LVGL installations.
3. Open `02_axp2101_example.ino`.
4. Select the board port, upload, then open Serial Monitor at 115200 baud.

## Expected Result

Configure the PMIC and report its status over serial.

## Troubleshooting

Check the board options, bundled libraries and peripheral connections. Enable OPI PSRAM for display/audio examples.
