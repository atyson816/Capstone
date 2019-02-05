/*
 * main.h
 *
 *  Created on: Dec 28, 2018
 *      Author: Austin
 */

#ifndef MAIN_H_
#define MAIN_H_
#include "adc12_b.h"
#include "cs.h"
#include "gpio.h"
#include "rtc_c.h"
#include "wdt_a.h"
// -------------------- Function Declarations --------------------
// Helper Block
void STATE_CHECK(void);
int currToUsrCompare(void); //TODO occurs after ADC has generated a new average value, comparing to the user's value
int valveOpen(void); //TODO This controls the valve being open and receives the valve response
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
typedef enum {INIT, SLEEP, POLL, RUNNING} state;

typedef struct {
    double temperature;
    double moisture;
} READ_RESULT;

typedef struct {
    double startTemp;
    double startMoisture;
    double finishTime;
} RUN_RESULT;

// Global Variables
#define MAXNODES 48
Calendar TIME;
state STATE = INIT;
int MOISTURE[MAXNODES]; // ADC Sampling put in this Variable
int TEMPERATURE[MAXNODES]; // ADC Sampling put in this Variable
int MOISTURE_DONE = 0; // This is to make sure Moisture only gets 48 of the 96 samples
int TEMPERATURE_DONE = 0; // This is to make sure Temperature only gets 48 of the 96 samples
int SAMPLES = 96; // This is used to loop to make sure all 48 temp and 48 moisture reading taken.
READ_RESULT CURR_TEMP_MOIST; // This holds average of sampling and value to be displayed
READ_RESULT USR_TEMP_MOIST; // Set this to the user's desired moisture and temperature
RUN_RESULT PREV_RESULTS[MAXNODES]; //TODO Set this up for the deterministic algorithm
#endif /* MAIN_H_ */
