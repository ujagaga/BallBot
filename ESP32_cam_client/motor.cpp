#include <ESP32Servo.h>
#include "motor.h"

// --- Servo objects ---
Servo servoMotor[3];

// --- Motor state ---
static uint32_t cmdTime = 0;
static int wheelSpeed = 0;
static uint8_t cmdTimeout = 255;

// --- Servo angles and increments ---
static uint8_t servoCurrentAngle[3] = {90, 90, 90};
static uint8_t servoTargetAngle[3]  = {90, 90, 90};
static const int servoIncrement[3]   = {SERVO_1_SPEED, SERVO_2_SPEED, SERVO_3_SPEED};

// --- Servo update interval ---
static uint32_t lastServoUpdate = 0;
const uint16_t SERVO_UPDATE_MS = 20; // update every 20ms

// --- Timeout setter ---
void MOTOR_setCmdTimeout(uint8_t timeout){
    cmdTimeout = timeout;
}

// --- Initialization ---
void MOTOR_init() {
    servoMotor[0].attach(SERVO_1_PIN);  
    servoMotor[1].attach(SERVO_2_PIN);  
    servoMotor[2].attach(SERVO_3_PIN);  

    pinMode(BLDC_SPEED_PIN, OUTPUT);
    pinMode(BLDC_DIR_PIN, OUTPUT);
    pinMode(BLDC_BRAKE_PIN, OUTPUT);
    digitalWrite(BLDC_SPEED_PIN, LOW);
    digitalWrite(BLDC_DIR_PIN, LOW);
    digitalWrite(BLDC_BRAKE_PIN, LOW);
}

// --- Set servo target ---
void MOTOR_setServo(int id, int angle) {
    if (id < 0 || id > 2) return;
    servoTargetAngle[id] = angle;
}

// --- Stop BLDC ---
void MOTOR_stop(){
    analogWrite(BLDC_SPEED_PIN, 0);
    digitalWrite(BLDC_BRAKE_PIN, HIGH);
    cmdTime = millis();
}

// --- Move BLDC ---
void MOTOR_move(int speed){  
    bool forward = speed > 0;  
    digitalWrite(BLDC_DIR_PIN, (int)forward); 

    if(speed < 0) speed = -speed;
    if(speed > 1023) speed = 1023;

    analogWrite(BLDC_SPEED_PIN, speed);
    cmdTime = millis();
}

// --- Process motor & servos ---
void MOTOR_process(){
    uint32_t now = millis();

    // --- BLDC motor timeout ---
    if ((now - cmdTime) > cmdTimeout) {
        analogWrite(BLDC_SPEED_PIN, 0);
        digitalWrite(BLDC_BRAKE_PIN, LOW);
    }

    // --- Servo smooth update every SERVO_UPDATE_MS ---
    if (now - lastServoUpdate >= SERVO_UPDATE_MS) {
        lastServoUpdate = now;
        for (int i = 0; i < 3; ++i) {
            int delta = servoTargetAngle[i] - servoCurrentAngle[i];
            if (delta != 0) {
                int step = min(abs(delta), servoIncrement[i]);
                if (delta > 0) servoCurrentAngle[i] += step;
                else servoCurrentAngle[i] -= step;
                servoMotor[i].write(servoCurrentAngle[i]);
            }
        }
    }
}
