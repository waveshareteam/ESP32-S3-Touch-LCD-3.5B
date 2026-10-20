[English](README.md) | [简体中文](README_ZH.md)

# Xiaozhi Application

## What It Does

Integrate voice chat, activation, wake-word detection and camera MCP tools with the product desktop. Local adaptations of the Espressif Xiaozhi and MCP components are kept alongside this application; reused Xiaozhi resources retain their license in [third_party/xiaozhi_esp32](third_party/xiaozhi_esp32/).

## Hardware Required

The board's ES8311 microphone and speaker, Wi-Fi and an activated Xiaozhi account. The camera tool requires an OV5640.

## How to Run

Build and flash the complete [factory project](../../README.md), including its model partition. Configure Wi-Fi in Settings, open Xiaozhi and complete activation before using Talk or the wake word.

## Expected Result

The application displays recognition and reply text and plays spoken replies. The camera tool captures a fresh image, shows its preview and sends it to the configured vision service. Music, Mic Test and Xiaozhi share the BSP audio session gate.

## Troubleshooting

Check network and activation status when chat cannot connect. Missing camera hardware or vision-service configuration returns an error. Audio capture pauses during spoken replies; Stop remains available on screen.
