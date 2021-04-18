/* ------------------------------------------------------------------------- *\
 * Espressif specific includes
 * ------------------------------------------------------------------------- */
#include "driver/i2s.h"
#include "esp_log.h"

/* ------------------------------------------------------------------------- *\
 * MbedCraft includes
 * ------------------------------------------------------------------------- */
#include "app_btplayer.h"
#include "mc_bt_a2dp.h"
#include "mc_bt_gap.h"

/* ------------------------------------------------------------------------- *\
 * Private function definitions
 * ------------------------------------------------------------------------- */
static void __a2dp_config_cb(esp_a2d_mcc_t *mcc);
static void __a2d_sink_data_cb(const uint8_t *buf, uint32_t len);
static bool __gap_cfm_req_cb(uint32_t num_val);
static void __i2s_init(void);

/* ------------------------------------------------------------------------- *\
 * Public function implementations
 * ------------------------------------------------------------------------- */
void app_btplayer_init(const char * const mount_path, size_t mount_path_size) {
    __i2s_init();

    mc_bt_gap_register_cfm_req_cb(__gap_cfm_req_cb);
    mc_bt_a2dp_register_configuration_cb(__a2dp_config_cb);
    mc_bt_a2dp_register_data_cb(__a2d_sink_data_cb);
}

/* ------------------------------------------------------------------------- *\
 * Private function implementations
 * ------------------------------------------------------------------------- */
static void __i2s_init(void) {
    i2s_config_t i2s_config = {
        .mode = I2S_MODE_MASTER | I2S_MODE_TX,                                  // Only TX
        .sample_rate = 44100,
        .bits_per_sample = 16,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,                           //2-channels
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .dma_buf_count = 6,
        .dma_buf_len = 60,
        .intr_alloc_flags = 0,                                                  //Default interrupt priority
        .tx_desc_auto_clear = true                                              //Auto clear tx descriptor on underflow
    };


    i2s_driver_install(0, &i2s_config, 0, NULL);
    i2s_pin_config_t pin_config = {
        .bck_io_num = CONFIG_I2S_BCK_PIN,
        .ws_io_num = CONFIG_I2S_WS_PIN,
        .data_out_num = CONFIG_I2S_DATA_PIN,
        .data_in_num = -1                                                       //Not used
    };

    i2s_set_pin(0, &pin_config);
}

static bool __gap_cfm_req_cb(uint32_t num_val) {
    ESP_LOGI(__func__, "Please compare the numeric value: %d", num_val);

    return true;
}

static void __a2dp_config_cb(esp_a2d_mcc_t *mcc) {
    if (mcc->type == ESP_A2D_MCT_SBC) {
        int sample_rate = 16000;
        char oct0 = mcc->cie.sbc[0];
        if (oct0 & (0x01 << 6)) {
            sample_rate = 32000;
        } else if (oct0 & (0x01 << 5)) {
            sample_rate = 44100;
        } else if (oct0 & (0x01 << 4)) {
            sample_rate = 48000;
        }
        i2s_set_clk(0, sample_rate, 16, 2);

        ESP_LOGI(__func__, "Configure audio player %x-%x-%x-%x",
                mcc->cie.sbc[0],
                mcc->cie.sbc[1],
                mcc->cie.sbc[2],
                mcc->cie.sbc[3]);
        ESP_LOGI(__func__, "Audio player configured, sample rate=%d", sample_rate);
    }
}

void __a2d_sink_data_cb(const uint8_t *buf, uint32_t len) {
    size_t bytes_written = 0;

    i2s_write(0, buf, len, &bytes_written, portMAX_DELAY);
}
