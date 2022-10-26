/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#include "Particle.h"
#line 1 "c:/Users/bobg5/Documents/GitHub/WaterLeakSensor/Firmware/WaterLeakDetector/src/WaterLeakDetector.ino"
/*********************************************************************************************************
    WaterLeakDetector: Detect a water leak and sound the alarm!  Read the ambient temperature
        and humidity, display them on a "servo meter" and publich the data to the cloud."

        This program reads two water level sensor analog outputs, converts each to a voltage,
        thresholds the voltages to detect a water leak condition, integrates the thresholds to filter
        out false readings, introduces some hysteresis into the alarm/reset process, and indicates the alarm.
        The program uses a non-blocking delay that reads each sensor every 20 milliseconds and requires
        5 readings above a threshold for either sensor in order to trigger an alarm.

        This program supports both an alarm and an indicator.  The indicator turns on and off
        based solely upon the threshold hysteresis.  The alarm also turns on and off based upon the
        threshold hysteresis but, additionally, mutes when a pushbutton is pressed and stays muted until the
        alarm indication is cleared.  Although two sensors are supported, there is only a single alarm
        indication.  Either sensor can trigger the alarm and both sensors must be in the non-alarm
        state for the alarm indication to be cleared.

        The indicator is normally lit to show that the system is working.  The indicator flashes when
        an alarm condition is detected (water level threshold) and remains flashing until the alarm
        condition is cleared.  The alarm is a piezo buzzer that is pulsed whenever there is an alarm
        condition and the mute pushbutton has not been pressed.

        When a new alarm condition is sensed, a message to this effect is published to the cloud.

        This program also reads ambient temperature and humidity from a DHT11 sensor.  This data might
        be useful to determine if a leak in a basement is due to a burst steam pipe.  The temperature
        and humidity are read out every 4 seconds (nominally).  This data is published to the cloud and
        is also indicated on  "servo meter"; the latter using a 10 value moving average.
        A toggle switch determine whether the servo meter displays the temperature or the humidity.

        This version of code also also includes a diff() function for computing time differences using
        millis(), for use in non-blocking delay functionality.

        This version includes all of Jim's Blynk integration code plus coding for a second alarm notification
        30 seconds after the first alarm notification.

        This version also fixes a bug in the previous Blynk integration code for proper handling of
        sending an alarm.


    author: Bob Glicksman, Jim Schrempp; 10/25/2022

    (c) 2017, 2021, 2022 Bob Glicksman and Jim Schrempp, Team Practical Projects

20221025:  Version 2.0:  Removed IFTTT and Blynk.  This version uses a custom built app (AI2) to read out
    the status of the WLD - temperature, humidity and alarms.  This version adds high and low temperature
    alarms to the previous water leak alarm.  This version uses an instance of the included WLDAlarmProcessor
    class to manage the three alarms.  Alarms are implemented as SMS text messages to the user's mobile
    phone, via Particle.publish() from the WLDAlarmProcessor instance.  Each such publication triggers
    a webhook in the Particle cloud to POST the alarm event to a Google Apps Script.  The script, in turn,
    sends an email to the user's mobile carrier SMS gateway, resulting in an SMS text to the user's mobile
    phone.  More information about this process can be found at:
    https://github.com/TeamPracticalProjects/Connectivity_Tools_with_Particle_Devices
    
    This version of the WLD firmware uses the same logic as earlier versions
    to detect and clear leak conditions and to average the temperature and humidity reading for the purposes
    of display, reporting, and detecting/clearing alarms.  The instance of the WLDAlarmProcessor class manages
    alarm holdoffs so that this firmware does not have to manage alarm states.  Rather, when this firmware
    detects an alarm condition, it calls the appropriate object method and when it determines that an alarm
    condition does not exist, it calls a re-arm method for that alarm.  The object manages alarm holdoffs so that
    any persistant alarm only alarms the user.

    Manual display of temperature/humidity (via the servo meter) and alarm/mute functionality are unchanged from
    earlier version of the WLD.

20210320:  Commented out Particle.publish() of temp and humidity b/c of Particles new pricing policy
20170530a: Added Blynk application notification of water detection with Blynk Terminal and LED.
***********************************************************************************************************/
#include <PietteTech_DHT.h> // non-blocking library for DHT11
#include <WLDAlarmProcessor.h>  // the alarm processor class

