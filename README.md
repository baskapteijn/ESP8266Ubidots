# ESP8266 Ubidots - ESP8266 sketches using the Ubidots IOT library for Arduino


## Sketches

Arduino IDE | Examples | DHT22 Temperature & Humidity:

* [DHT22](https://github.com/baskapteijn/ESP8266Ubidots/tree/master/sketches/DHT22/DHT22.ino)


## Hardware

**Pull-up resistor DAT pin**

* Connect an external ```3k3..10k``` pull-up resistor between the ```DAT``` and ```VCC``` pins only when:
  * Using a AM2302 sensor without a DT22 breakout PCB **and** the MCU IO pin has no built-in or external pull-up resistor.
* The DHT22 breakout PCB contains a ```3k3``` pull-up resistor between ```DAT``` and ```VCC```.
* Please refer to the MCU datasheet or board schematic for more information about IO pin pull-up resistors.

**External capacitor**

* Tip: Connect a ```100nF``` capacitor between the sensor pins ```VCC``` and ```GND``` when read errors occurs. This may stabilize the power supply.

**Connection DHT22 - ESP8266**

Some ESP8266 boards uses Arduino pin 2 -> GPIO4 which is D4 text on the board. Make sure you're using the right pin.

| DHT22 | ESP8266 / WeMos D1 R2 / ESP12E / NodeMCU |
| :---: | ---------------------------------------- |
|  GND  | GND                                      |
|  VCC  | 3.3V                                     |
|  DAT  | D2                                       |

Other MCU's may work, but are not tested.


## Documentation

* [DHT22 datasheet](https://www.google.com/search?q=DHT22+datasheet)


## Library dependencies

* [Esp](https://github.com/esp8266/Arduino.git)
* [ubidots-esp8266](https://github.com/ubidots/ubidots-esp8266)
* [ErriezDHT22](https://github.com/Erriez/ErriezDHT22.git)
* [ErriezTimestamp](https://github.com/Erriez/ErriezTimestamp.git) 

## Special thanks to

* [Erriez](https://github.com/Erriez)

