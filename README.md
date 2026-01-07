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

## Setting up a Raspberry Pi 4

The robot will connect to a wifi hotspot and then to the TCP server. As I am using a Raspberry pi 4, here is my setup.

1. Unblock the wifi by setting up the wifi country `sudo raspi-config`→ Localisation Options → WLAN Country ...

2. Configure wifi AP, ensure IP address and make it persistent
```
sudo nmcli device wifi hotspot ifname wlan0 ssid BallBot password BallBot123
sudo nmcli connection modify Hotspot ipv4.addresses 10.42.0.1/24
sudo nmcli connection modify Hotspot ipv4.method shared
sudo nmcli connection modify Hotspot connection.autoconnect yes
nmcli device status
```
If you need to change the password: 
```
sudo nmcli connection modify Hotspot wifi-sec.psk 'New_strong_password'
```

3. Clone this repository and install the servers
```
cd ~
git clone https://github.com/ujagaga/BallBot.git
cd BallBot/TcpServer
./install.sh
```

4. Open your web browser and navigate to the servers IP address at port 9000.

## Status
- The schematic and the 3D model are finished.
- The ESP32 CAM code is done and working
- The TCP and Flask servers are ready and working
- TODO: solder everything, put it together, test it and start developping the automation code.

![3D Preview](Screenshot.png)