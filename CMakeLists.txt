idf_component_register(SRCS "storage_wrapper.c" "console.c" "hexdump.c"
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES nvs_flash console)
