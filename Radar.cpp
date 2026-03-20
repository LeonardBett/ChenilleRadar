#include <Arduino.h>
#include <ESP32Servo.h>

#include "Radar.h"
#include "Config.h"

// Variables internes
Servo radarServo;
int currentPulse = 1500; // Milieu du signal (90° sur un 180°, 180° sur un 360°)
int pulseSens = 1;
const int pulseStep = 20; // Vitesse du balayage (plus petit = plus lent/fluide)

void setupRadar() {
    pinMode(PIN_TRIG, OUTPUT);
    pinMode(PIN_ECHO, INPUT);
    
    ESP32PWM::allocateTimer(0);
    radarServo.setPeriodHertz(50);
    radarServo.attach(PIN_SERVO); 

    // On initialise au "milieu" théorique
    radarServo.writeMicroseconds(1500);
}

float getDistance() {
    digitalWrite(PIN_TRIG, LOW);
    delayMicroseconds(5);
    digitalWrite(PIN_TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(PIN_TRIG, LOW);
    
    // Mesure du temps de retour de l'onde
    long duration = pulseIn(PIN_ECHO, HIGH, 30000); 
    
    if (duration == 0) return 400.0; // Distance max si pas d'écho
    
    // On calcul et renvoi la distance à partir de la durée
    return (duration * 0.0343) / 2.0;
}

void updateRadarAngle() {
    /* CALIBRAGE POUR SERVO 360° :
       - 500µs  = 0°
       - 1500µs = 180°  (milieu)
       - 2500µs = 360°
       
       Pour faire un balayage de 180° (entre le quart et les trois-quarts du moteur) :
       On va aller de 1000µs (90°) à 2000µs (270°)
    */
    const int MIN_PULSE = 1000; 
    const int MAX_PULSE = 2000; 

    if (currentPulse >= MAX_PULSE) pulseSens = -1;
    else if (currentPulse <= MIN_PULSE) pulseSens = 1;
    
    currentPulse += (pulseStep * pulseSens);
    
    radarServo.writeMicroseconds(currentPulse);
}

// Fonction temporaire d'affichage
void drawTerminalRadar(int p, float d) {
    const int barWidth = 30;
    const float maxDist = 150.0;

    int a = map(p, 1000, 2000, 0, 180);
    
    int filled = (min(d, maxDist) / maxDist) * barWidth;
    
    Serial.print("Angle: ");
    Serial.print(a);
    Serial.print("° |");
    
    for (int i = 0; i < barWidth; i++) {
        if (i < filled) Serial.print("#");
        else Serial.print("-");
    }
    
    Serial.print("| ");
    Serial.print(d, 1);
    Serial.println(" cm");
}