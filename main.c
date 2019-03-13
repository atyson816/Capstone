// Capstone Design Project
// Authors:
// Stefan Heincke
// Ockert Strydom
// Austin Tyson
// Kendal Zimmer
#include "main.h"
// ========================= INIT BLOCK ===========================
void init(void) {
    volatile unsigned int minute, hour, temperature_threshold, moisture_threshold;
    SCREEN = FIRST;
    display();
    //TODO Get all the Initial Values before the system will even begin.
    CURR_TIME.hourOne;
    CURR_TIME.hourTen;
    CURR_TIME.minOne;
    CURR_TIME.minTen;
    USR_TEMP_MOIST.moisture = moisture_threshold;
    USR_TEMP_MOIST.temperature = temperature_threshold;
    STATE = POLLING;
}

void RTC_INIT(void) {
    CSCTL0_H = CSKEY >> 8;
    CSCTL4 &= ~LFXTOFF;
    do {
        CSCTL5 &= ~LFXTOFFG;
        SFRIFG1 &= ~OFIFG;
    } while (SFRIFG1 & OFIFG);
    CSCTL0_H = 0;
    // Configure the RTC
    RTCCTL0_H = RTCKEY_H;
    RTCCTL0_L = RTCTEVIE | RTCAIE;
    RTCCTL1 =  RTCHOLD | RTCMODE;
    RTCHOUR = (CURR_TIME.hourTen * 10) + CURR_TIME.hourOne;
    RTCMIN = (CURR_TIME.minTen * 10) + CURR_TIME.minOne;
    RTCAMIN = 30 | RTCAE;
}
// ========================= HELPER BLOCK =========================

//TODO Implement this
//void currToUsrCompare(void) {
//    return;
//}

/*
 * This function opens the valve, called by currToUsrCompare
 */
void valveOpen(void) {
    P2OUT |= BIT5;
    __delay_cycles(6000000);
    P2OUT &= ~BIT5;
    WATERED = 1;
}

/*
 * This function closes the valve, will be called by currToUsrCompare
 */
void valveClose(void) {
    P3OUT |= BIT2;
    __delay_cycles(6000000);
    P3OUT &= ~BIT2;
}

// ========================= GPIO BLOCK =========================
/*
 * This Function Initializes the GPIO Ports for the buttons, and display, as
 * well as the sensor enables and valve control.
 */
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
    P1REN = BIT5;
    P1DIR &= ~ BIT5;
    P1OUT &= ~BIT5;
    P1IE = BIT5;
    P1IES = BIT5;
    P1IFG = 0;
    P2DIR &= ~ BIT4|BIT5|BIT7;
    P2REN = BIT4|BIT5|BIT7;
    P2OUT &= ~BIT4|BIT5|BIT7;
    P2IE = BIT4|BIT5|BIT7;
    P2IES = BIT4|BIT5|BIT7;
    P2IFG = 0;
    P4DIR &= ~ BIT7;
    P4REN = BIT7;
    P4OUT &= ~BIT7;
    P4IE = BIT7;
    P4IES = BIT7;
    P4IFG = 0;
    // Configuring P9.2 and 9.3 for ADC input sampling.
    P9DIR &= ~ BIT1|BIT2;
    P9SEL1 |= BIT1|BIT2;
    P9SEL0 |= BIT1|BIT2;
    PJDIR &= ~BIT4|BIT5;
    PJSEL0 = BIT4|BIT5;
    // Disable GPIO power-on default high-impedance.
    PM5CTL0 &= ~LOCKLPM5;
}


// ========================= ADC BLOCK =========================
/*
 * This function initializes the ADC for operation
 */
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

void runADC(void) {
    if (TEMP_STATUS || MOIST_STATUS) {
        do {
            ADC12CTL0 |= ADC12SC;
            __bis_SR_register(LPM3_bits + GIE);
            __no_operation();
        } while (TEMPERATURE_DONE == 0 && MOISTURE_DONE == 0);
        __no_operation();
    }
}

