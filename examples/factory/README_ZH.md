[English](README.md) | [简体中文](README_ZH.md)

# 带小智的出厂应用

## 功能

运行 320 x 480 桌面，包含设置、小智语音对话、触摸测试、相册、天气、音乐、传感器、2048、摄像头及麦克风测试。传感器应用显示 RTC 和 QMI8658 数据。

## 硬件需求

ESP32-S3-Touch-LCD-3.5B 或 ESP32-S3-Touch-LCD-3.5B-C、Type-C 数据线及扬声器。摄像头功能需要 OV5640；相册和音乐需要 FAT32 microSD 卡。小智需要 Wi-Fi 网络并完成服务激活。

## 怎么运行

启用 [product.json](../../.github/product.json) 指定的 ESP-IDF，然后在本目录执行：

```sh
idf.py set-target esp32s3
idf.py build
idf.py -p PORT flash monitor
```

将 `PORT` 替换为实际串口。从旧版固件升级时使用完整 `flash`，同时烧录新分区表、唤醒模型及应用。进入设置配置 Wi-Fi，再打开小智，按照界面提示完成激活。

## 预期现象

上电进入应用桌面。打开小智并完成激活后，说“你好小智”或点击 Talk 开始对话；摄像头工具可拍照并进行图像分析。其他应用可从桌面打开。

## 常见问题

无法唤醒时，请完整烧录工程，确保模型分区已写入。保持 `CONFIG_CODEC_I2C_BACKWARD_COMPATIBLE` 关闭，以共用 BSP 的 I2C 总线。连接失败时检查 Wi-Fi 和激活状态。[旧版出厂示例](../esp-idf/01_factory/README_ZH.md) 单独保留。
