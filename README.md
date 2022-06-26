# FastBit Embedded Brain Academy - Udemy Course: "STM32Fx Microcontroller Custom Bootloader Development"
Project workspace containing various course example plus a custom bootloader project written for STM32F446RE MCU. Used Keil uVision5, Nucleo-F446RE, Saleae Logic Analyzer.

**Detailed course notes can be found in the top-level directory**

## Directory Contents
```
**STM32CustomBootloader**
|
└───Nucleo-F446RE
    └───Blinky
    |   | - main.c
    |
    └───HOST
    |   └───C
    |   |   └───STM32_Programmer_V1
    |   |       | - user_app.bin
    |   |       |
    |   |       └───Sources
    |   |       |   | - BlCommands.c
    |   |       |   | - BlReplyProcessing.c
    |   |       |   | - fileops.c
    |   |       |   | - LinuxSerialPort.c
    |   |       |   | - main.c
    |   |       |   | - OSxSerialPort.c
    |   |       |   | - utilities.c
    |   |       |   | - WindowsSerialPort.c
    |   |       |
    |   |       └───Headers
    |   |           | - LinuxSerialPort.h
    |   |           | - main.h
    |   |           | - OSxSerialPort.h
    |   |           | - WindowsSerialPort.h
    |   |   
    |   └───Python
    |       | - STM32_Programmer_V1.py
    |       | - user_app.bin
    |
    └───User_app_STM32F446xx
    |	└───Core
    |       └───Inc
    |       |   | - main.h
    |       |   | - stm32f4xx_hal_conf.h
    |       |   | - stm32f4xx_it.h
    |       |
    |       └───Src
    |           | - main.c
    |           | - stm32f4xx_hal_msp.c
    |           | - stm32f4xx_it.c
    |           | - system_stm32f4xx.c
    |    		
    └───bootloader_STM32F446xx
        └───Core
            └───Inc
    	    |   | - main.h
    	    |   | - stm32f4xx_hal_conf.h
    	    |   | - stm32f4xx_it.h
    	    |
    	    └───Src
    	        | - main.c
    	        | - stm32f4xx_hal_msp.c
    	        | - stm32f4xx_it.c
                | - system_stm32f4xx.c
```
