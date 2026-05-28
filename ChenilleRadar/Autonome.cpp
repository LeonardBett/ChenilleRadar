#include <Temmie33-project-1_inferencing.h>

#include <Arduino.h>

#include "Autonome.h"
#include "Motors.h"
#include "Radar.h"
#include "Config.h"

// AI variables
float features[16];

// Design pattern state pour l'implementation du mode autonome
enum RobotState { MANUAL, DRIVING, OBSTACLE_DETECTED, SCANNING, DECIDING };
RobotState currentState;
bool isAutonomous = false;


String getAIPrediction() {
    // 1. On remplit le tableau de "features" avec tes 16 distances
    for (int i = 0; i < 16; i++) {
        features[i] = scanL[i];
    }

    // 2. Conversion pour le format Edge Impulse
    signal_t signal;
    numpy::signal_from_buffer(features, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, &signal);

    // 3. Exécution du classifieur
    ei_impulse_result_t result = { 0 };
    EI_IMPULSE_ERROR res = run_classifier(&signal, &result, false);

    if (res != EI_IMPULSE_OK) {
        return "ERROR";
    }

    // 4. On cherche le label avec la probabilité la plus haute
    float max_score = 0;
    int max_index = 0;

    for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
        if (result.classification[ix].value > max_score) {
            max_score = result.classification[ix].value;
            max_index = ix;
        }
    }

    // On ne valide que si l'IA est sûre à plus de 60%
    if (max_score > 0.5) {
        return String(result.classification[max_index].label);
    } else {
        return "INCERTAIN";
    }
}

void startAutonomousMode() {
  currentState = DRIVING;
  isAutonomous = true;
}

void stopAutonomousMode() {
  stopMotors();
  isAutonomous = false;
}

// Update le mode autonome du robot
void updateAutonomy() {
    switch (currentState) {
        case DRIVING:
            controlLeft(-300); 
            controlRight(-300);
            if (getFixedDistance() < 30.0) {
                currentState = OBSTACLE_DETECTED;
            }
            break;

        case OBSTACLE_DETECTED:
            stopMotors();
            currentState = SCANNING;
            break;

        case SCANNING:
            updateRadarAngle();
            currentState = DECIDING;
            break;

        case DECIDING:
            String decision = getAIPrediction();
            Serial.print("IA Decision: "); 
            Serial.println(decision);
            break;
    }
}