/*
 * This function turns on the temperature and moisture sensors, called by
 * currToUsrCompare()
 */
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

/*
 * This function will turn off the temperature and moisture sensors, and will
 * most likely be used by currToUsrCompare()
 */
void disableSensors(void) {
    P2OUT &= ~ BIT0|BIT1;
    __delay_cycles(500000);
}

/*
 * This Function is the main driver for the system, controlling which
 * functions occur when.
 */
void stateCheck(void) {
    if (STATE == INIT) {
        init();
    } else if (STATE == SLEEP) {
        __bis_SR_register(LPM3_bits + GIE);
        __no_operation();
    } else if (STATE == POLLING) {
        enableSensors();
        runADC();
        //TODO Add The Comparing Function
        //TODO
    }  else if (STATE == RUNNING) {
        //TODO Make the ADC actually Run to obtain the values
        //TODO Add the Comparing Function
        valveOpen();
        runADC();
    }
}

void TIMER_INIT(void) {
    TA0CCTL0 = CCIE;
    TA0CCR0 = 1000;
    TA0CTL = TASSEL_2 | MC_1;
    hd44780_clear_screen();
}

void timeDisplay(void){
    uint16_t testCURR_TIMEhourTen = 2;
    uint16_t testCURR_TIMEhourOne = 4;
    uint16_t testCURR_TIMEminTen = 5;
    uint16_t testCURR_TIMEminOne = 9;
    char* waterToggle = "ON";
    hd44780_write_string("    TIME",1,1,NO_CR_LF);
    hd44780_output_unsigned_16bit_value(testCURR_TIMEhourTen,2,1,9,NO_CR_LF);
    hd44780_output_unsigned_16bit_value(testCURR_TIMEhourOne,2,1,10,NO_CR_LF);
    hd44780_write_string(" ",1,11,NO_CR_LF);
    hd44780_output_unsigned_16bit_value(testCURR_TIMEminTen,2,1,12,NO_CR_LF);
    hd44780_output_unsigned_16bit_value(testCURR_TIMEminOne,2,1,13,NO_CR_LF);
    hd44780_write_string("   ",1,14,NO_CR_LF);
    hd44780_write_string("    WATER",2,1,NO_CR_LF);
    hd44780_write_string(waterToggle,2,11,NO_CR_LF);
    hd44780_write_string("    ",2,13,NO_CR_LF);
    __no_operation();
    __bis_SR_register(GIE);
}

