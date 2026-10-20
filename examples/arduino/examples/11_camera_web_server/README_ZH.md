[English](README.md) | [简体中文](README_ZH.md)

# 摄像头网页预览

## 功能

通过 2.4 GHz Wi-Fi 提供摄像头网页预览。

## 硬件需求

ESP32-S3-Touch-LCD-3.5B 或 ESP32-S3-Touch-LCD-3.5B-C。OV5640 摄像头和 2.4 GHz Wi-Fi。带外壳版本含摄像头，裸板版本需另配。

## 怎么运行

1. 按照[首页开发环境](../../../../README_ZH.md#development-environment)安装 Arduino-ESP32，并设置开发板参数。
2. 将随附 [libraries](../../libraries/) 中的库及共用 `lv_conf.h` 复制到 Arduino 草图目录的 `libraries` 中，避免同时安装多个 LVGL。
3. 打开 `11_camera_web_server.ino`。填写 2.4 GHz Wi-Fi 的 ssid 和 password。
4. 选择串口并上传，以 115200 波特率打开串口监视器。

## 预期现象

串口输出摄像头网址，在同一网络内的设备上打开即可查看。

## 常见问题

没有网址时检查 Wi-Fi 配置；摄像头初始化失败时检查型号、排线方向及 PSRAM 设置。
