#pragma once

#define HC_TRIG_PIN   1     // GPIO1 (TX)
#define HC_ECHO_PIN   3    // GPIO3 (RX)

extern void DISTANCE_init(void);
extern float DISTANCE_get(void);

