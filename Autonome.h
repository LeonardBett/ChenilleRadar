#ifndef AUTONOME_H
#define AUTONOME_H

// Update le mode autonome du robot
void updateAutonomy();
void startAutonomousMode();
void stopAutonomousMode();
String getAIPrediction();

extern bool isAutonomous;

#endif