// Constants and definitions
String dateTimeString();
void writeAlarmStatusString();
int setAlarmLimits(String alarmLimits);
int testAlarm(String nothing);
void setup();
void loop();
bool alarmIntegrator(float sensorAReading, float sensorBReading);
void nbFlashIndicator(boolean flash);
void nbSoundAlarm(boolean sound);
boolean nbWaterMeasureInterval(unsigned long delayTime);
boolean readPushButton();
unsigned long diff(unsigned long current, unsigned long last);
int readDHT(boolean startRead);
void meterTemp(float temperature);
void meterHumidity(float humidity);
#line 73 "c:/Users/bobg5/Documents/GitHub/WaterLeakSensor/Firmware/WaterLeakDetector/src/WaterLeakDetector.ino"
#define DHTTYPE  DHT11              // Sensor type DHT11/21/22/AM2301/AM2302
const int WATER_SENSOR_A_PIN = A0;
const int WATER_SENSOR_B_PIN = A1;
const int LED_PIN = D7;
const int ALARM_PIN = D6;
const int INDICATOR_PIN = D5;
const int BUTTON_PIN = D4;
const int DHTPIN = D2;        	    // Digital pin for communications
const int TOGGLE_PIN = D1;               // pin for temperature/humidity toggle switch
const int SERVO_PIN = A5;                // servo pin
#define DHT_SAMPLE_INTERVAL   4000  // Sample every 4 seconds
#define PARTICLE_PUBLISH_INTERVAL 60000 // Publish values every 60 seconds
const float WATER_LEVEL_THRESHOLD = 0.5;    // 0.5 volts or higher on either sensor triggers alarm

// servo calibration values
const int MIN_POS = 5;  // the minimum position value allowed
const int MAX_POS = 175;  // the maximum position value allowed

// meter face range values
const int HI_TEMP = 120;  // based upon meter dial face for temperature (F)
const int LO_TEMP = 40;   // based upon meter dial face for temperature (F)
const int HI_HUM = 100;  // based upon meter dial face for humidity (%RH)
const int LO_HUM = 0;  // based upon meter dial face for humidity (%RH)

// calculate meter dial ranges from hi and lo values
#define TEMP_RANGE (HI_TEMP - LO_TEMP)
#define HUM_RANGE (HI_HUM - LO_HUM)

// sensor return status codes
const unsigned int ACQUIRING  = 0;
const unsigned int COMPLETE_OK  = 1;
const unsigned int COMPLETE_ERROR  = 2;

// global to hold the result code from DHT sensor reading
int dhtResultCode;

// global to hold the smoothed values of humidity and temperature that we display and report
float mg_smoothedTemp = 0.0, mg_smoothedHumidity = 0.0; // smoothed for the display

// Lib instantiate
PietteTech_DHT DHT(DHTPIN, DHTTYPE);    // create DHT object to read temp and humidity
Servo myservo;  // create servo object to control a servo
WLDAlarmProcessor alarmer;  // create alarmer object to manage alarms

// Globals
boolean ledState = false;   // D7 LED is used for indicating water level measurements

    // These globals are for Particle.variable() data for cloud access by the app.
String info = "";   // this string will hold the firmware version number and the last reset time.
String temperature = "";   // this string will hold the currently averaged temperature
String humidity = "";   // this string will hold the currently averaged humidity
String currentAlarms = "0,0,0"; // this string holds the high temp alarm, low temp alarm, leak alarm
                                //  in a comma separated format. 0 is no alarm, any other number is an
                                //  alarm.
String lowTempAlarmLimit = "";    // this string holds the low temp alarm limit
String highTempAlarmLimit = "";   // this string holds the high temp alarm limit

    // This global structure holds the temperature alarm limits.  The limits are cloud accessible for reading 
    //  via the cloud variable "currentAlarmLimits" by the app.  The alarm limites are set by the app via 
    //  Particle.function() calls.  The latest values are stored in non-volitile EEPROM.
struct {
    uint8_t version;
    int16_t tempAlarmLowLimit;
    int16_t tempAlarmHighLimit;
} AlarmLimits;


struct {
    bool lowTempAlarm;
    bool highTempAlarm;
    bool waterLeakAlarm;
} Alarms;


// Utility functions

