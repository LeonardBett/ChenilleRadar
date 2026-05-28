#include <Arduino.h>
#include <ESP32Servo.h>

#include "Radar.h"
#include "Config.h"

// Le servomoteur a angle du radar
Servo radarServo;

const int pulseStep = -90; // Decide le nombre de mesure du baleyage
int MIN_PULSE = 1500; // Angle gauche max du radar
int MAX_PULSE = 500; // Angle droit max du radar

// Tableau des distances capturées par le dernier scan
float* scanL;
float* scanR;

// Variables volatiles pour l'interruption (ISR)
volatile unsigned long echoStartL = 0;
volatile unsigned long echoDurationL = 0;
volatile bool distanceReadyL = false;

volatile unsigned long echoStartR = 0;
volatile unsigned long echoDurationR = 0;
volatile bool distanceReadyR = false;

// Fonction d'interruption (exécutée en RAM prioritaire)
// Pour empecher les problemes du calcul de distance lié au bluetooth
void IRAM_ATTR echo_ISRL() {
    // On capture le temps quand l'echo part
    if (digitalRead(PIN_ECHO) == HIGH) {
        echoStartL = micros(); 
    } 
    // Puis on calcul la duree quand l'echo revient
    else { 
        echoDurationL = micros() - echoStartL; 
        distanceReadyL = true; 
    }
}
void IRAM_ATTR echo_ISRR() {
    // On capture le temps quand l'echo part
    if (digitalRead(PIN_ECHO2) == HIGH) {
        echoStartR = micros(); 
    } 
    // Puis on calcul la duree quand l'echo revient
    else { 
        echoDurationR = micros() - echoStartR; 
        distanceReadyR = true; 
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
    attachInterrupt(digitalPinToInterrupt(PIN_ECHO), echo_ISRL, CHANGE);
    attachInterrupt(digitalPinToInterrupt(PIN_ECHO2), echo_ISRR, CHANGE);
    
    ESP32PWM::allocateTimer(0);
    radarServo.setPeriodHertz(50);
    radarServo.attach(PIN_SERVO); 

    // On initialise le radar a gauche
    radarServo.writeMicroseconds(MIN_PULSE);

    scanL = (float*) malloc(sizeof(float) * 16);
    scanR = (float*) malloc(sizeof(float) * 16);
}

// Utilisé pour afficher le tableau de distance sur le serial, et la capture de données Edge Impulse
void printScanForEI() {
    for(int i = 0; i < 16; i++) {
        Serial.print(scanL[i]);
        Serial.print(",");
        Serial.print(scanR[i]);
        if (i < 15) {
            Serial.print(",");
        }
    }
    Serial.println();
}

// Methode interne qui capture la distance des deux hc-sr04 tournant sur servo
// Utilise une interruption pour éviter le probleme du calcul de distance faussé par le bt qui prends la main sur le CPU
void getDistance(float &distL, float &distR) {
    distanceReadyL = false;

    digitalWrite(PIN_TRIG, LOW);
    delayMicroseconds(3);
    digitalWrite(PIN_TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(PIN_TRIG, LOW);
    
    // Attente non-bloquante avec timeout
    unsigned long timeoutStart = millis();
    while (!distanceReadyL) {
        if (millis() - timeoutStart > 30) {
            distL = 400.0;
            break;
        }
        delay(1); // Rend la main pour éviter le plantage du Bluetooth
    }
    if (distanceReadyL) {
        distL = (echoDurationL*0.0343)/2.0;
    }

    // On attends un peu que le son passe avant de faire le deuxieme capteur
    delay(20);

    distanceReadyR = false;

    digitalWrite(PIN_TRIG2, LOW);
    delayMicroseconds(3);
    digitalWrite(PIN_TRIG2, HIGH);
    delayMicroseconds(10);
    digitalWrite(PIN_TRIG2, LOW);
    
    // Attente non-bloquante avec timeout
    timeoutStart = millis();
    while (!distanceReadyR) {
        if (millis() - timeoutStart > 30) {
            distR = 400.0;
            break;
        }
        delay(1); // Rend la main pour éviter le plantage du Bluetooth
    }
    if (distanceReadyR) {
        distR = (echoDurationR*0.0343)/2.0;
    }
}

// Donne la distance de face sans rotation
float getFixedDistance() {
    distanceReadyR = false;
    
    digitalWrite(PIN_TRIG2, LOW);
    delayMicroseconds(3);
    digitalWrite(PIN_TRIG2, HIGH);
    delayMicroseconds(10);
    digitalWrite(PIN_TRIG2, LOW);
    
    // Attente non-bloquante avec timeout
    unsigned long timeoutStart = millis();
    while(!distanceReadyR) {
        if(millis() - timeoutStart > 30) {
            return 400;
        }
        delay(1);
    }

    return (echoDurationR * 0.0343) / 2.0;
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

        getDistance(scanL[i], scanR[i]);
    }
    radarServo.writeMicroseconds(MIN_PULSE);
}

// Methode full IA temporaire
void printScan() {
    Serial.println("\n=== VISUALISATION RADAR DUAL ===");
    for(int i = 0; i < 16; i++) {
        int angle = i * (180 / 15);
        
        // Alignement
        if (angle < 10) Serial.print("  ");
        else if (angle < 100) Serial.print(" ");
        
        Serial.print(angle);
        Serial.print("° | Gauche: ");
        
        if (scanL[i] >= 400.0) {
            Serial.print("MAX   ");
        } else {
            Serial.print(scanL[i], 1);
            Serial.print(" cm");
        }
        
        Serial.print("  | Droit: ");
        
        if (scanR[i] >= 400.0) {
            Serial.println("MAX");
        } else {
            Serial.print(scanR[i], 1);
            Serial.println(" cm");
        }
    }
    Serial.println("================================\n");
}