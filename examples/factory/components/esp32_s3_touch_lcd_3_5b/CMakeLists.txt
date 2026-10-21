idf_component_register(
    SRCS "esp32_s3_touch_lcd_3_5b.cpp" "bsp_board.cpp"
    INCLUDE_DIRS "include"
    PRIV_INCLUDE_DIRS "priv_include"
    REQUIRES driver esp_lcd esp_lcd_axs15231b esp_lcd_touch
             esp_lvgl_adapter lvgl esp_codec_dev esp32-camera
             fatfs sdmmc esp_wifi esp_netif
    PRIV_REQUIRES XPowersLib sensorlib esp_io_expander_tca9554
                  button nvs_flash esp_timer esp_event)
