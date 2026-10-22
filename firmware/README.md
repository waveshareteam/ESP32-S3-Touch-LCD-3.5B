[English](README.md) | [简体中文](README_ZH.md)

# Firmware

| File | Description | Build Environment |
| --- | --- | --- |
| [ESP32-S3-Touch-LCD-3.5B_FactoryOnly_20261021.bin](ESP32-S3-Touch-LCD-3.5B_FactoryOnly_20261021.bin) | Factory desktop with 11 applications, including AIChats and Button Test | ESP-IDF v5.5.4 |

This directory contains only the latest factory image. It applies to ESP32-S3-Touch-LCD-3.5B and the enclosed ESP32-S3-Touch-LCD-3.5B-C. Camera functions require a compatible camera; the enclosed version includes an OV5640.

The merged image includes the bootloader, partition table, application and speech model. Flash the complete image at `0x0`.

In Flash Download Tool, select **ESP32-S3**, add this BIN at `0x0`, select the device's serial port and start flashing. If manual download mode is required, hold BOOT, press RESET, then release BOOT; press RESET after flashing to start the software.

Alternatively, install esptool and run the following command from this directory, replacing `PORT` with the device's serial port:

```sh
python -m esptool --chip esp32s3 --port PORT --baud 460800 write-flash 0x0 ESP32-S3-Touch-LCD-3.5B_FactoryOnly_20261021.bin
```

The desktop opens after startup. Short press PWR once to power on; press and hold PWR for 5 seconds to power off. See the [user guide](../examples/factory/USER_GUIDE.md) for Wi-Fi setup and application controls, or the [factory source](../examples/factory/README.md) to build the software.

SHA256: `f67e98c068921227345302b41cf1efa1fed60f620e637e192e16bba1002308f2`
