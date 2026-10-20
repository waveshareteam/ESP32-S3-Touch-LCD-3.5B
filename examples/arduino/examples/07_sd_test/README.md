[English](README.md) | [简体中文](README_ZH.md)

# microSD filesystem

## What It Does

Exercise microSD file creation, reading, writing and deletion.

## Hardware Required

ESP32-S3-Touch-LCD-3.5B or ESP32-S3-Touch-LCD-3.5B-C. FAT32 microSD card. Use a spare card: the example writes test files.

## How to Run

1. Install the Arduino-ESP32 version and use the board settings from the [root README](../../../../README.md#development-environment).
2. Copy all supplied libraries and the shared `lv_conf.h` from [libraries](../../libraries/) into your Arduino sketchbook's `libraries` folder. Avoid duplicate LVGL installations.
3. Open `07_sd_test.ino`.
4. Select the board port, upload, then open Serial Monitor at 115200 baud.

## Expected Result

Exercise microSD file creation, reading, writing and deletion.

## Troubleshooting

Check the board options, bundled libraries and peripheral connections. Enable OPI PSRAM for display/audio examples.
