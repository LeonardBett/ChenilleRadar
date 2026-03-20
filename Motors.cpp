#include <Arduino.h>
#include <ESP32Servo.h>

#include "Motors.h"
#include "Config.h"

Servo motorLeft;
Servo motorRight;

const int DEADZONE = 50; 

void setupMotors() {
    ESP32PWM::allocateTimer(1);
    motorLeft.setPeriodHertz(50);
    motorRight.setPeriodHertz(50);

    motorLeft.attach(PIN_MOT_G, 500, 2400);
    motorRight.attach(PIN_MOT_D, 500, 2400);

    stopMotors();
}

void stopLeft() {
    motorLeft.write(90); // 90 = Arrêt pour un servo 360°
}

void stopRight() {
    motorRight.write(90);
}

void stopMotors() {
    stopLeft();
    stopRight();
}

void controlLeft(int throttleL) {
    if (abs(throttleL) > DEADZONE) {
        // On utilise writeMicroseconds pour aller aux limites physiques (500 à 2500µs)
        // 500 = Vitesse max sens A | 1500 = Arrêt | 2500 = Vitesse max sens B
        int pulse = map(throttleL, -511, 511, 2500, 500);
        motorLeft.writeMicroseconds(pulse);
    } else {
        stopLeft();
    }
}

void controlRight(int throttleR) {
    if (abs(throttleR) > DEADZONE) {
        int pulse = map(throttleR, -511, 511, 2500, 500); 
        motorRight.writeMicroseconds(pulse);
    } else {
        stopRight();
    }
}