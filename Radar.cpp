#include <Arduino.h>
#include <ESP32Servo.h>

#include "Radar.h"
#include "Config.h"

// Le servo a angle du radar
Servo radarServo;

const int pulseStep = 62.5; // Decide le nombre de mesure du baleyage
int MIN_PULSE = 1000; // Angle gauche max du radar
int MAX_PULSE = 2000; // Angle droit max du radar

float* scan;

// Variables volatiles pour l'interruption (ISR)
volatile unsigned long echoStart = 0;
volatile unsigned long echoDuration = 0;
volatile bool distanceReady = false;

// Fonction d'interruption (exécutée en RAM prioritaire)
// Pour empecher les problemes du calcul de distance lié au bluetooth
void IRAM_ATTR echo_ISR() {
    // On capture le temps quand l'echo part
    if (digitalRead(PIN_ECHO) == HIGH) {
        echoStart = micros(); 
    } 
    // Puis on calcul la duree quand l'echo revient
    else { 
        echoDuration = micros() - echoStart; 
        distanceReady = true; 
    }
}

void setupRadar() {
    // On setup le hc-sr04 du radar (qui va tourner)
    pinMode(PIN_TRIG, OUTPUT);
    pinMode(PIN_ECHO, INPUT);

    // Puis celui du radar fixe, qui checkera juste la distance avec un obstacle
    pinMode(PIN_TRIG2, OUTPUT);
    pinMode(PIN_ECHO2, INPUT);

    // Attache l'interruption au pin ECHO
    attachInterrupt(digitalPinToInterrupt(PIN_ECHO), echo_ISR, CHANGE);
    
    ESP32PWM::allocateTimer(0);
    radarServo.setPeriodHertz(50);
    radarServo.attach(PIN_SERVO); 

    // On initialise le radar a gauche
    radarServo.writeMicroseconds(MIN_PULSE);

    scan = (float*) malloc(sizeof(float) * 16);
}

// Utilsié pour affihcer le tableau de distance sur le serial, pour l'entrainement Edge Impulse
void printScanForEI() {
    for(int i = 0; i < 16; i++) {
        Serial.print(scan[i]);
        if (i < 15) Serial.print(",");
    }
    Serial.println();
}

// Methode généré par IA pour afficher les data du module
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

// Methode interne qui capture la distance du hc-sr04 tournant sur servo
// Utilise une interruption pour éviter le probleme du calcul de distance faussé par le bt qui prends la main sur le CPU
float getDistance() {
    distanceReady = false;
    echoDuration = 0;

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

// Donne la distance du hc-sr04 fixe
float getFixedDistance() {
    digitalWrite(PIN_TRIG2, LOW);
    delayMicroseconds(3);
    digitalWrite(PIN_TRIG2, HIGH);
    delayMicroseconds(10);
    digitalWrite(PIN_TRIG2, LOW);
    
    long duration = pulseIn(PIN_ECHO2, HIGH, 30000); 
    if (duration == 0) return 400.0;
    return (duration * 0.0343) / 2.0;
}

// Active le servo du radar vers son prochain angle, et recupere la distance lié
void updateRadarAngle() {
    int currentPulse = MIN_PULSE;
    radarServo.writeMicroseconds(currentPulse);
    delay(40);

    for(int i=0; i<16; i++){
        currentPulse += pulseStep;
        radarServo.writeMicroseconds(currentPulse);
        delay(60);
        delay(10);
        scan[i] = getDistance();
    }
    radarServo.writeMicroseconds(MIN_PULSE);
}