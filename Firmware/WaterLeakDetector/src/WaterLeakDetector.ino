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


    author: Bob Glicksman, Jim Schrempp; 06/03/2017

    (c) 2017, 2021, Bob Glicksman and Jim Schrempp, Team Practical Projects

20210320:  Commented out Particle.publish() of temp and humidity b/c of Particles new pricing policy
20170530a: Added Blynk application notification of water detection with Blynk Terminal and LED.
***********************************************************************************************************/
//#define IFTTT_NOTIFY    // comment out if IFTTT alarm notification is not desired
#define BLYNK_NOTIFY    // comment out if you do not want Blynk to be active

#include <PietteTech_DHT.h> // non-blocking library for DHT11
#include <blynk.h>  // Blynk library

// Constants and definitions
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
#define SECOND_NOTIFY_DELAY 30000  // the second alarm notification comes 30 seconds after the first notification
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

#ifdef BLYNK_NOTIFY
//blynk
#include "blynk.h"

char auth[] = "REPLACE THIS WITH YOUR BLYNK ACCESS TOKEN";

#define BLYNK_VPIN_HUMIDITY V5
#define BLYNK_VPIN_TEMPERATURE V7
#define BLYNK_VPIN_ALARM V6
#define BLYNK_VPIN_TERMINAL V4

WidgetLED blynkLED1(BLYNK_VPIN_ALARM);


// These functions are called by Blynk application widgets to get the value of
// a "Virtual Pin"
BLYNK_READ(BLYNK_VPIN_HUMIDITY)
{
    Blynk.virtualWrite(BLYNK_VPIN_HUMIDITY, mg_smoothedHumidity);
}

BLYNK_READ(BLYNK_VPIN_TEMPERATURE)
{
    Blynk.virtualWrite(BLYNK_VPIN_TEMPERATURE, mg_smoothedTemp);
}


#endif

// Utility functions

String dateTimeString(){
    time_t timeNow = Time.now();
    String dateTime = Time.format(timeNow,TIME_FORMAT_DEFAULT) + " UTC";
    return dateTime;
}

// We leave the method calls so that we don't have to ifdef every place it might be
// called in the code.
void blynkRaiseAlarm()
{
#ifdef BLYNK_NOTIFY
    static int notifyCount = 0;
    notifyCount++;
    String blynkWarning = "WARNING: Water leak detected (" + String(notifyCount) + ") ";
    Blynk.notify(blynkWarning);

    blynkWriteTerminal(blynkWarning);
    blynkWriteTerminal(dateTimeString() + "\r\n");
#endif
}

void blynkWaterDetected(boolean detected)
{
#ifdef BLYNK_NOTIFY
    static boolean lastState = false;
    
    if(detected != lastState) { // publish only on a change of detected state
    
        if (detected) {
            blynkLED1.on();
        } else  {
            blynkLED1.off();
        }
        lastState = detected;
    }
#endif
}

void blynkReportRestart()
{
#ifdef BLYNK_NOTIFY
    blynkWriteTerminal("---------\r\n");
    blynkWriteTerminal("WLD restarted: ");
    blynkWriteTerminal(dateTimeString() + "\r\n");
#endif
}


void blynkWriteTerminal(String msg)
{
#ifdef BLYNK_NOTIFY
    Blynk.virtualWrite(BLYNK_VPIN_TERMINAL,msg);
#endif
}


// Globals
boolean ledState = false;   // D7 LED is used for indicating water level measurements

// setup()
void setup() {
    pinMode(LED_PIN, OUTPUT);
    pinMode(ALARM_PIN, OUTPUT);
    pinMode(INDICATOR_PIN, OUTPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(TOGGLE_PIN, INPUT_PULLUP);  // toggle switch uses an internal pullup
    myservo.attach(SERVO_PIN);  // attaches to the servo object
    DHT.begin();
    
#ifdef BLYNK_NOTIFY
    Blynk.begin(auth);
    blynkReportRestart();
    blynkLED1.off();
#endif

}  // end of setup()

// loop()
void loop() {
    static boolean mute = false;  // set to true to mute the audible alarm
    static boolean indicator = false;  // set to true to flash the indicator
    static boolean alarm = false;   // set to true to sound the alarm
    static boolean previousAlarmState = false;  // used to detect a new alarm
    static unsigned long lastReadTime = 0UL;    // DHT 11 reading time
    static unsigned long lastPublishTime = 0UL;  // Published particle event time
    static boolean newData = false; // flag to indicate DHT11 has new data
    static boolean toggle = false;  // hold the reading of the toggle switch; false for humidity, true for temperature
    static boolean lastToggle = false;  // hold the previous reading of the toggle switch
    static boolean firstNotification = false;  // indicator to use for a second alarm notification
   	static unsigned long firstNotifyTime;	// record time of first notification to time the second one

    // Non-blocking read of DHT11 data and publish and display it
    float currentTemp, currentHumidity;

#ifdef BLYNK_NOTIFY
    Blynk.run();
#endif

    static boolean onceUponRestart = true;
    if (onceUponRestart){
        onceUponRestart = false;

    }

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

	    newData = false; // don't publish results again until a new reading
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

    if((diff(millis(), lastPublishTime)) >= PARTICLE_PUBLISH_INTERVAL)  // we should publish our values
    {
        lastPublishTime = millis();
        // publish Smoothed temperature and humidity readings to the cloud
//        Particle.publish("Humidity Smoothed (%)", String(mg_smoothedHumidity));
//        Particle.publish("Temperature Smoothed (oF)", String(mg_smoothedTemp));
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
            blynkWaterDetected(true);
            if(mute == false) {
                alarm = true;
            } else {
                alarm = false;
            }
        } else {
            indicator = false;
            blynkWaterDetected(false);
            alarm = false;
            mute = false;   // reset alarm muting
        }

        #ifdef IFTTT_NOTIFY
            //  For IFTTT Notification: test if new alarm and publish it
           if((indicator == true) && (indicator != previousAlarmState)) {
             Particle.publish("Water leak alarm", Time.timeStr(Time.now()) + " Z");
           }
         #endif

    	// If we have a new alarm, then send a Blynk notification
 		if((indicator == true) && (indicator != previousAlarmState)) {
        	blynkRaiseAlarm();

        	// set conditions for the second alarm notification
        	firstNotification = true;
        	firstNotifyTime = millis();
    	}

    	previousAlarmState = indicator; // update old alarm state to present state
    }


    // process the second alarm notification after the first alarm notification
    if( (firstNotification == true) && (diff(millis(), firstNotifyTime) >= SECOND_NOTIFY_DELAY) )  {
        blynkRaiseAlarm();
        firstNotification = false;
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

boolean alarmIntegrator(float sensorAReading, float sensorBReading) {
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