// create a string of the current date-time in UTC
String dateTimeString(){
    time_t timeNow = Time.now();
    String dateTime = Time.format(timeNow,TIME_FORMAT_DEFAULT) + " UTC";
    return dateTime;
}   // end of dateTimeString()

// write the cloud accessible variable string that contains the current alarms
void writeAlarmStatusString() {
    currentAlarms = "";
    if(Alarms.lowTempAlarm == false) {
        currentAlarms += "0";
    } else {
        currentAlarms += "1";
    }
    currentAlarms += ",";

    if(Alarms.highTempAlarm == false) {
        currentAlarms += "0";
    } else {
        currentAlarms += "1";
    }
    currentAlarms += ",";  

    if(Alarms.waterLeakAlarm == false) {
        currentAlarms += "0";
    } else {
        currentAlarms += "1";
    }

}   // end of writeAlarmStatusString

// The high and low temperature limits are stored in a struct in non-volitile simulated EEPROM.  
//  This function reads the struct out of EEPROM and sets the global variable strings accordingly
void readLimitDataFromEEPROM() {

    // initialize the struct from the EEPROM
    int addr = 10;
    EEPROM.get(addr, AlarmLimits.version);
    addr = 20;
    EEPROM.get(addr, AlarmLimits.tempAlarmLowLimit);
    addr = 30;
    EEPROM.get(addr, AlarmLimits.tempAlarmHighLimit);

    // test that data from EEPROM is valid.  If not, set some defaults.
    if(AlarmLimits.version != 0) {
        AlarmLimits.tempAlarmLowLimit = 40;
        AlarmLimits.tempAlarmHighLimit = 105;
    }
    
    //  replace original data with the integer limits, as these are the real alarm limits
    lowTempAlarmLimit = String(AlarmLimits.tempAlarmLowLimit);
    highTempAlarmLimit = String(AlarmLimits.tempAlarmHighLimit); 

}   // end of readLimitDataFromEEPROM() 

// Cloud function to set the new temperature alarm limits and store as a struct into EEPROM
//  argument is a string containing "lowTempAlarmLimit" comma "highTempAlarmLimit"
int setAlarmLimits(String alarmLimits) {

    // Parse the comma delimited string from the app
    for(unsigned int i = 0; i < alarmLimits.length(); i++) {
        if (alarmLimits.charAt(i) == ',') {
            lowTempAlarmLimit = alarmLimits.substring(0, i);
            highTempAlarmLimit = alarmLimits.substring(i+1);
            break;  // we are done walking through the string
        }
    }

    // load up the AlarmLimits struct
    AlarmLimits.version = (uint8_t)0;
    AlarmLimits.tempAlarmLowLimit = (int16_t)(lowTempAlarmLimit.toInt());
    AlarmLimits.tempAlarmHighLimit = (int16_t)(highTempAlarmLimit.toInt());    

    // write the struct to EEPROM
    int addr = 10;
    EEPROM.put(addr, AlarmLimits.version);
    addr = 20;
    EEPROM.put(addr, AlarmLimits.tempAlarmLowLimit);
    addr = 30;
    EEPROM.put(addr, AlarmLimits.tempAlarmHighLimit);

    /***********************TESTING****************************/
    String testStruct = String(AlarmLimits.version);
    testStruct += ",";
    testStruct += AlarmLimits.tempAlarmLowLimit;
    testStruct += ",";
    testStruct += AlarmLimits.tempAlarmHighLimit;
    Particle.publish("The Struct Data", testStruct);
    /***********************TESTING****************************/

    //  replace original data with the integer limits, as these are the real alarm limits
    lowTempAlarmLimit = String(AlarmLimits.tempAlarmLowLimit);
    highTempAlarmLimit = String(AlarmLimits.tempAlarmHighLimit); 

    return 0;

}   // end of setAlarmLimits()

// Cloud function to send out a test alarm
int testAlarm(String nothing) {
    alarmer.sendTestAlarm(); 

    return 0;
    
}   // end of testAlarm


