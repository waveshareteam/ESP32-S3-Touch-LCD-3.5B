[English](README.md) | [简体中文](README_ZH.md)

# Legacy Factory Applications

## What It Does

Initialize the board peripherals, run the drawing/touch test, then open the application launcher using the on-screen EXIT control.

## Hardware Required

ESP32-S3-Touch-LCD-3.5B or ESP32-S3-Touch-LCD-3.5B-C. USB-C data cable, speaker and FAT32 microSD card. Camera functions require an OV5640; it is included with the enclosure version.

## How to Run

1. Activate ESP-IDF from [product.json](../../../.github/product.json).
2. Open a terminal here and run:

```sh
idf.py set-target esp32s3
idf.py build
idf.py -p PORT flash monitor
```

Replace `PORT` with your serial port. Change the preset Wi-Fi credentials in `main/main.cpp` to your own network before building.

## Expected Result

Drawing responds to touch. EXIT opens the applications. Available functions depend on the connected camera, speaker and microSD card.

## Troubleshooting

If SD initialization fails, insert a FAT32 card before resetting. Check the camera ribbon if camera initialization fails.
