[English](README.md) | [简体中文](README_ZH.md)

# 麦克风录音与回放

## 功能

录制两秒音频到内存，然后回放一次。

## 硬件需求

ESP32-S3-Touch-LCD-3.5B 或 ESP32-S3-Touch-LCD-3.5B-C。板载麦克风及已连接的扬声器。

## 怎么运行

1. 按照[首页开发环境](../../../../README_ZH.md#development-environment)安装 Arduino-ESP32，并设置开发板参数。
2. 将随附 [libraries](../../libraries/) 中的库及共用 `lv_conf.h` 复制到 Arduino 草图目录的 `libraries` 中，避免同时安装多个 LVGL。
3. 打开 `04_es8311_example.ino`。
4. 选择串口并上传，以 115200 波特率打开串口监视器。

## 预期现象

录制两秒音频到内存，然后回放一次。

## 常见问题

检查开发板参数、随附库及外设连接。显示和音频示例请启用 OPI PSRAM。
