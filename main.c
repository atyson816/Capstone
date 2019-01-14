// Capstone Design Project
// Authors:
// Stefan Heincke
// Ockert Strydom
// Austin Tyson
// Kendal Zimmer
#include "driverlib.h"
#include "main.h"
// ========================= HELPER BLOCK =========================

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
        ADC12_B_configureMemoryParam memParam = {0};
        memParam.memoryBufferControlIndex = ADC12_B_MEMORY_0;
        memParam.inputSourceSelect = ADC12_B_INPUT_A10; //Whatever the moisture sensor's analog wiring is.
        memParam.refVoltageSourceSelect = ADC12_B_VREFPOS_AVCC_VREFNEG_VSS;
        memParam.endOfSequence = ADC12_B_NOTENDOFSEQUENCE;
        memParam.windowComparatorSelect = ADC12_B_WINDOW_COMPARATOR_DISABLE;
        memParam.differentialModeSelect = ADC12_B_DIFFERENTIAL_MODE_DISABLE;
        ADC12_B_configureMemory(ADC12_B_BASE, &memParam);

        ADC12_B_configureMemoryParam memParam1 = {0};
        memParam.memoryBufferControlIndex = ADC12_B_MEMORY_1;
        memParam.inputSourceSelect = ADC12_B_INPUT_A11; //Whatever the temperature sensor's analog wiring is.
        memParam.refVoltageSourceSelect = ADC12_B_VREFPOS_AVCC_VREFNEG_VSS;
        memParam.endOfSequence = ADC12_B_ENDOFSEQUENCE;
        memParam.windowComparatorSelect = ADC12_B_WINDOW_COMPARATOR_DISABLE;
        memParam.differentialModeSelect = ADC12_B_DIFFERENTIAL_MODE_DISABLE;
        ADC12_B_configureMemory(ADC12_B_BASE, &memParam1);

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
        } else if (STATE == INIT) {
                //INIT(
        } else if (STATE == RUNNING) {

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
        int res;
        int ii;
        switch(__even_in_range(ADC12IV,12)) {
        case  0: break;                     // Vector  0:  No interrupt
        case  2: break;                     // Vector  2:  ADC12BMEMx Overflow
        case  4: break;                     // Vector  4:  Conversion time overflow
        case  6: break;                     // Vector  6:  ADC12BHI
        case  8: break;                     // Vector  8:  ADC12BLO
        case 10: break;                     // Vector 10:  ADC12BIN
        case 12:                            // Vector 12:  ADC12BMEM0
                res = ADC12_B_getResults(ADC12_B_BASE, ADC12_B_MEMORY_0);
                if (STATE == POLLM) {
                        if (currM == 50) {
                                for (ii = 0; ii < 50; ii++) {
                                        res += MOISTURE[ii];
                                }
                                CURR_TEMP_MOIST.moisture = res/50;
                        }
                } else if (STATE == POLLT) {

                }
                __bic_SR_register_on_exit(LPM3_bits);
        case 14: break;                     // Vector 14:  ADC12BMEM1
        case 16: break;                     // Vector 16:  ADC12BMEM2
        case 18: break;                     // Vector 18:  ADC12BMEM3
        case 20: break;                     // Vector 20:  ADC12BMEM4
        case 22: break;                     // Vector 22:  ADC12BMEM5
        case 24: break;                     // Vector 24:  ADC12BMEM6
        case 26: break;                     // Vector 26:  ADC12BMEM7
        case 28: break;                     // Vector 28:  ADC12BMEM8
        case 30: break;                     // Vector 30:  ADC12BMEM9
        case 32: break;                     // Vector 32:  ADC12BMEM10
        case 34: break;                     // Vector 34:  ADC12BMEM11
        case 36: break;                     // Vector 36:  ADC12BMEM12
        case 38: break;                     // Vector 38:  ADC12BMEM13
        case 40: break;                     // Vector 40:  ADC12BMEM14
        case 42: break;                     // Vector 42:  ADC12BMEM15
        case 44: break;                     // Vector 44:  ADC12BMEM16
        case 46: break;                     // Vector 46:  ADC12BMEM17
        case 48: break;                     // Vector 48:  ADC12BMEM18
        case 50: break;                     // Vector 50:  ADC12BMEM19
        case 52: break;                     // Vector 52:  ADC12BMEM20
        case 54: break;                     // Vector 54:  ADC12BMEM21
        case 56: break;                     // Vector 56:  ADC12BMEM22
        case 58: break;                     // Vector 58:  ADC12BMEM23
        case 60: break;                     // Vector 60:  ADC12BMEM24
        case 62: break;                     // Vector 62:  ADC12BMEM25
        case 64: break;                     // Vector 64:  ADC12BMEM26
        case 66: break;                     // Vector 66:  ADC12BMEM27
        case 68: break;                     // Vector 68:  ADC12BMEM28
        case 70: break;                     // Vector 70:  ADC12BMEM29
        case 72: break;                     // Vector 72:  ADC12BMEM30
        case 74: break;                     // Vector 74:  ADC12BMEM31
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
