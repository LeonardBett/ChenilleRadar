#ifndef MOTORS_H
#define MOTORS_H

void setupMotors();
void stopMotors();      // Arrête les deux
void stopLeft();        // Arrête le gauche
void stopRight();       // Arrête le droit

// Commande directe par stick (-511 à 511)
void controlRight(int throttleR);
void controlLeft(int throttleL);

#endif