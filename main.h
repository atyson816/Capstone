/*
 * main.h
 *
 *  Created on: Dec 28, 2018
 *      Author: Austin
 */

#ifndef MAIN_H_
#define MAIN_H_

// Function Declarations
void ADC_INIT(void);
void ADC_CTRL(void);
void GPIO_INIT(void);
void main(void);

// Global Declarations
typedef enum {INIT, SLEEP, POLL} state;
typedef struct {
    double temperature;
    double moisture;
} AVG_FINAL;

// Global Variables
#define MAXNODES 48
#define RUNNING 0
state STATE = INIT;
int MOISTURE[MAXNODES];
int TEMPERATURE[MAXNODES];
AVG_FINAL CURR_TEMP_MOIST;
AVG_FINAL USR_TEMP_MOIST;
AVG_FINAL AVG_TEMP_MOIST[MAXNODES];
#endif /* MAIN_H_ */
