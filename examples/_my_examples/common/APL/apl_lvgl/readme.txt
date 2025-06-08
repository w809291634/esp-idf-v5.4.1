F:\esp32_8266_files\esp-idf-v5.4.1\examples\_my_examples\lcd\spi_lcd_touch\managed_components\lvgl__lvgl

从这个文件夹中拷贝过来，没有修改什么，然后直接指定文件夹名字为 apl_lvgl

然后再应用的 cmakelist 中直接指定即可
# 匹配 src 目录下所有的 .c 文件
file(GLOB SRC_LIST "src/*.c")

idf_component_register(SRCS ${SRC_LIST}
                    PRIV_REQUIRES board board_config
                    REQUIRES spi_flash drv_led apl_utility apl_lvgl esp_lcd
                    INCLUDE_DIRS "include")
