[English](README.md) | [简体中文](README_ZH.md)

# LVGL 控件

## 功能

运行 LVGL 8 控件演示，并支持电容触摸操作。

## 硬件需求

ESP32-S3-Touch-LCD-3.5B 或 ESP32-S3-Touch-LCD-3.5B-C。Type-C 数据线。

## 怎么运行

1. 启用[仓库配置](../../../.github/product.json)中指定的 ESP-IDF 环境。
2. 在本示例目录打开终端。
3. 执行以下命令，将 `PORT` 替换为实际串口。

```sh
idf.py set-target esp32s3
idf.py build
idf.py -p PORT flash monitor
```

串口波特率为 115200，按 Ctrl+] 退出监视器。

## 预期现象

显示控件演示界面，触摸可操作控件。

## 常见问题

初始化失败时，检查 USB 供电和目标芯片。切换 ESP-IDF 版本后先执行 `idf.py fullclean` 再编译。
