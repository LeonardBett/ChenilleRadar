#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <Bluepad32.h>

// Structure pour regrouper toutes les données proprement
struct GamepadState {
    int ly, ry;         // Sticks verticaux
    int lx, rx;         // Sticks horizontaux
    uint32_t buttons;   // État actuel des boutons
    uint32_t prevButtons; // État précédent (pour détecter les clics)
    bool connected;
};

void setupController();
void updateController(); // À appeler dans la loop principale
bool isControllerConnected();
bool isJustPressed(uint32_t buttonMask);
bool isHeld(uint32_t buttonMask);

extern GamepadState gamepad;

#endif