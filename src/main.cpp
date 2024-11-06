#include <Arduino.h>
#include "UI.h"
#include "TRAIN.h"
#include "SEMAPHORE_T.h"

#define MOVING_FORWARD 0
#define MOVING_BACKWARD 1
#define STOPPED 2

UI ui;
Train train;
Semaphore semaphores;
int input;

void handleSound();
void vallleyTrainStateMachine();
String getStatusText(int input, int state, int trainState, int station, bool initiatedToRed);

void setup() {
    Serial.begin(115200);
    Serial.println("Starting Valley Train");

    ui.setupPinsAndSensors();
    train.initTrain();
    semaphores.init();

}

void loop() {

    input = ui.inputReceived();

    // handleSound();

    //vallleyTrainStateMachine();

}

void vallleyTrainStateMachine() {
    static int state = 0;
    static unsigned long previousMillis = 0;
    static unsigned long lastPrintTime = 0;
    static int trainState = STOPPED;
    static int station = 0;
    static bool initiatedToRed = false;

    #define WAITING_AT_SEMAPHORE_TIME 2000
    #define PRINT_TIME 2000
    #define GOING_BACKWARD_DELAY 2000

    enum TRAIN_STATE_TYPE {
        START,
        GOING_TO_STATION_X,
        WAITING_AT_STATION_X,
        GOING_TO_LAST_STATION,
        GOING_BACKWARD
    };

    if (millis() - lastPrintTime > PRINT_TIME) {
        Serial.println(getStatusText(input, state, trainState, station, initiatedToRed));
        lastPrintTime = millis();
    }

    if (input == BUTTON_PLAY_PAUSE && trainState != STOPPED) {
        train.stop();
        trainState = STOPPED;
    }
    else if (input == BUTTON_PLAY_PAUSE && trainState == STOPPED) {
        if (input == SENSOR_LAST_STATION) {
            train.moveBackward();
            trainState = MOVING_BACKWARD;
        }
        else {
            if (station != 0) {
                train.moveForward();
                state = GOING_TO_STATION_X;
            }
            else {
                state = START;
            }
            trainState = MOVING_FORWARD;
        }
    }
    else if (input == BUTTON_BACKWARDS) {
        if (station == 0) {
            Serial.println("Can't go backwards from start station");
            return;
        }
        if (trainState != STOPPED) {
            train.stop();
            trainState = STOPPED;
            delay(GOING_BACKWARD_DELAY);
        }
        train.moveBackward();
        trainState = MOVING_BACKWARD;
        state = GOING_BACKWARD;
    }
    if (input == BUTTON_LOOP) {
        //TODO Make train loop - mabye add a state of what the train is doing
    }

    switch (state) {
        case START:
            if (!initiatedToRed && semaphores.initToRed()) {
                initiatedToRed = true;
            }
            else if (input == SENSOR_START && trainState == MOVING_FORWARD) {
                train.moveForward();
                initiatedToRed = false;
                state = GOING_TO_STATION_X;
                Serial.println("Going to station 1");
            }
            break;
        case GOING_TO_STATION_X://Train is going to one of the stations
            
            switch (input)
            {
                case SENSOR_STATION_1:
                    station = 1;
                    break;
                case SENSOR_STATION_2:
                    station = 2;
                    break;
                case SENSOR_STATION_3:
                    station = 3;
                    break;
                case SENSOR_STATION_4:  
                    station = 4;
                    break;
                case SENSOR_STATION_5:
                    station = 5;
                    break;
                case SENSOR_STATION_6:
                    station = 6;
                    break;
                case SENSOR_LAST_STATION:
                    station = 7;
                    break;
                default:
                    break;
            }
            
            if (input == SENSOR_STATION_1 || input == SENSOR_STATION_2 || input == SENSOR_STATION_3 || input == SENSOR_STATION_4 || input == SENSOR_STATION_5 || input == SENSOR_STATION_6 || input == SENSOR_LAST_STATION) {
                train.stop();
                trainState = STOPPED;
                state = WAITING_AT_STATION_X;
                Serial.println("Reached station " + String(station) + ". Waiting");
                previousMillis = millis();
            }
        
            break;
        case WAITING_AT_STATION_X:
            if (millis() - previousMillis > WAITING_AT_SEMAPHORE_TIME) {
                if (station == 7) {//Last station
                    Serial.println("Youv'e reached the last station");
                    train.moveBackward();
                    trainState = MOVING_BACKWARD;
                    state = GOING_BACKWARD;
                    break;
                }
                else {
                    if (semaphores.setSemaphore(station,GREEN)) {
                        train.moveForward();
                        trainState = MOVING_FORWARD;
                        state = GOING_TO_STATION_X;
                        Serial.println("Going to station " + String(station + 1));
                        break;
                    }
                    break;
                }
            }
            break;
        case GOING_BACKWARD://Train is going backwards
            if (input == SENSOR_START && trainState == MOVING_BACKWARD) {
                train.stop();
                trainState = STOPPED;
                semaphores.initToRed();
                station = 0;
                state = START;
            }
            break;
    }

}

