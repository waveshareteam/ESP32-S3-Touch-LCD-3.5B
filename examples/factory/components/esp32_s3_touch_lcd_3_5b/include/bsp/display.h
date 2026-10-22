#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"
#include "esp_lcd_types.h"
#include "lvgl.h"
#include "bsp/config.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int max_transfer_sz; /*!< Maximum SPI transfer size, in bytes; zero selects the default. */
} bsp_display_config_t;

/* Create the QSPI panel. The PWM backlight stays off until bsp_display_show(). */
esp_err_t bsp_display_new(const bsp_display_config_t *config, esp_lcd_panel_handle_t *ret_panel,
                          esp_lcd_panel_io_handle_t *ret_io);

lv_display_t *bsp_display_start(void);
lv_display_t *bsp_display_get(void);
lv_indev_t *bsp_display_get_input_dev(void);
/* A zero timeout waits indefinitely. */
bool bsp_display_lock(uint32_t timeout_ms);
void bsp_display_unlock(void);

/* PWM starts at zero; requested brightness is saved until bsp_display_show(). */
esp_err_t bsp_display_brightness_init(void);
esp_err_t bsp_display_brightness_set(int percent);
int bsp_display_brightness_get(void);
esp_err_t bsp_display_backlight_off(void);
esp_err_t bsp_display_backlight_on(void);
/* Flush the first UI frame and wait for QSPI DMA before enabling backlight. */
esp_err_t bsp_display_show(void);

#ifdef __cplusplus
}
#endif