void timeDisplayFlash(void){
    uint16_t testCURR_TIMEhourTen = 2;
    uint16_t testCURR_TIMEhourOne = 4;
    uint16_t testCURR_TIMEminTen = 5;
    uint16_t testCURR_TIMEminOne = 9;
    char* waterToggle = "ON";
    if (CURSOR == 1){
        hd44780_write_string(" ",1,9,NO_CR_LF);
        __delay_cycles(300000);
        hd44780_output_unsigned_16bit_value(testCURR_TIMEhourTen,2,1,9,NO_CR_LF);
        __delay_cycles(300000);
    }
    else if (CURSOR == 2){
        hd44780_write_string(" ",1,10,NO_CR_LF);
        __delay_cycles(300000);
        hd44780_output_unsigned_16bit_value(testCURR_TIMEhourOne,2,1,10,NO_CR_LF);
        __delay_cycles(300000);
    }
    else if (CURSOR == 3){
        hd44780_write_string(" ",1,12,NO_CR_LF);
         __delay_cycles(300000);
        hd44780_output_unsigned_16bit_value(testCURR_TIMEminTen,2,1,12,NO_CR_LF);
        __delay_cycles(300000);
    }
    else if (CURSOR == 4){
        hd44780_write_string(" ",1,13,NO_CR_LF);
        __delay_cycles(300000);
        hd44780_output_unsigned_16bit_value(testCURR_TIMEminOne,2,1,13,NO_CR_LF);
        __delay_cycles(300000);
    }
    else if (CURSOR == 5){
        hd44780_write_string("   ",2,11,NO_CR_LF);
        __delay_cycles(300000);
        hd44780_write_string(waterToggle,2,11,NO_CR_LF);
        __delay_cycles(300000);
    }
}
void tempDisplay(void){
    double doubletestSetTemp10 = 5.6;
    uint16_t testSetTemp10 = doubletestSetTemp10;
    uint16_t testSetTemp1 = 0;
    uint16_t testCurTemp10 = 8;
    uint16_t testCurTemp1 = 1;
    uint16_t testCURR_TIMEhourTen = 2;
    uint16_t testCURR_TIMEhourOne = 4;
    uint16_t testCURR_TIMEminTen = 5;
    uint16_t testCURR_TIMEminOne = 9;
    char* plusOrMinus = "-";
    char* TEMP_STATUS = "OFF";

    //First Row
    hd44780_write_string("TEMP SET+",1,1,NO_CR_LF);
    hd44780_output_unsigned_16bit_value(testSetTemp10,2,1,10,NO_CR_LF);
    hd44780_output_unsigned_16bit_value(testSetTemp1,2,1,11,NO_CR_LF);
    hd44780_write_string("C ",1,12,NO_CR_LF);
    hd44780_write_string(TEMP_STATUS,1,14,NO_CR_LF);

    //Second Row
    hd44780_write_string("CUR",2,1,NO_CR_LF);
    hd44780_write_string(plusOrMinus,2,4,NO_CR_LF);
    hd44780_output_unsigned_16bit_value(testCurTemp10,2,2,5,NO_CR_LF);
    hd44780_output_unsigned_16bit_value(testCurTemp1,2,2,6,NO_CR_LF);
    hd44780_write_string("C TIME",2,7,NO_CR_LF);
    hd44780_output_unsigned_16bit_value(testCURR_TIMEhourTen,2,2,13,NO_CR_LF);
    hd44780_output_unsigned_16bit_value(testCURR_TIMEhourOne,2,2,14,NO_CR_LF);
    hd44780_output_unsigned_16bit_value(testCURR_TIMEminTen,2,2,15,NO_CR_LF);
    hd44780_output_unsigned_16bit_value(testCURR_TIMEminOne,2,2,16,NO_CR_LF);
    __no_operation();
    __bis_SR_register(GIE);
}
void tempDisplayFlash(void){
    uint16_t testSetTemp10 = 9;
    uint16_t testSetTemp1 = 0;
    char* TEMP_STATUS = "OFF";
    if (CURSOR == 1){
        hd44780_write_string(" ",1,9,NO_CR_LF);
        __delay_cycles(300000);
        hd44780_output_unsigned_16bit_value(testSetTemp10,2,1,10,NO_CR_LF);
        __delay_cycles(300000);
    }
    else if (CURSOR == 2){
        hd44780_write_string(" ",1,10,NO_CR_LF);
        __delay_cycles(300000);
        hd44780_output_unsigned_16bit_value(testSetTemp1,2,1,11,NO_CR_LF);
        __delay_cycles(300000);
    }
    else if (CURSOR == 3){
        hd44780_write_string("   ",1,14,NO_CR_LF);
         __delay_cycles(300000);
         hd44780_write_string(TEMP_STATUS,1,14,NO_CR_LF);
        __delay_cycles(300000);
    }
}

