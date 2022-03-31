# Embedded Systems Guessing Game

This was produced as part of an assignment for the 'CE223 - C Programming and Embedded Systems' module at University of Essex. This was a great exercise to practice various concepts of both C/C++ Programming and embedded systems development, using the ARM Mbed Cortex M3 (LPC1786).

## The Guessing Game

The task was to develop a guessing game on the ARM Mbed, using the ARM Mbed development board that was provided. A picture of the development board is shown below:

<img src="../images/../embedded-systems-guessing-game/images/board.jpg" width="350">

The Mbed will interact with the host PC using serial communication via the RS232 protocol. And thus, a host terminal application is also developed (essentially a client-server model).  More details about the game are shown below
- A difficulty setting for the game is entered using the first 3 switches (3 difficulty modes). The difficulty is set using the switches on the board.
- A secret number is entered on the Mbed Development, which is between 0 and 30.
- The PC host application receives a message via serial communication that the game is ready to start.
- The guessing number is entered on the PC application, and the Mbed receives this number via serial communication and processes it.
- The results of the guess and the updates score is shown on the PC application.
- Mechanisms for rejecting invalid inputs from both PC and Mbed is implemented.
- If the player loses, all the LEDs on the board displays red
- If the player wins, the LEDs display 'Christmas lights'.
- LCD screen displays status messages and whether the player loses or wins at the end of the game

<img src="../embedded-systems-guessing-game/images/terminal.png" width="350">

The image above shows the welcome screen on the terminal application, which is run on the host computer.

## ARM Mbed Development Board

The Mbed Development board contains the following:
- 4x4 Number pad
- LEDs which can display green or red, which is controlled via Serial Peripheral Interface (SPI).
- Switches which are controlled by the latch pin of TLC59281 IC.
- An Ethernet port (unused for this project)
- An ARM Mbed (LPC1786)

## Board Schematic

The schematic of the ARM Mbed Development board is given in the 'Board Schematic' folder in this repository. 

## Libraries used

- The RS232 C library by Teunis van Beelen was used for this project. A link to this library: [RS232 Library](https://www.teuniz.net/RS-232/)
- The 'TextLCD' library by Thomas Lunzer was used for the LCD screen. Link: [TextLCD Library](https://os.mbed.com/users/tlunzer/code/TextLCD/docs/tip/classTextLCD.html) 
