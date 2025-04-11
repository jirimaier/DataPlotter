[![build result](https://build.opensuse.org/projects/home:maierjir/packages/data-plotter/badge.svg?type=percent)](https://build.opensuse.org/package/show/home:maierjir/data-plotter)

[![Get it from Microsoft Store](https://img.shields.io/badge/Get%20it%20from-Microsoft%20Store-blue?style=for-the-badge&logo=microsoft)](https://apps.microsoft.com/detail/9NVBN2G853FP?hl=en-us&gl=CZ&ocid=pdpshare)
[![Download for Linux (openSUSE, Fedora, Debian, Raspbian)](https://img.shields.io/badge/Download-Linux%20(distributions)-green?style=for-the-badge&logo=linux)](https://software.opensuse.org//download.html?project=home%3Amaierjir&package=data-plotter)

Install on Linux:
```bash
sudo add-apt-repository ppa:cvut-fel-sdi/ppa
sudo apt update
sudo apt install data-plotter
```
Permission to open serial ports:
```bash
sudo usermod -a -G tty $USER
sudo usermod -a -G dialout $USER
```

Data Plotter is a tool for plotting data received over a serial port. Typical use cases are microcontroller-based SDI (Software-defined instruments), for example, an oscilloscope based on STM32 MCU, or data logging of some sensor connected to Arduino.

The communication protocol is designed to be as simple as possible to implement on the MCU firmware side.
Data can be transmitted in binary format or as text.

The capabilities of DataPlotter are similar to oscilloscopes, it can plot the data (obviously), perform FFT and automatic measurements, and save data to a CSV file or as an image.
There are also cursors and XY modes. The plot can operate either in oscilloscope mode (data received as an array of samples captured over a short time period) or in rolling mode (data received point-by-point and continuously appended to previous data over a long time period).
There is also support for logic (digital) channels.

There are various means of controlling the instrument from the DataPlotter:

There is an ANSI terminal emulator that supports escape sequences (you can move the cursor, change text color and background color, etc.).
This allows the creating of a pseudo-graphical UI for displaying measured data and settings.
It also includes an interactive feature: if a character in the terminal has a colorful background, it can be clicked to send that character to the instrument. 
This allows you to create buttons in the terminal.
The command sent by that button is in principle one byte (the character on that button) so it is very easy to process on the MCU (no parser, just UART ISR or whatever).
Some characters have good meaning on the buttons (e.g. + - / <>), but if you run out of meaningful characters, you can just hide the character by setting the font color the same as the background and label the button with text next to it.

Another way of dynamically creating instrument controls is QML (QtQuick language). The QML code can be sent to DataPlotter by the instrument (microcontroller) and the new GUI will be shown.
There is a minimalistic (easy-to-use) interface for sending and receiving data from the QML code. This offers almost endless possibilities for customization.

More info (in Czech language): https://embedded.fel.cvut.cz/platformy/dataplotter
