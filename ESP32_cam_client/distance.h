#ifndef MOTOR_H
#define MOTOR_H

#define HC_TRIG_PIN   1     // GPIO1 (TX)
#define HC_ECHO_PIN   13    // GPIO3 (RX)

extern void DISTANCE_init(void);
extern void float DISTANCE_get(void);

#endif