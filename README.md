# BallBot

An arduino robot to collect PingPong balls. It contains: 
1. ESP32 Cam module to provide optical info and WiFi controllabillity
2. Servo motors (9g servo)for ball claws, head (ESP32 cam) rotation and main drive wheel direction change
3. Stepper brushless DC motor as the main drive wheel (Gimbal Brushless Motor for Gopro 808) 
4. Stepper motor driver
5. Arduino nano module to signal the servo motors and the stepper motor driver based on UART messages from the ESP32 module
6. Li-Ion battery 3.7V (2 x 18650 in parallel)
7. Copper strip, self adhesive, to connect to the charger.
8. Li-Ion charger module with battery protection.

The casing is 3D printed using cheapest PLA.
The screws are 1mm x 10mm and 2.5mm x 16mm. If you are building your own please edit the 3D model to support what ever screws you are using.
The screw holles are made smaller and screws are heated using a soldering iron before they were screwd in, to melt the plastic and create threads.

## Status

Just starting the project, so it is not yet usable.