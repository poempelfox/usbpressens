# usbpressens
an USB connected barometric pressure sensor with an alphanumeric LED display

This was a very quick project, the following ready parts were stuck together:

* Microcontroller: Adafruit Feather 328P (3.3V 8MHz)
* Barometric Pressure sensor: Adafruit MPL3115A2 - I2C Barometric Pressure/Altitude/Temperature Sensor
* Adafruit 0.54" Quad Alphanumeric FeatherWing - Yellow/Green

This repository contains the material, mostly the firmware, for this project.

The barometric sensor is queried for the pressure once per second. The result is displayed (in hPa) on the alphanumeric display (4 digits). The result (with full resolution delivered by the sensor) is also sent through the USB-to-serial chip the microcontroller board has at 19200 baud, and can thus be logged on the PC it's connected to.
