// Capstone Design Project
// Authors:
// Stefan Heincke
// Ockert Strydom
// Austin Tyson
// Kendal Zimmer
#include "main.h"
// ========================= INIT BLOCK ===========================
void init(void) {
    unsigned int minutes, hours, temperature_threshold, moisture_threshold;
    //TODO get the user's input of the current time, and threshold values.
    while(1) {


        break;
    }
    USR_TEMP_MOIST.moisture = moisture_threshold;
    USR_TEMP_MOIST.temperature = temperature_threshold;
    // BCD is nicer to deal with in interrupts than binary.
    TIME.Seconds = 0;
    TIME.Minutes = minutes;
    TIME.Hours = hours;
    // Creating the calendar, not running yet.
    //Initialize LFXT1
    CS_turnOnLFXT(CS_LFXT_DRIVE_3);
    RTC_C_initCalendar(RTC_C_BASE, &TIME, RTC_C_FORMAT_BINARY);
    // Configuring the Polling Alarm for the Calendar every 30 minutes
    RTC_C_configureCalendarAlarmParam param1 = {0};
    param1.minutesAlarm = 30;
    RTC_C_configureCalendarAlarm(RTC_C_BASE, &param1);
    // Configure the RTC to make sure at least 2 hours occur before watering
    // again.
    RTC_C_setCalendarEvent(RTC_C_BASE, RTC_C_CALENDAREVENT_HOURCHANGE);
    RTC_C_clearInterrupt(RTC_C_BASE, RTC_C_TIME_EVENT_INTERRUPT + RTC_C_CLOCK_ALARM_INTERRUPT);
    RTC_C_enableInterrupt(RTC_C_BASE, RTC_C_TIME_EVENT_INTERRUPT + RTC_C_CLOCK_ALARM_INTERRUPT);
    RTC_C_startClock(RTC_C_BASE);
    STATE = POLLING;
}

// ========================= HELPER BLOCK =========================
void GPIOinputSetup(unsigned int port, unsigned int pin) {
    GPIO_setAsInputPinWithPullUpResistor(port, pin);
    GPIO_selectInterruptEdge(port, pin, GPIO_HIGH_TO_LOW_TRANSITION);
    GPIO_clearInterrupt(port, pin);
    GPIO_enableInterrupt(port, pin);
}

//TODO Implement this
void currToUsrCompare(void) {

}

/*
 * This function opens the valve, called by currToUsrCompare
 */
void valveOpen(void) {
    GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN5);
    __delay_cycles(6000000);
    GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN5);
    WATERED = 1;
}

/*
 * This function closes the valve, will be called by currToUsrCompare
 */
void valveClose(void) {
    GPIO_setOutputHighOnPin(GPIO_PORT_P3, GPIO_PIN2);
    __delay_cycles(6000000);
    GPIO_setOutputLowOnPin(GPIO_PORT_P3, GPIO_PIN2);
}

// ========================= GPIO BLOCK =========================
/*
 * This Function Initializes the GPIO Ports for the buttons, and display, as
 * well as the sensor enables and valve control.
 */
void GPIO_INIT(void) {
    // Clear any floating output -- Enables/Open/Close
    GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0 + GPIO_PIN1);
    GPIO_setOutputLowOnPin(GPIO_PORT_P3, GPIO_PIN2);
    GPIO_setOutputLowOnPin(GPIO_PORT_P9, GPIO_PIN4);
    // Configure GPIO to send out to sensors and valve controller
    // PIN0 is moisture_enable, 1 is temp_enable, 9.4 is valve_open, 10 is valve_close
    GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN0 + GPIO_PIN1);
    GPIO_setAsOutputPin(GPIO_PORT_P3, GPIO_PIN2);
    GPIO_setAsOutputPin(GPIO_PORT_P9, GPIO_PIN4);
    // Clear any floating output -- Display -- 12 I/O ports
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN3 + GPIO_PIN4 + GPIO_PIN6 + GPIO_PIN7);
    GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN2 + GPIO_PIN3 + GPIO_PIN6);
    GPIO_setOutputLowOnPin(GPIO_PORT_P3, GPIO_PIN0 + GPIO_PIN1 + GPIO_PIN3 + GPIO_PIN6 + GPIO_PIN7);
    // Configure display I/O
    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN3 + GPIO_PIN4 + GPIO_PIN6 + GPIO_PIN7);
    GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN2 + GPIO_PIN3 + GPIO_PIN6);
    GPIO_setAsOutputPin(GPIO_PORT_P3, GPIO_PIN0 + GPIO_PIN1 + GPIO_PIN3 + GPIO_PIN6 + GPIO_PIN7);
    // Input Pins -- 5 User Buttons
    GPIOinputSetup(GPIO_PORT_P1, GPIO_PIN5);
    GPIOinputSetup(GPIO_PORT_P2, GPIO_PIN4);
    GPIOinputSetup(GPIO_PORT_P2, GPIO_PIN5);
    GPIOinputSetup(GPIO_PORT_P2, GPIO_PIN7);
    GPIOinputSetup(GPIO_PORT_P4, GPIO_PIN7);
    // Disable GPIO power-on default high-impedance.
    PMM_unlockLPM5();
}

