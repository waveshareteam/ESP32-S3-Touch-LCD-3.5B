#include "bsp/esp-bsp.h"
#include "bsp_board_internal.h"

#include <algorithm>
#include <atomic>
#include <climits>
#include <cstdlib>
#include <cstring>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/spi_master.h"
#include "driver/ledc.h"
#include "driver/i2s_std.h"
#include "driver/sdmmc_host.h"
#include "esp_check.h"
#include "esp_log.h"
#include "esp_heap_caps.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "esp_lcd_axs15231b.h"
#include "esp_codec_dev_defaults.h"
#include "esp_codec_dev.h"
#include "esp_lv_adapter.h"

static const char *TAG = "bsp_3_5b";
static SemaphoreHandle_t lcd_transfer_done;
static uint16_t *lcd_dma_buffer;
static i2c_master_bus_handle_t i2c_bus;
static esp_lcd_panel_io_handle_t lcd_io;
static esp_lcd_panel_handle_t panel;
static esp_lcd_touch_handle_t touch;
static lv_display_t *display;
static lv_indev_t *input;
static SemaphoreHandle_t sd_mutex;
static sdmmc_card_t *sdcard;
static unsigned sd_leases;
static bool backlight_ready, display_started;
// Keep PWM off until the completed first UI frame has reached the LCD.
static bool backlight_released;
static int brightness;

/**************************************************************************************************
 * I2C interface
 **************************************************************************************************/
esp_err_t bsp_i2c_init(void)
{
    if (i2c_bus) return ESP_OK;
    ESP_RETURN_ON_ERROR(bsp_board_init_locks(), TAG, "Board locks");
    if (!sd_mutex) sd_mutex = xSemaphoreCreateMutex();
    if (!sd_mutex) return ESP_ERR_NO_MEM;
    i2c_master_bus_config_t cfg = {};
    cfg.i2c_port = BSP_I2C_NUM;
    cfg.sda_io_num = BSP_I2C_SDA;
    cfg.scl_io_num = BSP_I2C_SCL;
    cfg.clk_source = I2C_CLK_SRC_DEFAULT;
    cfg.glitch_ignore_cnt = 7;
    cfg.flags.enable_internal_pullup = true;
    return i2c_new_master_bus(&cfg, &i2c_bus);
}

i2c_master_bus_handle_t bsp_i2c_get_handle(void) { return i2c_bus; }

/**************************************************************************************************
 * SD card interface
 **************************************************************************************************/
static esp_err_t sdcard_mount_locked(void)
{
    if (sdcard) return ESP_OK;
    esp_vfs_fat_sdmmc_mount_config_t cfg = {};
    cfg.format_if_mount_failed = false;
    cfg.max_files = 8;
    cfg.allocation_unit_size = 16 * 1024;
    sdmmc_host_t host = SDMMC_HOST_DEFAULT();
    sdmmc_slot_config_t slot = SDMMC_SLOT_CONFIG_DEFAULT();
    slot.width = 1;
    slot.clk = BSP_SD_CLK;
    slot.cmd = BSP_SD_CMD;
    slot.d0 = BSP_SD_D0;
    slot.flags |= SDMMC_SLOT_FLAG_INTERNAL_PULLUP;
    esp_err_t ret = esp_vfs_fat_sdmmc_mount(BSP_SD_MOUNT_POINT, &host, &slot, &cfg, &sdcard);
    if (ret != ESP_OK) sdcard = nullptr;
    bsp_board_status.sdcard = ret;
    return ret;
}

esp_err_t bsp_sdcard_mount(void)
{
    if (!sd_mutex) return ESP_ERR_INVALID_STATE;
    xSemaphoreTake(sd_mutex, portMAX_DELAY);
    esp_err_t ret = sdcard_mount_locked();
    xSemaphoreGive(sd_mutex);
    return ret;
}

esp_err_t bsp_sdcard_acquire(void)
{
    if (!sd_mutex) return ESP_ERR_INVALID_STATE;
    xSemaphoreTake(sd_mutex, portMAX_DELAY);
    esp_err_t ret = sdcard_mount_locked();
    if (ret == ESP_OK) ++sd_leases;
    xSemaphoreGive(sd_mutex);
    return ret;
}

void bsp_sdcard_release(void)
{
    if (!sd_mutex) return;
    xSemaphoreTake(sd_mutex, portMAX_DELAY);
    if (sd_leases) --sd_leases;
    xSemaphoreGive(sd_mutex);
}

esp_err_t bsp_sdcard_unmount(void)
{
    if (!sd_mutex) return ESP_ERR_INVALID_STATE;
    xSemaphoreTake(sd_mutex, portMAX_DELAY);
    esp_err_t ret = sd_leases ? ESP_ERR_INVALID_STATE : ESP_OK;
    if (ret == ESP_OK && sdcard) {
        ret = esp_vfs_fat_sdcard_unmount(BSP_SD_MOUNT_POINT, sdcard);
        if (ret == ESP_OK) { sdcard = nullptr; bsp_board_status.sdcard = ESP_ERR_INVALID_STATE; }
    }
    xSemaphoreGive(sd_mutex);
    return ret;
}

esp_err_t bsp_sdcard_get_info(bsp_sdcard_info_t *info)
{
    if (!info) return ESP_ERR_INVALID_ARG;
    *info = {};
    if (!sd_mutex) return ESP_ERR_INVALID_STATE;
    xSemaphoreTake(sd_mutex, portMAX_DELAY);
    esp_err_t ret = ESP_ERR_INVALID_STATE;
    if (sdcard) {
        info->mounted = true;
        info->capacity_bytes = (uint64_t)sdcard->csd.capacity * sdcard->csd.sector_size;
        ret = esp_vfs_fat_info(BSP_SD_MOUNT_POINT, &info->total_bytes, &info->free_bytes);
    }
    xSemaphoreGive(sd_mutex);
    return ret;
}

