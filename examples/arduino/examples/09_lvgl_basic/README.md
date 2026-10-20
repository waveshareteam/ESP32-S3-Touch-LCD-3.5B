[English](README.md) | [简体中文](README_ZH.md)

# LVGL basic display

## What It Does

Display a label and two interactive switches using LVGL 9.

## Hardware Required

ESP32-S3-Touch-LCD-3.5B or ESP32-S3-Touch-LCD-3.5B-C. USB-C cable. This replaces the old LVGL 8 sketch with the shared LVGL 9 implementation.

## How to Run

1. Install the Arduino-ESP32 version and use the board settings from the [root README](../../../../README.md#development-environment).
2. Copy all supplied libraries and the shared `lv_conf.h` from [libraries](../../libraries/) into your Arduino sketchbook's `libraries` folder. Avoid duplicate LVGL installations.
3. Open `09_lvgl_basic.ino`.
4. Select the board port, upload, then open Serial Monitor at 115200 baud.

## Expected Result

Display a label and two interactive switches using LVGL 9.

## Troubleshooting

Check the board options, bundled libraries and peripheral connections. Enable OPI PSRAM for display/audio examples.
