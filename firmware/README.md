# Firmware

[English](README.md) | [简体中文](README_ZH.md)

- **Compatible models:** ESP32-S3-Touch-LCD-3.5B (without enclosure) and ESP32-S3-Touch-LCD-3.5B-C (with enclosure) share this factory firmware.
- **Camera support:** Supports the **OV5640 camera**. The enclosed model includes the module; the model without an enclosure requires a separately connected camera module for camera functions.
- **Web flashing:** Select **ESP32-S3-Touch-LCD-3.5B** in the [firmware center](https://hub.waveshare.net/).

| File | Description | Build Environment |
| --- | --- | --- |
| [ESP32-S3-Touch-LCD-3.5B_FactoryOnly_20261021.bin](ESP32-S3-Touch-LCD-3.5B_FactoryOnly_20261021.bin) | Factory desktop with 11 applications, including AIChats and Button Test | ESP-IDF v5.5.4 |

The merged image includes the bootloader, partition table, application and speech model. Flash the complete image at `0x0`.

In Flash Download Tool, select **ESP32-S3**, add this BIN at `0x0`, select the device's serial port and start flashing. If manual download mode is required, hold BOOT, press RESET, then release BOOT; press RESET after flashing to start the software.

Alternatively, install esptool and run the following command from this directory, replacing `PORT` with the device's serial port:

```sh
python -m esptool --chip esp32s3 --port PORT --baud 460800 write-flash 0x0 ESP32-S3-Touch-LCD-3.5B_FactoryOnly_20261021.bin
```

The desktop opens after startup. Short press PWR once to power on; press and hold PWR for 5 seconds to power off. See the [user guide](../examples/factory/USER_GUIDE.md) for Wi-Fi setup and application controls, or the [factory source](../examples/factory/README.md) to build the software.

SHA256: `f67e98c068921227345302b41cf1efa1fed60f620e637e192e16bba1002308f2`
