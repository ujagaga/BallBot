# ESP 32 Eye #

I made this for my robot to be able to stream video so I can process it remotelly.

## How to start ##

1. Copy "config.h.example" to "config.h" and adjust your credentials. This is to avoid posting my credentials to git.

2. Open Arduino IDE and install support for ESP32. Go to "Arduino > Preferences" and enter: 

        https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json 

into the “Additional Board Manager URLs” field

3. Go to "Tools > Board > Boards Manager" and search for "esp32" Install "esp32 by Espressif Systems" and "Arduino ESP32 Boards"

4. Go to "Sketch > Include Library > manage Libraries" and install "ArduinoOTA"

5. Connect your ESP32-cam board, select it in the boards manager and program it.

6. Open the Serial Monitor to view debugging messages to see which IP it serves on, so you can find it in your web browser.

7. Use a better 5V power supply than your computer USB port for better video quality.


## Contact ##

* web: http://www.radinaradionica.com
* email: ujagaga@gmail.com

## ESP32 Pinout
Label  	GPIO  	Use			Reason
D0	0	Sens Trig 10K to Vcc	must be HIGH during boot and LOW for flashing
TX0	1	No			Tx pin, used for flashing and debugging
D2	2	Direction 10K to GND	must be LOW during boot
RX0	3	No			Rx pin, used for flashing and debugging
D4	4	Speed			Connected to the on-board Flash LED
D12	12	Servo 1			must be LOW during boot
D13	13	Servo 2
D14	14	Servo 3
D15	15	No			must be HIGH during boot, prevents startup log if pulled LOW
RX2	16	Sens Echo