void moisDisplay(void){
    unsigned int umois;
    unsigned int umois100;
    unsigned int umois10;
    unsigned int umois1;
    umois = USR_TEMP_MOIST.moisture;
    umois100 = umois / 100;
    umois10 = (umois % 100) / 10;
    umois1 = umois % 10;
    uint16_t testCurMois100 = 0;
    uint16_t testCurMois10 = 5;
    uint16_t testCurMois1 = 6;
    uint16_t testCURR_TIMEhourTen = 2;
    uint16_t testCURR_TIMEhourOne = 3;
    uint16_t testCURR_TIMEminTen = 5;
    uint16_t testCURR_TIMEminOne = 9;

    //First Row
    hd44780_write_string("MOIS SET",1,1,NO_CR_LF);
    hd44780_output_unsigned_16bit_value(umois100,2,1,9,NO_CR_LF);
    hd44780_output_unsigned_16bit_value(umois10,2,1,10,NO_CR_LF);
    hd44780_output_unsigned_16bit_value(umois1,2,1,11,NO_CR_LF);
    hd44780_write_string("% ",1,12,NO_CR_LF);
    if (MOIST_STATUS == 1) hd44780_write_string("ON",1,14,NO_CR_LF);
    else if (MOIST_STATUS == 0) hd44780_write_string("OFF",1,14,NO_CR_LF);

    //Second Row
    hd44780_write_string("CUR",2,1,NO_CR_LF);
    hd44780_output_unsigned_16bit_value(testCurMois100,2,2,4,NO_CR_LF);
    hd44780_output_unsigned_16bit_value(testCurMois10,2,2,5,NO_CR_LF);
    hd44780_output_unsigned_16bit_value(testCurMois1,2,2,6,NO_CR_LF);
    hd44780_write_string("% TIME",2,7,NO_CR_LF);
    hd44780_output_unsigned_16bit_value(testCURR_TIMEhourTen,2,2,13,NO_CR_LF);
    hd44780_output_unsigned_16bit_value(testCURR_TIMEhourOne,2,2,14,NO_CR_LF);
    hd44780_output_unsigned_16bit_value(testCURR_TIMEminTen,2,2,15,NO_CR_LF);
    hd44780_output_unsigned_16bit_value(testCURR_TIMEminOne,2,2,16,NO_CR_LF);
    __no_operation();
    __bis_SR_register(GIE);
}

void moisDisplayFlash(void){
    unsigned int mois;
    unsigned int mois100;
    unsigned int mois10;
    unsigned int mois1;
    mois = USR_TEMP_MOIST.moisture;
    mois100 = mois / 100;
    mois10 = (mois % 100) / 10;
    mois1 = mois % 10;
    char* moisToggle = "OFF";
    if (CURSOR == 1){
        hd44780_write_string(" ",1,9,NO_CR_LF);
        __delay_cycles(300000);
        hd44780_output_unsigned_16bit_value(mois100,2,1,9,NO_CR_LF);
        __delay_cycles(300000);
    }
    else if (CURSOR == 2){
        hd44780_write_string(" ",1,10,NO_CR_LF);
        __delay_cycles(300000);
        hd44780_output_unsigned_16bit_value(mois10,2,1,10,NO_CR_LF);
        __delay_cycles(300000);
    }
    else if (CURSOR == 3){
        hd44780_write_string(" ",1,11,NO_CR_LF);
        __delay_cycles(300000);
        hd44780_output_unsigned_16bit_value(mois1,2,1,11,NO_CR_LF);
        __delay_cycles(300000);
    }
    else if (CURSOR == 4){
        hd44780_write_string("   ",1,14,NO_CR_LF);
         __delay_cycles(300000);
         hd44780_write_string(moisToggle,1,14,NO_CR_LF);
        __delay_cycles(300000);
    }
}

