# ESP32-HID (GamePad, Mouse, Keyboard, BLE remote control)

Program for ESP32 that turns it into a BLE gamepad that can be use on Windows or Mac.

HID Examples https://bit.ly/2IPKHK8
BLE Libraries https://github.com/nkolban/ESP32_BLE_Arduino

### 2019-09-26
CPU-FREQ设80M, 降低工作电流(130 -> 70MA)
当数据变化时才发送摇控数据, 检测间隔2MS, 按键debounce 2*5 = 10MS
加入mode, 切换mode后, 会将摇杆映射到U D L R 及 A B O X
