/*******************************************************************************
 * WLDAlarmProcessor:  class to generate and manage WLD alarms
 * 
 * The WLDAlarmProcessor class is designed to be used by the Water Leak Detector (WLD)
 * Particle firmware.  WLD firmware creates an instance of this calss and uses its methods to manage
 * and send alarms to the designed user.  Alarms are SMS texts to the user's mobile phone; however,
 * this class uses Particle.publish() to publish WLDAlarm events to the Particle cloud.  A webhook
 * running on the Particle cloud POSTs event data to a Google Apps Script.  This Script sends
 * emails to the SMS gateway of the user's mobile carrier that results in the SMS text being sent
 * to the user's mobile device.  More information aobut this process can be found at:
 * https://github.com/TeamPracticalProjects/Connectivity_Tools_with_Particle_Devices
 * 
 * The WLD firmware is responsible for determining wheter or not alarm conditions are present, and
 * then calling methods on the instantiated object to either send a alarm (when it detects and
 * alarm condition) or else to arm for an alarm (when it detects a normal condition).  The instantiated
 * object manages alarms so that an SMS text is send at most once per day, for any persistant alarm
 * condition.  The WLD firmware can send alarms using the instantiated object's sendAlarm..() methods
 * each and every time it detects an alarm condition, regardless of how frequently this occurs.  The
 * instantiated object manages the sensding of SMS texts to the user so that the user only receives one alarm
 * per day for any persistent alarm.  Conversely, the WLD firmmware should continuously re-arm an alarm
 * whenever a non-alarm condition is detected so that any new alarm condition will alert the user
 * immediately.
 * 
 * This class supports three active alarm types, and an additional alarm test (to make sure that the
 * process of publication --> webhook --> script --> SMS is working):
 * 
 * - High Temperature Alarm:  called (with the actual temperature as the argument) whenever a too-high
 * temperature is detected by the WLD firmware.
 * 
 * - Low Temperature Alarm:  called (with the actual temperature as the argument) whenever a too-low
 * temperature is detected by the WLD firmware.
 * 
 * - Leak Alarm: called whenever the WLD firmware detects a water leak.
 * 
 * In addition, the class supports three re-arm methods:
 * 
 * - re-arm High Temperature Alarm: called whenever the WLD firmware determines that the temperature is not too
 * high (relgardless of whether the temperature is normal or too low).
 * 
 * - re-arm Low Temperature Alarm: called whenever the WLD firmware determines that the temperature is not too
 * low (relgardless of whether the temperature is normal or too high).
 * 
 * - re-arm Water Leak Detection: called whenever the WLD firmware determines that no water leak is detected.
 * 
 * See: "WLD V2 Concept Document" for further details:   
 * https://docs.google.com/document/d/1WXK0372C2H_zPN31xgyE5MKP1GyY8HaXpQrTsiYnKkg/edit?usp=sharing
 * 
 * 
 * By: Bob Glicksman, Jim Schrempp, Team Practical Projects
 * (c) 2022, Bob Glicksman, Jim Schrempp, Team Practical Projects
 * 
 * version 1.0: 10/25/22.  Initial release
 * 
 *******************************************************************************/
#include <WLDAlarmProcessor.h>

// Constructor
WLDAlarmProcessor::WLDAlarmProcessor() {
    // follow convention and put all initializations in begin() method   
}   // end of Constructor

// Initialization
void WLDAlarmProcessor::begin() {
    // initialize all of the internal variables.

    _lowTempAlarmArm = true;    // arm the alarm upon initialization
    _highTempAlarmArm = true;   // arm the alarm upon initialization
    _leakAlarmArm = true;       // arm the alarm upon initialization

    _lowTempLastAlarm = 0L;
    _highTempLastAlarm = 0L;
    _leakLastAlarm = 0L;

}   // end of begin()

// Methods for handling alarms, arming and field testing

