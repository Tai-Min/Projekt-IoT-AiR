# Projekt-IoT-AiR
Project for IoT class.

### Features
* Easy to connect to desired WiFi via [ESPTouch](https://www.espressif.com/en/products/software/esp-touch/overview),
* configurable MQTT with persistent config memory (broker ip, port, username, password, namespace),
* HTTP server for MQTT configuration (toggleable by physical pin for security reasons),
* Humidity / pressure / temperature measurements.

### Connect to WiFi
* Press Smart Config button and then reset the device via EN button,
* release the button when Smart Config diode is set,
* connect the device to WiFi via i.e [Esptouch app](https://play.google.com/store/apps/details?id=com.khoazero123.iot_esptouch_demo&hl=pl&gl=US),
* wait untill Smart Config diode is cleared and WiFi diode is set*. </br>
\* If this does not happen in like 20 seconds then try again.

### Connect to MQTT
* Click HTTP button and wait for blue diode to set,
* in web browser type \<device-ip>/mqtt,
* enter all MQTT credentials and apply,
* click HTTP button again to close HTTP server,
* wait untill HTTP diode is cleared.


### Measurements
Measurements are available in following topics:
* Pressure: \<namespace>/pressure
* Temperature: \<namespace>/temperature
* Humidity: \<namespace>/humidity

### Schematic
![Project's schematics](https://github.com/Tai-Min/Projekt-IoT-AiR/blob/master/media/sch.png "Project's schematics")