void handleSound() {

    ui.playSound();

    if (input == BUTTON_VOLUME_UP) {
        //TODO Increase train speed
        ui.changeVolume(VOLUME_UP);
    }
    if (input == BUTTON_VOLUME_DOWN) {
        //TODO Decrease train speed
        ui.changeVolume(VOLUME_DOWN);
    }
    if (input == BUTTON_SOUND_ON_OFF) {
        ui.changeVolume(CHANGE_STATE);
    }
}

String getStatusText(int input, int state, int trainState, int station, bool initiatedToRed) {
    String stateText;
    String trainStateText;
    String stationText;
    String redInitText = initiatedToRed ? "Yes" : "No";
    String inputText;

    switch (input) {
        case BUTTON_SOUND_ON_OFF: inputText = "BUTTON_SOUND_ON_OFF"; break;
        case BUTTON_VOLUME_UP: inputText = "BUTTON_VOLUME_UP"; break;
        case BUTTON_VOLUME_DOWN: inputText = "BUTTON_VOLUME_DOWN"; break;
        case BUTTON_PLAY_PAUSE: inputText = "BUTTON_PLAY_PAUSE"; break;
        case BUTTON_LOOP: inputText = "BUTTON_LOOP"; break;
        case BUTTON_BACKWARDS: inputText = "BUTTON_BACKWARDS"; break;
        case SENSOR_START: inputText = "SENSOR_START"; break;
        case SENSOR_STATION_1: inputText = "SENSOR_STATION_1"; break;
        case SENSOR_STATION_2: inputText = "SENSOR_STATION_2"; break;
        case SENSOR_STATION_3: inputText = "SENSOR_STATION_3"; break;
        case SENSOR_STATION_4: inputText = "SENSOR_STATION_4"; break;
        case SENSOR_STATION_5: inputText = "SENSOR_STATION_5"; break;
        case SENSOR_STATION_6: inputText = "SENSOR_STATION_6"; break;
        case SENSOR_LAST_STATION: inputText = "SENSOR_LAST_STATION"; break;
        default: inputText = "NO_INPUTS_RECEIVED"; break;
    }

    // Convert state to human-readable text
    switch (state) {
        case 0: stateText = "START"; break;
        case 1: stateText = "GOING_TO_STATION_X"; break;
        case 2: stateText = "WAITING_AT_STATION_X"; break;
        case 3: stateText = "GOING_TO_LAST_STATION"; break;
        case 4: stateText = "GOING_BACKWARD"; break;
        default: stateText = "UNKNOWN"; break;
    }

    // Convert trainState to human-readable text
    switch (trainState) {
        case MOVING_FORWARD: trainStateText = "MOVING_FORWARD"; break;
        case MOVING_BACKWARD: trainStateText = "MOVING_BACKWARD"; break;
        case STOPPED: trainStateText = "STOPPED"; break;
        default: trainStateText = "UNKNOWN"; break;
    }

    // Convert station number to human-readable text
    if (station == 0) {
        stationText = "Start Station";
    } else if (station >= 1 && station <= 6) {
        stationText = "Station " + String(station);
    } else if (station == 7) {
        stationText = "Last Station";
    } else {
        stationText = "No Station";
    }

    // Return a formatted status string
    return "Input: " + inputText + ", State: " + stateText + ", Train State: " + trainStateText + ", Station: " + stationText + ", Initiated to Red: " + redInitText;
}
