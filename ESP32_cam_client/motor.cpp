#include <ESP32Servo.h>
#include "config.h"

Servo servoArms;
Servo servoHands;
Servo servoBody;


void MOTOR_init() {
  servoArms.attach(MOTOR_1_PIN);  
  servoHands.attach(MOTOR_2_PIN);  
  servoBody.attach(MOTOR_3_PIN);  

}

void MOTOR_set(int id, int angle) {
  if(id == 0){
    servoArms.write(angle);   
  }else if(id == 1){
    servoHands.write(angle);   
  }else if(id == 2){
    servoBody.write(angle);   
  } 
}