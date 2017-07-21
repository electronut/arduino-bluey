# Arduino Core for Electronut labs bluey

Uploading without programmer requires bootloader programmed on the board. [Repo](https://github.com/ntavish/bluey_serial_dfu_bootloader/)

## Installing

### Board Manager

 1. [Download and install the Arduino IDE](https://www.arduino.cc/en/Main/Software) (At least v1.6.12)
 2. Start the Arduino IDE
 3. Go into Preferences
 4. Add ```https://sandeepmistry.github.io/arduino-nRF5/package_nRF5_boards_index.json``` as an "Additional Board Manager URL"
 5. Open the Boards Manager from the Tools -> Board menu and install "Nordic Semiconductor nRF5 Boards"
 6. Select your nRF5 board from the Tools -> Board menu

__NOTE:__ During installation it takes the Arduino IDE a few minutes to extract the tools after they have been downloaded, please be patient.

#### OS Specific Setup

##### OS X

No additional setup required.

##### Linux

No additional setup required.

#####  Windows

###### Driver Setup for Segger J-Link

 1. Download [Zadig](http://zadig.akeo.ie)
 2. Plugin Segger J-Link or DK board
 3. Start ```Zadig```
 4. Select ```Options -> List All Devices```
 5. Select ```J-Link (Interface 2)``` from the device dropdown
 6. Click ```Replace Driver```

__NOTE__: To roll back to the original driver go to: Device Manager -> Open Device -> Update Driver

### Flashing a SoftDevice

 1. ```cd <SKETCHBOOK>```, where ```<SKETCHBOOK>``` is your Arduino Sketch folder:
  * OS X: ```~/Documents/Arduino```
  * Linux: ```~/Arduino```
  * Windows: ```~/Documents/Arduino```
 2. Create the following directories: ```tools/nRF5FlashSoftDevice/tool/```
 3. Download [nRF5FlashSoftDevice.jar](https://github.com/sandeepmistry/arduino-nRF5/releases/download/tools/nRF5FlashSoftDevice.jar) to ```<SKETCHBOOK>/tools/nRF5FlashSoftDevice/tool/```
 4. Restart the Arduino IDE
 5. Select your nRF board from the Tools -> Board menu
 6. Select a SoftDevice from the Tools -> "SoftDevice: " menu
 7. Select a Programmer (J-Link, ST-Link V2, or CMSIS-DAP) from the Tools -> "Programmer: " menu
 8. Select Tools -> nRF5 Flash SoftDevice
 9. Read license agreement
 10. Click "Accept" to accept license and continue, or "Decline" to decline and abort
 11. If accepted, SoftDevice binary will be flashed to the board

### From git (for core development)

 1. Follow steps from Board Manager section above
 2. ```cd <SKETCHBOOK>```, where ```<SKETCHBOOK>``` is your Arduino Sketch folder:
  * OS X: ```~/Documents/Arduino```
  * Linux: ```~/Arduino```
  * Windows: ```~/Documents/Arduino```
 3. Create a folder named ```hardware```, if it does not exist, and change directories to it
 4. Clone this repo: ```git clone https://github.com/sandeepmistry/arduino-nRF5.git sandeepmistry/nRF5```
 5. Restart the Arduino IDE

## BLE

This Arduino Core does **not** contain any Arduino style API's for BLE functionality. All the relevant Nordic SoftDevice (S110, S130, S132) header files are included build path when a SoftDevice is selected via the `Tools` menu.

### Recommend BLE Libraries

 * [BLEPeripheral](https://github.com/sandeepmistry/arduino-BLEPeripheral)
   * v0.3.0 and greater, available via the Arduino IDE's library manager.
   * Supports peripheral mode only.

## Low Frequency Clock Source (LFCLKSRC)

If the selected board has an external 32 kHz crystal connected, it will be used as the source for the low frequency clock. Otherwise the internal 32 kHz RC oscillator will be used. The low frequency clock is used by the `delay(ms)` and `millis()` Arduino API's.

Bluey has an additional menu item under `Tools -> Low Frequency Clock` that allows you to select the low frequency clock source.

## Credits

This is mostly Sandeep Mistry's work, forked from [here](https://github.com/sandeepmistry/arduino-nRF5/).

This core is based on the [Arduino SAMD Core](https://github.com/arduino/ArduinoCore-samd) and licensed under the same [GPL License](LICENSE)

The following tools are used:

 * [GCC ARM Embedded](https://launchpad.net/gcc-arm-embedded) as the compiler
 * A [forked](https://github.com/sandeepmistry/openocd-code-nrf5) version of [OpenOCD](http://openocd.org) to flash sketches
 * [nrfutil](http://infocenter.nordicsemi.com/topic/com.nordic.infocenter.tools/dita/tools/nrfutil/nrfutil_intro.html)
