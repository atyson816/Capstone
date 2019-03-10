/*
 * main.h
 *
 *  Created on: Dec 28, 2018
 *      Author: Austin
 */

#ifndef MAIN_H_
#define MAIN_H_
#include <stdio.h>
#include <stdlib.h>
#include <msp430fr6989.h>
#include <hd44780.h>
// -------------------- Function Declarations --------------------
// Helper Block
void STATE_CHECK(void);
void currToUsrCompare(void); //TODO occurs after ADC has generated a new average value, comparing to the user's value
void valveOpen(void); //This controls the valve being open and receives the valve response
// Math Block
int flowRate(void); //TODO This will be the algorithm to determine water absorption
int timeCheck(void); //TODO This will utilize the RTC for time-checking
// GPIO Block
void GPIO_INIT(void);
// ADC Block
void ADC_INIT(void);
void ADC_CTRL(void);
//
void main(void);

// Global Type Def's
typedef enum {INIT, SLEEP, POLLING, RUNNING} state;

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

#define MAXNODES 48
state STATE = INIT;
unsigned int WATERED = 0;
unsigned int MOISTURE[MAXNODES]; // ADC Sampling put in this Variable
unsigned int TEMPERATURE[MAXNODES]; // ADC Sampling put in this Variable
unsigned int MOISTURE_DONE = 0; // This is to make sure Moisture only gets 48 samples
unsigned int TEMPERATURE_DONE = 0; // This is to make sure Temperature only gets 48 samples
unsigned int mSampleIdx = 0; // This is the moisture sample index for the ADC12_ISR.
unsigned int tSampleIdx = 0; // This is the temperature sample index for the ADC12_ISR.
unsigned int TEMP_STATUS = 1; // This is used to determine if temperature sensor is on or off.
unsigned int MOIST_STATUS = 1; // This is used to determine if moisture sensor is on or off.
READ_RESULT CURR_TEMP_MOIST; // This holds average of sampling and value to be displayed
READ_RESULT USR_TEMP_MOIST; // Set this to the user's desired moisture and temperature
RUN_RESULT PREV_RESULTS[MAXNODES]; //TODO Set this up for the deterministic algorithm
#endif /* MAIN_H_ */
