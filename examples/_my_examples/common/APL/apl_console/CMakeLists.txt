# 匹配 src 目录下所有的 .c 文件
file(GLOB SRC_LIST "*.c")

# 注册组件
idf_component_register(
    SRCS ${SRC_LIST}
    INCLUDE_DIRS "."
    REQUIRES apl_console_cmd_nvs apl_console_cmd_system apl_console_cmd_wifi fatfs
)
