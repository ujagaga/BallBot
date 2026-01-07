#include <ESP32Servo.h>
#include "motor.h"

Servo servoArms;
Servo servoHands;
Servo servoBody;
uint32_t cmdTime = 0;

void MOTOR_init() {
  servoArms.attach(SERVO_1_PIN);  
  servoHands.attach(SERVO_2_PIN);  
  servoBody.attach(SERVO_3_PIN);  
  pinMode(BLDC_SPEED_PIN, OUTPUT);
  pinMode(BLDC_DIR_PIN, OUTPUT);
  pinMode(BLDC_BRAKE_PIN, OUTPUT);
  digitalWrite(BLDC_SPEED_PIN, LOW);
  digitalWrite(BLDC_DIR_PIN, LOW);
  digitalWrite(BLDC_BRAKE_PIN, LOW);
}

void MOTOR_setServo(int id, int angle) {
  if(id == 0){
    servoArms.write(angle);   
  }else if(id == 1){
    servoHands.write(angle);   
  }else if(id == 2){
    servoBody.write(angle);   
  } 
}

void MOTOR_setDirection(bool forward){
  digitalWrite(BLDC_DIR_PIN, (int)forward);
}

void MOTOR_stop(){
  analogWrite(BLDC_SPEED_PIN, 0);
  digitalWrite(BLDC_BRAKE_PIN, HIGH);
  cmdTime = millis();
}

void MOTOR_move(int speed){
  analogWrite(BLDC_SPEED_PIN, speed);
  cmdTime = millis();
}

void MOTOR_process(){
  if((millis() - cmdTime) > MOTOR_CMD_TIMEOUT){
    analogWrite(BLDC_SPEED_PIN, 0);
    digitalWrite(BLDC_BRAKE_PIN, LOW);
  }
}
