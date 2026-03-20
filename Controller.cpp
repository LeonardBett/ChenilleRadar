#include <Arduino.h>
#include <Bluepad32.h>

#include "Controller.h"

ControllerPtr myControllers[BP32_MAX_GAMEPADS];
int xbox_stick_ly = 0;
int xbox_stick_ry = 0;

// Fonction appelée quand une manette se connecte
void onConnectedController(ControllerPtr ctl) {
    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        if (myControllers[i] == nullptr) {
            myControllers[i] = ctl;
            Serial.println("Manette Xbox connectee !");
            break;
        }
    }
}

// Fonction appelée quand une manette se déconnecte
void onDisconnectedController(ControllerPtr ctl) {
    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        if (myControllers[i] == ctl) {
            myControllers[i] = nullptr;
            Serial.println("Manette Xbox deconnectee...");
            break;
        }
    }
}

void setupController() {
    // Initialise Bluepad32
    BP32.setup(&onConnectedController, &onDisconnectedController);
    // Oublie les anciennes manettes appairées (optionnel mais propre)
    BP32.forgetBluetoothKeys();
}

void updateController() {
    // Obligatoire pour mettre à jour l'état du Bluetooth
    BP32.update();

    xbox_stick_ly = 0;
    xbox_stick_ry = 0;

    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        ControllerPtr ctl = myControllers[i];

        if (ctl && ctl->isConnected()) {
            // Lecture des sticks (valeurs entre -511 et 512)
            xbox_stick_ly = ctl->axisY();
            xbox_stick_ry = ctl->axisRY();
        }
    }
}

bool isControllerConnected(){
    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        if (myControllers[i] != nullptr) {
            return true;
        }
    }
    return false;
}