// ========================= ADC BLOCK =========================
/*
 * This function initializes the ADC for operation
 */
void ADC_INIT(void) {
        // Initializing and enabling the ADC for Functioning
        ADC12_B_initParam initParam = {0};
        initParam.sampleHoldSignalSourceSelect = ADC12_B_SAMPLEHOLDSOURCE_SC;
        initParam.clockSourceSelect = ADC12_B_CLOCKSOURCE_MCLK;
        initParam.clockSourceDivider = ADC12_B_CLOCKDIVIDER_1;
        initParam.clockSourcePredivider = ADC12_B_CLOCKPREDIVIDER__1;
        initParam.internalChannelMap = ADC12_B_MAPINTCH0;

        ADC12_B_init(ADC12_B_BASE, &initParam);
        ADC12_B_enable(ADC12_B_BASE);
}

/*
 * This Function Configures the ADC for use with the Sensors
 */
void ADC_CTRL(void) {
        ADC12_B_setupSamplingTimer(ADC12_B_BASE,
                                   ADC12_B_CYCLEHOLD_4_CYCLES,
                                   ADC12_B_CYCLEHOLD_4_CYCLES,
                                   ADC12_B_MULTIPLESAMPLESENABLE);
        // Setting up the Memory Config for the Moisture Sensor
        ADC12_B_configureMemoryParam memParam = {0};
        memParam.memoryBufferControlIndex = ADC12_B_MEMORY_0;
        memParam.inputSourceSelect |= ADC12_B_INPUT_A10; //Whatever the moisture sensor's analog wiring is.
        memParam.refVoltageSourceSelect |= ADC12_B_VREFPOS_AVCC_VREFNEG_VSS;
        memParam.endOfSequence |= ADC12_B_NOTENDOFSEQUENCE;
        memParam.windowComparatorSelect |= ADC12_B_WINDOW_COMPARATOR_DISABLE;
        memParam.differentialModeSelect |= ADC12_B_DIFFERENTIAL_MODE_DISABLE;
        ADC12_B_configureMemory(ADC12_B_BASE, &memParam);
        // Setting up the Memory Config for the Temperature Sensor.
        ADC12_B_configureMemoryParam memParam1 = {0};
        memParam.memoryBufferControlIndex = ADC12_B_MEMORY_1;
        memParam.inputSourceSelect |= ADC12_B_INPUT_A11; //Whatever the temperature sensor's analog wiring is.
        memParam.refVoltageSourceSelect |= ADC12_B_VREFPOS_AVCC_VREFNEG_VSS;
        memParam.endOfSequence |= ADC12_B_ENDOFSEQUENCE;
        memParam.windowComparatorSelect |= ADC12_B_WINDOW_COMPARATOR_DISABLE;
        memParam.differentialModeSelect |= ADC12_B_DIFFERENTIAL_MODE_DISABLE;
        ADC12_B_configureMemory(ADC12_B_BASE, &memParam1);
        // Clearing and Enabling Interrupts for our 2 ADC signals
        ADC12_B_clearInterrupt(ADC12_B_BASE, 0, ADC12_B_IFG0);
        ADC12_B_enableInterrupt(ADC12_B_BASE, ADC12_B_IE0 + ADC12_B_IE1, 0, 0);
}

/*
 * This function turns on the temperature and moisture sensors, called by
 * currToUsrCompare()
 */
void enableSensors(void) {
    GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN0 + GPIO_PIN1);
    // Delay to ensure that the sensors are now enabled.
    __delay_cycles(500000);
}

/*
 * This function will turn off the temperature and moisture sensors, and will
 * most likely be used by currToUsrCompare()
 */
void disableSensors(void) {
    GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0 + GPIO_PIN1);
    // Delay to ensure that the sensors are now disabled.
    __delay_cycles(500000);
}

/*
 * This function runs the ADC until all 96 samples are collected
 */
void runADC(void) {
    while (SAMPLES > 0) {
        // This delay puts around 5 seconds for obtaining all samples
        __delay_cycles(85000);
        ADC12_B_startConversion(ADC12_B_BASE, ADC12_B_MEMORY_0 + ADC12_B_MEMORY_1,
             ADC12_B_SEQOFCHANNELS);
        __bis_SR_register(LPM3_bits + GIE);
        __no_operation();
    }
    SAMPLES = 96;
    MOISTURE_DONE = 0;
    TEMPERATURE_DONE = 0;
}