void display(void) {

    if (SCREEN == FIRST) {
        SCREEN = MOIS; //Current screen that is being tested
        moisDisplay(); //Prints screen once in case you are flashing a part and haven't initialized the screen
    } else if (SCREEN == TIME) {
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

void main(void) {
    volatile unsigned int temp;
    // Disable Watchdog Timer while Initializing.

    WDTCTL = WDTPW | WDTHOLD;
    // Initialize on startup
    GPIO_INIT();
    ADC_INIT();
    TIMER_INIT();
    RTC_INIT();
    SCREEN = FIRST;
    while(1) {
        display();
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
    volatile unsigned int tRes, mRes, ii;
    switch(__even_in_range(ADC12IV,12)) {
    case  0: break;                     // Vector  0:  No interrupt
    case  2: break;                     // Vector  2:  ADC12BMEMx Overflow
    case  4: break;                     // Vector  4:  Conversion time overflow
    case  6: break;                     // Vector  6:  ADC12BHI
    case  8: break;                     // Vector  8:  ADC12BLO
    case 10: break;                     // Vector 10:  ADC12BIN
    case 12:                            // Vector 12:  ADC12BMEM0
    case 14:                            // Vector 14:  ADC12BMEM1
        if (TEMP_STATUS) {
            if (TEMPERATURE_DONE == 0) {
                tRes = ADC12MEM1;
                if (tSampleIdx == 29) {
                    TEMPERATURE_DONE = 1;
                    tSampleIdx = 0;
                    for (ii = 29; ii > 0; ii--) {
                        tRes += TEMPERATURE[ii];
                    }
                    CURR_TEMP_MOIST.temperature = (((tRes/29) * 0.0033) / 0.06);
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
                if (mSampleIdx == 29) {
                    MOISTURE_DONE = 1;
                    mSampleIdx = 0;
                    for (ii = 29; ii > 0; ii--) {
                        mRes += MOISTURE[ii];
                    }
                    CURR_TEMP_MOIST.moisture = (((mRes/29) * 0.0033) / 0.033);
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
}

#pragma vector = PORT1_VECTOR
__interrupt

/*
 * The Interrupt Handler for Port 1
 */
void Port_1(void) {
    //SEL = 0;
    P1IFG &= ~BIT5;
}

#pragma vector = PORT2_VECTOR
__interrupt

/*
 * The Interrupt Handler for Port 2
 */
void Port_2(void) {
    if (P2IFG & BIT4) { // UP
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
                /*else if (CURSOR == 5){
                    if(waterToggle == "OFF") waterToggle == "ON";
                    else if (waterToggle == "ON") waterToggle == "OFF";
                }*/
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
        P2IFG = 0;
    } else if (P2IFG & BIT5) { // DOWN
        if (SEL == 1) { // In Screen
            if (SCREEN == TIME && CURSOR != 5) CURSOR++;
            else CURSOR = CURSOR;
            if (SCREEN == MOIS && CURSOR != 4) CURSOR++;
            else CURSOR = CURSOR;
            if (SCREEN == TEMP && CURSOR != 3) CURSOR++;
            else CURSOR = CURSOR;
        } else if (SEL == 2) { // In Value
            if (SCREEN == TIME){
                if (CURSOR == 1){
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
                /*else if (CURSOR == 5){
                    if(waterToggle == "OFF") waterToggle == "ON";
                    else if (waterToggle == "ON") waterToggle == "OFF";
                }*/
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
        P2IFG = 0;
    } else if (P2IFG & BIT7) { // SELECT
        if (SEL == 1) SEL = 2;
        else if (SEL == 0) SEL = 1;
        else SEL = 2;
        P2IFG = 0;
    }

}

#pragma vector = PORT4_VECTOR
__interrupt

/*
 * The Interrupt Handler for Port 4
 */
void Port_4(void) {
    //if (SEL != 0) SEL--;
    P4IFG &= ~BIT7;
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
                if (count == 2) {
                    count = 0;
                    WATERED = 0;
                } else {
                    count += 1;
                }
            } else if (WATERED == 0) {
                count = 0;
            }
            __bis_SR_register_on_exit(LPM3_bits);
            break;
        case RTCIV_RTCAIFG:
        // Every half hour, polls the sensors, if haven't watered in last 2 hours.
            if (WATERED == 0) STATE = POLLING;
            if ((RTCAMIN & 0x63) == 30) RTCAMIN = 60 | 0x80;
            else RTCAMIN = 30 | 0x80;
            __bis_SR_register_on_exit(LPM3_bits);
            break;
    }
}