/**************************************************************************************************
 * I2S audio interface: one shared ES8311 for playback and microphone capture.
 **************************************************************************************************/
#if CONFIG_CODEC_I2C_BACKWARD_COMPATIBLE
#error "This BSP uses i2c_master: disable CONFIG_CODEC_I2C_BACKWARD_COMPATIBLE."
#endif

static i2s_chan_handle_t tx, rx;
static const audio_codec_data_if_t *data_if;
static const audio_codec_ctrl_if_t *ctrl_if;
static const audio_codec_gpio_if_t *gpio_if;
static const audio_codec_if_t *codec_if;
static esp_codec_dev_handle_t codec;
static SemaphoreHandle_t session_mutex, control_mutex;
static bool opened;
static bool voice_capture;
static std::atomic<bsp_audio_owner_t> audio_owner{BSP_AUDIO_OWNER_NONE};
static uint8_t opened_channels;
static constexpr unsigned DMA_DESCRIPTORS = 6, DMA_FRAMES = 240;
static int volume = 70;

static esp_err_t codec_result(int ret) { return ret == ESP_CODEC_DEV_OK ? ESP_OK : ESP_FAIL; }

static void bsp_audio_cleanup(void)
{
    if (codec) { esp_codec_dev_delete(codec); codec = nullptr; }
    if (codec_if) { audio_codec_delete_codec_if(codec_if); codec_if = nullptr; }
    if (ctrl_if) { audio_codec_delete_ctrl_if(ctrl_if); ctrl_if = nullptr; }
    if (gpio_if) { audio_codec_delete_gpio_if(gpio_if); gpio_if = nullptr; }
    if (data_if) { audio_codec_delete_data_if(data_if); data_if = nullptr; }
    if (tx) { i2s_channel_disable(tx); i2s_del_channel(tx); tx = nullptr; }
    if (rx) { i2s_channel_disable(rx); i2s_del_channel(rx); rx = nullptr; }
    opened = false;
}

static esp_err_t bsp_audio_init_channels(void)
{
    i2s_chan_config_t channel = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
    channel.dma_desc_num = DMA_DESCRIPTORS;
    channel.dma_frame_num = DMA_FRAMES;
    channel.auto_clear_after_cb = true;
    ESP_RETURN_ON_ERROR(i2s_new_channel(&channel, &tx, &rx), TAG, "I2S channels");
    i2s_std_config_t cfg = {};
    cfg.clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(16000);
    cfg.slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO);
    cfg.gpio_cfg.mclk = BSP_I2S_MCLK;
    cfg.gpio_cfg.bclk = BSP_I2S_BCLK;
    cfg.gpio_cfg.ws = BSP_I2S_LRCLK;
    cfg.gpio_cfg.dout = BSP_I2S_DOUT;
    cfg.gpio_cfg.din = BSP_I2S_DIN;
    ESP_RETURN_ON_ERROR(i2s_channel_init_std_mode(tx, &cfg), TAG, "I2S TX");
    ESP_RETURN_ON_ERROR(i2s_channel_init_std_mode(rx, &cfg), TAG, "I2S RX");
    ESP_RETURN_ON_ERROR(i2s_channel_enable(tx), TAG, "I2S TX enable");
    ESP_RETURN_ON_ERROR(i2s_channel_enable(rx), TAG, "I2S RX enable");
    audio_codec_i2s_cfg_t data = {};
    data.port = I2S_NUM_0;
    data.tx_handle = tx;
    data.rx_handle = rx;
    data_if = audio_codec_new_i2s_data(&data);
    if (!data_if) return ESP_ERR_NO_MEM;
    audio_codec_i2c_cfg_t ctrl = {};
    ctrl.port = BSP_I2C_NUM;
    ctrl.addr = ES8311_CODEC_DEFAULT_ADDR;
    ctrl.bus_handle = bsp_i2c_get_handle();
    ctrl_if = audio_codec_new_i2c_ctrl(&ctrl);
    gpio_if = audio_codec_new_gpio();
    if (!ctrl_if || !gpio_if) return ESP_ERR_NO_MEM;
    es8311_codec_cfg_t chip = {};
    chip.codec_mode = ESP_CODEC_DEV_WORK_MODE_BOTH;
    chip.ctrl_if = ctrl_if;
    chip.gpio_if = gpio_if;
    chip.pa_pin = BSP_AUDIO_PA;
    chip.use_mclk = true;
    chip.hw_gain.pa_voltage = 5.0f;
    chip.hw_gain.codec_dac_voltage = 3.3f;
    codec_if = es8311_codec_new(&chip);
    if (!codec_if) return ESP_FAIL;
    esp_codec_dev_cfg_t dev = {};
    dev.dev_type = ESP_CODEC_DEV_TYPE_IN_OUT;
    dev.codec_if = codec_if;
    dev.data_if = data_if;
    codec = esp_codec_dev_new(&dev);
    if (!codec) return ESP_ERR_NO_MEM;
    // Verify the original 48 kHz / mono format without starting a factory recording.
    esp_codec_dev_sample_info_t fs = {};
    fs.sample_rate = 48000;
    fs.channel = 1;
    fs.bits_per_sample = 16;
    ESP_RETURN_ON_ERROR(codec_result(esp_codec_dev_open(codec, &fs)), TAG, "Codec open");
    ESP_RETURN_ON_ERROR(codec_result(esp_codec_dev_set_out_mute(codec, true)), TAG, "Codec mute");
    ESP_RETURN_ON_ERROR(codec_result(esp_codec_dev_set_out_vol(codec, volume)), TAG, "Codec volume");
    ESP_RETURN_ON_ERROR(codec_result(esp_codec_dev_set_in_gain(codec, 40.0f)), TAG, "Mic gain");
    return codec_result(esp_codec_dev_close(codec));
}

