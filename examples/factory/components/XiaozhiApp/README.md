[English](README.md) | [简体中文](README_ZH.md)

# Xiaozhi Application

## What It Does

Use AIChats on the desktop for voice conversations, wake-word activation, recognized text and reply subtitles. With a compatible camera connected, capture photos for vision analysis.

## Hardware Required

Use ESP32-S3-Touch-LCD-3.5B or ESP32-S3-Touch-LCD-3.5B-C, connect a speaker and prepare a 2.4 GHz Wi-Fi network with Internet access. Camera functions require an OV5640; follow the on-screen activation instructions on first use.

## How to Run

Build and flash the complete [factory project](../../README.md), including its model partition. Configure Wi-Fi in Settings, open Xiaozhi and complete activation before using Talk or the wake word.

## Expected Result

The application displays recognition and reply text and plays spoken replies. Ask about the camera view to capture a photo for vision analysis; exit the current conversation before switching to Music or Mic Test.

## Troubleshooting

Check network and activation status when chat cannot connect. Missing camera hardware or vision-service configuration returns an error. Audio capture pauses during spoken replies; Stop remains available on screen.
