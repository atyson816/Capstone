/*
 * main.h
 *
 *  Created on: Dec 28, 2018
 *      Authors: Austin
 *             : Ockert
 */

#ifndef MAIN_H_
#define MAIN_H_
#include <stdio.h>
#include <stdlib.h>
#include <hd44780.h>
// -------------------- Function Declarations --------------------
// Helper Block
void delay(void);
void display(void);
void timeDisplay(void);
void timeDisplayFlash(void);
void tempDisplay(void);
void tempDisplayFlash(void);
void moisDisplay(void);
void moisDisplayFlash(void);
void stateCheck(void);
void enableSensors(void);
void disableSensors(void);
void currToUsrCompare(void);
void valveOpen(void); //This controls the valve being open and receives the valve response
void valveClose(void);
// Math Block
void flowRate(void); //TODO This will be the algorithm to determine water absorption
void timeCheck(void); //TODO This will utilize the RTC for time-checking
// GPIO Block
void GPIO_INIT(void);
// ADC Block
void ADC_INIT(void);
void ADC_CTRL(void);
//
void main(void);

// Global Type Def's
typedef enum {MASTERON, MASTEROFF, SLEEP, POLLING, RUNNING} state;
typedef enum {TIME, TEMP, MOIS} screens;

typedef struct {
    unsigned int hourTen;
    unsigned int hourOne;
    unsigned int minTen;
    unsigned int minOne;
} time;

typedef struct {
    double temperature;
    double moisture;
} READ_RESULT;

typedef struct {
    double startTemp;
    double startMoisture;
    double finishTime;
} RUN_RESULT;

//Variables
volatile unsigned int D0P3 = BIT3;
volatile unsigned int D1P3 = BIT6;
volatile unsigned int D2P3 = BIT7;
volatile unsigned int D3P2 = BIT2;
volatile unsigned int D4P1 = BIT3;
volatile unsigned int D5P3 = BIT0;
volatile unsigned int D6P3 = BIT1;
volatile unsigned int D7P2 = BIT3;
volatile unsigned int RSP1 = BIT4;
volatile unsigned int ENP2 = BIT6;
// SEL=0: UP/DN scroll screens      SEL increases itself to 1       BACK does nothing
// SEL=1: UP/DN scroll CURSOR       SEL increases itself to 2       BACK decreases SEL to 0
// SEL=2: UP/DN scroll values       SEL does nothing                BACK decreases SEL to 1
volatile unsigned int SEL = 0;
// CURSOR=0: Deactivated
// TIME:
// CURSOR=1 HOUR 10's place         (0-2)
// CURSOR=2 HOUR 1's place          (0-9)
// CURSOR=3 MINUTES 10's place      (0-5)
// CURSOR=4 MINUTES 1's place       (0-9)
// CURSOR=5 WATER toggle            (ON/OFF)
// TEMP:
// CURSOR=1 TEMP 10's place         (0-5)
// CURSOR=2 TEMP 1's place          (0-9)
// CURSOR=3 TEMP sensor toggle      (ON/OFF)
// MOIS:
// CURSOR=1 MOISTURE 100's place    (0-1)
// CURSOR=2 MOISTURE 10's place     (0-9)
// CURSOR=3 MOISTURE 1's place      (0-9)
// CURSOR=4 MOISTURE sensor toggle  (ON/OFF)
unsigned int firstRun = 1;
volatile unsigned int CURSOR = 0;
volatile state STATE = SLEEP;
volatile screens SCREEN = TIME;
#define MAXNODES 30
volatile unsigned int MASTEROVERRIDE = 0;
volatile unsigned int WATERED = 0;
volatile unsigned int WATERING = 0;
volatile int MOISTURE[MAXNODES]; // ADC Sampling put in this Variable
volatile int TEMPERATURE[MAXNODES]; // ADC Sampling put in this Variable
volatile unsigned int MOISTURE_DONE = 0; // This is to make sure Moisture only gets 48 samples
volatile unsigned int TEMPERATURE_DONE = 0; // This is to make sure Temperature only gets 48 samples
volatile unsigned int mSampleIdx = 0; // This is the moisture sample index for the ADC12_ISR.
volatile unsigned int tSampleIdx = 0; // This is the temperature sample index for the ADC12_ISR.
volatile unsigned int TEMP_STATUS = 1; // This is used to determine if temperature sensor is on or off.
volatile unsigned int MOIST_STATUS = 1; // This is used to determine if moisture sensor is on or off.
volatile READ_RESULT CURR_TEMP_MOIST; // This holds average of sampling and value to be displayed
volatile READ_RESULT USR_TEMP_MOIST; // Set this to the user's desired moisture and temperature
RUN_RESULT PREV_RESULTS[MAXNODES]; //TODO Set this up for the deterministic algorithm
volatile time CURR_TIME;
volatile unsigned int count = 0;
#endif /* MAIN_H_ */
