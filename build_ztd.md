## build sp flash tool 
### 安装qt5 
```
https://download.qt.io/archive/qt/5.14/5.14.2/
https://www.bilibili.com/video/BV1JSUKYyE7b?spm_id_from=333.788.videopod.sections

```  

### 编译命令
```
1. 使用make.sh 编译
./make.sh debug clean
./make.sh debug 

2. 使用make编译
make

```  

## gdb 调试 

### 可用的命令行参数 

使用./SPFlashToolV6 -h 可以看到可用的命令行参数如下

```
piter@Y9000p:/mnt/wsl/PHYSICALDRIVE1/Work/TestFW/sp_flash_tool/v6/_Output/linux/debug$ ./SPFlashToolV6 -h
\sysname:Linux
 nodename:Y9000p
 release:5.15.153.1-microsoft-standard-WSL2
 version:#1 SMP Fri Mar 29 23:14:13 UTC 2024
 machine:x86_64
 
  domainame:
 cur_kernel_version = 5.15.153.1
Usage: ./SPFlashToolV6 [options]

Options:
  -i, --config <config xml file>         console mode configuration file,
                                         mandatory with config.xml.
                                                SPFlashToolV6.exe -i config.xml

  -f, --flash_xml <flash xml file>       flash xml file path, mandatory without
                                         config.xml.
                                                SPFlashToolV6.exe -c download -f
                                         flash.xml

  -c, --operation <operation>            flash tool features, mandatory without
                                         config.xml, just support:
                                                    format
                                                    download
                                                    format-download
                                                    firmware-upgrade
                                                    dram-repair
                                                    write-efuse
                                                    read-efuse
                                                    download-cert
                                                  SPFlashToolV6.exe -c download
                                         -f flash.xml

  -e, --cert_file <cert file>            certification file path.
                                                SPFlashToolV6.exe -e cert_file.cert -f
                                         flash.xml -c download

  -p, --com_port <com port>              com port, format is as COM4+COM6 (BROM
                                         download), COM5 (preloader download) on
                                         Windows and format is as 5-2+1-6(BROM
                                         download), 5-1(preloader download) on
                                         Linux.
                                                  SPFlashToolV6.exe -p COM4+COM6
                                         -f flash.xml -c download
                                                  SPFlashToolV6 -p 5-1+1-6 -i
                                         config.xml

  -t, --battery_mode <battery mode>      Specify battery mode. with, without
                                         and auto are available. if no specify
                                         the -t argument, the auto mode used.   
                                            Only valid without config.xml.
                                                  SPFlashToolV6.exe -f flash.xml
                                         -c download -t without

  -b, --reboot                           reboot the device.
                                                SPFlashToolV6.exe -b -i config.xml

  --disable_storage_life_cycle_check     disable storage life cycle check
                                         feature.
                                                  SPFlashToolV6.exe -f flash.xml
                                         -c download
                                         --disable_storage_life_cycle_check

  -a, --auth_file <auth file>            authentication file path.
                                                SPFlashToolV6.exe -a auth_file.auth -f
                                         flash.xml -c download

  --file <dram repair file>              Specify the file which can be used for
                                         dram-repair, read-efuse and write-efuse
                                         command.
                                                  SPFlashToolV6.exe -f flash.xml
                                         -c write-efuse --file efuse.xml

  -l, --da_log_channel <DA log channel>  Specify the DA log channel. USB and
                                         UART are available. if no specify the
                                         -l argument, the UART will be default
                                         used.
                                                  SPFlashToolV6.exe -f flash.xml
                                         -c write-efuse --file efuse.xml -l USB

  -h, --help                             Displays help on commandline options.
  --help-all                             Displays help including Qt specific
                                         options.
  -v, --version                          Displays version information.

device_poll_thread exit.
```

### .vscode/launch.json文件配置
```
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "Debug SP Flash Tool",
            "type": "cppdbg",
            "request": "launch",
            "program": "/mnt/wsl/PHYSICALDRIVE1/Work/TestFW/sp_flash_tool/v6/_Output/linux/debug/SPFlashToolV6",
           "args": [
                "-f", "/mnt/wsl/PHYSICALDRIVE1/Work/TestFW/sp_flash_tool/v6/flash.xml",
                "-c", "download",
                "-p", "5-1"
            ],
            "stopAtEntry": true,
            "cwd": "/mnt/wsl/PHYSICALDRIVE1/Work/TestFW/sp_flash_tool/v6/_Output/linux/debug/",
            "environment": [
                {
                    "name": "LD_LIBRARY_PATH",
                    "value": "/mnt/wsl/PHYSICALDRIVE1/Work/TestFW/sp_flash_tool/v6/_Output/linux/debug/lib:${env:LD_LIBRARY_PATH}"
                }
            ],
            "externalConsole": true,
            "MIMode": "gdb",
            "setupCommands": [
                {
                    "description": "Enable pretty-printing for gdb",
                    "text": "-enable-pretty-printing",
                    "ignoreFailures": true
                },
                {
                    "description": "Set Disassembly Flavor to Intel",
                    "text": "-gdb-set disassembly-flavor intel",
                    "ignoreFailures": true
                }
            ],
            "miDebuggerPath": "/usr/bin/gdb",
            "sourceFileMap": {
                "/mnt/wsl/PHYSICALDRIVE1/Work": "/mnt/wsl/PHYSICALDRIVE1/Work"
            }
        }
    ]
}
```

### .vscode/settings.json文件配置
```
{
    "C_Cpp.intelliSenseEngine": "Disabled", // 禁用 C/C++ 扩展的 IntelliSense
}
```
