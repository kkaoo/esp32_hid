# ESP32-HID (GamePad, Mouse, Keyboard, BLE remote control)

Program for ESP32 that turns it into a BLE gamepad that can be use on Windows or Mac.

HID Examples https://bit.ly/2IPKHK8
BLE Libraries https://github.com/nkolban/ESP32_BLE_Arduino

### 2019-09-26
CPU-FREQ设80M, 降低工作电流(130 -> 70MA)
当数据变化时才发送摇控数据, 检测间隔2MS, 按键debounce 2*5 = 10MS
加入mode, 切换mode后, 会将摇杆映射到U D L R 及 A B O X

### 2019-10-26
调整一些IO了
IO33分离出来做电池电量检测
修正右摇杆Z轴，R轴互换的BUG。
IO4改成输出LOW，然后使X，Y，Z，R轴硬件上实现电平翻转，程序上去掉软件翻转功能（ 0 <-> 4095）
解决sleep=50UA问题：因为LED PWM PIN没有进行初始化成OUTPUT MODE
调整REPORT表
    已知问题----》 GPIO0不能PULL DOWN
    已知问题----》 蓝牙连接不太稳定，会掉线，可能和REPORT表有关. 树莓派常常不能正常工作。