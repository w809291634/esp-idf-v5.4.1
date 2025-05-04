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
