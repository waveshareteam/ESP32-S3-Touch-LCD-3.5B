#pragma once

#include "esp_lcd_touch.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    void *dummy; /*!< Reserved for future use. */
} bsp_touch_config_t;

/* Create AXS15231B coordinate input on the shared I2C bus. */
esp_err_t bsp_touch_new(const bsp_touch_config_t *config, esp_lcd_touch_handle_t *ret_touch);
esp_lcd_touch_handle_t bsp_touch_get_handle(void);

#ifdef __cplusplus
}
#endif
