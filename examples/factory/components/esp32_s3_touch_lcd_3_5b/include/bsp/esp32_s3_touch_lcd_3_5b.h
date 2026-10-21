#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "esp_err.h"
#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "driver/spi_master.h"
#include "bsp/config.h"
#include "bsp/display.h"
#include "bsp/touch.h"
#include "bsp/board.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Shared I2C: power, IO expander, RTC, IMU, audio, touch and camera control. */
#define BSP_I2C_NUM             I2C_NUM_0
#define BSP_I2C_SDA             GPIO_NUM_8
#define BSP_I2C_SCL             GPIO_NUM_7
#define BSP_I2C_FREQ_HZ         400000

/* AXS15231B QSPI display and PWM backlight. */
#define BSP_LCD_SPI_HOST        SPI2_HOST
#define BSP_LCD_MOSI            GPIO_NUM_1
#define BSP_LCD_SCLK            GPIO_NUM_5
#define BSP_LCD_DATA1           GPIO_NUM_2
#define BSP_LCD_DATA2           GPIO_NUM_3
#define BSP_LCD_DATA3           GPIO_NUM_4
#define BSP_LCD_CS              GPIO_NUM_12
#define BSP_LCD_RST             GPIO_NUM_NC
#define BSP_LCD_BACKLIGHT       GPIO_NUM_6

/* AXS15231B touch uses the shared I2C bus. Reset is shared through EXIO1. */
#define BSP_TOUCH_RST           GPIO_NUM_NC
#define BSP_TOUCH_INT           GPIO_NUM_NC

/* BOOT is active-low; PWR is active-high on TCA9554 EXIO6. */
#define BSP_BUTTON_BOOT         GPIO_NUM_0
#define BSP_BUTTON_PWR_EXIO     (1U << 6)

/* Independent 1-bit SDMMC interface. */
#define BSP_SD_CLK              GPIO_NUM_11
#define BSP_SD_CMD              GPIO_NUM_10
#define BSP_SD_D0               GPIO_NUM_9
#define BSP_SD_MOUNT_POINT      "/sdcard"

/* ES8311 I2S: DOUT/DIN directions are relative to the MCU. */
#define BSP_I2S_MCLK            GPIO_NUM_44
#define BSP_I2S_BCLK            GPIO_NUM_13
#define BSP_I2S_LRCLK           GPIO_NUM_15
#define BSP_I2S_DOUT            GPIO_NUM_16
#define BSP_I2S_DIN             GPIO_NUM_14
#define BSP_AUDIO_PA            GPIO_NUM_NC

/* DVP camera; SCCB control uses the shared I2C bus. */
#define BSP_CAMERA_XCLK         GPIO_NUM_38
#define BSP_CAMERA_PCLK         GPIO_NUM_41
#define BSP_CAMERA_VSYNC        GPIO_NUM_17
#define BSP_CAMERA_HREF         GPIO_NUM_18
#define BSP_CAMERA_D0           GPIO_NUM_45
#define BSP_CAMERA_D1           GPIO_NUM_47
#define BSP_CAMERA_D2           GPIO_NUM_48
#define BSP_CAMERA_D3           GPIO_NUM_46
#define BSP_CAMERA_D4           GPIO_NUM_42
#define BSP_CAMERA_D5           GPIO_NUM_40
#define BSP_CAMERA_D6           GPIO_NUM_39
#define BSP_CAMERA_D7           GPIO_NUM_21

esp_err_t bsp_i2c_init(void);
i2c_master_bus_handle_t bsp_i2c_get_handle(void);

/* ES8311 is one shared full-duplex codec, not an ES7210 echo-reference array.
 * One task acquires, opens, reads/writes, closes and releases a session.
 * Volume can be changed from another task; init does not play audio. */
esp_err_t bsp_audio_init(void);
bool bsp_audio_acquire(uint32_t timeout_ms);
void bsp_audio_release(void);
esp_err_t bsp_audio_open(uint32_t sample_rate, uint8_t channels);
esp_err_t bsp_audio_close(void);
esp_err_t bsp_audio_write(const void *data, size_t size);
/* Push silence through the complete TX ring before pausing/closing playback. */
esp_err_t bsp_audio_drain(void);
esp_err_t bsp_audio_read(void *data, size_t size);
esp_err_t bsp_audio_volume_set(int percent);
int bsp_audio_volume_get(void);

/* One shared ES8311. Ordinary APPs use the task-owned API above; Xiaozhi
 * holds one session across its control/input/playback tasks. Stop/join all
 * voice tasks before close/release. RX slots contain one physical microphone. */
typedef enum { BSP_AUDIO_OWNER_NONE, BSP_AUDIO_OWNER_APP, BSP_AUDIO_OWNER_XIAOZHI } bsp_audio_owner_t;
bsp_audio_owner_t bsp_audio_session_get_owner(void);
const char *bsp_audio_owner_name(bsp_audio_owner_t owner);
esp_err_t bsp_audio_session_acquire(bsp_audio_owner_t owner);
esp_err_t bsp_audio_session_release(bsp_audio_owner_t owner);
#define BSP_AUDIO_VOICE_CHANNELS 2
esp_err_t bsp_audio_voice_open(uint32_t sample_rate, bool capture);
esp_err_t bsp_audio_voice_read(void *pcm, size_t bytes, size_t *read, uint32_t timeout_ms);
esp_err_t bsp_audio_voice_write(void *pcm, size_t bytes, size_t *written, uint32_t timeout_ms);
esp_err_t bsp_audio_voice_mute(bool mute);
esp_err_t bsp_audio_voice_close(void);

#ifdef __cplusplus
}
#endif
