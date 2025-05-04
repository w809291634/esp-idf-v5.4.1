@echo off
:: 设置你的ESP-IDF项目根目录（或作为参数传入）
set "PROJECT_PATH=F:\esp32_8266_files\vscode_idf_5.4.1_release\esp-idf-v5.4.1\examples\get-started\hello_world"

:: 默认串口号和波特率
set "DEFAULT_COM=COM3"
set "DEFAULT_BPS=460800"

:: 获取当前终端所在目录
set "CURRENT_DIR=%cd%"

:: 检查是否传入了参数
if "%~1"=="" (
    call :usage
    exit /b 1
)

:::::::::::::::::::::::::::::::::::::::::::::
:: 判断参数并调用对应函数
:::::::::::::::::::::::::::::::::::::::::::::

if /i "%~1"=="build" (
    call :build
) else if /i "%~1"=="menuconfig" (
    call :menuconfig
) else if /i "%~1"=="clean" (
    call :clean
) else if /i "%~1"=="fullclean" (
    call :fullclean
) else if /i "%~1"=="size" (
    call :size
) else if /i "%~1"=="size-components" (
    call :size_components
) else if /i "%~1"=="size-files" (
    call :size_files
) else if /i "%~1"=="reconfigure" (
    call :reconfigure
) else if /i "%~1"=="set-target" (
    if "%~2"=="" (
        echo Error: Target not specified.
        call :usage_set_target
        exit /b 1
    )
    call :set_target %~2
) else if /i "%~1"=="flash" (
    if "%~2"=="" (
        echo Using default port: %DEFAULT_COM%, baud rate: %DEFAULT_BPS%
        call :flash %DEFAULT_COM% %DEFAULT_BPS%
    ) else (
        call :flash %~2 %~3
    )
) else if /i "%~1"=="monitor" (
    if "%~2"=="" (
        echo Using default port: %DEFAULT_COM%, baud rate: %DEFAULT_BPS%
        call :monitor %DEFAULT_COM% %DEFAULT_BPS%
    ) else (
        call :monitor %~2 %~3
    )
) else if /i "%~1"=="flash_monitor" (
    if "%~2"=="" (
        echo Using default port: %DEFAULT_COM%, baud rate: %DEFAULT_BPS%
        call :flash_monitor %DEFAULT_COM% %DEFAULT_BPS%
    ) else (
        call :flash_monitor %~2 %~3
    )
) else (
    echo Invalid command: %~1
    call :usage
    exit /b 1
)

exit /b 0

::::::::::::::::::::::::::::::::::
:: 函数定义开始
::::::::::::::::::::::::::::::::::

:back_dir
    echo Switching back to original directory: %CURRENT_DIR%
    cd /d "%CURRENT_DIR%"
    exit /b

:usage
    echo Usage: %0 [command]
    echo Commands:
    echo   build
    echo   menuconfig
    echo   clean
    echo   fullclean
    echo   size
    echo   size-components
    echo   size-files
    echo   reconfigure
    echo   set-target ^<target^>
    echo   flash [port] [baud_rate]
    exit /b

:usage_set_target
    echo Usage: %0 set-target ^<target^>
    echo Example: %0 set-target esp32s3
    exit /b

:switch_to_project
    echo Switching to project directory: %PROJECT_PATH%
    cd /d "%PROJECT_PATH%" && (
        exit /b 0
    ) || (
        echo Error: Failed to enter project directory, please check if the path exists.
        call :back_dir
        exit /b 1
    )

::::::::::::::::::::::::::::::::::
:: 各个命令实现
::::::::::::::::::::::::::::::::::

:build
    call :switch_to_project
    echo Start building the project...
    idf.py build
    call :back_dir
    exit /b 0

:menuconfig
    call :switch_to_project
    echo Open the menuconfig configuration screen...
    idf.py menuconfig
    call :back_dir
    exit /b 0

:clean
    call :switch_to_project
    echo Cleaning the build files...
    idf.py clean
    call :back_dir
    exit /b 0

:fullclean
    call :switch_to_project
    echo Full cleaning (distclean) the project...
    idf.py fullclean
    call :back_dir
    exit /b 0

:size
    call :switch_to_project
    echo Showing overall firmware size...
    idf.py size
    call :back_dir
    exit /b 0

:size_components
    call :switch_to_project
    echo Showing component-wise firmware size...
    idf.py size-components
    call :back_dir
    exit /b 0

:size_files
    call :switch_to_project
    echo Showing file-level firmware size...
    idf.py size-files
    call :back_dir
    exit /b 0

:reconfigure
    call :switch_to_project
    echo Reconfiguring the project with CMake...
    idf.py reconfigure
    call :back_dir
    exit /b 0

:set_target
    call :switch_to_project
    echo Setting target to: %~1
    idf.py set-target %~1
    call :back_dir
    exit /b 0

:flash
    call :switch_to_project
    echo Flashing to device on port: %~1 at baud rate: %~2
    idf.py flash -p %~1 -b %~2
    call :back_dir
    exit /b 0

:monitor
    call :switch_to_project
    echo Starting monitor on port: %~1 at baud rate: %~2
    idf.py monitor -p %~1 -b %~2
    call :back_dir
    exit /b 0

:flash_monitor
    call :switch_to_project
    echo Flashing to device on port: %~1 at baud rate: %~2
    idf.py flash -p %~1 -b %~2
    echo Starting monitor...
    idf.py monitor -p %~1 -b %~2
    call :back_dir
    exit /b 0