// method to create and publish a low temperature alarm event to the Particle Cloud
void WLDAlarmProcessor::sendLowTemperatureAlarm(float theAlarmTemperature) {
    String alarmMsg = "Low temperature detected. Temperature = ";
    unsigned long holdoffTime;

    holdoffTime =  WLDAlarmProcessor::diff(millis(), _lowTempLastAlarm); 

    // publish an alarm if it is armed, or else if it is more than ONE_DAY 
    //  since the last time the alarm was published
    if( (_lowTempAlarmArm == true) || ( holdoffTime  >= ONE_DAY) ) {
        alarmMsg += String(theAlarmTemperature);
        Particle.publish("WLDAlarmLowTemp", alarmMsg);
        _lowTempAlarmArm = false;   // disarm the alarm
        _lowTempLastAlarm = millis();  // record the time of the alarm
    } 
    return;
}   // end of sendLowTemperateAlarm()

// method to create and publish a high temperature alarm event to the Particle Cloud
void WLDAlarmProcessor::sendHighTemperatureAlarm(float theAlarmTemperature) {
    String alarmMsg = "High temperature detected. Temperature = ";
    unsigned long holdoffTime;
    
    holdoffTime = WLDAlarmProcessor::diff( millis(), _highTempLastAlarm );

    // publish an alarm if it is armed, or else if it is more than ONE_DAY 
    //  since the last time the alarm was published   
    if( (_highTempAlarmArm == true) || ( holdoffTime >= ONE_DAY) ) {  
        alarmMsg += String(theAlarmTemperature);
        Particle.publish("WLDAlarmHighTemp", alarmMsg);
        _highTempAlarmArm = false;   // disarm the alarm
        _highTempLastAlarm = millis();  // record the time of the alarm
    } 
    return;

}   // end of sendHighTemperatureAlarm()

// method to create and publish a water leak alarm event to the Particle Cloud
void WLDAlarmProcessor::sendWaterLeakAlarm() {
    String alarmMsg = "Water Leak Alarm Detected.";
    unsigned long holdoffTime;

    holdoffTime = WLDAlarmProcessor::diff( millis(), _leakLastAlarm );

    // publish an alarm if it is armed, or else if it is more than ONE_DAY 
    //  since the last time the alarm was published
    if( (_leakAlarmArm == true) || ( holdoffTime >= ONE_DAY) ) {  
        String _alarmMsg = "Water leak Detected";
        Particle.publish("WLDAlarmWaterLeak", _alarmMsg);
        _leakAlarmArm = false;   // disarm the alarm
        _leakLastAlarm = millis();  // record the time of the alarm
    } 
    return;

}   // end of sendWaterLeakAlarm()

// method to create and publish a test alarm event to the Particle Cloud for field testing purposes
void WLDAlarmProcessor::sendTestAlarm() {

    String alarmMsg = "This is a test of the WLD alarm system";
    Particle.publish("WLDAlarmTest", alarmMsg);

    return;
}   // end of sendTestAlarm()

// method to re-arm the low temperatre alarm so that a new alarm can be sent immediately
void WLDAlarmProcessor::armLowTempAlarm() {

    _lowTempAlarmArm = true;
    return;

}   // end of armLowTempAlarm()

// method to clear out the holdoff for the high temperatre alarm so that a new alarm can be sent immediately
void WLDAlarmProcessor::armHighTempAlarm() {

    _highTempAlarmArm = true;
    return;

}   // end of armHighTempAlarm() 

// method to clear out the holdoff for the water leak alarm so that a new alarm can be sent immediately
void WLDAlarmProcessor::armLeakAlarm() {

   _leakAlarmArm = true;
   return;

}   // end of armLeakAlarm() 

// Methods for debugging purposes

unsigned long WLDAlarmProcessor::get_lowTempLastAlarm() {

    return _lowTempLastAlarm;

}   // end of get_lowTempLastAlarm()

unsigned long WLDAlarmProcessor::get_highTempLastAlarm() {

    return _highTempLastAlarm;

}   // end of get_highTempLastAlarm()


unsigned long WLDAlarmProcessor::get_leakLastAlarm() {

    return _leakLastAlarm;

}   // end of get_leakLastAlarm()

// private methods

// diff(): take the difference between two unsigned long variables, accounting for variable overflow
//  Used to ensure that alarm holdoffs won't be fooled upon millis() overflow
unsigned long WLDAlarmProcessor::diff(unsigned long current, unsigned long last)  {
    const unsigned long MAX = 0xffffffff;  // an unsigned long is 4 bytes
    unsigned long difference;

    if (current < last) {       // overflow condition
        difference = (MAX - last) + current;
    } else {
        difference = current - last;
    }
    return difference;
}  // end of diff()