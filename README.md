# BallBot

An arduino robot to collect PingPong balls. It contains: 
1. ESP32 Cam module to provide optical info and WiFi controllabillity
2. 3 servo motors (9g servo) for ball claws, head (ESP32 cam) and drive wheel direction rotation
3. Brushless DC motor as the main drive wheel (Gimbal Brushless Motor for Gopro 808) 
4. Brushless motor driver
5. Li-Ion battery 3.7V (2 x 18650 in parallel)
6. 2 nickel strips on the bottom of the battery pack as charging rails
7. Li-Ion charger module with battery protection and 5V boost module to power the ESP32 Cam module
8. DC/DC boost module to raise the battery voltage to 6V for the BLDC motor driver and to power the servo motors

The casing is 3D printed using cheapest PLA.
The screws are 1mm x 10mm. If you are building your own please edit the 3D model to support what ever screws you are using.
The screw holles are made smaller and screws are heated using a soldering iron before they were screwd in, to melt the plastic and create threads.

## Server

1. Raspberry Pi 4 is configured as a WiFi access point
2. RPI4 runs a Python TCP server which:
	- receives the video stream from the ESP32 CAM module
	- receives debugging messages
	- receives ultrasound sensor distance measurements
	- sends commands to control the motors
	- provides firmware updates for ESP32 module
3. RPI4 runs a Flask app to enable manual control, firmware upload, firmware update triggering and access to an API for external control
4. A secondary python app will be created to automate the robot operation 

## Status

Just starting the project, so it is not yet usable. For now, I believe the schematic and the 3D model are finished.

![3D Preview](Screenshot.png)