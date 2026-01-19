#include <Arduino.h>
#include "distance.h"

#ifdef USE_ULTRASOUND

void DISTANCE_init()
{
    pinMode(HC_TRIG_PIN, OUTPUT);
    pinMode(HC_ECHO_PIN, INPUT);

    digitalWrite(HC_TRIG_PIN, LOW);
}

/*
 * Measure distance in millimeters
 * Returns:
 *   > 0 : valid distance in mm
 *   -1  : timeout / no echo
 */
int32_t DISTANCE_get()
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
        return -1;
    }

    // Speed of sound ≈ 343 m/s
    // distance = (duration / 2) * 0.343 mm/µs
    int32_t distance_cm = (int32_t)((duration * 0.343f) / 2.0f);

    return distance_cm;
}

#else

#include <Wire.h>
#include <VL53L1X.h>

VL53L1X sensor;

static bool initialized = false;

void DISTANCE_init()
{
    if (!Wire.begin(I2C_SDA, I2C_SCL)) {
        return;
    }

    sensor.setTimeout(500);
    if (!sensor.init()) {
        return;
    }

    // Configure for maximum range (4 meters)
    sensor.setDistanceMode(VL53L1X::Long);
    // 50ms timing budget for a good balance of speed and accuracy
    sensor.setMeasurementTimingBudget(50000);
    
    initialized = true;
}

int32_t DISTANCE_get()
{
    if(initialized){
        sensor.readSingle();
        return ((int32_t)sensor.read());
    }
    return -1;
}

#endif
