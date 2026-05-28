#ifndef RADAR_H
#define RADAR_H

void setupRadar();
float getFixedDistance();
void updateRadarAngle();
void printScanForEI();

extern float* scanL; 

#endif