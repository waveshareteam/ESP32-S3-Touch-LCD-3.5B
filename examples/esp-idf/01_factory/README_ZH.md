[English](README.md) | [简体中文](README_ZH.md)

# 旧版出厂应用

## 功能

初始化板载外设，先进入绘画 / 触摸测试，点击屏幕 EXIT 后打开应用界面。

## 硬件需求

ESP32-S3-Touch-LCD-3.5B 或 ESP32-S3-Touch-LCD-3.5B-C。Type-C 数据线、扬声器及 FAT32 microSD 卡。摄像头功能需要 OV5640，带外壳版本已包含。

## 怎么运行

1. 启用 [product.json](../../../.github/product.json) 中指定的 ESP-IDF。
2. 在本目录打开终端并执行：

```sh
idf.py set-target esp32s3
idf.py build
idf.py -p PORT flash monitor
```

将 `PORT` 替换为实际串口；编译前请将 `main/main.cpp` 中的预设 Wi-Fi 账号改为自己的网络。

## 预期现象

触摸可绘画，点击 EXIT 可进入应用界面；摄像头、音频及存储功能取决于是否连接对应外设。

## 常见问题

SD 初始化失败时，插入 FAT32 卡后复位；摄像头初始化失败时检查排线。
