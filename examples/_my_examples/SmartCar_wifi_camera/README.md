| Supported Targets | ESP32 | ESP32-C2 | ESP32-C3 | ESP32-C5 | ESP32-C6 | ESP32-C61 | ESP32-H2 | ESP32-P4 | ESP32-S2 | ESP32-S3 | Linux |
| ----------------- | ----- | -------- | -------- | -------- | -------- | --------- | -------- | -------- | -------- | -------- | ----- |

# Hello World Example
CONSOLE_STORE_HISTORY 
1、如果激活 CONSOLE_STORE_HISTORY ，需要使用 partitions_example.csv 分区表，建立文件系统 storage,  data, fat,     ,        1M, 分区
FORCE_SUPPORTS_ESCAPE_SEQ
2、强制进行转义字符，如果终端不支持的话，可能会出现无法输入的问题，开启后，可以补全和历史

连接 esp32-camera 这个wifi，然后手机中直接可以查看如下网址即可
http://192.168.4.1/stream

