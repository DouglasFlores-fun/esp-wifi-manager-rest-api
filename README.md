# ESP WIFI MANAGER REST API.

ESP WIFI MANAFER REST API is another library for wifi manager to esp (working with only esp8266 at the moment this file was edited).
This library enable rest api server on the esp to setup the wifi module, this allow that the electronic can continue with the hardware and the software developer can create an mobile app indepentely.

## Features

- Wifi SSID setup
- Wifi password setup
- Wifi credential stored on EEPROM.
- Check if it has wifi credentials stored.

## REST API PATH

| PATH | METHOD | PARAMETERS | DESCRIPTION |
| ------ | ------ | ------ | ------ |
| /scanWifi | GET | none  | Start scaning wifi networks available. |
| /scanStatus | GET | none | Check if the scan has ended, or it is still working |
| /networks | GET | none | Get the scanned networks |
| /updateWifi | POST |  | Send wifi credentials to use |
| /end | GET | none | End server mode |

## PROCEDURES AN FUNCTIONS

| FUNCTION | TYPE | PARAMETERS | RESULT | DESCRIPTION |
| ------ | ------ | ------ | ------ | ------ |
| begin(cb, serial) | bool | **cb**: callback function (bool *notExit, bool *startServer) <br /> **serial**: Serial Stream | Same result as startWifiClient()  |  Start wifiManager and define serial stream for debug |
| startServer() | void | none | none | Stop wifi client and start server mode |
| stopServer() | void | none | none | Stop server mode and starts client mode |
| startWifiClient() | bool | none | **true**: It has wifi credentials and has connected to ap. <br > **false**: It can't connect to AP because it does not have credentials stored on EEPROM | Start wifi client and stop server mode |
| checkWifi() | bool | none | **true**: exit from callback indicating to something wants to start server <br /> **false**: exit from callback and do nothing | Check if wifi is connected to AP | 
| stopWifi() | void | none | none | Stop wifi client  |
| loop() | void | none | none  | It is use to control the system process, this function should be called whithin the loop procedure, and avoid delay as much as possible |
| checkMode() | uint8_t | none | - wifiSetupServerMode <br>  - wifiSetupClientMode <br> - wifiSetupNotMode  | Check in which mode the device is working |

## Requirements
This library use third party libraries:
- [ESPAsyncTCP](https://github.com/me-no-dev/ESPAsyncTCP)
- [ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer)
- [ArduinoJson](https://github.com/bblanchon/ArduinoJson)

**Note: You should add theses libraries manually to the arduino IDE**

## Tutorial
You can check a tutorial in one of my blogs, available at: [electronicayprogramacionsv.blogspot.com](electronicayprogramacionsv.blogspot.com)

## License

MIT

