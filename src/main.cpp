#include <Arduino.h>
#include "UI.h"
#include "TRAIN.h"
#include "SEMAPHORE_T.h"

#define MOVING_FORWARD 0
#define MOVING_BACKWARD 1
#define STOPPED 2

#define LOOP_LED_ON 1
#define LOOP_LED_OFF 0

UI ui;
Train train;
Semaphore semaphores;
int input;

bool loopEnabled = false;

void handleSoundAndLoop();
void vallleyTrainStateMachine();
String getStatusText(int input, int state, int trainState, int station, bool initiatedToRed);
void loopAnalysis();
void semaphoresTest();

void setup() {
    Serial.begin(115200);
    Serial.println("Starting Valley Train");

    ui.setupPinsAndSensors();
    train.initTrain();
    semaphores.init();

}

void loop() {

    input = ui.inputReceived();

    handleSoundAndLoop();

    vallleyTrainStateMachine();

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
        TURN_GREEN_LIGHT_ON_AT_STATION_X,
        WAITING_AT_STATION_X,
        GOING_TO_LAST_STATION,
        WAIT_BEFORE_GOING_BACKWARD,
        GOING_BACKWARD,
        WAITING_BEFORE_NEXT_LOOP
    };

    if (millis() - lastPrintTime > PRINT_TIME) {
        //Serial.println(getStatusText(input, state, trainState, station, initiatedToRed));
        lastPrintTime = millis();
    }

    if (input == BUTTON_PLAY_PAUSE && trainState != STOPPED) {
        train.stop();
        trainState = STOPPED;
    }
    else if (input == BUTTON_PLAY_PAUSE && trainState == STOPPED) {
        state = GOING_TO_STATION_X;
        train.moveForward();
        trainState = MOVING_FORWARD;
    }
    else if (input == BUTTON_BACKWARDS) {
        train.stop();
        trainState = STOPPED;
        previousMillis = millis();
        state = WAIT_BEFORE_GOING_BACKWARD;
    }

    switch (state) {
        case START:
            if (!initiatedToRed && semaphores.initToRed()) {
                initiatedToRed = true;
            }
            else if (trainState == MOVING_FORWARD) {
                //train.moveForward();
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
                state = TURN_GREEN_LIGHT_ON_AT_STATION_X;
                Serial.println("Reached station " + String(station) + ". Waiting");
                previousMillis = millis();
            }
            break;
        case TURN_GREEN_LIGHT_ON_AT_STATION_X:
            if (station != 7) {
                if (semaphores.setSemaphore(station,GREEN)) {
                    previousMillis = millis();
                    state = WAITING_AT_STATION_X;
                    break;
                }
            }
            else {
                Serial.println("Station 7 reached, not turning green light on");
                previousMillis = millis();
                state = WAITING_AT_STATION_X;
                break;
            }
            break;
        case WAITING_AT_STATION_X:
            if (millis() - previousMillis > WAITING_AT_SEMAPHORE_TIME) {
                if (station != 7) {
                    train.moveForward();
                    trainState = MOVING_FORWARD;
                    state = GOING_TO_STATION_X;
                    Serial.println("Going to station " + String(station + 1));
                    break;
                }
                else if (station == 7) {
                    Serial.println("Youv'e reached the last station");
                    train.moveBackward();
                    trainState = MOVING_BACKWARD;
                    state = GOING_BACKWARD;
                    break;                
                }
                else {
                    Serial.println("Error, station not found");
                    break;
                }
            }
            break;
        case WAIT_BEFORE_GOING_BACKWARD:
            if (millis() - previousMillis > GOING_BACKWARD_DELAY) {
                trainState = MOVING_BACKWARD;
                train.moveBackward();
                state = GOING_BACKWARD;
            }
            break;
        case GOING_BACKWARD://Train is going backwards
            if (input == SENSOR_START && trainState == MOVING_BACKWARD) {
                train.stop();
                trainState = STOPPED;
                station = 0;
                state = START;
                if (loopEnabled) {
                    state = WAITING_BEFORE_NEXT_LOOP;
                    previousMillis = millis();
                    break;
                }
            }
            break;
        case WAITING_BEFORE_NEXT_LOOP:
            if (!initiatedToRed && semaphores.initToRed()) {
                initiatedToRed = true;
            }
            if (millis() - previousMillis > WAITING_AT_SEMAPHORE_TIME) {
                initiatedToRed = false;
                train.moveForward();
                trainState = MOVING_FORWARD;
                state = GOING_TO_STATION_X;
                Serial.println("New Loop, Going to station 1");
            }
            break;
    }

}

void handleSoundAndLoop() {
static unsigned long lastSoundTime = 0;
static bool firstTimePlaying = true;
#define PLAYING_TIME 324000

    if (firstTimePlaying) {
        ui.playSound();
        firstTimePlaying = false;
    }

    if (millis() - lastSoundTime > PLAYING_TIME) {
        ui.playSound();
        lastSoundTime = millis();
    }
    
    if (input == BUTTON_VOLUME_UP) {
        ui.changeVolume(VOLUME_UP);
    }
    if (input == BUTTON_VOLUME_DOWN) {
        ui.changeVolume(VOLUME_DOWN);
    }
    if (input == BUTTON_SOUND_ON_OFF) {
        ui.changeVolume(CHANGE_STATE);
    }

    if (input == BUTTON_LOOP) {
        loopEnabled = !loopEnabled; // Toggle the state of loopEnabled
        if (loopEnabled) {
            ui.turnLoopLED(LOOP_LED_ON);
            Serial.println("Loop mode enabled!");
        } else {
            ui.turnLoopLED(LOOP_LED_OFF);
            Serial.println("Loop mode disabled!");
        }
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

void loopAnalysis() {
    static unsigned long loopStartTime = 0;
    static unsigned long loopEndTime = 0;
    static unsigned long minLoopTime = 0xFFFFFFFF;
    static unsigned long maxLoopTime = 0;
    static unsigned long loopCount = 0;
    static unsigned long totalLoopTime = 0;
    static unsigned long lastPrintTime = 0;

    // Measure the start time of the loop
    if (loopStartTime == 0) {
        loopStartTime = micros();
        return;
    }

    // Measure the end time of the loop
    loopEndTime = micros();
    unsigned long loopDuration = loopEndTime - loopStartTime;

    // Update statistics
    minLoopTime = min(minLoopTime, loopDuration);
    maxLoopTime = max(maxLoopTime, loopDuration);
    totalLoopTime += loopDuration;
    loopCount++;

    // Print analysis every second (1000 milliseconds)
    if (millis() - lastPrintTime >= 1000) {
        Serial.println("---- Loop Analysis (Microseconds) ----");
        Serial.print("Min Loop Time: ");
        Serial.print(minLoopTime);
        Serial.println(" us");

        Serial.print("Max Loop Time: ");
        Serial.print(maxLoopTime);
        Serial.println(" us");

        Serial.print("Avg Loop Time: ");
        Serial.print(totalLoopTime / loopCount);
        Serial.println(" us");

        Serial.print("Loop Count: ");
        Serial.println(loopCount);

        // Reset statistics for the next analysis period
        minLoopTime = 0xFFFFFFFF;
        maxLoopTime = 0;
        totalLoopTime = 0;
        loopCount = 0;
        lastPrintTime = millis(); // Update the last print time
    }

    // Reset loop start time for the next measurement
    loopStartTime = loopEndTime;
}
