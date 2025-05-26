# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

list(APPEND EXTRA_COMPONENT_DIRS "$ENV{IDF_PATH}/examples/_my_examples/common/DRV/drv_led")
# 能够减少编译时间
set(COMPONENTS main)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(base_template)