esp_err_t bsp_audio_init(void)
{
    if (codec) return ESP_OK;
    ESP_RETURN_ON_ERROR(bsp_i2c_init(), TAG, "I2C bus");
    // esp_codec_dev uses the 8-bit address; i2c_master_probe takes 7 bits.
    ESP_RETURN_ON_ERROR(i2c_master_probe(bsp_i2c_get_handle(), ES8311_CODEC_DEFAULT_ADDR >> 1, 100), TAG, "ES8311 probe");
    if (!session_mutex) session_mutex = xSemaphoreCreateMutex();
    if (!control_mutex) control_mutex = xSemaphoreCreateMutex();
    if (!session_mutex || !control_mutex) return ESP_ERR_NO_MEM;
    esp_err_t ret = bsp_audio_init_channels();
    if (ret != ESP_OK) bsp_audio_cleanup();
    return ret;
}

bool bsp_audio_acquire(uint32_t timeout_ms)
{
    if (!codec || !session_mutex) return false;
    const TickType_t started = xTaskGetTickCount();
    do {
        bsp_audio_owner_t expected = BSP_AUDIO_OWNER_NONE;
        if (audio_owner.compare_exchange_strong(expected, BSP_AUDIO_OWNER_APP)) {
            if (xSemaphoreTake(session_mutex, 0) == pdTRUE) return true;
            audio_owner.store(BSP_AUDIO_OWNER_NONE);
        }
        if (timeout_ms && xTaskGetTickCount() - started >= pdMS_TO_TICKS(timeout_ms)) return false;
        vTaskDelay(pdMS_TO_TICKS(10));
    } while (true);
}

static bool owns_session(void)
{
    return session_mutex && xSemaphoreGetMutexHolder(session_mutex) == xTaskGetCurrentTaskHandle();
}

void bsp_audio_release(void)
{
    if (!owns_session()) return;
    bsp_audio_close();
    xSemaphoreGive(session_mutex);
    audio_owner.store(BSP_AUDIO_OWNER_NONE);
}

static esp_err_t open_codec(uint32_t sample_rate, uint8_t channels)
{
    if (!codec) return ESP_ERR_INVALID_STATE;
    if (sample_rate < 8000 || sample_rate > 96000 || (channels != 1 && channels != 2)) return ESP_ERR_INVALID_ARG;
    xSemaphoreTake(control_mutex, portMAX_DELAY);
    if (opened) {
        esp_err_t ret = codec_result(esp_codec_dev_close(codec));
        if (ret != ESP_OK) { xSemaphoreGive(control_mutex); return ret; }
        opened = false;
    }
    esp_codec_dev_sample_info_t fs = {};
    fs.sample_rate = sample_rate;
    fs.channel = channels;
    fs.bits_per_sample = 16;
    esp_err_t ret = codec_result(esp_codec_dev_open(codec, &fs));
    if (ret == ESP_OK) ret = codec_result(esp_codec_dev_set_out_vol(codec, volume));
    if (ret == ESP_OK) ret = codec_result(esp_codec_dev_set_in_gain(codec, 40.0f));
    if (ret == ESP_OK) ret = codec_result(esp_codec_dev_set_out_mute(codec, false));
    if (ret != ESP_OK) esp_codec_dev_close(codec);
    opened = ret == ESP_OK;
    opened_channels = opened ? channels : 0;
    xSemaphoreGive(control_mutex);
    return ret;
}

esp_err_t bsp_audio_open(uint32_t sample_rate, uint8_t channels)
{
    return owns_session() ? open_codec(sample_rate, channels) : ESP_ERR_INVALID_STATE;
}

static esp_err_t close_codec(void)
{
    if (!codec) return ESP_ERR_INVALID_STATE;
    xSemaphoreTake(control_mutex, portMAX_DELAY);
    esp_err_t ret = ESP_OK;
    if (opened) {
        esp_codec_dev_set_out_mute(codec, true);
        ret = codec_result(esp_codec_dev_close(codec));
        if (ret == ESP_OK) opened = false;
    }
    xSemaphoreGive(control_mutex);
    return ret;
}

esp_err_t bsp_audio_close(void)
{
    return owns_session() ? close_codec() : ESP_ERR_INVALID_STATE;
}

esp_err_t bsp_audio_write(const void *data, size_t size)
{
    if (!owns_session() || !opened) return ESP_ERR_INVALID_STATE;
    if (!data || !size || size > INT_MAX) return ESP_ERR_INVALID_ARG;
    return codec_result(esp_codec_dev_write(codec, const_cast<void *>(data), (int)size));
}

esp_err_t bsp_audio_read(void *data, size_t size)
{
    if (!owns_session() || !opened) return ESP_ERR_INVALID_STATE;
    if (!data || !size || size > INT_MAX) return ESP_ERR_INVALID_ARG;
    return codec_result(esp_codec_dev_read(codec, data, (int)size));
}

esp_err_t bsp_audio_drain(void)
{
    if (!owns_session() || !opened) return ESP_ERR_INVALID_STATE;
    const int16_t silence[256] = {};
    // One complete ring plus one write ensures previously queued PCM has left TX.
    size_t remaining = DMA_DESCRIPTORS * DMA_FRAMES * opened_channels * sizeof(int16_t) + sizeof(silence);
    while (remaining) {
        size_t count = remaining < sizeof(silence) ? remaining : sizeof(silence);
        ESP_RETURN_ON_ERROR(bsp_audio_write(silence, count), TAG, "Drain audio");
        remaining -= count;
    }
    return ESP_OK;
}

esp_err_t bsp_audio_volume_set(int percent)
{
    if (percent < 0 || percent > 100) return ESP_ERR_INVALID_ARG;
    if (!control_mutex || !codec) return ESP_ERR_INVALID_STATE;
    xSemaphoreTake(control_mutex, portMAX_DELAY);
    esp_err_t ret = opened ? codec_result(esp_codec_dev_set_out_vol(codec, percent)) : ESP_OK;
    if (ret == ESP_OK) volume = percent;
    xSemaphoreGive(control_mutex);
    return ret;
}

