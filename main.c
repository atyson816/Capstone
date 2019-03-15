// Capstone Design Project
// Authors:
// Stefan Heincke
// Ockert Strydom
// Austin Tyson
// Kendal Zimmer
#include "main.h"
// ========================= INIT BLOCK ===========================
void GPIO_INIT(void) {
    // Clear all floating for power consumption
    P1DIR = 0xFF;
    P1OUT = 0;
    P2DIR = 0xFF;
    P2OUT = 0;
    P3DIR = 0xFF;
    P3OUT = 0;
    P4DIR = 0xFF;
    P4OUT = 0;
    P5DIR = 0xFF;
    P5OUT = 0;
    P6DIR = 0xFF;
    P6OUT = 0;
    P7DIR = 0xFF;
    P7OUT = 0;
    P8DIR = 0xFF;
    P8OUT = 0;
    P9DIR = 0xFF;
    P9OUT = 0;
    P10DIR = 0xFF;
    P10OUT = 0;
    PJDIR = 0xFF;
    PJOUT = 0;
    // Sensor Outputs:
    // P2.0 is moisture_enable
    // P2.1 is temp_enable
    // p3.2 is valve_open
    // p9.4 is valve_close
    // Display Outputs
    // P1.3,4,6,7 .. P2.2,3,6 .. P3.0,1,3,6,7
    // Input Pins -- 5 User Buttons
    // P1.5 is Master Control
    // P2.4 is Up
    // P2.5 is Down
    // P2.7 is SEL
    // P4.7 is Back
    P1DIR &= ~ BIT5;
    P1REN = BIT5;
    P1OUT |= BIT5;
    P1IE = BIT5;
    P1IES &= ~BIT5;
    P2DIR &= ~ BIT4|BIT5|BIT7;
    P2REN = BIT4|BIT5|BIT7;
    P2OUT &= ~BIT4|BIT5|BIT7;
    P2OUT |= BIT4|BIT5|BIT7;
    P2IE = BIT4|BIT5|BIT7;
    P2IES &= ~BIT4|BIT5|BIT7;
    P4DIR &= ~ BIT7;
    P4REN = BIT7;
    P4OUT &= ~BIT7;
    P4OUT |= BIT7;
    P4IE = BIT7;
    P4IES &= ~BIT7;
    // Configuring P9.2 and 9.3 for ADC input sampling.
    P9DIR &= ~BIT2|BIT3;
    P9SEL1 |= BIT2|BIT3;
    P9SEL0 |= BIT2|BIT3;
    PJDIR &= ~BIT4|BIT5;
    PJSEL0 = BIT4|BIT5;
    // Disable GPIO power-on default high-impedance.
    P1IFG = 0;
    P2IFG = 0;
    P4IFG = 0;
    PM5CTL0 &= ~LOCKLPM5;
}

void ADC_INIT(void) {
    ADC12CTL0 &= ~ADC12ENC;
    ADC12CTL0 = ADC12SHT0_8 | ADC12MSC | ADC12ON;
    ADC12CTL1 = ADC12SSEL_2 | ADC12CONSEQ_3 | ADC12SHP | ADC12PDIV_2 | ADC12DIV_7;
    ADC12CTL2 = ADC12RES_1; //10 bit ADC
    ADC12CTL3 |= ADC12CSTARTADD_0;
    ADC12MCTL0 |= ADC12INCH_10; //9.2
    ADC12MCTL1 |= ADC12INCH_11 | ADC12EOS; //9.3
    ADC12IFGR0 &= ~ ADC12IFG0|ADC12IFG1;
    ADC12IER0 |= ADC12IE1;
    ADC12CTL0 |= ADC12ENC;
}

void TIMER_INIT(void) {
    TA0CCTL0 = CCIE;
    TA0CCR0 = 1000;
    TA0CTL = TASSEL_2 | MC_1;
    hd44780_clear_screen();
}

