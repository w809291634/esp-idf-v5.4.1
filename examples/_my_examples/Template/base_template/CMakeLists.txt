# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

# 添加组件搜索目录
list(APPEND EXTRA_COMPONENT_DIRS "$ENV{IDF_PATH}/examples/_my_examples/common/APL/apl_utility")
list(APPEND EXTRA_COMPONENT_DIRS "$ENV{IDF_PATH}/examples/_my_examples/common/DRV/drv_led")
list(APPEND EXTRA_COMPONENT_DIRS "$ENV{IDF_PATH}/examples/_my_examples/common/APL/apl_console")
list(APPEND EXTRA_COMPONENT_DIRS "$ENV{IDF_PATH}/examples/_my_examples/common/APL/apl_console_cmd_nvs")
list(APPEND EXTRA_COMPONENT_DIRS "$ENV{IDF_PATH}/examples/_my_examples/common/APL/apl_console_cmd_system")
list(APPEND EXTRA_COMPONENT_DIRS "$ENV{IDF_PATH}/examples/_my_examples/common/APL/apl_console_cmd_wifi")

# 能够减少编译时间
set(COMPONENTS main esp_psram)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(base_template)
