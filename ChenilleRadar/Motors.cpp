#include <Arduino.h>
#include <ESP32Servo.h>

#include "Motors.h"
#include "Config.h"

// Servomoteurs utilisées pour le déplacement
Servo motorLeft;
Servo motorRight;

// Pour gérer la zone morte des manettes
const int DEADZONE = 50; 

// Initialise les servomoteurs utilisées pour le déplacement
void setupMotors() {
    // On créer la réference utilisée pour l'utilisation des servos
    ESP32PWM::allocateTimer(1);
    motorLeft.setPeriodHertz(50);
    motorRight.setPeriodHertz(50);

    // On lie les pins de l'esp32 aux variables servos
    motorLeft.attach(PIN_MOT_G, 500, 2400);
    motorRight.attach(PIN_MOT_D, 500, 2400);

    // On arrete les servos par sécurités
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