int bsp_audio_volume_get(void)
{
    if (!control_mutex) return volume;
    xSemaphoreTake(control_mutex, portMAX_DELAY);
    int result = volume;
    xSemaphoreGive(control_mutex);
    return result;
}

bsp_audio_owner_t bsp_audio_session_get_owner(void) { return audio_owner.load(); }
const char *bsp_audio_owner_name(bsp_audio_owner_t owner)
{
    return owner == BSP_AUDIO_OWNER_XIAOZHI ? "Xiaozhi" : owner == BSP_AUDIO_OWNER_APP ? "Music/Mic Test" : "None";
}
esp_err_t bsp_audio_session_acquire(bsp_audio_owner_t owner)
{
    if (!codec) return ESP_ERR_INVALID_STATE;
    if (owner != BSP_AUDIO_OWNER_XIAOZHI) return ESP_ERR_INVALID_ARG;
    bsp_audio_owner_t expected = BSP_AUDIO_OWNER_NONE;
    return audio_owner.compare_exchange_strong(expected, owner) ? ESP_OK : ESP_ERR_INVALID_STATE;
}
esp_err_t bsp_audio_session_release(bsp_audio_owner_t owner)
{
    if (owner != BSP_AUDIO_OWNER_XIAOZHI) return ESP_ERR_INVALID_ARG;
    if (opened) return ESP_ERR_INVALID_STATE;
    return audio_owner.compare_exchange_strong(owner, BSP_AUDIO_OWNER_NONE) ? ESP_OK : ESP_ERR_INVALID_STATE;
}
esp_err_t bsp_audio_voice_open(uint32_t sample_rate, bool capture)
{
    if (audio_owner.load() != BSP_AUDIO_OWNER_XIAOZHI || opened) return ESP_ERR_INVALID_STATE;
    esp_err_t ret = open_codec(sample_rate, 2);
    voice_capture = ret == ESP_OK && capture;
    if (ret == ESP_OK) ESP_LOGI(TAG, "Xiaozhi ES8311: %lu Hz, single microphone, no hardware echo reference", (unsigned long)sample_rate);
    return ret;
}
esp_err_t bsp_audio_voice_read(void *pcm, size_t bytes, size_t *read, uint32_t timeout_ms)
{
    if (read) *read = 0;
    if (!pcm || !read || !bytes || bytes % 4) return ESP_ERR_INVALID_ARG;
    if (audio_owner.load() != BSP_AUDIO_OWNER_XIAOZHI || !opened || !voice_capture) return ESP_ERR_INVALID_STATE;
    return i2s_channel_read(rx, pcm, bytes, read, timeout_ms);
}
esp_err_t bsp_audio_voice_write(void *pcm, size_t bytes, size_t *written, uint32_t timeout_ms)
{
    if (written) *written = 0;
    if (!pcm || !written || !bytes || bytes % 4) return ESP_ERR_INVALID_ARG;
    if (audio_owner.load() != BSP_AUDIO_OWNER_XIAOZHI || !opened) return ESP_ERR_INVALID_STATE;
    return i2s_channel_write(tx, pcm, bytes, written, timeout_ms);
}
esp_err_t bsp_audio_voice_mute(bool mute)
{
    if (audio_owner.load() != BSP_AUDIO_OWNER_XIAOZHI || !opened) return ESP_ERR_INVALID_STATE;
    xSemaphoreTake(control_mutex, portMAX_DELAY);
    esp_err_t ret = codec_result(esp_codec_dev_set_out_mute(codec, mute));
    xSemaphoreGive(control_mutex);
    return ret;
}
esp_err_t bsp_audio_voice_close(void)
{
    if (audio_owner.load() != BSP_AUDIO_OWNER_XIAOZHI) return ESP_ERR_INVALID_STATE;
    esp_err_t ret = close_codec();
    if (ret == ESP_OK) voice_capture = false;
    return ret;
}

/**************************************************************************************************
 * Display interface
 **************************************************************************************************/
esp_err_t bsp_display_brightness_init(void)
{
    if (backlight_ready) return ESP_OK;
    ESP_RETURN_ON_ERROR(gpio_set_level(BSP_LCD_BACKLIGHT, 0), TAG, "Backlight off level");
    ESP_RETURN_ON_ERROR(gpio_set_direction(BSP_LCD_BACKLIGHT, GPIO_MODE_OUTPUT), TAG, "Backlight off GPIO");
    ledc_timer_config_t timer = {};
    timer.speed_mode = LEDC_LOW_SPEED_MODE;
    timer.duty_resolution = LEDC_TIMER_10_BIT;
    timer.timer_num = LEDC_TIMER_1;
    timer.freq_hz = 5000;
    timer.clk_cfg = LEDC_AUTO_CLK;
    ESP_RETURN_ON_ERROR(ledc_timer_config(&timer), TAG, "Backlight timer");
    ledc_channel_config_t channel = {};
    channel.gpio_num = BSP_LCD_BACKLIGHT;
    channel.speed_mode = LEDC_LOW_SPEED_MODE;
    channel.channel = LEDC_CHANNEL_1; // Camera XCLK owns channel 0, timer 0.
    channel.timer_sel = LEDC_TIMER_1;
    channel.duty = 0;
    ESP_RETURN_ON_ERROR(ledc_channel_config(&channel), TAG, "Backlight PWM");
    backlight_ready = true;
    return ESP_OK;
}

esp_err_t bsp_display_brightness_set(int percent)
{
    if (percent < 0 || percent > 100) return ESP_ERR_INVALID_ARG;
    if (!backlight_ready) return ESP_ERR_INVALID_STATE;
    const int duty = backlight_released ? percent * 1023 / 100 : 0;
    ESP_RETURN_ON_ERROR(ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, duty), TAG, "Backlight duty");
    ESP_RETURN_ON_ERROR(ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1), TAG, "Backlight update");
    brightness = percent;
    return ESP_OK;
}
int bsp_display_brightness_get(void) { return brightness; }

