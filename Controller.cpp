#include <Arduino.h>
#include <Bluepad32.h>

#include "Controller.h"

GamepadState gamepad = {0, 0, 0, 0, 0, 0, false};

ControllerPtr myControllers[BP32_MAX_GAMEPADS];

// --- Utilitaires intelligents ---

// Vrai uniquement à l'instant où on appuie (Front montant)
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
    BP32.update();

    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        ControllerPtr ctl = myControllers[i];

        if (ctl && ctl->isConnected()) {
            // Sauvegarde de l'ancien état pour le prochain tour
            gamepad.prevButtons = gamepad.buttons;

            // Mise à jour des valeurs
            gamepad.lx = ctl->axisX();
            gamepad.ly = ctl->axisY();
            gamepad.rx = ctl->axisRX();
            gamepad.ry = ctl->axisRY();
            gamepad.buttons = ctl->buttons();
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