#include <stdio.h>
#include <string.h>

#include "esp_console.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_system.h"

#include "mc_bt.h"
#include "mc_fs.h"
#include "mc_nvs.h"

#include "app_console.h"
#include "app_btplayer.h"

const char _mount_path[] = "/data";

void app_main(void)
{
    /*
     * NVS is used to store some secrets (credentials) as well as the
     * PHY calibration data
     */
    mc_nvs_init();

    /*
     * Console command history can be stored to and loaded from a file.
     * The easiest way to do this is to use FATFS filesystem on top of
     * wear_levelling library.
     */
    mc_fs_init(_mount_path);

    mc_bt_init(CONFIG_BT_DEVICE_NAME);

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    app_console_init(_mount_path, sizeof(_mount_path));
    app_btplayer_init(_mount_path, sizeof(_mount_path));
}
