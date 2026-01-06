#include "config.h"
/* HC-SR04 ultrasound module distance mesuring functions */

void DISTANCE_init()
{
    pinMode(HC_TRIG_PIN, OUTPUT);
    pinMode(HC_ECHO_PIN, INPUT);

    digitalWrite(HC_TRIG_PIN, LOW);
}

/*
 * Measure distance in centimeters
 * Returns:
 *   > 0 : valid distance in cm
 *   -1  : timeout / no echo
 */
float DISTANCE_get()
{
    // Ensure clean trigger
    digitalWrite(HC_TRIG_PIN, LOW);
    delayMicroseconds(2);

    // 10 µs trigger pulse
    digitalWrite(HC_TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(HC_TRIG_PIN, LOW);

    // Wait for echo (timeout ~25 ms => ~4 m)
    unsigned long duration = pulseIn(
        HC_ECHO_PIN,
        HIGH,
        25000UL
    );

    if (duration == 0) {
        // No echo received
        return -1.0f;
    }

    // Speed of sound ≈ 343 m/s
    // distance = (duration / 2) * 0.0343 cm/µs
    float distance_cm = (duration * 0.0343f) / 2.0f;

    return distance_cm;
}