void RTC_INIT(void) {
    CSCTL0_H = CSKEY >> 8;
    CSCTL4 &= ~LFXTOFF;
    do {
        CSCTL5 &= ~LFXTOFFG;
        SFRIFG1 &= ~OFIFG;
    } while (SFRIFG1 & OFIFG);
    CSCTL0_H = 0;
    CURR_TIME.hourTen = 0;
    CURR_TIME.hourOne = 0;
    CURR_TIME.minOne = 0;
    CURR_TIME.minTen = 0;
    // Configure the RTC
    RTCCTL0_H = RTCKEY_H;
    RTCCTL0_L = RTCTEVIE | RTCAIE;
    RTCCTL0 &= ~RTCRDYIFG;
    RTCCTL1 =  RTCHOLD | RTCMODE | RTCTEV_0;
    RTCHOUR = (CURR_TIME.hourTen * 10) + CURR_TIME.hourOne;
    RTCMIN = (CURR_TIME.minTen * 10) + CURR_TIME.minOne;
    RTCAMIN = 30 | RTCAE;
    RTCCTL1 &= ~RTCHOLD;
}

void RTC_UPDATE(void) {
    RTCCTL1 |= RTCHOLD;
    RTCHOUR = (CURR_TIME.hourTen * 10) + CURR_TIME.hourOne;
    RTCMIN = (CURR_TIME.minTen * 10) + CURR_TIME.minOne;
    RTCCTL1 &= ~RTCHOLD;
}

// ======================== CONTROL BLOCK =========================
void currToUsrCompare(void) {
    if (MASTEROVERRIDE == 0) {
        if (WATERED == 0) {
            if (CURR_TEMP_MOIST.moisture < USR_TEMP_MOIST.moisture) {
                STATE = RUNNING;
            } else if (CURR_TEMP_MOIST.temperature < USR_TEMP_MOIST.temperature) {
                STATE = RUNNING;
            } else if (STATE == RUNNING) {
                valveClose();
                disableSensors();
                STATE = SLEEP;
            } else {
                disableSensors();
                STATE = SLEEP;
            }
        } else {
            STATE = SLEEP;
        }
    }
}

void valveOpen(void) {
    WATERED = 1;
    WATERING = 1;
    P9OUT |= BIT4;
    __delay_cycles(6000000);
    P9OUT &= ~BIT4;
}

void valveClose(void) {
    WATERING = 0;
    P3OUT |= BIT2;
    __delay_cycles(6000000);
    P3OUT &= ~BIT2;
}

void runADC(void) {
    int ii;
    volatile int tempM = 0, tempT = 0;
    if (TEMP_STATUS || MOIST_STATUS) {
        do {
            ADC12CTL0 |= ADC12SC;
            __bis_SR_register(LPM3_bits + GIE);
            __no_operation();
        } while (TEMPERATURE_DONE == 0 && MOISTURE_DONE == 0);
        for (ii = 29; ii >= 0; ii--) {
            tempM += MOISTURE[ii];
            tempT += TEMPERATURE[ii];
        }
        ADC12CTL0 &= ~ADC12SC;
        TEMPERATURE_DONE = 0;
        MOISTURE_DONE = 0;
        CURR_TEMP_MOIST.moisture = ((tempM/30) * 0.1);
        CURR_TEMP_MOIST.temperature = ((tempT/30 - 93) * 0.0538);
        __no_operation();
    }
}

void enableSensors(void) {
    if (TEMP_STATUS && MOIST_STATUS) {
        P2OUT |= BIT0|BIT1;
        __delay_cycles(500000);
    } else if (TEMP_STATUS) {
        P2OUT |= BIT1;
        __delay_cycles(500000);
    } else if (MOIST_STATUS) {
        P2OUT |= BIT0;
        __delay_cycles(500000);
    }
}

void disableSensors(void) {
    P2OUT &= ~ BIT0|BIT1;
    __delay_cycles(500000);
}

void stateCheck(void) {
    if (STATE == SLEEP) {
        display();
        __no_operation();
    } else if (STATE == POLLING) {
        enableSensors();
        runADC();
        display();
        currToUsrCompare();
    }  else if (STATE == RUNNING) {
        valveOpen();
        enableSensors();
        runADC();
        display();
        currToUsrCompare();
    }
}

void delay(void) {
    if (SEL == 1) __delay_cycles(300000);
    else if (SEL == 2) __delay_cycles(100000);
}