esp_err_t bsp_display_backlight_off(void) { return bsp_display_brightness_set(0); }
esp_err_t bsp_display_backlight_on(void) { return bsp_display_brightness_set(CONFIG_BSP_LCD_BRIGHTNESS); }

esp_err_t bsp_display_show(void)
{
    if (!display_started || !display || !lcd_io) return ESP_ERR_INVALID_STATE;
    if (!bsp_display_lock(0)) return ESP_ERR_TIMEOUT;
    lv_obj_invalidate(lv_display_get_screen_active(display));
    lv_refr_now(display);
    // Drain queued color transfers before releasing the backlight.
    esp_err_t ret = esp_lcd_panel_io_tx_param(lcd_io, -1, nullptr, 0);
    if (ret == ESP_OK) {
        backlight_released = true;
        ret = bsp_display_brightness_set(brightness);
        if (ret != ESP_OK) backlight_released = false;
    }
    bsp_display_unlock();
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Initial UI frame flushed; backlight enabled at %d%%", brightness);
    }
    return ret;
}

static const axs15231b_lcd_init_cmd_t lcd_init_cmds[] = {
    {0xBB, (const uint8_t[]){0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x5A, 0xA5}, 8, 0},
    {0xA0, (const uint8_t[]){0xC0, 0x10, 0x00, 0x02, 0x00, 0x00, 0x04, 0x3F, 0x20, 0x05, 0x3F, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00}, 17, 0},
    {0xA2, (const uint8_t[]){0x30, 0x3C, 0x24, 0x14, 0xD0, 0x20, 0xFF, 0xE0, 0x40, 0x19, 0x80, 0x80, 0x80, 0x20, 0xf9, 0x10, 0x02, 0xff, 0xff, 0xF0, 0x90, 0x01, 0x32, 0xA0, 0x91, 0xE0, 0x20, 0x7F, 0xFF, 0x00, 0x5A}, 31, 0},
    {0xD0, (const uint8_t[]){0xE0, 0x40, 0x51, 0x24, 0x08, 0x05, 0x10, 0x01, 0x20, 0x15, 0x42, 0xC2, 0x22, 0x22, 0xAA, 0x03, 0x10, 0x12, 0x60, 0x14, 0x1E, 0x51, 0x15, 0x00, 0x8A, 0x20, 0x00, 0x03, 0x3A, 0x12}, 30, 0},
    {0xA3, (const uint8_t[]){0xA0, 0x06, 0xAa, 0x00, 0x08, 0x02, 0x0A, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x00, 0x55, 0x55}, 22, 0},
    {0xC1, (const uint8_t[]){0x31, 0x04, 0x02, 0x02, 0x71, 0x05, 0x24, 0x55, 0x02, 0x00, 0x41, 0x00, 0x53, 0xFF, 0xFF, 0xFF, 0x4F, 0x52, 0x00, 0x4F, 0x52, 0x00, 0x45, 0x3B, 0x0B, 0x02, 0x0d, 0x00, 0xFF, 0x40}, 30, 0},
    {0xC3, (const uint8_t[]){0x00, 0x00, 0x00, 0x50, 0x03, 0x00, 0x00, 0x00, 0x01, 0x80, 0x01}, 11, 0},
    {0xC4, (const uint8_t[]){0x00, 0x24, 0x33, 0x80, 0x00, 0xea, 0x64, 0x32, 0xC8, 0x64, 0xC8, 0x32, 0x90, 0x90, 0x11, 0x06, 0xDC, 0xFA, 0x00, 0x00, 0x80, 0xFE, 0x10, 0x10, 0x00, 0x0A, 0x0A, 0x44, 0x50}, 29, 0},
    {0xC5, (const uint8_t[]){0x18, 0x00, 0x00, 0x03, 0xFE, 0x3A, 0x4A, 0x20, 0x30, 0x10, 0x88, 0xDE, 0x0D, 0x08, 0x0F, 0x0F, 0x01, 0x3A, 0x4A, 0x20, 0x10, 0x10, 0x00}, 23, 0},
    {0xC6, (const uint8_t[]){0x05, 0x0A, 0x05, 0x0A, 0x00, 0xE0, 0x2E, 0x0B, 0x12, 0x22, 0x12, 0x22, 0x01, 0x03, 0x00, 0x3F, 0x6A, 0x18, 0xC8, 0x22}, 20, 0},
    {0xC7, (const uint8_t[]){0x50, 0x32, 0x28, 0x00, 0xa2, 0x80, 0x8f, 0x00, 0x80, 0xff, 0x07, 0x11, 0x9c, 0x67, 0xff, 0x24, 0x0c, 0x0d, 0x0e, 0x0f}, 20, 0},
    {0xC9, (const uint8_t[]){0x33, 0x44, 0x44, 0x01}, 4, 0},
    {0xCF, (const uint8_t[]){0x2C, 0x1E, 0x88, 0x58, 0x13, 0x18, 0x56, 0x18, 0x1E, 0x68, 0x88, 0x00, 0x65, 0x09, 0x22, 0xC4, 0x0C, 0x77, 0x22, 0x44, 0xAA, 0x55, 0x08, 0x08, 0x12, 0xA0, 0x08}, 27, 0},
    {0xD5, (const uint8_t[]){0x40, 0x8E, 0x8D, 0x01, 0x35, 0x04, 0x92, 0x74, 0x04, 0x92, 0x74, 0x04, 0x08, 0x6A, 0x04, 0x46, 0x03, 0x03, 0x03, 0x03, 0x82, 0x01, 0x03, 0x00, 0xE0, 0x51, 0xA1, 0x00, 0x00, 0x00}, 30, 0},
    {0xD6, (const uint8_t[]){0x10, 0x32, 0x54, 0x76, 0x98, 0xBA, 0xDC, 0xFE, 0x93, 0x00, 0x01, 0x83, 0x07, 0x07, 0x00, 0x07, 0x07, 0x00, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x00, 0x84, 0x00, 0x20, 0x01, 0x00}, 30, 0},
    {0xD7, (const uint8_t[]){0x03, 0x01, 0x0b, 0x09, 0x0f, 0x0d, 0x1E, 0x1F, 0x18, 0x1d, 0x1f, 0x19, 0x40, 0x8E, 0x04, 0x00, 0x20, 0xA0, 0x1F}, 19, 0},
    {0xD8, (const uint8_t[]){0x02, 0x00, 0x0a, 0x08, 0x0e, 0x0c, 0x1E, 0x1F, 0x18, 0x1d, 0x1f, 0x19}, 12, 0},
    {0xD9, (const uint8_t[]){0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F}, 12, 0},
    {0xDD, (const uint8_t[]){0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F}, 12, 0},
    {0xDF, (const uint8_t[]){0x44, 0x73, 0x4B, 0x69, 0x00, 0x0A, 0x02, 0x90}, 8, 0},
    {0xE0, (const uint8_t[]){0x3B, 0x28, 0x10, 0x16, 0x0c, 0x06, 0x11, 0x28, 0x5c, 0x21, 0x0D, 0x35, 0x13, 0x2C, 0x33, 0x28, 0x0D}, 17, 0},
    {0xE1, (const uint8_t[]){0x37, 0x28, 0x10, 0x16, 0x0b, 0x06, 0x11, 0x28, 0x5C, 0x21, 0x0D, 0x35, 0x14, 0x2C, 0x33, 0x28, 0x0F}, 17, 0},
    {0xE2, (const uint8_t[]){0x3B, 0x07, 0x12, 0x18, 0x0E, 0x0D, 0x17, 0x35, 0x44, 0x32, 0x0C, 0x14, 0x14, 0x36, 0x3A, 0x2F, 0x0D}, 17, 0},
    {0xE3, (const uint8_t[]){0x37, 0x07, 0x12, 0x18, 0x0E, 0x0D, 0x17, 0x35, 0x44, 0x32, 0x0C, 0x14, 0x14, 0x36, 0x32, 0x2F, 0x0F}, 17, 0},
    {0xE4, (const uint8_t[]){0x3B, 0x07, 0x12, 0x18, 0x0E, 0x0D, 0x17, 0x39, 0x44, 0x2E, 0x0C, 0x14, 0x14, 0x36, 0x3A, 0x2F, 0x0D}, 17, 0},
    {0xE5, (const uint8_t[]){0x37, 0x07, 0x12, 0x18, 0x0E, 0x0D, 0x17, 0x39, 0x44, 0x2E, 0x0C, 0x14, 0x14, 0x36, 0x3A, 0x2F, 0x0F}, 17, 0},
    {0xA4, (const uint8_t[]){0x85, 0x85, 0x95, 0x82, 0xAF, 0xAA, 0xAA, 0x80, 0x10, 0x30, 0x40, 0x40, 0x20, 0xFF, 0x60, 0x30}, 16, 0},
    {0xA4, (const uint8_t[]){0x85, 0x85, 0x95, 0x85}, 4, 0},
    {0xBB, (const uint8_t[]){0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 8, 0},
    {0x13, (const uint8_t[]){0x00}, 0, 0},
    {0x11, (const uint8_t[]){0x00}, 0, 120},
    {0x2C, (const uint8_t[]){0x00, 0x00, 0x00, 0x00}, 4, 0},
};

esp_err_t bsp_display_new(const bsp_display_config_t *config, esp_lcd_panel_handle_t *ret_panel,
                          esp_lcd_panel_io_handle_t *ret_io)
{
    if (!ret_panel || !ret_io) return ESP_ERR_INVALID_ARG;
    if (panel || lcd_io) return ESP_ERR_INVALID_STATE;
    *ret_panel = nullptr;
    *ret_io = nullptr;
    esp_lcd_panel_handle_t new_panel = nullptr;
    esp_lcd_panel_io_handle_t new_io = nullptr;
    bool bus_created = false;
    auto create = [&]() -> esp_err_t {
        ESP_RETURN_ON_ERROR(bsp_display_brightness_init(), TAG, "Backlight");
        ESP_RETURN_ON_ERROR(bsp_i2c_init(), TAG, "I2C");
        ESP_RETURN_ON_ERROR(bsp_io_expander_init(), TAG, "Expander");
        ESP_RETURN_ON_ERROR(bsp_power_init(), TAG, "Power");
        spi_bus_config_t bus = {};
        bus.sclk_io_num = BSP_LCD_SCLK;
        bus.data0_io_num = BSP_LCD_MOSI;
        bus.data1_io_num = BSP_LCD_DATA1;
        bus.data2_io_num = BSP_LCD_DATA2;
        bus.data3_io_num = BSP_LCD_DATA3;
        bus.max_transfer_sz = config && config->max_transfer_sz > 0 ? config->max_transfer_sz :
                              BSP_LCD_H_RES * CONFIG_BSP_LCD_BUFFER_HEIGHT * 2;
        ESP_RETURN_ON_ERROR(spi_bus_initialize(BSP_LCD_SPI_HOST, &bus, SPI_DMA_CH_AUTO), TAG, "LCD SPI");
        bus_created = true;
        esp_lcd_panel_io_spi_config_t io = AXS15231B_PANEL_IO_QSPI_CONFIG(BSP_LCD_CS, nullptr, nullptr);
        io.pclk_hz = CONFIG_BSP_LCD_SPI_CLOCK_HZ;
        ESP_RETURN_ON_ERROR(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)BSP_LCD_SPI_HOST, &io, &new_io), TAG, "LCD IO");
        axs15231b_vendor_config_t vendor = {};
        vendor.init_cmds = lcd_init_cmds;
        vendor.init_cmds_size = sizeof(lcd_init_cmds) / sizeof(lcd_init_cmds[0]);
        vendor.flags.use_qspi_interface = 1;
        esp_lcd_panel_dev_config_t cfg = {};
        cfg.reset_gpio_num = BSP_LCD_RST;
        cfg.rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB;
        cfg.bits_per_pixel = 16;
        cfg.vendor_config = &vendor;
        ESP_RETURN_ON_ERROR(esp_lcd_new_panel_axs15231b(new_io, &cfg, &new_panel), TAG, "AXS15231B create");
        ESP_RETURN_ON_ERROR(esp_lcd_panel_reset(new_panel), TAG, "LCD reset");
        ESP_RETURN_ON_ERROR(esp_lcd_panel_init(new_panel), TAG, "LCD init");
        // Driver 1.0.0 interprets this argument as 'off'.
        return esp_lcd_panel_disp_on_off(new_panel, false);
    };
    esp_err_t ret = create();
    if (ret != ESP_OK) {
        if (new_panel) esp_lcd_panel_del(new_panel);
        if (new_io) esp_lcd_panel_io_del(new_io);
        if (bus_created) spi_bus_free(BSP_LCD_SPI_HOST);
        return ret;
    }
    *ret_panel = new_panel;
    *ret_io = new_io;
    return ESP_OK;
}

