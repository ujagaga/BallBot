#pragma once

#define SERVO_STEER_PIN (15)
#define SERVO_CLAW_PIN (13)
#define SERVO_ARM_PIN (14)
#define BLDC_SPEED_PIN (12)
#define BLDC_DIR_PIN (2)
#define BLDC_TAHO_PIN (4)

#define SERVO_STEER_SPEED (3)
#define SERVO_CLAW_SPEED (4)
#define SERVO_ARM_SPEED (5)

#define SERVO_STEER_MIN (12)
#define SERVO_STEER_MAX (160)
#define SERVO_CLAW_MIN (83)
#define SERVO_CLAW_MAX (105)
#define SERVO_ARM_MIN (45)
#define SERVO_ARM_MAX (140)

#define STEERING_STRAIGHT_ANGLE (85)
#define STEERING_PER_PULSE (20)

#define SERVO_PWM_FREQ (50)
#define SERVO_PWM_RES (16)
#define SERVO_MIN_US (500)
#define SERVO_MAX_US (2500)

// ---- BLDC PWM ----
#define BLDC_PWM_FREQ (20000)
#define BLDC_PWM_RES (10)

#define MOTOR_TIMEOUT (1000)

extern void MOTOR_init(void);
extern void MOTOR_setSteerServo(int angle);
extern void MOTOR_setArmServo(int angle);
extern void MOTOR_setClawServo(int angle);
extern void MOTOR_incrementSteerServo(int angle);
extern void MOTOR_incrementArmServo(int angle);
extern void MOTOR_incrementClawServo(int angle);
extern void MOTOR_stopAll(void);
extern void MOTOR_process(void);
extern void MOTOR_setTimeout(uint32_t timeToSet);
extern void MOTOR_moveToDistance(uint32_t pulses, int speed,
                                 bool keepDirection);
extern void MOTOR_grabBall(void);
