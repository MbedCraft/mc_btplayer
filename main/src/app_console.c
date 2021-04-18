#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_console.h"
#include "esp_system.h"
#include "esp_log.h"
#include "argtable3/argtable3.h"
#include "linenoise/linenoise.h"

#include "mc_cmd_system.h"
#include "mc_cmd_nvs.h"

#include "mc_console.h"
#include "mc_nvs.h"
#include "mc_fs.h"

#define CONSOLE_TASK_STACK_SIZE 4*1024
#define CONSOLE_TASK_PRIORITY 2

#ifdef CONFIG_ESP_CONSOLE_USB_CDC
#error This example is incompatible with USB CDC console. Please try "console_usb" example instead.
#endif // CONFIG_ESP_CONSOLE_USB_CDC

const char prompt_str[] = CONFIG_IDF_TARGET;

const char _history_filename[] = "history.txt";

char * history_path;

void app_console_task(void * v_param) {
    mc_console_run(prompt_str, sizeof(prompt_str));

    /* Delete the task when the console is closed */
    vTaskDelete(NULL);

    free(history_path);
}

void app_console_init(const char * const mount_path, size_t mount_path_size) {
    int history_path_size = mount_path_size + 1 + sizeof(_history_filename);
    history_path = malloc(history_path_size);
    strlcpy(history_path, mount_path, mount_path_size);
    strlcat(history_path, "/", mount_path_size+1);
    strlcat(history_path, _history_filename, history_path_size);

    mc_console_init(history_path);

    /* Register console commands */
    mc_cmd_system_register();
    mc_cmd_nvs_register();

    /* Start tasks */
    BaseType_t err = xTaskCreate(
            app_console_task,
            "console_task",
            CONSOLE_TASK_STACK_SIZE,
            NULL,
            CONSOLE_TASK_PRIORITY,
            NULL);

    if (err != pdTRUE) {
        ESP_LOGE(__func__, "Create console_task failed");
    }
}

