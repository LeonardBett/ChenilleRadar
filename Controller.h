#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <Bluepad32.h>

void setupController();
void updateController(); // À appeler dans la loop principale
bool isControllerConnected();

// Pour que les autres fichiers (Motors) puissent lire les sticks
extern int xbox_stick_ly;
extern int xbox_stick_ry;

#endif