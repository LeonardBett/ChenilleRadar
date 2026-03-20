#ifndef RADAR_H
#define RADAR_H

void setupRadar();
float getDistance();
void updateRadarAngle();
void drawTerminalRadar(int a, float d);

// On rend l'angle accessible au reste du programme si besoin
extern int currentPulse; 

#endif