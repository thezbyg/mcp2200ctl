# mcp2200ctl - MCP2200 USB-to-UART serial converter control program.

## mcp2200ctl

mcp2200ctl is a command-line program which allows controlling additional features found in MCP2200 USB-to-UART serial converter ([http://www.microchip.com/wwwproducts/en/MCP2200](http://www.microchip.com/wwwproducts/en/MCP2200)). 

mcp2200ctl has the following features:

 * RX/TX led mode control.
 * RX/TX/RTS/CTS invert control.
 * EEPROM read/write.
 * GPIO read/write.
 * GPIO direction and default value control.
 * Hardware flow control.
 * USB suspend and configure state indication pin control.
 * Ability to set USB manufacturer and product strings.
 * Ability to set custom vendor and product IDs.

## mcp2200gui

mcp2200gui is a GTK 3 based graphical program with the same purpose as mcp2200ctl.

![mcp2200gui screenshot](/share/screenshot.png?raw=true "Optional Title")

## mcp2200ctl usage

List all detected MCP2200 devices:
```shell
mcp2200ctl list
```
```
  1, "Microchip Technology Inc.", "MCP2200 USB Serial Port Emulator", "0000988086", "/dev/hidraw4"
```

Get current configuration:
```shell
mcp2200ctl configure
```
```
Default baud rate: 9600
GPIO directions: ioiooioo
GPIO default values: 11000111
GPIO values: 01000111
RX LED (GPIO pin 6): blink
TX LED (GPIO pin 7): blink
Blink speed: fast
Invert RX/TX/RTS/CTS: off
USB suspend pin (GPIO pin 0): off
USB configuration pin (GPIO pin 1): on
Hardware flow control RTS/CTS: off
```

Get current GPIO values:
```shell
mcp2200ctl get
```
```
GPIO values: 01000111
```

Get current manufacturer, product and serial strings:
```shell
mcp2200ctl describe
```
```
Manufacturer: Microchip Technology Inc.
Product: MCP2200 USB Serial Port Emulator
Serial: 0000988086
```

Get current EEPROM value at address 0x01:
```shell
mcp2200ctl get-eeprom --address=01
```
```
EEPROM value: 12
```

## Building from source

### Compiler

Some of C++11 features are required. Compilation is currently only tested on gcc version 5.3.1.

### Build dependencies

CMake 3.1 or newer ([https://cmake.org](https://cmake.org)).

### Dependencies

 * HIDAPI 0.7 or newer ([http://www.signal11.us/oss/hidapi](http://www.signal11.us/oss/hidapi)).
 * Boost 1.58 or newer ([http://www.boost.org](http://www.boost.org)).
 * Used boost libraries:
   * Program Options.
   * Test.
   * Filesystem.

 * Additinal graphical application dependencies:
   * GTK 3.21 or newer ([http://www.gtk.org/](http://www.gtk.org/)).
   * libudev 231 or newer ([https://www.freedesktop.org/wiki/Software/systemd/](https://www.freedesktop.org/wiki/Software/systemd/)).
   * jsoncpp 1.7 or newer ([https://github.com/open-source-parsers/jsoncpp](https://github.com/open-source-parsers/jsoncpp)).

### Building

```shell
mkdir build
cd build
cmake ../
make
make install
```
