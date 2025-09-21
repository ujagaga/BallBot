#ifndef MOTOR_H
#define MOTOR_H

#define MOTOR_1_PIN   12
#define MOTOR_2_PIN   13
#define MOTOR_3_PIN   14

extern void MOTOR_init(void);
extern void MOTOR_set(int id, int angle);

#endif