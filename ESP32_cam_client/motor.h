#pragma once

#define SERVO_1_PIN               (13)
#define SERVO_2_PIN               (14)
#define SERVO_3_PIN               (15)
#define BLDC_SPEED_PIN            (12)
#define BLDC_DIR_PIN              (2)
#define BLDC_TAHO_PIN             (0)

#define SERVO_1_SPEED             (1)
#define SERVO_2_SPEED             (1)
#define SERVO_3_SPEED             (1)

#define STEARING_SERVO            (1)
#define STEARING_STRAIGHT_ANGLE   (90)
#define STEARING_PER_PULSE        (10)

#define MOTOR_CMD_TIMEOUT (1000)

extern void MOTOR_init(void);
extern void MOTOR_setServo(int id, int angle);
extern void MOTOR_process(void);
extern void MOTOR_moveToDistance(uint32_t pulses, int speed, bool keepDirection);