#pragma once

#define SERVO_1_PIN     (13)
#define SERVO_2_PIN     (14)
#define SERVO_3_PIN     (15)
#define BLDC_SPEED_PIN  (12)
#define BLDC_DIR_PIN    (2)
#define BLDC_BRAKE_PIN  (4)

#define MOTOR_CMD_TIMEOUT (1000)

extern void MOTOR_init(void);
extern void MOTOR_setServo(int id, int angle);
extern void MOTOR_setDirection(bool forward);
extern void MOTOR_stop(void);
extern void MOTOR_move(int speed);
extern void MOTOR_process(void);
