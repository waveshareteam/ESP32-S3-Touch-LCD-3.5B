# 出厂应用工程

[English](README.md) | [简体中文](README_ZH.md)

## 功能

提供 320 x 480 竖屏桌面，包含 Settings、AIChats、Touch Test、Photos、Weather、Music、Sensor、2048、Camera、Mic Test 和 Button Test，共 11 个应用。Sensor 显示硬件 RTC 时间及 QMI8658 加速度、陀螺仪数据。

## 硬件需求

使用 ESP32-S3-Touch-LCD-3.5B 或带外壳的 ESP32-S3-Touch-LCD-3.5B-C，准备 USB-C 数据线。摄像头、音频和 SD 功能分别需要兼容摄像头、扬声器和 FAT32 microSD 卡；带外壳版包含 OV5640 摄像头。

## 怎么运行

恢复仓库随附的出厂软件请参照[固件烧录说明](../../firmware/README_ZH.md)。从源码编译时，请按[开发环境](../../README_ZH.md#development-environment)配置指定版本的 ESP-IDF，在本目录依次执行 `idf.py set-target esp32s3`、`idf.py build`、`idf.py -p PORT flash monitor`，将 `PORT` 替换为设备的实际串口。

## 预期现象

启动完成后，屏幕亮起并显示桌面，Settings 位于第一、AIChats 位于第二。点击图标进入应用，设置中可调背光和音量、扫描或配置 Wi-Fi、查看 SD 信息。

短按 PWR 按键一次开机，长按 5 秒关机。在 Button Test 中测试 PWR 时请短按。

各应用操作、配网步骤和 SD 卡文件夹要求请参照[使用说明](USER_GUIDE_ZH.md)。

## 常见问题

天气和 AIChats 需要能够访问互联网，AIChats 需按屏幕提示完成激活。若提示缺少语音模型，请烧录完整出厂固件或执行完整的 `idf.py flash`，不要只烧录应用；其他问题请参照[使用说明](USER_GUIDE_ZH.md#常见问题)。
