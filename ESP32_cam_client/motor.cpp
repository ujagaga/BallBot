#include <ESP32Servo.h>
#include <Arduino.h>
#include "motor.h"

// --- Motor state ---
static uint32_t lastServoUpdate = 0;
const uint16_t SERVO_UPDATE_MS = 20;
volatile uint32_t wheel_pulses = 0;

// --- Servo angles and increments ---
struct ServoMotor {
    uint8_t currentAngle;
    uint8_t targetAngle;
    uint8_t increment;
    Servo motor;
};
ServoMotor mSteer;
ServoMotor mClaw;
ServoMotor mArm;

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

static inline void servoMotorInit(
    ServoMotor &m,
    uint8_t startAngle,
    uint8_t speed,
    uint8_t pin
) {
    m.currentAngle = startAngle;
    m.targetAngle  = startAngle;
    m.increment    = speed;

    m.motor.attach(pin);
    m.motor.write(startAngle);
}


void IRAM_ATTR wheel_isr() {
    wheel_pulses++;
}

void MOTOR_initTahoMeter(){
    pinMode(BLDC_TAHO_PIN, INPUT_PULLUP); 
    // attachInterrupt(digitalPinToInterrupt(BLDC_TAHO_PIN), wheel_isr, FALLING);
}

// --- Initialization ---
void MOTOR_init() {
    servoMotorInit(mSteer, 90, SERVO_STEER_SPEED, SERVO_STEER_PIN);
    servoMotorInit(mClaw,  90, SERVO_CLAW_SPEED, SERVO_CLAW_PIN);
    servoMotorInit(mArm,   90, SERVO_ARM_SPEED, SERVO_ARM_PIN);     
    pinMode(BLDC_TAHO_PIN, INPUT_PULLUP); 
    pinMode(BLDC_SPEED_PIN, OUTPUT);
    pinMode(BLDC_DIR_PIN, OUTPUT);
    digitalWrite(BLDC_SPEED_PIN, LOW);
    digitalWrite(BLDC_DIR_PIN, LOW);
}

// --- Set servo target ---
static inline uint8_t clampSteerAngle(int a) {
    if (a < SERVO_STEER_MIN) return SERVO_STEER_MIN;
    if (a > SERVO_STEER_MAX) return SERVO_STEER_MAX;
    return (uint8_t)a;
}

static inline uint8_t clampArmAngle(int a) {
    if (a < SERVO_ARM_MIN) return SERVO_ARM_MIN;
    if (a > SERVO_ARM_MAX) return SERVO_ARM_MAX;
    return (uint8_t)a;
}

static inline uint8_t clampClawAngle(int a) {
    if (a < SERVO_CLAW_MIN) return SERVO_CLAW_MIN;
    if (a > SERVO_CLAW_MAX) return SERVO_CLAW_MAX;
    return (uint8_t)a;
}

void MOTOR_setSteerServo(int angle) {    
    mSteer.targetAngle = clampSteerAngle(angle);
}

void MOTOR_incrementSteerServo(int angle) {    
    mSteer.targetAngle += angle;
    mSteer.targetAngle = clampSteerAngle(mSteer.targetAngle);
    pinMode(BLDC_TAHO_PIN, INPUT_PULLDOWN); 
    delay(100);
    pinMode(BLDC_TAHO_PIN, INPUT_PULLUP); 
  
}

void MOTOR_setArmServo(int angle) {    
    mArm.targetAngle = clampArmAngle(angle);
}

void MOTOR_incrementArmServo(int angle) {    
    mArm.targetAngle += angle;
    mArm.targetAngle = clampArmAngle(mArm.targetAngle);
    pinMode(BLDC_TAHO_PIN, INPUT_PULLDOWN); 
    delay(100);
    pinMode(BLDC_TAHO_PIN, INPUT_PULLUP); 
}

void MOTOR_setClawServo(int angle) {    
    mClaw.targetAngle = clampClawAngle(angle);
}

void MOTOR_incrementClawServo(int angle) {    
    mClaw.targetAngle += angle;
    mClaw.targetAngle = clampClawAngle(mClaw.targetAngle);
    pinMode(BLDC_TAHO_PIN, INPUT_PULLDOWN); 
    delay(100);
    pinMode(BLDC_TAHO_PIN, INPUT_PULLUP); 
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
    currentMove.lastProcessedPuls = 0;
    currentMove.speed = speed;
    currentMove.active = true;
    currentMove.keepDir = keepDirection;
    moveBldc(speed);
}

static void processServo(ServoMotor &m, int minAngle, int maxAngle){
    int delta = m.targetAngle - m.currentAngle;
    if (delta != 0) {
        int step = min(abs(delta), (int)m.increment);
        if (delta > 0) m.currentAngle += step;
        else m.currentAngle -= step;

        if (m.currentAngle < minAngle) m.currentAngle = minAngle;
        if (m.currentAngle > maxAngle) m.currentAngle = maxAngle;

        m.motor.write(m.currentAngle);
    }
}

// --- Process BLDC motor & servos ---
void MOTOR_process(){   
    return;

    uint32_t now = millis();

    if (now - lastServoUpdate >= SERVO_UPDATE_MS) {
        lastServoUpdate = now;        

        if(currentMove.active) {
            uint32_t pulses = get_pulses(); 
            if(currentMove.keepDir && (currentMove.lastProcessedPuls != pulses)){
                // New pulse received. Set target for steering.
                currentMove.lastProcessedPuls = pulses;
                int targetDelta = STEERING_STRAIGHT_ANGLE - mSteer.currentAngle;                
                if (targetDelta != 0) {
                    int targetStep = min(abs(targetDelta), STEERING_PER_PULSE);
                    mSteer.targetAngle = mSteer.currentAngle + (targetStep > 0 ? targetStep : -targetStep);                   
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
        processServo(mSteer, SERVO_STEER_MIN, SERVO_STEER_MAX);          
        processServo(mClaw, SERVO_CLAW_MIN, SERVO_CLAW_MAX);
        processServo(mArm, SERVO_ARM_MIN, SERVO_ARM_MAX);  
    }    
}

