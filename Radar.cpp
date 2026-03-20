#include <Arduino.h>
#include <ESP32Servo.h>

#include "Radar.h"
#include "Config.h"

// Variables internes
Servo radarServo;

const int pulseStep = 62.5; // Vitesse du balayage (plus petit = plus lent/fluide)
int MIN_PULSE = 1000; 
int MAX_PULSE = 2000; 

float* scan;

// Variables volatiles pour l'interruption (ISR)
volatile unsigned long echoStart = 0;
volatile unsigned long echoDuration = 0;
volatile bool distanceReady = false;

// Fonction d'interruption (exécutée en RAM prioritaire)
void IRAM_ATTR echo_ISR() {
    if (digitalRead(PIN_ECHO) == HIGH) {
        echoStart = micros(); 
    } else {
        echoDuration = micros() - echoStart; 
        distanceReady = true; 
    }
}

void setupRadar() {
    pinMode(PIN_TRIG, OUTPUT);
    pinMode(PIN_ECHO, INPUT);

    // Attache l'interruption au pin ECHO
    attachInterrupt(digitalPinToInterrupt(PIN_ECHO), echo_ISR, CHANGE);
    
    ESP32PWM::allocateTimer(0);
    radarServo.setPeriodHertz(50);
    radarServo.attach(PIN_SERVO); 

    // On initialise au "milieu" théorique
    radarServo.writeMicroseconds(MIN_PULSE);

    scan = (float*) malloc(sizeof(float) * 16);
}

// AI for debug
void printScan() {
    // 1. L'affichage brut d'origine (sur une ligne)
    Serial.print("Scan : [");
    for(int i = 0; i < 16; i++) {
        Serial.print(scan[i]);
        if (i < 15) Serial.print(", ");
    }
    Serial.println("]");

    // 2. L'affichage visuel du radar
    Serial.println("\n=== VISUALISATION RADAR 180° ===");
    
    const float MAX_VISUAL_DIST = 100.0; // Distance max pour remplir la barre (en cm)
    const int BAR_LENGTH = 30;           // Nombre de caractères de la barre

    for(int i = 0; i < 16; i++) {
        // Calcul de l'angle pour l'affichage (16 mesures sur 180° = pas de 12°)
        int angle = i * (180 / 15);

        // Alignement du texte pour que ce soit droit
        if (angle < 10) Serial.print("  ");
        else if (angle < 100) Serial.print(" ");
        Serial.print(angle);
        Serial.print("° |");

        float dist = scan[i];
        int filled = 0;

        // Calcul du remplissage
        if (dist < 400.0) { 
            filled = (min(dist, MAX_VISUAL_DIST) / MAX_VISUAL_DIST) * BAR_LENGTH;
        }

        // Dessin de la barre
        for (int j = 0; j < BAR_LENGTH; j++) {
            if (dist >= 400.0) {
                Serial.print("~"); // Symbole pour hors de portée / timeout
            } else if (j < filled) {
                Serial.print("#"); // Zone pleine
            } else {
                Serial.print("-"); // Zone vide
            }
        }

        // Affichage de la valeur exacte au bout de la barre
        Serial.print("| ");
        if (dist > 400.0) {
            Serial.println("HORS PORTÉE");
        } else {
            Serial.print(dist, 1); // 1 décimale
            Serial.println(" cm");
        }
    }
    Serial.println("================================\n");
}

float getDistance() {
    distanceReady = false;

    digitalWrite(PIN_TRIG, LOW);
    delayMicroseconds(3);
    digitalWrite(PIN_TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(PIN_TRIG, LOW);
    
    // Attente non-bloquante avec timeout
    unsigned long timeoutStart = millis();
    while (!distanceReady) {
        if (millis() - timeoutStart > 30) {
            return 400.0; 
        }
        delay(1); // Rend la main pour éviter le plantage du Bluetooth
    }
    
    return (echoDuration * 0.0343) / 2.0;
}

void updateRadarAngle() {
    /* CALIBRAGE POUR SERVO 360° :
       - 500µs  = 0°
       - 1500µs = 180°  (milieu)
       - 2500µs = 360°
       
       Pour faire un balayage de 180° (entre le quart et les trois-quarts du moteur) :
       On va aller de 1000µs (90°) à 2000µs (270°)
    */
    int currentPulse = MIN_PULSE;
    radarServo.writeMicroseconds(currentPulse);
    delay(40);

    for(int i=0; i<16; i++){
        currentPulse += pulseStep;
        radarServo.writeMicroseconds(currentPulse);
        delay(60);
        scan[i] = getDistance();
    }
    radarServo.writeMicroseconds(MIN_PULSE);
    printScan();
}