// ***************** DISPLAY BLOCK *****************
void display(void) {
    if (SCREEN == TIME) {
        if(CURSOR != 0){
            timeDisplayFlash();
        }
        else {
            timeDisplay();
        }
    } else if (SCREEN == TEMP) {
        if(CURSOR != 0){
            tempDisplayFlash();
        }
        else {
            tempDisplay();
        }
    } else if (SCREEN == MOIS) {
        if(CURSOR != 0){
            moisDisplayFlash();
        }
        else {
            moisDisplay();
        }
    }
}

void timeDisplay(void) {
    unsigned int hourTen, hourOne, minTen, minOne;
    hourTen = CURR_TIME.hourTen;
    hourOne = CURR_TIME.hourOne;
    minTen = CURR_TIME.minTen;
    minOne = CURR_TIME.minOne;
    hd44780_write_string("    TIME",1,1,NO_CR_LF);
    hd44780_output_unsigned_16bit_value(hourTen,2,1,9,NO_CR_LF);
    hd44780_output_unsigned_16bit_value(hourOne,2,1,10,NO_CR_LF);
    hd44780_write_string(" ",1,11,NO_CR_LF);
    hd44780_output_unsigned_16bit_value(minTen,2,1,12,NO_CR_LF);
    hd44780_output_unsigned_16bit_value(minOne,2,1,13,NO_CR_LF);
    hd44780_write_string("   ",1,14,NO_CR_LF);
    hd44780_write_string("    WATER ",2,1,NO_CR_LF);
    if (WATERING == 1) hd44780_write_string("ON ",2,11,NO_CR_LF);
    else if (WATERING == 0) hd44780_write_string("OFF",2,11,NO_CR_LF);
    hd44780_write_string("   ",2,14,NO_CR_LF);
    __no_operation();
    __bis_SR_register(GIE);
}

void timeDisplayFlash(void) {
    unsigned int hourTen, hourOne, minTen, minOne;
    hourTen = CURR_TIME.hourTen;
    hourOne = CURR_TIME.hourOne;
    minTen = CURR_TIME.minTen;
    minOne = CURR_TIME.minOne;
    if (CURSOR == 1){
        hd44780_write_string(" ",1,9,NO_CR_LF);
        delay();
        hd44780_output_unsigned_16bit_value(hourTen,2,1,9,NO_CR_LF);
        delay();
    }
    else if (CURSOR == 2){
        hd44780_write_string(" ",1,10,NO_CR_LF);
        delay();
        hd44780_output_unsigned_16bit_value(hourOne,2,1,10,NO_CR_LF);
        delay();
    }
    else if (CURSOR == 3){
        hd44780_write_string(" ",1,12,NO_CR_LF);
        delay();
        hd44780_output_unsigned_16bit_value(minTen,2,1,12,NO_CR_LF);
        delay();
    }
    else if (CURSOR == 4){
        hd44780_write_string(" ",1,13,NO_CR_LF);
        delay();
        hd44780_output_unsigned_16bit_value(minOne,2,1,13,NO_CR_LF);
        delay();
    }
    else if (CURSOR == 5){
        hd44780_write_string("   ",2,11,NO_CR_LF);
        delay();
        if (WATERING == 1) hd44780_write_string("ON ",2,11,NO_CR_LF);
        else if (WATERING == 0) hd44780_write_string("OFF",2,11,NO_CR_LF);
        delay();
    }
}

