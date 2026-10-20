[English](README.md) | [简体中文](README_ZH.md)

# microSD audio playback

## What It Does

Play 1.mp3 from the microSD card through the ES8311 codec.

## Hardware Required

ESP32-S3-Touch-LCD-3.5B or ESP32-S3-Touch-LCD-3.5B-C. Speaker and FAT32 microSD card; put 1.mp3 in the card root.

## How to Run

1. Install the Arduino-ESP32 version and use the board settings from the [root README](../../../../README.md#development-environment).
2. Copy all supplied libraries and the shared `lv_conf.h` from [libraries](../../libraries/) into your Arduino sketchbook's `libraries` folder. Avoid duplicate LVGL installations.
3. Open `01_i2s_audio.ino`.
4. Select the board port, upload, then open Serial Monitor at 115200 baud.

## Expected Result

Play 1.mp3 from the microSD card through the ES8311 codec.

## Troubleshooting

Check the board options, bundled libraries and peripheral connections. Enable OPI PSRAM for display/audio examples.
