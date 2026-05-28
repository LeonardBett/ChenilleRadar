#include <Arduino.h>

#include "Radar.h"
#include "Config.h"
#include "Controller.h"
#include "Motors.h"
#include "Autonome.h"

void setup() {
    Serial.begin(115200);  // Serial où seront affiché les print
    delay(100); // PAUSE de 2 sec pour que la batterie se stabilise
    
    setupMotors();
    delay(100);

    setupRadar();
    delay(100);

    setupController(); // On lance le Bluetooth EN DERNIER
}

void loop() {
  updateController(); // Met à jour l'état de la manette

  if (isControllerConnected()) {

    // Spam de donnée fausse pour etre detecté par le data forwader d'edge impulse 
    if (isHeld(BUTTON_X)) {
        // On envoie en boucle tant que le bouton est maintenu
        Serial.println("10.0,15.0,20.0,25.0,30.0,35.0,40.0,45.0,50.0,55.0,60.0,65.0,70.0,75.0,80.0,85.0");
        delay(100); // 10Hz, demandé pour le data forwarder
        return;
    }

    // On switch du mode auto vers manuel (ou l'inverse)
    if(isJustPressed(BUTTON_B)){
      if(!isAutonomous) {
        startAutonomousMode();
      } else {
        stopAutonomousMode();
      }
    }

    if(isAutonomous) {
      updateAutonomy(); // On met a jour l'état du mode autonome
    } 
    else {
      // On controle le robot avec les input joystick
      controlLeft(gamepad.ly);  
      controlRight(gamepad.ry);

      if(isJustPressed(BUTTON_A)){
        updateRadarAngle(); // On lance le scan du radar
        printScanForEI(); // On ecrit sur le serial le tableau de distance pour l'entrainement edge inpulse
      }
      if(isJustPressed(BUTTON_Y)){
        updateRadarAngle();
        String decision = getAIPrediction();
        Serial.print("IA Decision: "); 
        Serial.println(decision);
      }
    }
  } else {
    stopMotors(); // Arrêt de sécurité en cas de déconnexion
  }
  delay(50); // Délai minimal pour la stabilité du processeur
}