void tempDisplay(void) {
    int utemp, utemp10, utemp1;
    int ctemp, ctemp10, ctemp1;
    unsigned int hourTen, hourOne, minTen, minOne;
    utemp = USR_TEMP_MOIST.temperature;
    utemp10 = utemp / 10;
    utemp1 = utemp % 10;
    ctemp = CURR_TEMP_MOIST.temperature;
    ctemp10 = ctemp / 10;
    ctemp1 = ctemp % 10;
    hourTen = CURR_TIME.hourTen;
    hourOne = CURR_TIME.hourOne;
    minTen = CURR_TIME.minTen;
    minOne = CURR_TIME.minOne;
    //First Row
    hd44780_write_string("TEMP SET",1,1,NO_CR_LF);
    hd44780_write_string("+",1,9,NO_CR_LF);
    hd44780_output_unsigned_16bit_value(utemp10,2,1,10,NO_CR_LF);
    hd44780_output_unsigned_16bit_value(utemp1,2,1,11,NO_CR_LF);
    hd44780_write_string("C ",1,12,NO_CR_LF);
    if (TEMP_STATUS) hd44780_write_string("ON ",1,14,NO_CR_LF);
    else if (TEMP_STATUS == 0) hd44780_write_string("OFF",1,14,NO_CR_LF);

    //Second Row
    hd44780_write_string("CUR",2,1,NO_CR_LF);
    if (ctemp < 0) hd44780_write_string("-",2,4,NO_CR_LF);
    else if (ctemp >= 0) hd44780_write_string("+",2,4,NO_CR_LF);
    hd44780_output_unsigned_16bit_value(ctemp10,2,2,5,NO_CR_LF);
    hd44780_output_unsigned_16bit_value(ctemp1,2,2,6,NO_CR_LF);
    hd44780_write_string("C TIME",2,7,NO_CR_LF);
    hd44780_output_unsigned_16bit_value(hourTen,2,2,13,NO_CR_LF);
    hd44780_output_unsigned_16bit_value(hourOne,2,2,14,NO_CR_LF);
    hd44780_output_unsigned_16bit_value(minTen,2,2,15,NO_CR_LF);
    hd44780_output_unsigned_16bit_value(minOne,2,2,16,NO_CR_LF);
    __no_operation();
    __bis_SR_register(GIE);
}

void tempDisplayFlash(void) {
    unsigned int utemp, utemp10, utemp1;
    utemp = USR_TEMP_MOIST.temperature;
    utemp10 = utemp / 10;
    utemp1 = utemp % 10;
    if (CURSOR == 1) {
        hd44780_write_string(" ",1,10,NO_CR_LF);
        delay();
        hd44780_output_unsigned_16bit_value(utemp10,2,1,10,NO_CR_LF);
        hd44780_output_unsigned_16bit_value(utemp1,2,1,11,NO_CR_LF);
        delay();
    }
    else if (CURSOR == 2) {
        hd44780_write_string(" ",1,11,NO_CR_LF);
        delay();
        hd44780_output_unsigned_16bit_value(utemp10,2,1,10,NO_CR_LF);
        hd44780_output_unsigned_16bit_value(utemp1,2,1,11,NO_CR_LF);
        delay();
    }
    else if (CURSOR == 3) {
        hd44780_write_string("   ",1,14,NO_CR_LF);
        delay();
        if (TEMP_STATUS) hd44780_write_string("ON ",1,14,NO_CR_LF);
        else if (TEMP_STATUS == 0) hd44780_write_string("OFF",1,14,NO_CR_LF);
        delay();
    }
}

void moisDisplay(void) {
    unsigned int umois, cmois;
    unsigned int umois100, cmois100;
    unsigned int umois10, cmois10;
    unsigned int umois1, cmois1;
    unsigned int hourTen, hourOne, minTen, minOne;
    umois = USR_TEMP_MOIST.moisture;
    umois100 = umois / 100;
    umois10 = (umois % 100) / 10;
    umois1 = umois % 10;
    cmois = CURR_TEMP_MOIST.moisture;
    cmois100 = cmois / 100;
    cmois10 = (cmois % 100) / 10;
    cmois1 = cmois % 10;
    hourTen = CURR_TIME.hourTen;
    hourOne = CURR_TIME.hourOne;
    minTen = CURR_TIME.minTen;
    minOne = CURR_TIME.minOne;

    //First Row
    hd44780_write_string("MOIS SET",1,1,NO_CR_LF);
    hd44780_output_unsigned_16bit_value(umois100,2,1,9,NO_CR_LF);
    hd44780_output_unsigned_16bit_value(umois10,2,1,10,NO_CR_LF);
    hd44780_output_unsigned_16bit_value(umois1,2,1,11,NO_CR_LF);
    hd44780_write_string("% ",1,12,NO_CR_LF);
    if (MOIST_STATUS == 1) hd44780_write_string("ON ",1,14,NO_CR_LF);
    else if (MOIST_STATUS == 0) hd44780_write_string("OFF",1,14,NO_CR_LF);

    //Second Row
    hd44780_write_string("CUR",2,1,NO_CR_LF);
    hd44780_output_unsigned_16bit_value(cmois100,2,2,4,NO_CR_LF);
    hd44780_output_unsigned_16bit_value(cmois10,2,2,5,NO_CR_LF);
    hd44780_output_unsigned_16bit_value(cmois1,2,2,6,NO_CR_LF);
    hd44780_write_string("% TIME",2,7,NO_CR_LF);
    hd44780_output_unsigned_16bit_value(hourTen,2,2,13,NO_CR_LF);
    hd44780_output_unsigned_16bit_value(hourOne,2,2,14,NO_CR_LF);
    hd44780_output_unsigned_16bit_value(minTen,2,2,15,NO_CR_LF);
    hd44780_output_unsigned_16bit_value(minOne,2,2,16,NO_CR_LF);
    __no_operation();
    __bis_SR_register(GIE);
}

