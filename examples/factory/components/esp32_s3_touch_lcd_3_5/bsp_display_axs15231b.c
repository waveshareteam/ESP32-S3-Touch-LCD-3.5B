#include "bsp_axs15231b.h"
#include "esp_lcd_axs15231b.h"
#include "esp_check.h"
#include "driver/gpio.h"

/* Exact panel command sequence from 3.5B/01_factory. */
#include "axs15231b_init.inc"

esp_err_t bsp_axs15231b_panel_new(esp_lcd_panel_io_handle_t io,
                                esp_lcd_panel_handle_t *panel)
{
    axs15231b_vendor_config_t vendor = {
        .init_cmds = lcd_init_cmds,
        .init_cmds_size = sizeof(lcd_init_cmds) / sizeof(lcd_init_cmds[0]),
        .flags.use_qspi_interface = 1,
    };
    esp_lcd_panel_dev_config_t config = {
        .reset_gpio_num = GPIO_NUM_NC,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
        .bits_per_pixel = 16,
        .vendor_config = &vendor,
    };
    ESP_RETURN_ON_ERROR(esp_lcd_new_panel_axs15231b(io, &config, panel), "bsp_3_5b", "AXS15231B create");
    ESP_RETURN_ON_ERROR(esp_lcd_panel_reset(*panel), "bsp_3_5b", "LCD reset");
    ESP_RETURN_ON_ERROR(esp_lcd_panel_init(*panel), "bsp_3_5b", "LCD init");
    /* Driver 1.0.0 interprets this argument as 'off', as in the old BSP. */
    return esp_lcd_panel_disp_on_off(*panel, false);
}
