# 匹配 src 目录下所有的 .c 文件
file(GLOB SRC_LIST "src/*.c")

idf_component_register(SRCS ${SRC_LIST}
                    PRIV_REQUIRES board board_config
                    REQUIRES spi_flash drv_led apl_utility apl_lvgl esp_lcd
                    INCLUDE_DIRS "include")