#include <ESP32Servo.h>
#include "motor.h"

Servo servoArms;
Servo servoHands;
Servo servoBody;
static uint32_t cmdTime = 0;
static int wheelSpeed = 0;
static uint8_t cmdTimeout = 255;


void MOTOR_setCmdTimeout(uint8_t timeout){
  cmdTimeout = timeout;
}

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

void MOTOR_stop(){
  analogWrite(BLDC_SPEED_PIN, 0);
  digitalWrite(BLDC_BRAKE_PIN, HIGH);
  cmdTime = millis();
}

void MOTOR_move(int speed){  
  bool forward = speed > 0;  
  digitalWrite(BLDC_DIR_PIN, (int)forward); 

  if(speed < 0){
    speed = -1 * speed;
  }
  if(speed > 1023){
    speed = 1023;
  }

  analogWrite(BLDC_SPEED_PIN, speed);
  cmdTime = millis();
}

void MOTOR_process(){
  if((millis() - cmdTime) > cmdTimeout){
    analogWrite(BLDC_SPEED_PIN, 0);
    digitalWrite(BLDC_BRAKE_PIN, LOW);
  }
}
