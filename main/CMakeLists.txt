idf_component_register( SRCS
                            "src/btplayer.c"
                            "src/app_console.c"
                            "src/app_btplayer.c"
                        INCLUDE_DIRS "inc"
                        PRIV_REQUIRES
                            # MbedCraft components
                            mc_bt
                            mc_cmd_system
                            mc_cmd_nvs
                            mc_console
                            mc_fs
                            mc_nvs
                            # ESP specifics components
                            esp_event
                            bt
                        )
