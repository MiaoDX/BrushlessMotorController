# MotorController

Brushless Motor Controller using BGC 2-Axis Board, arduino code.

# Burn the bootloader

We should burn the bootloader first in order to upload the new sketch into the board. See [Reviving a Clone Basecam brushless gimbal controller](http://www.brokking.net/article_revive_bgc.html) and [Using an Arduino as an AVR ISP (In-System Programmer)](https://www.arduino.cc/en/Tutorial/ArduinoISP). The former one dose the exact thing we are doing and gives the wiring, the latter one mentioned we should burn `ArduinoISP` example into UNO first.

# Usage

* Open code with Arduino
* Choose 工具->开发板->Arduino pro or pro mini
* choose 工具->处理器->ATmega 328(5v,16Hz)
* choose 工具->编程器->USBasp
* Press 上传

# Requirement 

Only tested in BGC 2-Axis Board.

Official BGC Board is not OK, since its ROM cannot be modified.

  
