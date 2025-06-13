# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
set(COMPONENTS main esp_wifi)
project(wifi_softAP)
set(EXCLUDE_VERSION_GENERATION TRUE)