void moisDisplayFlash(void) {
    unsigned int mois;
    unsigned int mois100;
    unsigned int mois10;
    unsigned int mois1;
    mois = USR_TEMP_MOIST.moisture;
    mois100 = mois / 100;
    mois10 = (mois % 100) / 10;
    mois1 = mois % 10;
    if (CURSOR == 1) {
        hd44780_write_string(" ",1,9,NO_CR_LF);
        delay();
        hd44780_output_unsigned_16bit_value(mois100,2,1,9,NO_CR_LF);
        hd44780_output_unsigned_16bit_value(mois10,2,1,10,NO_CR_LF);
        hd44780_output_unsigned_16bit_value(mois1,2,1,11,NO_CR_LF);
        delay();
    }
    else if (CURSOR == 2) {
        hd44780_write_string(" ",1,10,NO_CR_LF);
        delay();
        hd44780_output_unsigned_16bit_value(mois100,2,1,9,NO_CR_LF);
        hd44780_output_unsigned_16bit_value(mois10,2,1,10,NO_CR_LF);
        hd44780_output_unsigned_16bit_value(mois1,2,1,11,NO_CR_LF);
        delay();
    }
    else if (CURSOR == 3) {
        hd44780_write_string(" ",1,11,NO_CR_LF);
        delay();
        hd44780_output_unsigned_16bit_value(mois100,2,1,9,NO_CR_LF);
        hd44780_output_unsigned_16bit_value(mois10,2,1,10,NO_CR_LF);
        hd44780_output_unsigned_16bit_value(mois1,2,1,11,NO_CR_LF);
        delay();
    }
    else if (CURSOR == 4){
        hd44780_write_string("   ",1,14,NO_CR_LF);
        delay();
        if (MOIST_STATUS == 1) hd44780_write_string("ON ",1,14,NO_CR_LF);
        else if (MOIST_STATUS == 0) hd44780_write_string("OFF",1,14,NO_CR_LF);
        delay();
    }
}

// **************** MAIN ******************
void main(void) {    // Disable Watchdog Timer while Initializing.
    WDTCTL = WDTPW | WDTHOLD;
    // Initialize on startup
    GPIO_INIT();
    ADC_INIT();
    TIMER_INIT();
    RTC_INIT();
    P1IFG = 0;
    P2IFG = 0;
    P4IFG = 0;
    while(1) {
        stateCheck();
    }
}

// ==================== INTERRUPTS ===================
#pragma vector = ADC12_VECTOR
__interrupt

/*
 * Interrupt Handler for the ADC Module
 */
void ADC12_ISR(void) {
    volatile unsigned int tRes, mRes, ii;
    switch(__even_in_range(ADC12IV,12)) {
    case 14:                            // Vector 14:  ADC12BMEM1
        if (TEMP_STATUS) {
            if (TEMPERATURE_DONE == 0) {
                tRes = ADC12MEM1;
                if (tSampleIdx == 30) {
                    TEMPERATURE_DONE = 1;
                    tSampleIdx = 0;
                    __no_operation();
                } else {
                    TEMPERATURE[tSampleIdx] = tRes;
                    tSampleIdx ++;
                    __no_operation();
                }
            }
        }
        if (MOIST_STATUS) {
            if (MOISTURE_DONE == 0) {
                mRes = ADC12MEM0;
                if (mSampleIdx == 30) {
                    MOISTURE_DONE = 1;
                    mSampleIdx = 0;
                    __no_operation();
                } else {
                    MOISTURE[mSampleIdx] = mRes;
                    mSampleIdx++;
                    __no_operation();
                }
            }
        }
        __bic_SR_register_on_exit(LPM3_bits | GIE);
        break;
    default: break;
    }
}

