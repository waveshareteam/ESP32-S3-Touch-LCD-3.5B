<div align="center">
  <h1>ESP32-S3-Touch-LCD-3.5B</h1>
  <p><strong>ESP32-S3 3.5 英寸 320 × 480 QSPI 电容触摸 LCD 开发板</strong></p>
  <p><a href="README.md">English</a> | <a href="README_ZH.md">简体中文</a></p>
  <p>
    <a href="https://github.com/waveshareteam/ESP32-S3-Touch-LCD-3.5B/actions/workflows/build.yml"><img alt="Build" src="https://github.com/waveshareteam/ESP32-S3-Touch-LCD-3.5B/actions/workflows/build.yml/badge.svg"></a>
    <a href="https://github.com/waveshareteam/ESP32-S3-Touch-LCD-3.5B/releases"><img alt="Latest Release" src="https://img.shields.io/github/v/release/waveshareteam/ESP32-S3-Touch-LCD-3.5B"></a>
    <a href="LICENSE"><img alt="License" src="https://img.shields.io/github/license/waveshareteam/ESP32-S3-Touch-LCD-3.5B"></a>
  </p>
  <p>
    <a href="https://www.waveshare.net/shop/ESP32-S3-Touch-LCD-3.5B.htm">🌐 产品页</a> &middot;
    <a href="#quick-start">📦 固件</a> &middot;
    <a href="examples/esp-idf/">🧩 ESP-IDF</a> &middot;
    <a href="examples/arduino/examples/">🔧 Arduino</a> &middot;
    <a href="https://docs.waveshare.net/ESP32-S3-Touch-LCD-3.5B/">📚 文档</a>
  </p>
  <p><img src="docs/images/ESP32-S3-Touch-LCD-3.5B-family.jpg" alt="ESP32-S3-Touch-LCD-3.5B and ESP32-S3-Touch-LCD-3.5B-C" width="500"></p>
  <p>左：不带外壳；右：带外壳（含 OV5640 摄像头）。</p>
</div>

---

## ✨ 概述

本仓库适用于 ESP32-S3-Touch-LCD-3.5B 系列的两个版本，示例共用。涉及摄像头等外设时，请根据实际配件选择示例。

