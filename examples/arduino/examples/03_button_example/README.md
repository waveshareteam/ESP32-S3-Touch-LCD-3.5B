[English](README.md) | [简体中文](README_ZH.md)

# BOOT button

## What It Does

Report button click, double-click and long-press events.

## Hardware Required

ESP32-S3-Touch-LCD-3.5B or ESP32-S3-Touch-LCD-3.5B-C. USB-C cable.

## How to Run

1. Install the Arduino-ESP32 version and use the board settings from the [root README](../../../../README.md#development-environment).
2. Copy all supplied libraries and the shared `lv_conf.h` from [libraries](../../libraries/) into your Arduino sketchbook's `libraries` folder. Avoid duplicate LVGL installations.
3. Open `03_button_example.ino`.
4. Select the board port, upload, then open Serial Monitor at 115200 baud.

## Expected Result

Report button click, double-click and long-press events.

## Troubleshooting

Check the board options, bundled libraries and peripheral connections. Enable OPI PSRAM for display/audio examples.
