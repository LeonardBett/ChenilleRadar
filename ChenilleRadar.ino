#include <Arduino.h>

#include "Radar.h"
#include "Config.h"
#include "Controller.h"
#include "Motors.h"

void setup() {
    Serial.begin(115200);  // Serial où seront affiché les print
    delay(100); // PAUSE de 2 sec pour que la batterie se stabilise
    
    Serial.println("Initialisation Moteurs...");
    setupMotors();
    delay(100);

    Serial.println("Initialisation Radar...");
    setupRadar();
    delay(100);

    Serial.println("Lancement Bluetooth...");
    setupController(); // On lance le Bluetooth EN DERNIER
}

void loop() {
  updateController(); // Met à jour l'état de la manette

  if (isControllerConnected()) {
    controlLeft(gamepad.ly);  
    controlRight(gamepad.ry);

    if(isJustPressed(BUTTON_A)){
      updateRadarAngle();
    }
  } 
  else {
    stopMotors(); // Arrêt de sécurité en cas de déconnexion
  }



  delay(50); // Délai minimal pour la stabilité du processeur
}