| 型号 | 外壳 | 摄像头 |
| --- | --- | --- |
| [ESP32-S3-Touch-LCD-3.5B](https://www.waveshare.net/shop/ESP32-S3-Touch-LCD-3.5B.htm) | 不带外壳 | 提供摄像头接口，默认不含摄像头 |
| [ESP32-S3-Touch-LCD-3.5B-C](https://www.waveshare.net/shop/ESP32-S3-Touch-LCD-3.5B-C.htm) | 带外壳 | 包含 OV5640 摄像头 |

| 项目 | 配置 |
| --- | --- |
| 主控 / 无线 | ESP32-S3；2.4 GHz Wi-Fi 和 Bluetooth LE |
| Flash / PSRAM | 16 MB Flash / 8 MB PSRAM |
| 显示 / 触摸 | 3.5 英寸 320 × 480 LCD；AXS15231B；QSPI 显示、I2C 电容触摸 |
| 电源管理 | AXP2101 |
| 运动 / 时钟 | QMI8658 惯性传感器 / PCF85063 RTC |
| 音频 | ES8311 编解码器、麦克风及扬声器接口 |
| 扩展 | microSD、摄像头接口、TCA9554 IO 扩展、Type-C |

原理图的 PCB V1/V2 是硬件修订版本，与是否带外壳不是同一分类。

<a id="quick-start"></a>

## 📦 快速开始

1. 使用 Type-C 数据线将开发板连接至电脑。
2. 下载[带小智的出厂固件](firmware/ESP32-S3-Touch-LCD-3.5B_XiaoZhi_20261020.bin)，在 Flash Download Tool 中选择 ESP32-S3。
3. 将合并固件烧录到 `0x0`。如需手动进入下载模式，按住 BOOT，按一下 RESET，再松开 BOOT。
4. 以 115200 波特率打开串口，检查示例的预期现象。

出厂固件源码位于 [examples/factory](examples/factory/)，包含应用桌面和小智语音对话；[旧版出厂示例](examples/esp-idf/01_factory/README_ZH.md) 单独保留。可自行编译源码，或从[产品文档](https://docs.waveshare.net/ESP32-S3-Touch-LCD-3.5B/)获取官方固件。

## 🧪 示例

### ESP-IDF

| 示例 | 功能 |
| --- | --- |
| [01_factory](examples/esp-idf/01_factory/README_ZH.md) | 旧版出厂绘画 / 触摸测试及应用 |
| [02_qmi8658_example](examples/esp-idf/02_qmi8658_example/) | QMI8658 惯性传感器 |
| [03_pcf85063_example](examples/esp-idf/03_pcf85063_example/) | PCF85063 实时时钟 |
| [04_axp2101_example](examples/esp-idf/04_axp2101_example/) | AXP2101 电源管理 |
| [05_lvgl_example](examples/esp-idf/05_lvgl_example/) | LVGL 控件 |
| [06_lvgl_image](examples/esp-idf/06_lvgl_image/) | 内置图片浏览 |
| [07_lvgl_image_sd](examples/esp-idf/07_lvgl_image_sd/) | microSD 图片浏览 |

### Arduino

| 示例 | 功能 |
| --- | --- |
| [01_i2s_audio](examples/arduino/examples/01_i2s_audio/) | microSD 音频播放 |
| [02_axp2101_example](examples/arduino/examples/02_axp2101_example/) | AXP2101 电源管理 |
| [03_button_example](examples/arduino/examples/03_button_example/) | BOOT 按键 |
| [04_es8311_example](examples/arduino/examples/04_es8311_example/) | 麦克风录音与回放 |
| [05_pcf85063_example](examples/arduino/examples/05_pcf85063_example/) | PCF85063 实时时钟 |
| [06_qmi8658_example](examples/arduino/examples/06_qmi8658_example/) | QMI8658 惯性传感器 |
| [07_sd_test](examples/arduino/examples/07_sd_test/) | microSD 文件系统 |
| [08_gfx_helloworld](examples/arduino/examples/08_gfx_helloworld/) | GFX 显示 |
| [09_lvgl_basic](examples/arduino/examples/09_lvgl_basic/) | LVGL 基础显示 |
| [10_lvgl_arduino_v9](examples/arduino/examples/10_lvgl_arduino_v9/) | LVGL 触摸控件 |
| [11_camera_web_server](examples/arduino/examples/11_camera_web_server/) | 摄像头网页预览 |

Arduino 示例统一使用随附的 LVGL 9.2.2；ESP-IDF 外设示例和旧版出厂工程使用 LVGL 8.4，新版出厂工程使用 LVGL 9.6.0~1。各 ESP-IDF 工程独立管理依赖。

<a id="development-environment"></a>

## 🛠️ 开发环境

| 框架 | 版本 | 示例数量 |
| --- | --- | --- |
| ESP-IDF | v5.5.4 | 7 个及出厂固件源码 |
| Arduino-ESP32 | 3.3.10 | 11 个 |

版本及 Arduino 开发板参数统一配置在 [product.json](.github/product.json)。Arduino 选择 **ESP32S3 Dev Module**，Flash 16 MB、PSRAM OPI、Flash QIO、启动时启用 USB CDC，并选择 16 MB Flash 下 3 MB APP / 9 MB FATFS 分区。将随附 Arduino 库及共用的 `lv_conf.h` 复制到草图目录的 `libraries` 文件夹。



## 📚 资料

- [不带外壳版本产品页](https://www.waveshare.net/shop/ESP32-S3-Touch-LCD-3.5B.htm)
- [带外壳版本产品页](https://www.waveshare.net/shop/ESP32-S3-Touch-LCD-3.5B-C.htm)
- [中文产品文档](https://docs.waveshare.net/ESP32-S3-Touch-LCD-3.5B/)
- [V1 原理图](hardware/schematic/ESP32-S3-Touch-LCD-3.5B-V1-Schematic.pdf)
- [V2 原理图](hardware/schematic/ESP32-S3-Touch-LCD-3.5B-V2-Schematic.pdf)

## 🤝 支持

- [官方技术支持](https://docs.waveshare.net/ESP32-S3-Touch-LCD-3.5B/Technical-Support/)

## 📄 许可证

第一方代码采用 [Apache-2.0](LICENSE) 许可证，随附第三方组件保留各自的许可证与来源说明。
