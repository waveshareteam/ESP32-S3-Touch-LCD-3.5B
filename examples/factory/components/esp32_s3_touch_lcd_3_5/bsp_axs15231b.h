#pragma once

#include "driver/i2c_master.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_touch.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t bsp_axs15231b_panel_new(esp_lcd_panel_io_handle_t io,
                                esp_lcd_panel_handle_t *panel);
esp_err_t bsp_axs15231b_touch_new(i2c_master_bus_handle_t bus,
                                esp_lcd_touch_handle_t *touch);

#ifdef __cplusplus
}
#endif
