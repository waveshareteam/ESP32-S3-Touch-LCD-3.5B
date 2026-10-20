[English](README.md) | [简体中文](README_ZH.md)

# microSD 图片浏览

## 功能

浏览 microSD 卡 images 目录中的图片。

## 硬件需求

ESP32-S3-Touch-LCD-3.5B 或 ESP32-S3-Touch-LCD-3.5B-C。Type-C 数据线及 FAT32 microSD 卡。

## 怎么运行

1. 启用[仓库配置](../../../.github/product.json)中指定的 ESP-IDF 环境。
2. 在本示例目录打开终端。将示例附带的 images 文件夹复制到 microSD 卡根目录，上电前插入卡。
3. 执行以下命令，将 `PORT` 替换为实际串口。

```sh
idf.py set-target esp32s3
idf.py build
idf.py -p PORT flash monitor
```

串口波特率为 115200，按 Ctrl+] 退出监视器。

## 预期现象

屏幕显示卡内图片，点击图片可切换。

## 常见问题

不显示图片时，检查 FAT32 格式、images 目录及图片解码支持，建议先使用随附图片。
