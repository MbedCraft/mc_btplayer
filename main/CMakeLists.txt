idf_component_register( SRCS
                            "src/btplayer.c"
                            "src/app_console.c"
                            "src/app_btplayer.c"
                        INCLUDE_DIRS "inc"
                        PRIV_REQUIRES
                            # MbedCraft components
                            mc_aac_decoder
                            mc_bt
                            mc_cmd_aac
                            mc_cmd_fs
                            mc_cmd_nvs
                            mc_cmd_system
                            mc_console
                            mc_fs
                            mc_nvs
                            # ESP specifics components
                            esp_event
                            bt
                        )

spiffs_create_partition_image(storage ../resources FLASH_IN_PROJECT)
