[English](README.md) | [简体中文](README_ZH.md)

# microSD image gallery

## What It Does

Browse images from the images directory on a microSD card.

## Hardware Required

ESP32-S3-Touch-LCD-3.5B or ESP32-S3-Touch-LCD-3.5B-C. USB-C data cable and a FAT32 microSD card.

## How to Run

1. Activate the ESP-IDF version listed in the [repository configuration](../../../.github/product.json).
2. Open a terminal in this example directory. Copy the supplied `images/` folder to the microSD card root and insert the card before powering on.
3. Run the commands below; replace `PORT` with your serial port.

```sh
idf.py set-target esp32s3
idf.py build
idf.py -p PORT flash monitor
```

Serial monitor: 115200 baud. Exit with Ctrl+].

## Expected Result

The screen displays a card image; tap it to advance.

## Troubleshooting

If no image appears, check FAT32 formatting, the images directory and image decoder support. Start with the supplied images.
