// Capstone Design Project
// Authors:
// Stefan Heincke
// Ockert Strydom
// Austin Tyson
// Kendal Zimmer

// enumeration to define the three different states
enum currentState {
    STANDBY,
    PANDC,
    RUNNING
};

// structure of the storage of temperatures attributes
struct temperature {
    float currentTemp;
    float userTemp;
};

// structure of the storage of temperatures attributes
struct moisture {
    float currentMoisture;
    float userMoisture;
};

main()
{
/*
 * TODO
 * Initialize display and create a function to display a startup
 * message and ask for TIME:DAY:MONTH and userTemp and userMoisture
 */

/*
 * The standby state is entered when the device is done watering
 * and is left upon user or timer interrupt
 */
while (currentState = STANDBY) {
    /*
     * TODO
     * Enter sleep mode and change state when interrupted via timer
     * or user interrupt
     */
}

/*
 * The running state is entered upon interrupt and checks
 * whether or not the system has run recently and if the moisture
 * and temperature values are within user defined thresholds.
 */
while (currentState = PANDC) {
    if (wateredRecently) {
        currentState = STANDBY;
    }
    /*
     * TODO
     * ENABLE TEMP/MOISTURE SENSOR AND ADC SAMPLING
     * COLLECT RUNNING AVERAGE HERE OR CALL RUNNING AVERAGE FUNCTION
     * LOAD THE VALUE INTO CURRENT TEMP/CURRENT MOISTURE
     */
    if (currentTemp > userTemp){
        currentState =  STANDBY;
        /* DISABLE TEMP SENSOR AND ADC SAMPLING */
    }
    else if (currentTemp < userTemp){
        currentState = RUNNING;
    }
    if (currentMoisture > userMoisture){
        currentState =  STANDBY;
        /* DISABLE MOISTURE SENSOR AND ADC SAMPLING */
    }
    else if (currentMoisture < userMoisture){
        currentState = RUNNING;
        }
}

while (currentState = RUNNING) {
    return 3
}
}
