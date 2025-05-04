@echo off
:: 设置你的ESP-IDF项目根目录（或者作为参数传入）
set "PROJECT_PATH=F:\esp32_8266_files\vscode_idf_5.4.1_release\esp-idf-v5.4.1\examples\get-started\hello_world"

:: 获取当前终端所在目录
set "CURRENT_DIR=%cd%"

:: 检查是否传入了参数
if "%~1"=="" (
    echo Usage: %0 [build^|menuconfig]
    exit /b 1
)

:: 判断参数并调用对应函数
if /i "%~1"=="build" (
    call :build
) else if /i "%~1"=="menuconfig" (
    call :menuconfig
) else (
    echo Invalid command: %~1
    echo Usage: %0 [build^|menuconfig]
    exit /b 1
)

exit /b 0

::::::::::::::::::::::::::::::::::
:: 函数定义开始
::::::::::::::::::::::::::::::::::
:back_dir
    echo Switching to directory: %CURRENT_DIR%
    cd /d "%CURRENT_DIR%"
    exit /b
    
:build
    echo Switching to project directory: %PROJECT_PATH%
    cd /d "%PROJECT_PATH%" && (
        echo Start building the project...
        idf.py build
        call :back_dir
    ) || (
        echo ERROR: Failed to enter project directory, please make sure the path exists.
        call :back_dir
        exit /b 1
    )
    call :back_dir
    exit /b 0

:menuconfig
    echo Switching to project directory. %PROJECT_PATH%
    cd /d "%PROJECT_PATH%" && (
        echo Open the menuconfig configuration screen...
        idf.py menuconfig
        call :back_dir
    ) || (
        echo Error: Failed to enter project directory, please check if the path exists.
        call :back_dir
        exit /b 1
    )
    call :back_dir
    exit /b 0