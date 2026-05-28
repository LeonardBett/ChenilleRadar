#include <Arduino.h>
#include <Bluepad32.h>

#include "Controller.h"

GamepadState gamepad = {0, 0, 0, 0, 0, 0, false};

ControllerPtr myControllers[BP32_MAX_GAMEPADS];

// Vrai uniquement à l'instant où on appuie
bool isJustPressed(uint32_t buttonMask) {
    return (gamepad.buttons & buttonMask) && !(gamepad.prevButtons & buttonMask);
}

// Vrai tant que le bouton est maintenu
bool isHeld(uint32_t buttonMask) {
    return (gamepad.buttons & buttonMask);
}

// Fonction appelée quand une manette se connecte
void onConnectedController(ControllerPtr ctl) {
    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        if (myControllers[i] == nullptr) {
            myControllers[i] = ctl;
            Serial.println("Manette Xbox connectée");
            break;
        }
    }
}

// Fonction appelée quand une manette se déconnecte
void onDisconnectedController(ControllerPtr ctl) {
    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        if (myControllers[i] == ctl) {
            myControllers[i] = nullptr;
            Serial.println("Manette Xbox deconnectée");
            break;
        }
    }
}

// Initialise Bluepad32
void setupController() {
    // Initialise Bluepad32
    BP32.setup(&onConnectedController, &onDisconnectedController);
    // Oublie les anciennes manettes appairées
    BP32.forgetBluetoothKeys();
}

// Met à jour l'état de la manette
void updateController() {
    BP32.update();

    // On parcours et met à jour l'état de toutes les manettes connectées
    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        ControllerPtr ctl = myControllers[i];
        if (ctl && ctl->isConnected()) {

            // Sauvegarde de l'ancien état pour la prochaine itération
            gamepad.prevButtons = gamepad.buttons;

            // Mise à jour des inputs
            gamepad.lx = ctl->axisX();
            gamepad.ly = ctl->axisY();
            gamepad.rx = ctl->axisRX();
            gamepad.ry = ctl->axisRY();
            gamepad.buttons = ctl->buttons();
        }
    }
}

// Renvoi true si au moins une manette est connectée
bool isControllerConnected(){
    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        if (myControllers[i] != nullptr) {
            return true;
        }
    }
    return false;
}