# mcp2200ctl - MCP2200 USB-to-UART serial converter control program.

[![Build Status](https://dev.azure.com/thezbyg/mcp2200ctl/_apis/build/status/thezbyg.mcp2200ctl?branchName=master)](https://dev.azure.com/thezbyg/mcp2200ctl/_build/latest?definitionId=1&branchName=master)

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

![mcp2200gui screenshot](/share/screenshot.png?raw=true "mcp2200gui screenshot")

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

Some of C++17 features are required. Compilation is currently only tested on the following compilers:

 * gcc 9.3
 * gcc 10.2
 * clang 10.0
 * clang 11.0

### Build dependencies

CMake 3.13 or newer ([https://cmake.org](https://cmake.org)).

### Dependencies

 * HIDAPI 0.7 or newer ([http://www.signal11.us/oss/hidapi](http://www.signal11.us/oss/hidapi)).
 * Boost 1.67 or newer ([http://www.boost.org](http://www.boost.org)).
 * Used boost libraries:
   * Program Options.
   * Test.
   * Filesystem.

 * Additional graphical application dependencies:
   * GTK 3.21 or newer ([http://www.gtk.org/](http://www.gtk.org/)).
   * libudev 231 or newer ([https://www.freedesktop.org/wiki/Software/systemd/](https://www.freedesktop.org/wiki/Software/systemd/)).
   * jsoncpp 1.9.4 or newer ([https://github.com/open-source-parsers/jsoncpp](https://github.com/open-source-parsers/jsoncpp)).

On Debian, and on Debian based Linux distributions, the following command can be used to get all of the build dependencies:

```shell
apt-get install cmake pkg-config libhidapi-dev libgtk-3-dev libudev-dev libjsoncpp-dev libboost-filesystem-dev libboost-program-options-dev libboost-test-dev
```

### Building

```shell
mkdir build
cd build
cmake ../
make
make install
```
