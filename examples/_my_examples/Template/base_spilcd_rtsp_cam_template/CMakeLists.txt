# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

# 定义一个函数，用于将指定目录下的所有子目录加入 EXTRA_COMPONENT_DIRS
function(add_component_dirs_from root_dir)
    if(EXISTS "${root_dir}")
        FILE(GLOB subdirs RELATIVE "${root_dir}" "${root_dir}/*")
        foreach(dir ${subdirs})
            set(subdir_path "${root_dir}/${dir}")
            if(IS_DIRECTORY "${subdir_path}")
                message(STATUS "Adding component dir: ${subdir_path}")
                list(APPEND EXTRA_COMPONENT_DIRS "${subdir_path}")
            endif()
        endforeach()
        # 将更新后的变量传递回父作用域
        set(EXTRA_COMPONENT_DIRS ${EXTRA_COMPONENT_DIRS} PARENT_SCOPE)
    else()
        message(WARNING "Directory not found: ${root_dir}")
    endif()
endfunction()

# 使用函数添加 APL 和 DRV 下的所有子目录
set(APL_DIR "$ENV{IDF_PATH}/examples/_my_examples/common/APL")
set(DRV_DIR "$ENV{IDF_PATH}/examples/_my_examples/common/DRV")
add_component_dirs_from(${APL_DIR})
add_component_dirs_from(${DRV_DIR})

# 可选：打印当前列表（调试用）
# message(STATUS "Current EXTRA_COMPONENT_DIRS = ${EXTRA_COMPONENT_DIRS}")
# 能够减少编译时间
# set(COMPONENTS main esp_psram apl_lvgl esp_lcd)
set(EXCLUDE_VERSION_GENERATION TRUE)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(base_template)
