# PID_Thermostat

## 这是什么

一个基于STM32F030的PID恒温控制器。本来想做3D打印机热床用的，现在就是个温控demo。

## 硬件

- STM32F030K6T6
- MAX6675 热电偶测温
- EC11旋转编码器
- P-MOSFET(AOD4184A) 加热驱动
- 自己画的4层PCB

## 软件

- 增量式PID算法
- 软件SPI读MAX6675
- 软件I2C驱动OLED
- EC11编码器驱动

## 怎么用

Keil打开PID_Thermostat.ioc，编译烧录。拧编码器调目标温度，显示屏第一行是当前温度，第二行是目标温度。

## 安全

热电偶断线或者超过120度会自动关加热。