/**************************************************************************************************
 * Touch interface
 **************************************************************************************************/
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

esp_err_t bsp_touch_new(const bsp_touch_config_t *config, esp_lcd_touch_handle_t *ret_touch)
{
    (void)config;
    if (!ret_touch) return ESP_ERR_INVALID_ARG;
    if (touch) return ESP_ERR_INVALID_STATE;
    *ret_touch = nullptr;
    ESP_RETURN_ON_ERROR(bsp_i2c_init(), TAG, "I2C");
    ESP_RETURN_ON_ERROR(bsp_io_expander_init(), TAG, "Expander");
    bsp_axs_touch_t *state = static_cast<bsp_axs_touch_t *>(calloc(1, sizeof(bsp_axs_touch_t)));
    if (!state) return ESP_ERR_NO_MEM;
    i2c_device_config_t cfg = {};
    cfg.dev_addr_length = I2C_ADDR_BIT_LEN_7;
    cfg.device_address = 0x3b;
    cfg.scl_speed_hz = BSP_I2C_FREQ_HZ;
    esp_err_t err = i2c_master_bus_add_device(i2c_bus, &cfg, &state->device);
    if (err != ESP_OK) { free(state); return err; }
    state->base.config.x_max = BSP_LCD_H_RES;
    state->base.config.y_max = BSP_LCD_V_RES;
    state->base.config.rst_gpio_num = BSP_TOUCH_RST;
    state->base.config.int_gpio_num = BSP_TOUCH_INT;
    state->base.data.lock = (portMUX_TYPE)portMUX_INITIALIZER_UNLOCKED;
    state->base.read_data = read_touch;
    state->base.get_xy = get_touch;
    state->base.del = delete_touch;
    *ret_touch = &state->base;
    return ESP_OK;
}

