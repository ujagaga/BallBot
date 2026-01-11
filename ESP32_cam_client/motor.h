#pragma once

#define SERVO_STEER_PIN           (13)
#define SERVO_CLAW_PIN            (14)
#define SERVO_ARM_PIN             (15)
#define BLDC_SPEED_PIN            (12)
#define BLDC_DIR_PIN              (2)
#define BLDC_TAHO_PIN             (4)

#define SERVO_STEER_SPEED         (1)
#define SERVO_CLAW_SPEED          (1)
#define SERVO_ARM_SPEED           (1)

#define SERVO_STEER_MIN           (15)
#define SERVO_STEER_MAX           (165)
#define SERVO_CLAW_MIN            (70)
#define SERVO_CLAW_MAX            (100)
#define SERVO_ARM_MIN             (10)
#define SERVO_ARM_MAX             (170)

#define STEERING_STRAIGHT_ANGLE   (90)
#define STEERING_PER_PULSE        (10)

extern void MOTOR_init(void);
extern void MOTOR_setSteerServo(int angle);
extern void MOTOR_setArmServo(int angle);
extern void MOTOR_setClawServo(int angle);
extern void MOTOR_incrementSteerServo(int angle);
extern void MOTOR_incrementArmServo(int angle);
extern void MOTOR_incrementClawServo(int angle);
extern void MOTOR_process(void);
extern void MOTOR_initTahoMeter(void);
extern void MOTOR_moveToDistance(uint32_t pulses, int speed, bool keepDirection);