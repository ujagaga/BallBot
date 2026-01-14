#include <Arduino.h>
#include "motor.h"

// ======================================================
// ==================== STATE ============================
// ======================================================

static uint32_t lastServoUpdate = 0;
const uint16_t SERVO_UPDATE_MS = 20;

volatile uint32_t wheel_pulses = 0;

// ---------------- Servo ----------------

struct ServoMotor {
    uint8_t currentAngle;
    uint8_t targetAngle;
    uint8_t increment;
    uint8_t pin;
};

static ServoMotor mSteer;
static ServoMotor mClaw;
static ServoMotor mArm;

// ---------------- Distance move ----------------

struct DistanceMove {
    uint32_t targetPulses;
    uint32_t lastProcessedPuls;
    int speed;
    bool keepDir;
    bool active;
};

static DistanceMove currentMove;

// ======================================================
// ==================== UTIL =============================
// ======================================================

static inline uint32_t angleToDuty(uint8_t angle) {
    uint32_t us = map(angle, 0, 180, SERVO_MIN_US, SERVO_MAX_US);
    uint32_t maxDuty = (1UL << SERVO_PWM_RES) - 1;
    return (us * maxDuty) / (1000000UL / SERVO_PWM_FREQ);
}

static inline void servoWrite(const ServoMotor &m) {
    ledcWrite(m.pin, angleToDuty(m.currentAngle));
}

static uint32_t get_pulses() {
    noInterrupts();
    uint32_t p = wheel_pulses;
    interrupts();
    return p;
}

static inline uint8_t clamp(int v, int minv, int maxv) {
    if (v < minv) return minv;
    if (v > maxv) return maxv;
    return (uint8_t)v;
}

// ======================================================
// ==================== ISR ==============================
// ======================================================

void IRAM_ATTR wheel_isr() {
    wheel_pulses++;
}

// ======================================================
// ==================== INIT =============================
// ======================================================

static void servoInit(
    ServoMotor &m,
    uint8_t startAngle,
    uint8_t speed,
    uint8_t pin
) {
    m.currentAngle = startAngle;
    m.targetAngle  = startAngle;
    m.increment    = speed;
    m.pin          = pin;

    // Attach hardware PWM to pin
    ledcAttach(pin, SERVO_PWM_FREQ, SERVO_PWM_RES);
    servoWrite(m);
}

void MOTOR_init() {

    // ---- Servos ----
    servoInit(mSteer, 90, SERVO_STEER_SPEED, SERVO_STEER_PIN);
    servoInit(mClaw,  90, SERVO_CLAW_SPEED,  SERVO_CLAW_PIN);
    servoInit(mArm,   90, SERVO_ARM_SPEED,   SERVO_ARM_PIN);

    // ---- BLDC direction ----
    pinMode(BLDC_DIR_PIN, OUTPUT);
    digitalWrite(BLDC_DIR_PIN, LOW);

    // ---- BLDC PWM ----
    ledcAttach(BLDC_SPEED_PIN, BLDC_PWM_FREQ, BLDC_PWM_RES);
    ledcWrite(BLDC_SPEED_PIN, 0);

    // ---- Tacho ----
    pinMode(BLDC_TAHO_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(BLDC_TAHO_PIN), wheel_isr, FALLING);
}

// ======================================================
// ==================== SERVO API ========================
// ======================================================

void MOTOR_setSteerServo(int a) { mSteer.targetAngle = clamp(a, SERVO_STEER_MIN, SERVO_STEER_MAX); }
void MOTOR_setArmServo(int a)   { mArm.targetAngle   = clamp(a, SERVO_ARM_MIN,   SERVO_ARM_MAX); }
void MOTOR_setClawServo(int a)  { mClaw.targetAngle  = clamp(a, SERVO_CLAW_MIN,  SERVO_CLAW_MAX); }

void MOTOR_incrementSteerServo(int d) { MOTOR_setSteerServo(mSteer.targetAngle + d); }
void MOTOR_incrementArmServo(int d)   { MOTOR_setArmServo(mArm.targetAngle + d); }
void MOTOR_incrementClawServo(int d)  { MOTOR_setClawServo(mClaw.targetAngle + d); }

// ======================================================
// ==================== BLDC =============================
// ======================================================

static void moveBldc(int speed) {

    bool forward = speed >= 0;
    digitalWrite(BLDC_DIR_PIN, forward);

    uint32_t duty = abs(speed);
    if (duty > 1023) duty = 1023;

    ledcWrite(BLDC_SPEED_PIN, duty);
}

void MOTOR_moveToDistance(uint32_t pulses, int speed, bool keepDirection) {

    noInterrupts();
    wheel_pulses = 0;
    interrupts();

    currentMove.targetPulses = pulses;
    currentMove.lastProcessedPuls = 0;
    currentMove.speed = speed;
    currentMove.keepDir = keepDirection;
    currentMove.active = true;

    moveBldc(speed);
}

// ======================================================
// ==================== PROCESS ==========================
// ======================================================

static void processServo(ServoMotor &m, int minA, int maxA) {

    int delta = m.targetAngle - m.currentAngle;
    if (!delta) return;

    int step = min(abs(delta), (int)m.increment);
    m.currentAngle += (delta > 0) ? step : -step;
    m.currentAngle = clamp(m.currentAngle, minA, maxA);

    servoWrite(m);
}

void MOTOR_process() {

    uint32_t now = millis();
    if (now - lastServoUpdate < SERVO_UPDATE_MS) return;
    lastServoUpdate = now;

    if (currentMove.active) {

        uint32_t pulses = get_pulses();

        if (currentMove.keepDir && pulses != currentMove.lastProcessedPuls) {
            currentMove.lastProcessedPuls = pulses;

            int delta = STEERING_STRAIGHT_ANGLE - mSteer.currentAngle;
            if (delta) {
                int step = min(abs(delta), STEERING_PER_PULSE);
                mSteer.targetAngle = mSteer.currentAngle +
                                     ((delta > 0) ? step : -step);
            }
        }

        if (pulses >= currentMove.targetPulses) {
            moveBldc(0);
            currentMove.active = false;
        } else {
            moveBldc(currentMove.speed);
        }
    }

    processServo(mSteer, SERVO_STEER_MIN, SERVO_STEER_MAX);
    processServo(mClaw,  SERVO_CLAW_MIN,  SERVO_CLAW_MAX);
    processServo(mArm,   SERVO_ARM_MIN,   SERVO_ARM_MAX);
}
