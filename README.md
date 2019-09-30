# ESP32-HID

### 线路图
![Alt text](/doc/ESP32_HID_sch_v2.jpg)

原模组CH340串口IC静态电流1.6MA，如下图中：
将CH340的VCC挑起来，连接到USB5V，将4脚挑起来接一个0.1UF到GND

修改后还50UA的漏电流，应该是LDO的静态电流，暂时忽略。

![Alt text](/doc/module_fly_wire.jpeg)

### 实物图
挑掉牛屎IC后飞线
![Alt text](/doc/gamepad1.jpg)
连接ESP32模组
![Alt text](/doc/gamepad2.jpg)
原USB线的孔扩大后可以放下USB头，可以充电和烧录程序。
![Alt text](/doc/gamepad3.jpeg)
装好上盖后的成品图
![Alt text](/doc/gamepad4.jpeg)

### 系统兼容
    测试WIN10 64位: 正常
    测试MACOS 10: 正常
    测试RPI-4B buster: 正常（兼容性不是特别好）, 使用模拟器玩dino恐龙快打时感觉有些延迟（在MACOS很正常）

### 树莓派
buster配对蓝牙时，在我的RPI4B有时会出现AUTH TIME的错误，使用bluetoothctl查看蓝牙的状态蓝牙连接成功，但没有成功配对。
手动主屏点蓝牙图标关闭，重新打开，重新配对后正常。是何原因未知。

然后三连发： 安装测试软件jstest, 查看JS设备端口(比如js0), 运行jstest测试

    sudo apt install jstest
    ls /dev/input/
    jstest /dev/input/js0

![Alt text](/doc/rpi4b_jstest.png)

### MACOS
使用App来测试(Controllers Lite - macos)
![Alt text](/doc/gamepad_test1.png)

使用模拟器测试(OpenEmu - macos)
![Alt text](/doc/gamepad_test2.png)