#pragma vector = TIMER0_A0_VECTOR
__interrupt

/*
 * Interrupt Handler for the Timer Module
 */
void TIMER0_A0_ISR(void) {
    hd44780_timer_isr();
    __no_operation();
}

#pragma vector = PORT1_VECTOR
__interrupt

/*
 * The Interrupt Handler for Port 1
 */
void Port_1(void) {
    P1IE = 0;
    __delay_cycles(100000);
    SEL = 0;
    CURSOR = 0;
    SCREEN = TIME;
    if (WATERING == 0) {
        valveOpen();
    } else if (WATERING == 1) {
        valveClose();
    }
    P1IFG = 0;
    P1IE |= BIT5;
    __delay_cycles(100000);
}

#pragma vector = PORT2_VECTOR
__interrupt

/*
 * The Interrupt Handler for Port 2
 */
void Port_2(void) {
    P2IE &= ~BIT4|BIT5|BIT7;
    __delay_cycles(100000);
    if (P2IFG & BIT4) { // UP
        P2IFG = 0;
        if (SEL == 1) { // In Screen
            if (CURSOR != 1) CURSOR--;
            else CURSOR = 1;
        } else if (SEL == 2) { // In Value
            if (SCREEN == TIME){
                if (CURSOR == 1){
                    if (CURR_TIME.hourTen < 2) CURR_TIME.hourTen ++;
                }
                else if (CURSOR == 2){
                    if ((CURR_TIME.hourTen == 2) && (CURR_TIME.hourOne > 3)) CURR_TIME.hourOne = 0;
                    else if ((CURR_TIME.hourTen == 2) && (CURR_TIME.hourOne < 3)) CURR_TIME.hourOne++;
                    else if ((CURR_TIME.hourTen < 2) && (CURR_TIME.hourOne < 9)) CURR_TIME.hourOne++;
                }
                else if (CURSOR == 3){
                    if(CURR_TIME.minTen < 5 ) CURR_TIME.minTen ++;
                }
                else if (CURSOR == 4){
                    if (CURR_TIME.minOne < 9) CURR_TIME.minOne++;
                }
                else if (CURSOR == 5){
                    if (WATERING == 0) {
                        MASTEROVERRIDE = 1;
                        STATE = RUNNING;
                        WATERING = 1;
                    }
                    else if (WATERING == 1) {
                        MASTEROVERRIDE = 0;
                        STATE = SLEEP;
                        WATERING = 0;
                    }
                }
            }
            else if (SCREEN == TEMP){
                if (CURSOR == 1){
                    if (USR_TEMP_MOIST.temperature/10 != 5) USR_TEMP_MOIST.temperature += 10;
                }
                else if (CURSOR == 2){
                    if (USR_TEMP_MOIST.temperature < 50) USR_TEMP_MOIST.temperature++;
                }
                else if (CURSOR == 3){
                    if(TEMP_STATUS == 1) TEMP_STATUS = 0;
                    else if (TEMP_STATUS == 0) TEMP_STATUS = 1;
                }
            }
            else if (SCREEN == MOIS) {
                if (CURSOR == 1) {
                    USR_TEMP_MOIST.moisture = 100;
                }
                else if (CURSOR == 2) {
                    if (USR_TEMP_MOIST.moisture < 90) USR_TEMP_MOIST.moisture += 10;
                    else USR_TEMP_MOIST.moisture = 100;
                }
                else if (CURSOR == 3) {
                    if(USR_TEMP_MOIST.moisture < 100) USR_TEMP_MOIST.moisture++;
                    else USR_TEMP_MOIST.moisture = 100;
                }
                else if (CURSOR == 4){
                    if(MOIST_STATUS == 1) MOIST_STATUS = 0;
                    else if (MOIST_STATUS == 0) MOIST_STATUS = 1;
                }
            }
        } else if (SEL == 0) { // Screen to Screen
            if (SCREEN == TIME) SCREEN = MOIS;
            else if (SCREEN == TEMP) SCREEN = TIME;
            else if (SCREEN == MOIS) SCREEN = TEMP;
        }
    } else if (P2IFG & BIT5) { // DOWN
        P2IFG = 0;
        __no_operation();
        if (SEL == 1) { // In Screen
            if (SCREEN == TIME && CURSOR != 5) CURSOR++;
            else CURSOR = CURSOR;
            if (SCREEN == MOIS && CURSOR != 4) CURSOR++;
            else CURSOR = CURSOR;
            if (SCREEN == TEMP && CURSOR != 3) CURSOR++;
            else CURSOR = CURSOR;
        } else if (SEL == 2) { // In Value
            if (SCREEN == TIME){
                if (CURSOR == 1) {
                    if (CURR_TIME.hourTen != 0) CURR_TIME.hourTen--;
                }
                else if (CURSOR == 2){
                    if(CURR_TIME.hourOne != 0) CURR_TIME.hourOne--;
                }
                else if (CURSOR == 3){
                    if(CURR_TIME.minTen != 0) CURR_TIME.minTen--;
                }
                else if (CURSOR == 4){
                    if(CURR_TIME.minOne != 0) CURR_TIME.minOne--;
                }
                else if (CURSOR == 5){
                    if (WATERING == 1) {
                        MASTEROVERRIDE = 0;
                        STATE = SLEEP;
                        WATERING = 0;
                    }
                    else if (WATERING == 0) {
                        MASTEROVERRIDE = 1;
                        STATE = RUNNING;
                        WATERING = 1;
                    }
                }
            }
            else if (SCREEN == TEMP){
                if (CURSOR == 1){
                    if (USR_TEMP_MOIST.temperature > 9) USR_TEMP_MOIST.temperature-= 10;
                }
                else if (CURSOR == 2){
                    if(USR_TEMP_MOIST.temperature > 0) USR_TEMP_MOIST.temperature--;
                }
                else if (CURSOR == 3){
                    if(TEMP_STATUS) TEMP_STATUS = 0;
                    else if (TEMP_STATUS == 0) TEMP_STATUS = 1;
                }
            }
            else if (SCREEN == MOIS){
                if (CURSOR == 1){
                    if (USR_TEMP_MOIST.moisture == 100) USR_TEMP_MOIST.moisture = 0;
                }
                else if (CURSOR == 2){
                    if(USR_TEMP_MOIST.moisture > 9) USR_TEMP_MOIST.moisture -= 10;
                }
                else if (CURSOR == 3){
                    if(USR_TEMP_MOIST.moisture > 0) USR_TEMP_MOIST.moisture--;
                }
                else if (CURSOR == 4){
                    if(MOIST_STATUS == 1) MOIST_STATUS = 0;
                    else if (MOIST_STATUS == 0) MOIST_STATUS = 1;
                }
            }
        } else { // Screen to Screen
            if (SCREEN == TIME) SCREEN = TEMP;
            else if (SCREEN == TEMP) SCREEN = MOIS;
            else if (SCREEN == MOIS) SCREEN = TIME;
        }
    } else if (P2IFG & BIT7) { // SELECT
        P2IFG = 0;
        __no_operation();
        if (SEL == 1) SEL = 2;
        else if (SEL == 0) {
            CURSOR = 1;
            SEL = 1;
        }
        else SEL = 2;
    }
    __delay_cycles(100000);
    __no_operation();
    P2IE = BIT4|BIT5|BIT7;
}

#pragma vector = PORT4_VECTOR
__interrupt

/*
 * The Interrupt Handler for Port 4
 */
void Port_4(void) {
    P4IFG = 0;
    __delay_cycles(100000);
    if (SEL != 0) {
        if (SEL == 1) CURSOR = 0;
        SEL--;
    }
    __delay_cycles(100000);
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
            if (WATERED == 1) {
                if (count == 1) {
                    count = 0;
                    WATERED = 0;
                } else {
                    count += 1;
                }
            } else if (WATERED == 0) {
                count = 0;
            }
            RTC_UPDATE();
            __bis_SR_register_on_exit(LPM3_bits);
            break;
        case RTCIV_RTCAIFG:
        // Every half hour, polls the sensors, if haven't watered in last 2 hours.
            if (WATERED == 0) STATE = POLLING;
            if ((RTCAMIN & 0x3F) == 30) RTCAMIN = 60 | 0x80;
            else RTCAMIN = 30 | 0x80;
            __bis_SR_register_on_exit(LPM3_bits);
            break;
    }
}
