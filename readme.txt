idf.py set-target <target>
idf.py menuconfig
idf.py build
idf.py clean
idf.py fullclean
idf.py flash -p COM38 -b 460800
idf.py monitor -p COM38 -b 115200
idf.py size
idf.py size-components
idf.py size-files
idf.py reconfigure

idf_build flash COM38  --> idf.py flash -p COM38 -b (DEFAULT_FBPS使用默认下载)
idf_build flash  --> idf.py flash -p (DEFAULT_COM使用默认下载) -b (DEFAULT_FBPS使用默认下载)
idf_build monitor COM38  --> idf.py monitor -p COM38 -b (DEFAULT_MBPS使用默认监控)
idf_build monitor  --> idf.py monitor -p (DEFAULT_COM使用默认监控) -b (DEFAULT_MBPS使用默认监控)

使用工具链
esp-idf-tools_-v5.4.1.zip

修改：pyvenv.cfg
F:\esp32_8266_files\esp-idf-v5.4.1\esp-idf-tools_for_idf_v5_4_1\python_env\idf5.4_py3.11_env\pyvenv.cfg
    home = F:\esp32_8266_files\vscode_idf_5.4.1_release\esp-idf-tools\tools\idf-python\3.11.2
    include-system-site-packages = false
    version = 3.11.2
    executable = F:\esp32_8266_files\vscode_idf_5.4.1_release\esp-idf-tools\tools\idf-python\3.11.2\python.exe
    command = F:\esp32_8266_files\vscode_idf_5.4.1_release\esp-idf-tools\tools\idf-python\3.11.2\python.exe -m venv --clear F:\esp32_8266_files\vscode_idf_5.4.1_release\esp-idf-tools\python_env\idf5.4_py3.11_env

    将上面中 F:\esp32_8266_files\esp-idf-v5.4.1\esp-idf-tools_for_idf_v5_4_1 统一替换为
    IDF_TOOLS_PATH 环境变量路径，否则python使用失败

修改：idf_build.bat       进行指定编译，下载
    ::::::::: 配置 :::::::::
    :: 设置你的ESP-IDF项目根目录（或作为参数传入）
    set "PROJECT_PATH=F:\esp32_8266_files\vscode_idf_5.4.1_release\esp-idf-v5.4.1\examples\get-started\hello_world"
    :: 默认串口号和波特率
    set "DEFAULT_COM=COM4"
    set "DEFAULT_FBPS=460800"
    set "DEFAULT_MBPS=115200"
    ::::::::: 配置 :::::::::

修改：idf_cmd_init.bat  初始化IDF环境
    ::::::::: 配置 :::::::::
    set IDF_TOOLS_PATH=F:\esp32_8266_files\vscode_idf_5.4.1_release\esp-idf-tools
    set __IDF_PATH=F:\esp32_8266_files\vscode_idf_5.4.1_release\esp-idf-v5.4.1
    set __IDF_PYTHON=%IDF_TOOLS_PATH%\python_env\idf5.4_py3.11_env\Scripts\python.exe
    set __IDF_GIT=%IDF_TOOLS_PATH%/tools/idf-git/2.39.2/cmd/git.exe
    ::::::::: 配置 :::::::::
