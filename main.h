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
#include "rtc_b.h"
#include "wdt_a.h"
// -------------------- Function Declarations --------------------
// Helper Block
void STATE_CHECK(void);
// Math Block
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
state STATE = INIT;
int MOISTURE[MAXNODES]; //TODO ADC Sampling put in this Variable
int TEMPERATURE[MAXNODES]; //TODO ADC Sampling put in this Variable
const int currM = 0; //TODO Index for Moisture List
const int currT = 0; //TODO Index for Temperature List
READ_RESULT CURR_TEMP_MOIST; //TODO This holds average of sampling and value to be displayed
READ_RESULT USR_TEMP_MOIST; //TODO Set this to the user's desired moisture and temperature
RUN_RESULT PREV_RESULTS[MAXNODES]; //TODO Set this up for the deterministic algorithm
#endif /* MAIN_H_ */