/**************************************************************************************************
 * LVGL display and input initialization
 **************************************************************************************************/
static bool IRAM_ATTR lcd_transfer_complete(esp_lcd_panel_io_handle_t,
                                           esp_lcd_panel_io_event_data_t *, void *)
{
    BaseType_t task_woken = pdFALSE;
    xSemaphoreGiveFromISR(lcd_transfer_done, &task_woken);
    return task_woken == pdTRUE;
}

static void lcd_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *pixels)
{
    // QSPI has no independent row-address command: each frame must start at row 0.
    // Keep the full LVGL frame in PSRAM and send consecutive internal DMA strips.
    configASSERT(area->x1 == 0 && area->y1 == 0 &&
                 area->x2 == BSP_LCD_H_RES - 1 && area->y2 == BSP_LCD_V_RES - 1);
    for (int y = 0; y < BSP_LCD_V_RES; y += CONFIG_BSP_LCD_BUFFER_HEIGHT) {
        const int rows = std::min(CONFIG_BSP_LCD_BUFFER_HEIGHT, BSP_LCD_V_RES - y);
        const size_t count = BSP_LCD_H_RES * rows;
        memcpy(lcd_dma_buffer, pixels + y * BSP_LCD_H_RES * 2, count * 2);
        lv_draw_sw_rgb565_swap(lcd_dma_buffer, count);
        ESP_ERROR_CHECK(esp_lcd_panel_draw_bitmap(panel, 0, y, BSP_LCD_H_RES, y + rows, lcd_dma_buffer));
        if (xSemaphoreTake(lcd_transfer_done, pdMS_TO_TICKS(1000)) != pdTRUE) {
            ESP_LOGE(TAG, "LCD DMA transfer timed out at row %d", y);
            // Never reuse a buffer while a failed/timed-out DMA could still own it.
            ESP_ERROR_CHECK(ESP_ERR_TIMEOUT);
        }
    }
    lv_display_flush_ready(disp);
}

