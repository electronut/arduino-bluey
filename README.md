# Arduino Core for Nordic Semiconductor nRF5 based boards

Program [Nordic Semiconductor](https://www.nordicsemi.com) nR5 based boards with the [Arduino](https://www.arduino.cc) IDE.

## Supported boards

 * [nRF52 DK](https://www.nordicsemi.com/eng/Products/Bluetooth-Smart-Bluetooth-low-energy/nRF52-DK)

## Installing

### Board Manager

 1. [Download and install the Arduino IDE](https://www.arduino.cc/en/Main/Software)
 2. Start the Arduino IDE
 3. Go into Preferences
 4. Add ```http://sandeepmistry.github.io/arduino-nRF5/package_nRF5_boards_index.json``` as an "Additional Board Manager URL"
 5. Open the Boards Manager from the Tools -> Board menu and install "Nordic Semiconductor nRF5 Boards"
 6. Select your nRF5 board from the Tools -> Board menu

### From git (for core development)

 1. Follow steps from Board Manager section above
 2. ```cd <SKETCHBOOK>/hardware```, where ```<SKETCHBOOK>``` is your Arduino Sketch folder:
  * OS X: ```~/Documents```
  * Linux: ```~/Sketchbook```
  * Windows: ```~/Documents```
 3. Create the following folder: ```hardware/nRF5``` and change directories to it
 4. Clone this repo: ```git clone https://github.com/sandeepmistry/arduino-nRF5.git nRF5```

## Credits

This core is based on the [Arduino SAMD Core](https://github.com/arduino/ArduinoCore-samd) and licensed under the same [GPL License](LICENSE)
