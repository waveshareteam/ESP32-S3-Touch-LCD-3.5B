# 固件

[English](README.md) | [简体中文](README_ZH.md)

- **适用型号：** ESP32-S3-Touch-LCD-3.5B（不带外壳版）、ESP32-S3-Touch-LCD-3.5B-C（带外壳版），两款共用本出厂固件。
- **摄像头支持：** 已适配 **OV5640 摄像头**。带外壳版随附该模组，不带外壳版需另配并连接模组后使用摄像头功能。
- **网页烧录：** 在[固件中心](https://hub.waveshare.net/)统一选择 **ESP32-S3-Touch-LCD-3.5B**。

| 文件 | 说明 | 编译环境 |
| --- | --- | --- |
| [ESP32-S3-Touch-LCD-3.5B_FactoryOnly_20261021.bin](ESP32-S3-Touch-LCD-3.5B_FactoryOnly_20261021.bin) | 提供 11 个应用的出厂桌面，包含 AIChats 和 Button Test | ESP-IDF v5.5.4 |

合并镜像包含引导程序、分区表、应用和语音模型，完整镜像烧录地址为 `0x0`。

使用 Flash Download Tool，选择 **ESP32-S3**，添加上述 BIN 并将烧录地址设为 `0x0`，选择设备对应的串口后开始烧录。若需手动进入下载模式，按住 BOOT，按一下 RESET，再松开 BOOT；烧录完成后按 RESET 启动。

也可安装 esptool 后，在本目录运行以下命令，将 `PORT` 替换为设备对应的串口：

```sh
python -m esptool --chip esp32s3 --port PORT --baud 460800 write-flash 0x0 ESP32-S3-Touch-LCD-3.5B_FactoryOnly_20261021.bin
```

启动后进入应用桌面。短按一次 PWR 按键开机，长按 5 秒关机。配网与应用操作请参照[使用说明](../examples/factory/USER_GUIDE_ZH.md)，自行编译请参照[出厂源码](../examples/factory/README_ZH.md)。

SHA256：`f67e98c068921227345302b41cf1efa1fed60f620e637e192e16bba1002308f2`