static lv_display_t *bsp_display_lcd_init(void)
{
    lcd_transfer_done = xSemaphoreCreateBinary();
    lcd_dma_buffer = static_cast<uint16_t *>(heap_caps_malloc(
        BSP_LCD_H_RES * CONFIG_BSP_LCD_BUFFER_HEIGHT * 2, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL));
    if (!lcd_transfer_done || !lcd_dma_buffer) {
        bsp_board_status.display = ESP_ERR_NO_MEM;
        return nullptr;
    }
    bsp_display_config_t panel_cfg = {};
    panel_cfg.max_transfer_sz = BSP_LCD_H_RES * CONFIG_BSP_LCD_BUFFER_HEIGHT * 2;
    bsp_board_status.display = bsp_display_new(&panel_cfg, &panel, &lcd_io);
    if (bsp_board_status.display != ESP_OK) return nullptr;
    esp_lv_adapter_display_config_t cfg = {};
    cfg.panel = panel;
    cfg.panel_io = lcd_io;
    cfg.profile.interface = ESP_LV_ADAPTER_PANEL_IF_OTHER;
    cfg.profile.rotation = ESP_LV_ADAPTER_ROTATE_0;
    cfg.profile.hor_res = BSP_LCD_H_RES;
    cfg.profile.ver_res = BSP_LCD_V_RES;
    cfg.profile.buffer_height = BSP_LCD_V_RES;
    cfg.profile.use_psram = true;
    cfg.profile.require_double_buffer = false;
    cfg.tear_avoid_mode = ESP_LV_ADAPTER_TEAR_AVOID_MODE_NONE;
    cfg.te_sync.gpio_num = -1;
    lv_display_t *disp = esp_lv_adapter_register_display(&cfg);
    if (!disp) {
        bsp_board_status.display = ESP_ERR_NO_MEM;
        return nullptr;
    }
    lv_display_set_render_mode(disp, LV_DISPLAY_RENDER_MODE_FULL);
    lv_display_set_flush_cb(disp, lcd_flush);
    esp_lcd_panel_io_callbacks_t callbacks = {};
    callbacks.on_color_trans_done = lcd_transfer_complete;
    bsp_board_status.display = esp_lcd_panel_io_register_event_callbacks(lcd_io, &callbacks, nullptr);
    return bsp_board_status.display == ESP_OK ? disp : nullptr;
}

static lv_indev_t *bsp_display_indev_init(lv_display_t *disp)
{
    bsp_board_status.touch = bsp_touch_new(nullptr, &touch);
    if (bsp_board_status.touch != ESP_OK) return nullptr;
    esp_lv_adapter_touch_config_t cfg = ESP_LV_ADAPTER_TOUCH_DEFAULT_CONFIG(disp, touch);
    lv_indev_t *indev = esp_lv_adapter_register_touch(&cfg);
    if (!indev) bsp_board_status.touch = ESP_ERR_NO_MEM;
    return indev;
}

lv_display_t *bsp_display_start(void)
{
    if (display_started) return display;
    bsp_board_status.display = bsp_display_brightness_init();
    if (bsp_board_status.display != ESP_OK) return nullptr;
    // Board startup is single-threaded; APPs use display_lock after this returns.
    bsp_board_status.i2c = bsp_i2c_init();
    if (bsp_board_status.i2c != ESP_OK) return nullptr;
    bsp_board_status.expander = bsp_io_expander_init();
    if (bsp_board_status.expander != ESP_OK) return nullptr;
    bsp_board_status.power = bsp_power_init();
    if (bsp_board_status.power != ESP_OK) return nullptr;
    {
        esp_lv_adapter_config_t adapter = ESP_LV_ADAPTER_DEFAULT_CONFIG();
        // Xtensa GUI callbacks and APP/snapshot transitions need their own stack
        // budget; free PSRAM does not enlarge the adapter's default 8 KB stack.
        adapter.task_stack_size = 16 * 1024;
        bsp_board_status.display = esp_lv_adapter_init(&adapter);
        if (bsp_board_status.display != ESP_OK) goto failed;
        display = bsp_display_lcd_init();
        if (!display) goto failed;
        input = bsp_display_indev_init(display);
        if (bsp_board_status.touch != ESP_OK) {
            ESP_LOGW(TAG, "Touch: %s", esp_err_to_name(bsp_board_status.touch));
            if (touch) { esp_lcd_touch_del(touch); touch = nullptr; }
        }
        bsp_board_status.display = esp_lv_adapter_start();
        if (bsp_board_status.display != ESP_OK) goto failed;
        bsp_board_status.display = bsp_display_brightness_set(CONFIG_BSP_LCD_BRIGHTNESS);
        if (bsp_board_status.display != ESP_OK) goto failed;
    }
    display_started = true;
    ESP_LOGI(TAG, "AXS15231B QSPI %dx%d RGB565; LVGL %d.%d.%d, full-frame PSRAM + DMA strips",
             BSP_LCD_H_RES, BSP_LCD_V_RES, LVGL_VERSION_MAJOR, LVGL_VERSION_MINOR, LVGL_VERSION_PATCH);
    return display;

failed:
    ESP_LOGE(TAG, "Display startup failed: %s", esp_err_to_name(bsp_board_status.display));
    bsp_display_backlight_off();
    backlight_released = false;
    if (esp_lv_adapter_is_initialized()) esp_lv_adapter_deinit();
    input = nullptr;
    display = nullptr;
    if (touch) { esp_lcd_touch_del(touch); touch = nullptr; }
    if (panel) { esp_lcd_panel_del(panel); panel = nullptr; }
    if (lcd_io) {
        esp_lcd_panel_io_del(lcd_io);
        lcd_io = nullptr;
        spi_bus_free(BSP_LCD_SPI_HOST);
    }
    if (lcd_dma_buffer) { heap_caps_free(lcd_dma_buffer); lcd_dma_buffer = nullptr; }
    if (lcd_transfer_done) { vSemaphoreDelete(lcd_transfer_done); lcd_transfer_done = nullptr; }
    return nullptr;
}

lv_display_t *bsp_display_get(void) { return display; }
lv_indev_t *bsp_display_get_input_dev(void) { return input; }
esp_lcd_touch_handle_t bsp_touch_get_handle(void) { return touch; }
bool bsp_display_lock(uint32_t timeout_ms)
{
    return esp_lv_adapter_lock(timeout_ms ? (int32_t)timeout_ms : -1) == ESP_OK;
}
void bsp_display_unlock(void) { esp_lv_adapter_unlock(); }
