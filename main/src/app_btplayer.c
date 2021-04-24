/* ------------------------------------------------------------------------- *\
 * Standard Includes
 * ------------------------------------------------------------------------- */
#include <string.h>

/* ------------------------------------------------------------------------- *\
 * Espressif specific includes
 * ------------------------------------------------------------------------- */
#include "driver/i2s.h"
#include "esp_log.h"

/* ------------------------------------------------------------------------- *\
 * MbedCraft includes
 * ------------------------------------------------------------------------- */
#include "app_btplayer.h"
#include "mc_aac.h"
#include "mc_assert.h"
#include "mc_bt_a2dp.h"
#include "mc_bt_gap.h"

/* ------------------------------------------------------------------------- *\
 * Private function definitions
 * ------------------------------------------------------------------------- */
static void __a2dp_config_cb(esp_a2d_mcc_t *mcc);
static void __a2dp_connection_cb(bool connected);
static bool __gap_cfm_req_cb(uint32_t num_val);
static void __i2s_init(void);

/* ------------------------------------------------------------------------- *\
 * Private variables
 * ------------------------------------------------------------------------- */
static int __cur_bits_per_sample;
static int __cur_sample_rate;
static int __cur_channels;
static const char __sounds_dir[] = "/data/sounds/aac/";
static const char __sound_conn_pass_is_fn[] = "conn_pass_is.aac";
static const char __sound_connected_fn[] = "connected.aac";
static const char __sound_disconnected_fn[] = "disconnected.aac";
static const char __sound_digits_fn[][6] = {
    "0.aac",
    "1.aac",
    "2.aac",
    "3.aac",
    "4.aac",
    "5.aac",
    "6.aac",
    "7.aac",
    "8.aac",
    "9.aac",
};

/* ------------------------------------------------------------------------- *\
 * Public function implementations
 * ------------------------------------------------------------------------- */
void app_btplayer_init(const char * const mount_path, size_t mount_path_size) {
    __i2s_init();

    mc_aac_init(app_btplayer_i2s_config, app_byplayer_i2s_send_data);
    mc_bt_gap_register_cfm_req_cb(__gap_cfm_req_cb);
    mc_bt_a2dp_register_connection_cb(__a2dp_connection_cb);
    mc_bt_a2dp_register_configuration_cb(__a2dp_config_cb);
    mc_bt_a2dp_register_data_cb(app_byplayer_i2s_send_data);
}

void app_btplayer_i2s_config(
        int sample_rate,
        int bits_per_sample,
        int channels) {

    if (    0
            || (sample_rate != __cur_sample_rate)
            || (bits_per_sample != __cur_bits_per_sample)
            || (channels != __cur_channels)) {

        __cur_sample_rate = sample_rate;
        __cur_bits_per_sample = bits_per_sample;
        __cur_channels = channels;

        ESP_LOGI(__func__, "Reconfiguring i2s: sample_rate(%d), bit_per_sample(%d), channels(%d)",
                __cur_sample_rate, __cur_bits_per_sample, __cur_channels);

        i2s_set_clk(0, sample_rate, bits_per_sample, channels);
    }
}

void app_byplayer_i2s_send_data(const uint8_t *buf, uint32_t len) {
    size_t bytes_written = 0;

    i2s_write(0, buf, len, &bytes_written, portMAX_DELAY);
}

/* ------------------------------------------------------------------------- *\
 * Private function implementations
 * ------------------------------------------------------------------------- */
static void __i2s_init(void) {
    __cur_bits_per_sample = 16;
    __cur_sample_rate = 44100;
    __cur_channels = 2;

    i2s_config_t i2s_config = {
        .mode = I2S_MODE_MASTER | I2S_MODE_TX,                                  // Only TX
        .sample_rate = __cur_sample_rate,
        .bits_per_sample = __cur_bits_per_sample,
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

int __build_path_and_play(const char * const filename, int filename_size) {
    size_t ret;
    char path[64];

    ret = strlcpy(path, __sounds_dir, sizeof(path));
    ASSERTE_RET(ret == sizeof(__sounds_dir) - 1, -1,
            "Internal error 1: %d", ret);
    ret = strlcat(path, filename, sizeof(path));
    ASSERTE_RET(ret == sizeof(__sounds_dir) + filename_size - 2, -1,
            "Internal error 2: %d", ret);

    mc_aac_play_file(path);

    return 0;
}

static void __a2dp_connection_cb(bool connected) {
    if (true == connected) {
        __build_path_and_play(
                __sound_connected_fn,
                sizeof(__sound_connected_fn));
        // Configuring i2s for default a2dp settings
        // FIXME: Check is A2DP module should not call the
        // __a2dp_config_cb callback at connection instead
        app_btplayer_i2s_config(44100, 16, 2);
    } else {
        __build_path_and_play(
                __sound_disconnected_fn,
                sizeof(__sound_conn_pass_is_fn));
    }
}

static bool __gap_cfm_req_cb(uint32_t num_val) {
    int digits[16];
    int nb_digits;

    ESP_LOGI(__func__, "Please compare the numeric value: %d", num_val);

    __build_path_and_play(
            __sound_conn_pass_is_fn,
            sizeof(__sound_conn_pass_is_fn));

    for (nb_digits = 0; num_val != 0; nb_digits++) {
        ASSERTE_RET(nb_digits < (sizeof(digits)/sizeof(digits[0])), -1,
                "Confirmation password is too long");
        digits[nb_digits] = num_val % 10;
        num_val /= 10;
    }

    while (nb_digits--) {
        __build_path_and_play(
                __sound_digits_fn[digits[nb_digits]],
                sizeof(__sound_digits_fn[digits[nb_digits]]));
    }

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

        app_btplayer_i2s_config(sample_rate, 16, 2);

        ESP_LOGI(__func__, "Configure audio player %x-%x-%x-%x",
                mcc->cie.sbc[0],
                mcc->cie.sbc[1],
                mcc->cie.sbc[2],
                mcc->cie.sbc[3]);
        ESP_LOGI(__func__, "Audio player configured, sample rate=%d", sample_rate);
    }
}

