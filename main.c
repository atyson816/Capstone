// Capstone Design Project
// Authors:
// Stefan Heincke
// Ockert Strydom
// Austin Tyson
// Kendal Zimmer
#include "main.h"
// ========================= HELPER BLOCK =========================
int currToUsrCompare(void) {
    return 0;
}

int valveOpen(void) {
    return 0;
}
// ========================= GPIO BLOCK =========================
/*
 * This Function Initializes the GPIO Ports for the buttons, Display, and
 * the ternary functions for the ADC.
 */
void GPIO_INIT(void) {
        //GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_,GPIO_PIN);
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
 * This Function Configures the ADC for use with the Moisture Sensor and also will begin the conversion for the ADC.
 */
void ADC_CTRL(void) {
        ADC12_B_setupSamplingTimer(ADC12_B_BASE,
                                   ADC12_B_CYCLEHOLD_16_CYCLES,
                                   ADC12_B_CYCLEHOLD_16_CYCLES,
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
        ADC12_B_enableInterrupt(ADC12_B_BASE, ADC12_B_IE0 | ADC12_B_IE1, 0, 0);
}

/*
 * This Function is the main driver for the system, controlling which
 * functions occur when.
 */
void STATE_CHECK(void) {
        if (STATE == SLEEP) {
                // Puts the Device to sleep waiting on Interrupts.
                __bis_SR_register(LPM3_bits + GIE);
                __no_operation();
        } else if (STATE == POLL) {
                ADC_CTRL();
                //TODO Make the ADC actually Run to obtain the values
                //TODO Add The Comparing Function
                //TODO
        } else if (STATE == INIT) {
                //TODO Get the User's Information (INIT Function)
                //TODO
        } else if (STATE == RUNNING) {
                //TODO Make the ADC actually Run to obtain the values
                //TODO Add the Comparing Function
                //TODO Add the Running Function
        }
}

void main(void) {
        // Sets Frequency to 1MHz low, 1MHz high
        CS_setDCOFreq(CS_DCORSEL_0, CS_DCOFSEL_0);
        // Sets Master Clock (System and CPU clock to 1 MHz)
        CS_initClockSignal(CS_MCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
        // Disable Watchdog Timer while Initializing.
        WDT_A_hold(WDT_A_BASE);
        // Call GPIO_INIT before ADC Because GPIO formats for ADC use
        GPIO_INIT();
        ADC_INIT();
        while(1) {
                // State Check Call/Switch
                STATE_CHECK();
        }
}

// ==================== INTERRUPTS ===================
#pragma vector = ADC12_VECTOR
__interrupt

/*
 * Interrupt Handler for the ADC Module
 */
void ADC12_ISR(void) {
        unsigned int res;
        unsigned int ii;
        unsigned int currM;
        unsigned int currT;
        switch(__even_in_range(ADC12IV,12)) {
        case  0: break;                     // Vector  0:  No interrupt
        case  2: break;                     // Vector  2:  ADC12BMEMx Overflow
        case  4: break;                     // Vector  4:  Conversion time overflow
        case  6: break;                     // Vector  6:  ADC12BHI
        case  8: break;                     // Vector  8:  ADC12BLO
        case 10: break;                     // Vector 10:  ADC12BIN
        case 12:                            // Vector 12:  ADC12BMEM0
                currM = 48;
                res = ADC12_B_getResults(ADC12_B_BASE, ADC12_B_MEMORY_0);
                if (currM == 0) {
                    for (ii = 0; ii > 0; ii--) {
                        res += MOISTURE[ii];
                    }
                    CURR_TEMP_MOIST.moisture = res/48;
                } else {
                    MOISTURE[currM] = res;
                    currM --;
                }
                __bic_SR_register_on_exit(LPM3_bits);
        case 14:                         // Vector 14:  ADC12BMEM1
                currT = 48;
                res = ADC12_B_getResults(ADC12_B_BASE, ADC12_B_MEMORY_1);
                if (currT == 0) {
                    for (ii = 0; ii > 0; ii--) {
                        res += TEMPERATURE[ii];
                    }
                    CURR_TEMP_MOIST.temperature = res/48;
                } else {
                    TEMPERATURE[currT] = res;
                    currT --;
                }
                __bic_SR_register_on_exit(LPM3_bits);
        case 76: break;                     // Vector 76:  ADC12BRDY
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
