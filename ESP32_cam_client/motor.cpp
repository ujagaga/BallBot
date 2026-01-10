#include <ESP32Servo.h>
#include "motor.h"

// --- Servo objects ---
Servo servoMotor[3];

// --- Motor state ---
static int wheelSpeed = 0;

// --- Servo angles and increments ---
static uint8_t servoCurrentAngle[3] = {90, 90, 90};
static uint8_t servoTargetAngle[3]  = {90, 90, 90};
static const int servoIncrement[3]   = {SERVO_1_SPEED, SERVO_2_SPEED, SERVO_3_SPEED};

// --- Servo update interval ---
static uint32_t lastServoUpdate = 0;
const uint16_t SERVO_UPDATE_MS = 20; // update every 20ms

volatile uint32_t wheel_pulses = 0;
struct DistanceMove {
    uint32_t targetPulses;
    uint32_t lastProcessedPuls;
    int speed;
    bool keepDir;
    bool active;
};
DistanceMove currentMove;

static uint32_t get_pulses() {
    noInterrupts();
    uint32_t p = wheel_pulses;
    interrupts();

    return p;
}

void IRAM_ATTR wheel_isr() {
    wheel_pulses++;

}

// --- Initialization ---
void MOTOR_init() {
    servoMotor[0].attach(SERVO_1_PIN);  
    servoMotor[1].attach(SERVO_2_PIN);  
    servoMotor[2].attach(SERVO_3_PIN);  

    pinMode(BLDC_TAHO_PIN, INPUT_PULLUP); 
    attachInterrupt(digitalPinToInterrupt(BLDC_TAHO_PIN), wheel_isr, FALLING);

    pinMode(BLDC_SPEED_PIN, OUTPUT);
    pinMode(BLDC_DIR_PIN, OUTPUT);
    digitalWrite(BLDC_SPEED_PIN, LOW);
    digitalWrite(BLDC_DIR_PIN, LOW);
}

// --- Set servo target ---
void MOTOR_setServo(int id, int angle) {
    if (id < 0 || id > 2) return;
    servoTargetAngle[id] = angle;
}

// --- Move BLDC ---
static void moveBldc(int speed){  
    bool forward = speed > 0;  
    digitalWrite(BLDC_DIR_PIN, (int)forward); 

    if(speed < 0) speed = -speed;
    if(speed > 1023) speed = 1023;

    analogWrite(BLDC_SPEED_PIN, speed);
}

void MOTOR_moveToDistance(uint32_t pulses, int speed, bool keepDirection) {
    noInterrupts();
    wheel_pulses = 0;
    interrupts();
    currentMove.targetPulses = pulses;
    currentMove.speed = speed;
    currentMove.active = true;
    currentMove.keepDir = keepDirection;
    moveBldc(speed);
}

static void processServo(int id){
    int delta = servoTargetAngle[id] - servoCurrentAngle[id];
    if (delta != 0) {
        int step = min(abs(delta), servoIncrement[id]);
        if (delta > 0) servoCurrentAngle[id] += step;
        else servoCurrentAngle[id] -= step;
        servoMotor[id].write(servoCurrentAngle[id]);
    }
}

// --- Process BLDC motor & servos ---
void MOTOR_process(){   
    uint32_t now = millis();

    if (now - lastServoUpdate >= SERVO_UPDATE_MS) {
        lastServoUpdate = now;
        for (int i = 0; i < 3; ++i) { 
            if(i == STEARING_SERVO){                
                if(currentMove.active) {
                    uint32_t pulses = get_pulses(); 
                    if(currentMove.keepDir && (currentMove.lastProcessedPuls != pulses)){
                        // New pulse received. Set target for stearing.
                        currentMove.lastProcessedPuls = pulses;
                        int targetDelta = STEARING_STRAIGHT_ANGLE - servoCurrentAngle[i];
                        if (targetDelta != 0) {
                            int targetStep = min(abs(targetDelta), STEARING_PER_PULSE);
                            if (targetDelta > 0) servoTargetAngle[i] += targetStep;
                            else servoTargetAngle[i] -= targetStep;
                        }
                    }

                    // Set BLDC speed
                    if(pulses >= currentMove.targetPulses) {
                        moveBldc(0); 
                        currentMove.active = false;
                    } else {
                        moveBldc(currentMove.speed); // keep moving
                    }
                }
            }
            processServo(i);          
        }        
    }    
}

