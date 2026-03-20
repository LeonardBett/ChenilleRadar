#ifndef RADAR_H
#define RADAR_H

void setupRadar();
float getDistance();
void updateRadarAngle();
void drawTerminalRadar(int a, float d);

extern float* scan; 

#endif