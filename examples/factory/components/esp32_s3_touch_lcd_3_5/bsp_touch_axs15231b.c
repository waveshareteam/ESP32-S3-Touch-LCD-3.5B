#include <stdlib.h>
#include "bsp_axs15231b.h"

typedef struct {
    esp_lcd_touch_t base;
    i2c_master_dev_handle_t device;
} bsp_axs_touch_t;

static esp_err_t read_touch(esp_lcd_touch_handle_t touch)
{
    bsp_axs_touch_t *state = (bsp_axs_touch_t *)touch;
    const uint8_t command[11] = {0xb5, 0xab, 0xa5, 0x5a, 0, 0, 0, 0x0e, 0, 0, 0};
    uint8_t data[14] = {0};
    esp_err_t err = i2c_master_transmit_receive(state->device, command, sizeof(command),
                                              data, sizeof(data), 100);
    portENTER_CRITICAL(&touch->data.lock);
    touch->data.points = 0;
    if (err == ESP_OK && data[0] != 0xff && data[1] > 0 && data[1] <= 2) {
        const uint16_t x = ((data[2] & 0x0f) << 8) | data[3];
        const uint16_t y = ((data[4] & 0x0f) << 8) | data[5];
        const uint8_t event = data[2] >> 6;
        if (event != 1 && x < touch->config.x_max && y < touch->config.y_max) {
            touch->data.coords[0].x = x;
            touch->data.coords[0].y = y;
            touch->data.coords[0].strength = 1;
            touch->data.points = 1;
        }
    }
    portEXIT_CRITICAL(&touch->data.lock);
    return err;
}

static bool get_touch(esp_lcd_touch_handle_t touch, uint16_t *x, uint16_t *y,
                      uint16_t *strength, uint8_t *count, uint8_t maximum)
{
    portENTER_CRITICAL(&touch->data.lock);
    *count = maximum && touch->data.points ? 1 : 0;
    if (*count) {
        x[0] = touch->data.coords[0].x;
        y[0] = touch->data.coords[0].y;
        if (strength) strength[0] = 1;
    }
    touch->data.points = 0;
    portEXIT_CRITICAL(&touch->data.lock);
    return *count != 0;
}

static esp_err_t delete_touch(esp_lcd_touch_handle_t touch)
{
    bsp_axs_touch_t *state = (bsp_axs_touch_t *)touch;
    esp_err_t err = i2c_master_bus_rm_device(state->device);
    free(state);
    return err;
}

esp_err_t bsp_axs15231b_touch_new(i2c_master_bus_handle_t bus,
                                esp_lcd_touch_handle_t *touch)
{
    if (!bus || !touch) return ESP_ERR_INVALID_ARG;
    *touch = NULL;
    bsp_axs_touch_t *state = calloc(1, sizeof(*state));
    if (!state) return ESP_ERR_NO_MEM;
    i2c_device_config_t config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = 0x3b,
        .scl_speed_hz = 400000,
    };
    esp_err_t err = i2c_master_bus_add_device(bus, &config, &state->device);
    if (err != ESP_OK) { free(state); return err; }
    state->base.config.x_max = 320;
    state->base.config.y_max = 480;
    state->base.config.rst_gpio_num = GPIO_NUM_NC;
    state->base.config.int_gpio_num = GPIO_NUM_NC;
    state->base.data.lock = (portMUX_TYPE)portMUX_INITIALIZER_UNLOCKED;
    state->base.read_data = read_touch;
    state->base.get_xy = get_touch;
    state->base.del = delete_touch;
    *touch = &state->base;
    return ESP_OK;
}
