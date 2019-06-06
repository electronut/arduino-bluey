# Arduino Core for Electronut labs bluey

Uploading without programmer requires bootloader programmed on the board. The repo [electronut/bluey_serial_dfu_bootloader](https://github.com/electronut/bluey_serial_dfu_bootloader/) 
contains the source and precompiled hex file of the bootloader as `hex/s132_nrf52_2.0.0_softdevice.hex`.

The bootloader has the softdevice s132 in it, so no other action is needed to use the BLEPeripheral library.

Bootloader mode is triggered by pressing and holding both prss buttons on bluey and releasing the reset button. The blue LED will start blinkng at an accelerating rate rpeatedly to 
indicate that the bootloader mode is active.

## Installing

### Board Manager

 1. [Download and install the Arduino IDE](https://www.arduino.cc/en/Main/Software) (At least v1.6.12)
 2. Start the Arduino IDE
 3. Go into Preferences
 4. Add ```https://raw.githubusercontent.com/electronut/arduino-bluey/master/docs/package_electronutlabs_boards_index.json``` as an "Additional Board Manager URL"
 5. Open the Boards Manager from the Tools -> Board menu and install "Electronut labs nRF5 Boards"
 6. Select 'Electronut labs bluey' from the Tools -> Board menu

__NOTE:__ During installation it takes the Arduino IDE a few minutes to extract the tools after they have been downloaded, please be patient.

__NOTE2:__ We have seen that on linux, for some reason the nrfutil binary is
marked as not-executable sometimes. If you get an error while compiling, you may
have to manually mark it as executable. Open a terminal and ype in
`chmod +x $HOME/.arduino15/packages/electronutlabs/tools/nrfutil/0.5.2-SFE/dist/nrfutil`.

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