// setup()
void setup() {
    pinMode(LED_PIN, OUTPUT);
    pinMode(ALARM_PIN, OUTPUT);
    pinMode(INDICATOR_PIN, OUTPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(TOGGLE_PIN, INPUT_PULLUP);  // toggle switch uses an internal pullup
    myservo.attach(SERVO_PIN);  // attaches to the servo object

    DHT.begin();
    alarmer.begin();

    // declare Cloud variables and functions
    Particle.variable("Info", info);
    Particle.variable("Temperature", temperature);
    Particle.variable("Humidity", humidity);
    Particle.variable("Alarms", currentAlarms);
    Particle.variable("Low Temp Alarm Limit", lowTempAlarmLimit);
    Particle.variable("High Temp Alarm Limit", highTempAlarmLimit);

    Particle.function("Set Temp Alarm Limits", setAlarmLimits);
    Particle.function("Send a test alarm", testAlarm);

    // set the information global
    info = "Firmware Verison 2.0. Last reset at: ";
    info += dateTimeString();

    // clear out alarm structure
    Alarms.lowTempAlarm = false;
    Alarms.highTempAlarm = false;
    Alarms.waterLeakAlarm = false;


    // read the temp alarm limits from EEPROM into the struct and set the global variables
    void readLimitDataFromEEPROM(); 

    /***********************TESTING****************************/
    String testStruct = String(AlarmLimits.version);
    testStruct += ",";
    testStruct += AlarmLimits.tempAlarmLowLimit;
    testStruct += ",";
    testStruct += AlarmLimits.tempAlarmHighLimit;
    Particle.publish("The Struct Data", testStruct);
    /***********************TESTING****************************/

}  // end of setup()


// loop()
void loop() {
    static boolean mute = false;  // set to true to mute the audible alarm
    static boolean indicator = false;  // set to true to flash the indicator
    static boolean alarm = false;   // set to true to sound the alarm
    static unsigned long lastReadTime = 0UL;    // DHT 11 reading time
    static unsigned long lastPublishTime = 0UL;  // Published particle event time
    static boolean newData = false; // flag to indicate DHT11 has new data
    static boolean toggle = false;  // hold the reading of the toggle switch; false for humidity, true for temperature
    static boolean lastToggle = false;  // hold the previous reading of the toggle switch

    float currentTemp, currentHumidity;

    //  read the toggle switch position and set the boolean for type of display accordingly
    if(digitalRead(TOGGLE_PIN) == LOW)  {   // indicates a temperature reading
        toggle = true;
    } else {
        toggle = false;
    }

    // determine if the toggle state has changed
    if(toggle != lastToggle) {  // user has changed the toggle switch state
        lastToggle = toggle;
        if(toggle == false) {   // display humidity now
	        meterHumidity(mg_smoothedHumidity);
        } else {
            meterTemp(mg_smoothedTemp); // display temperature now
        }
    }

    // Non-blocking read of DHT11 data and publish and display it
    int sensorStatus = readDHT(false);  // refresh the sensor status but don't start a new reading

	if(sensorStatus != ACQUIRING) {
        if(newData == true) { // we have new data
            currentTemp = DHT.getFahrenheit();
            currentHumidity = DHT.getHumidity();

            // Smooth the readings for display
            if (mg_smoothedTemp < 10) {   // first time init
                mg_smoothedTemp = currentTemp;
            }
            if (mg_smoothedHumidity < 10){  // first time init
                mg_smoothedHumidity = currentHumidity;
            }
            
            // 10 point moving average
            mg_smoothedTemp =  (0.9 * mg_smoothedTemp) +  (0.1 * currentTemp);
            mg_smoothedHumidity =  (0.9 * mg_smoothedHumidity) +  (0.1 * currentHumidity);

	        // set temperature or humidiy on the servo meter
	        if(toggle == true)  {   // temperature reading called for
	            meterTemp(mg_smoothedTemp);
	        }  else  {  // humidity reading called for
	         meterHumidity(mg_smoothedHumidity);
	        }
  
            // set the cloud temperature and humidity globals
            temperature = String(mg_smoothedTemp);
            humidity = String(mg_smoothedHumidity);

            // test for low and high temperature alarms and set the flags
            if(mg_smoothedTemp < lowTempAlarmLimit.toFloat()) { // we have a low temperature alarm
                Alarms.lowTempAlarm = true; // set the low temperature alarm flag
            } else {    // no low temperature alarm now
                Alarms.lowTempAlarm = false; // set the low temperature alarm flag
            }
        
            if(mg_smoothedTemp > highTempAlarmLimit.toFloat()) { // we have a high temperature alarm
                Alarms.highTempAlarm = true; // set the high temperature alarm flag
                alarmer.sendHighTemperatureAlarm(mg_smoothedTemp); // send out the alarm for processing
            } else {    // no high temperature alarm now
                Alarms.highTempAlarm = false; // set the low temperature alarm flag
            }

            // process the alarm flags to send or reset the alarms, as appropriate
            if(Alarms.lowTempAlarm == true) {   // low temp alarm needs processing
                alarmer.sendLowTemperatureAlarm(mg_smoothedTemp); // send out the alarm for processing
                alarmer.armHighTempAlarm();  // reset the alarm processing for a new alarm in the future
            } else if(Alarms.highTempAlarm == true) {     // high temp alarm needs processing
                alarmer.sendHighTemperatureAlarm(mg_smoothedTemp); // send out the alarm for processing
                alarmer.armLowTempAlarm();  // reset the alarm processing for a new alarm in the future            
            } else {    // not temp alarms, therefore both alarms need rearming
                alarmer.armHighTempAlarm();  // reset the alarm processing for a new alarm in the future
                alarmer.armLowTempAlarm();  // reset the alarm processing for a new alarm in the future 
            }

            // write out the current status of all alarms
            writeAlarmStatusString();

	        newData = false; // don't publish/process results again until a new reading
        }

        if((diff(millis(), lastReadTime)) >= DHT_SAMPLE_INTERVAL) { // we are ready for a new reading
          readDHT(true);  // start a new reading
          newData = true; // set flag to indicate that a new reading will result
          lastReadTime = millis();


            // toggle the D7 LED to indicate loop timing for DHT11 reading
            ledState = !ledState;
            if (ledState) {
                digitalWrite(LED_PIN, HIGH);
            } else {
                digitalWrite(LED_PIN, LOW);
            }
        }
    }

    // measure and test water level at pre-determined interval
    if(nbWaterMeasureInterval(20) == false) {  // 20 ms between sensor readings

        // read the water level from the sensor; convert to a voltage value
        int waterLevelA, waterLevelB;
        float waterSensorVoltageA, waterSensorVoltageB;

        waterLevelA = analogRead(WATER_SENSOR_A_PIN);
        waterLevelB = analogRead(WATER_SENSOR_B_PIN);
        waterSensorVoltageA = ((float)waterLevelA * 3.3) / 4095;
        waterSensorVoltageB = ((float)waterLevelB * 3.3) / 4095;

        // integrate and threshold measurement for alarm
        if(alarmIntegrator(waterSensorVoltageA, waterSensorVoltageB) == true) {
            indicator = true;
            // process a water leak detection
            Alarms.waterLeakAlarm = true;   // set the alarm flag
            alarmer.sendWaterLeakAlarm();   // send the alarm for processing
            writeAlarmStatusString();   // update the alarm status global string

            if(mute == false) {
                alarm = true;
            } else {
                alarm = false;
            }
        } else {
            indicator = false;
            // no water leak alarm so process and rearm
            Alarms.waterLeakAlarm = false;   // set the alarm flag
            alarmer.armLeakAlarm();   // send the alarm for processing
            writeAlarmStatusString();   // update the alarm status global string

            alarm = false;
            mute = false;   // reset alarm muting
        }
    }

    // process the mute pushbutton
    if(readPushButton() == true) {
        mute = true; // set the alarm mute flag
        alarm = false; // mute the alarm right now
    }

    // refresh non-blocking alarm & indicator status
    nbFlashIndicator(indicator);
    nbSoundAlarm(alarm);
    
} // end of loop()

/* alarmIntegrator():  function that thresholds sensor voltage readings and integrates the values.
    parameters:
        sensorReading - the sensor voltage reading to be thresholded and integrated
    return:
        the alarm value as a boolean.  The alarm value is set after 5 thresholds are accumulated
            for either sensor and stays set until 5 under-thresholds are accumulated for both sensors
*/
bool alarmIntegrator(float sensorAReading, float sensorBReading) {
    const byte ALARM_LIMIT = 5;     // 5 thresholds are required to trigger an alarm, then 5 under thresholds
                                    //  are required to reset the alarm condition.

    static byte integratedValueA = 0; // threshold exceeded accumulator for sensor A
    static byte integratedValueB = 0; // threshold exceeded accumulator for sensor B
    static boolean lastAlarmState = false;
    boolean thresholdedReadingA;
    boolean thresholdedReadingB;

   // test to see if threshold was exceeded on each sensor
    if(sensorAReading > WATER_LEVEL_THRESHOLD) {
        thresholdedReadingA = true;
    } else {
        thresholdedReadingA = false;
    }

    if(sensorBReading > WATER_LEVEL_THRESHOLD) {
        thresholdedReadingB = true;
    } else {
        thresholdedReadingB = false;
    }

    // integrate thresholds or under thresholds; clamp them at ALARM_LIMIT and 0
    if(thresholdedReadingA == true)  {       // increment integrator
        if(integratedValueA < ALARM_LIMIT)  {
            integratedValueA++;
        } else {
            integratedValueA = ALARM_LIMIT;  // clamp at max value
        }
    } else {    // thresholdedReading was false
        if(integratedValueA > 0) {           // decrement integrator
            integratedValueA--;
        }  else  {
            integratedValueA = 0;    // clamp at zero
        }
    }

    if(thresholdedReadingB == true)  {       // increment integrator
        if(integratedValueB < ALARM_LIMIT)  {
            integratedValueB++;
        } else {
            integratedValueB = ALARM_LIMIT;  // clamp at max value
        }
    } else {    // thresholdedReading was false
        if(integratedValueB > 0) {           // decrement integrator
            integratedValueB--;
        }  else  {
            integratedValueB = 0;    // clamp at zero
        }
    }


    // Determine the return value
    if((integratedValueA >= ALARM_LIMIT) || (integratedValueB >= ALARM_LIMIT)) {    // either integrator at the limit
        lastAlarmState = true;
        return true;
    } else {
        if((integratedValueA <= 0) && (integratedValueB <= 0)) {    // both integrators at zero
            lastAlarmState = false;
            return false;
        } else {
            return lastAlarmState;  // in between condition, no change in alarm return value
        }
    }
}  // end of alarmIntegrator()

/* nbFlashIndicator():  non-blocking function to flash the indicator LED when alarming
                        or light it constantly when not alarming
    parameters:
        flash - true to flash the LED, false to light it constantly
*/
void nbFlashIndicator(boolean flash) {
    const unsigned long FLASH_INTERVAL = 150; // 150 ms on and off
    static boolean lastOn = true;   // start with LED on
    static unsigned long lastTime = millis();

    if(flash == true) {     // flashes the LED
        if(diff(millis(), lastTime) >= FLASH_INTERVAL) { // flip the LED state
            lastOn = !lastOn;
            lastTime = millis();
        }

    } else {  // not flashing the LED
        lastOn = true;
    }

    if(lastOn == true)  {
        digitalWrite(INDICATOR_PIN, HIGH);
    } else {
        digitalWrite(INDICATOR_PIN, LOW);
    }
    return;
}   // end of nbFlashIndicator()

/* nbSoundAlarm():  non-blocking function to sound the alarm
    parameters:
        sound - sound the audible alarm
*/
void nbSoundAlarm(boolean sound) {
    const unsigned long BEEP_INTERVAL = 50; // 50 ms on and off
    static boolean lastOn = false;   // start with alarm off
    static unsigned long lastTime = millis();

    if(sound == true) {     // sound the buzzer
        if(diff(millis(), lastTime) >= BEEP_INTERVAL) { // flip the buzzer state
            lastOn = !lastOn;
            lastTime = millis();
        }

    } else {  // not alarming
        lastOn = false;
    }

    if(lastOn == true)  {
        digitalWrite(ALARM_PIN, HIGH);
    } else {
        digitalWrite(ALARM_PIN, LOW);
    }
    return;
}   // end of nbSoundAlarm()

/* nbWaterMeasureInterval(): non-blocking interval delay for water level measurements
    parameters:
        delayTime - number of milliseconds in the delay
    retrun:
        delay in progress - true if delay is in progress, otherwise false
*/
boolean nbWaterMeasureInterval(unsigned long delayTime) {
    static boolean lastState = false;
    static unsigned long lastTime;
    static unsigned long currentTime;

    // if not currently in timing, start timing
    if(lastState == false) {
        lastTime = millis();
        lastState = true;   // in measurement
        return true;
    }

    // currently timing, so test for completion and process accordingly
    currentTime = millis();
    if(diff(currentTime, lastTime) < delayTime) { // time not yet expired
        lastState = true;  // timing in process
        return true;
    } else {  // time has expired
        lastTime = currentTime;
        lastState = false;  // timing has expired
        return false;
    }
}  // end of nbWaterMeasureInterval

/* readPushButton: read the mute pushbutton with debouncing
    parameters: none
    return:
        flag that indicates the push button status
*/
boolean readPushButton() {
    const unsigned long DEBOUNCE_TIME = 10;  // 10 milliseconds

    // state variable states
    const byte OFF = 0;
    const byte DEBOUNCING = 1;
    const byte DEBOUNCED = 2;

    static byte lastState = OFF;
    static unsigned long beginTime;

    if(digitalRead(BUTTON_PIN) == LOW) {  // button has been pressed
        switch (lastState) {
            case OFF:       // new button press
                beginTime = millis();
                lastState = DEBOUNCING;
                return false;
                break;
            case DEBOUNCING:    // wait until debouncing time is over
                if(diff(millis(), beginTime) < DEBOUNCE_TIME) {
                    return false;
                } else {
                    lastState = DEBOUNCED;
                    return true;
                }
                break;
            case DEBOUNCED:     // stay in this state until button is released
                return false;
                break;
            default:
                return false;
        }

    } else {        // the button has been released
        lastState = OFF;
        return false;
    }
}  // end of readPushButton


/* diff(): function to measure time differences using millis() that corrects for millis() overflow.
    paramters:
        current - the current time value from millis(), as unsigned long
        last - the previous time value from millis(), as unsigned long
    return:
        the difference between current and last, as unsigned long
*/
unsigned long diff(unsigned long current, unsigned long last)  {
    const unsigned long MAX = 0xffffffff;  // an unsigned long is 4 bytes
    unsigned long difference;

    if (current < last) {       // overflow condition
        difference = (MAX - last) + current;
    } else {
        difference = current - last;
    }
    return difference;
}  // end of diff()


/*
readDHT():  read temperature and humidity from the DHT11 sensor
    arguments:
        startRead:  true to start a reading, false otherwise
    return: status code, per global definitions
*/
int readDHT(boolean startRead) {
    static int _state = COMPLETE_OK;

    if(_state == ACQUIRING) {  // test to see if we are done
        if(DHT.acquiring() == false) { // done acquriring
            dhtResultCode = DHT.getStatus();  // store the result code fromt he library
           if(dhtResultCode == DHTLIB_OK) {
               _state = COMPLETE_OK;
           } else {
               _state = COMPLETE_ERROR;
           }
        }
    }
    else { // we were not in the process of acquiring
        if(startRead == true) {  // we must start a new reading of sensor data
           _state = ACQUIRING;  // set the state to acquiring
            DHT.acquire(); // start the acquisition
        }
    }

    return _state;

}  // end of readDHT()

/* meterTemp():  display temperature reading on the servo meter
    arguments:
        temperature: the temperature to display on the meter
*/
void meterTemp(float temperature)  {
    int _temp, _mve, _cmd;

    _temp = (int)(temperature + 0.5);  // round and truncate to an integer

    // clamp temperature to within dial limits
    if(_temp < LO_TEMP) {
        _temp = LO_TEMP;
    } else if(_temp > HI_TEMP) {
        _temp = HI_TEMP;
    }

    _mve = (_temp - LO_TEMP) * (MAX_POS - MIN_POS) / TEMP_RANGE;
    _cmd = MAX_POS - _mve;
    myservo.write(_cmd);

    return;
}  // end of meterTemp()

/* meterHumidity():  display humidity reading on the servo meter
    arguments:
        humidity: the humidity to display on the meter
*/
void meterHumidity(float humidity)  {
    int _hum, _mve, _cmd;

    _hum = (int)(humidity + 0.5);  // round and truncate to an integer

    // clamp temperature to within dial limits
    if(_hum < LO_HUM) {
        _hum = LO_HUM;
    } else if(_hum > HI_HUM) {
        _hum = HI_HUM;
    }

    _mve = (_hum - LO_HUM) * (MAX_POS - MIN_POS) / HUM_RANGE;
    _cmd = MAX_POS - _mve;
    myservo.write(_cmd);

    return;
}  // end of meterHumidity()
