# 匹配 src 目录下所有的 .c 文件
file(GLOB SRC_LIST "src/*.c")

idf_component_register(SRCS ${SRC_LIST}
                    REQUIRES spi_flash board drv_led apl_utility board_config
                    INCLUDE_DIRS "include")