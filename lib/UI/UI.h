#ifndef UI_H
#define UI_H

#include <Arduino.h>
#include <FastLED.h>

// Pin definitions
#define NUM_LEDS 6  // Increased to handle the 6 additional LEDs
#define DATA_PIN 26

#define SEL0_IN 2
#define SEL1_IN 13
#define SEL2_IN 14
#define SEL3_IN 27
#define IO_IN 32

#define VOLUME_UP 0
#define VOLUME_DOWN 1
#define CHANGE_STATE 2

enum BUTTON_SENSORS_INPUTS {
    BUTTON_SOUND_ON_OFF = 0,
    BUTTON_VOLUME_UP = 1,
    BUTTON_VOLUME_DOWN = 2,
    BUTTON_PLAY_PAUSE = 3,
    BUTTON_LOOP = 4,
    BUTTON_BACKWARDS = 5,
    SENSOR_START = 6,
    SENSOR_STATION_1 = 7,
    SENSOR_STATION_2 = 8,
    SENSOR_STATION_3 = 9,
    SENSOR_STATION_4 = 10,
    SENSOR_STATION_5 = 11,
    SENSOR_STATION_6 = 12,
    SENSOR_LAST_STATION = 13,  
    NO_INPUTS_RECEIVED = -1,
};

class UI {
public:
    UI();
    BUTTON_SENSORS_INPUTS inputReceived();
    void playSound();
    void setupPinsAndSensors();
    void changeVolume(int volume);
    void turnLoopLED(int state);
private:
    BUTTON_SENSORS_INPUTS buttonState;
    void printButtonName(BUTTON_SENSORS_INPUTS button);
    void selectMuxChannel(int channel);  
    bool isBusy();
    void setVolume(int volume);
    void executeCMD(byte CMD, byte Par1, byte Par2);
    CRGB leds[NUM_LEDS];
    int currentVolume = 20;
};

#endif // UI_H
