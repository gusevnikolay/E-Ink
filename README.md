### E-Ink library
This library is a simplified and easy-to-use library from WaveShare.
https://www.waveshare.com/wiki/1.54inch_e-Paper_Module

This library requires several hardware-dependent functions for the implementation to work, an example of which is shown for the STM32F4Discovery board.
The project for Keil uVision has the library itself and a way to use it.

As a demonstration of the work, a C# project has been attached, which allows you to upload custom pictures via the UART interface at 115200 baud.

Connecting the display to the development as follows:

PA0 - UART-TX
PA1 - UART-RX

PA5 - SCK
PA7 - MOSI
PC4 - DC
PC5 - RST
PB0 - BUSY
PB1 - CS


