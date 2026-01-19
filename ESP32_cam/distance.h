#pragma once

// #define USE_ULTRASOUND

#ifdef USE_ULTRASOUND
/* HC-SR04 ultrasound distance mesuring */
#define HC_TRIG_PIN   (1)     // GPIO1 (TX)
#define HC_ECHO_PIN   (3)     // GPIO3 (RX)

#else

/* TOF-400C Laser distance mesuring module */
#define I2C_SCL       (1)     // GPIO1 (TX)
#define I2C_SDA       (3)     // GPIO3 (RX)
#endif

extern void DISTANCE_init(void);
extern int32_t DISTANCE_get(void);
