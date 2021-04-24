#pragma once

#include <stddef.h>

void app_btplayer_i2s_config(
        int sample_rate,
        int bits_per_sample,
        int channels);
void app_byplayer_i2s_send_data(const uint8_t *buf, uint32_t len);
void app_btplayer_init(const char * const mount_path, size_t mount_path_size);