/*
 * This Function is the main driver for the system, controlling which
 * functions occur when.
 */
void stateCheck(void) {
    if (STATE == SLEEP) {
        // Puts the Device to sleep waiting on RTC/Button Interrupts.
        __bis_SR_register(LPM3_bits + GIE);
        __no_operation();
    } else if (STATE == POLLING) {
        enableSensors();
        runADC();
        //TODO Add The Comparing Function
        //TODO
    } else if (STATE == INIT) {
        init();
        //TODO
    } else if (STATE == RUNNING) {
        //TODO Make the ADC actually Run to obtain the values
        //TODO Add the Comparing Function
        valveOpen();
        runADC();
    }
}

void main(void) {
    // Disable Watchdog Timer while Initializing.
    WDT_A_hold(WDT_A_BASE);
    // Initialize on startup
    GPIO_INIT();
    //ADC_INIT();
    //ADC_CTRL();
    while(1) {
        enableSensors();
        __delay_cycles(10000000);
        disableSensors();
        __delay_cycles(10000000);
        //GPIO_setOutputLowOnPin(GPIO_PORT_PB, GPIO_PIN1);
        //__delay_cycles(100000000);
        //GPIO_setOutputHighOnPin(GPIO_PORT_PB, GPIO_PIN1);
        //GPIO_setOutputLowOnPin(GPIO_PORT_PB, GPIO_PIN0);
        //__delay_cycles(100000000);
            // State Check Call/Switch
            //stateCheck();
    }
}

// ==================== INTERRUPTS ===================
#pragma vector = ADC12_VECTOR
__interrupt

/*
 * Interrupt Handler for the ADC Module
 */
void ADC12_ISR(void) {
    unsigned long res;
    unsigned int ii;
    unsigned int currM = 47;
    unsigned int currT = 47;
    switch(__even_in_range(ADC12IV,12)) {
    case  0: break;                     // Vector  0:  No interrupt
    case  2: break;                     // Vector  2:  ADC12BMEMx Overflow
    case  4: break;                     // Vector  4:  Conversion time overflow
    case  6: break;                     // Vector  6:  ADC12BHI
    case  8: break;                     // Vector  8:  ADC12BLO
    case 10: break;                     // Vector 10:  ADC12BIN
    case 12:                            // Vector 12:  ADC12BMEM0
        if (MOISTURE_DONE == 0) {
            res = ADC12_B_getResults(ADC12_B_BASE, ADC12_B_MEMORY_0);
            if (currM == 0) {
                MOISTURE_DONE = 1;
                currM = 47;
                for (ii = 0; ii > 0; ii--) {
                    res += MOISTURE[ii];
                }
                CURR_TEMP_MOIST.moisture = res/48;
            } else {
                MOISTURE[currM] = res;
                currM --;
                SAMPLES --;
            }
        }
        __bis_SR_register_on_exit(LPM3_bits);
        break;
    case 14:                         // Vector 14:  ADC12BMEM1
        if (TEMPERATURE_DONE == 0) {
            res = ADC12_B_getResults(ADC12_B_BASE, ADC12_B_MEMORY_1);
            if (currT == 0) {
                TEMPERATURE_DONE = 1;
                currT = 47;
                for (ii = 0; ii > 0; ii--) {
                    res += TEMPERATURE[ii];
                }
                CURR_TEMP_MOIST.temperature = res/48;
            } else {
                TEMPERATURE[currT] = res;
                currT --;
                SAMPLES --;
            }
        }
        __bis_SR_register_on_exit(LPM3_bits);
        break;
    default: break;
    }
}

#pragma vector = PORT1_VECTOR
__interrupt

/*
 * The Interrupt Handler for Port 1
 */
void Port_1(void) {

}

#pragma vector = PORT2_VECTOR
__interrupt

/*
 * The Interrupt Handler for Port 2
 */
void Port_2(void) {

}

#pragma vector = PORT4_VECTOR
__interrupt

/*
 * The Interrupt Handler for Port 4
 */
void Port_4(void) {

}


#pragma vector = RTC_VECTOR
__interrupt

/*
** Interrupt handler for the RTC_C
*/
void RTC_ISR(void) {
    int count = 0;
    switch(__even_in_range(RTCIV, 16)) {
        case RTCIV_RTCTEVIFG:
        // Checks to make sure at least 2 hours between watering.
            if (count == 2) {
                count = 0;
                WATERED = 0;
            } else count += 1;
            __bis_SR_register_on_exit(LPM3_bits);
            break;
        case RTCIV_RTCAIFG:
        // Every half hour, polls the sensors, if haven't watered in last 2 hours.
            if (WATERED == 0) STATE = POLLING;
            __bis_SR_register_on_exit(LPM3_bits);
            break;
    }
}
