/*******************************************************************************
 * WLDAlarmProcessor:  class to generate and manage WLD alarms
 * See: "WLD V2 Concept Document" for further details:   
 * https://docs.google.com/document/d/1WXK0372C2H_zPN31xgyE5MKP1GyY8HaXpQrTsiYnKkg/edit?usp=sharing
 * 
 * By: Bob Glicksman, Jim Schrempp, Team Practical Projects
 * (c) 2022, Bob Glicksman, Jim Schrempp, Team Practical Projects
 * 
 * version 1.0: 10/23/22.  Initial release
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

    _alarmMsg = "";

}

// Methods for handling alarms, holdoffs and filed testing

// method to create and publish a low temperature alarm event to the Particle Cloud
void WLDAlarmProcessor::sendLowTemperatureAlarm(float theAlarmTemperature) {

    if(_lowTempAlarmArm == true) {  // alarm is armed, so publish the alarm event
        _alarmMsg += String(theAlarmTemperature);
        Particle.publish("WLDAlarmLowTemp", _alarmMsg);
        _lowTempAlarmArm = false;   // disarm the alarm
        _lowTempLastAlarm = millis();  // record the time of the alarm

    } else {    // see if enough time has passed to re-arm the alarm to send the alarm event
        if( (millis() - _lowTempLastAlarm) >= ONE_DAY ) {   // hold off time has expired, send the alarm
            _alarmMsg += String(theAlarmTemperature);
            Particle.publish("WLDAlarmLowTemp", _alarmMsg);
            _lowTempAlarmArm = false;   // disarm the alarm
            _lowTempLastAlarm = millis();  // record the time of the alarm
        }
    }
    _alarmMsg = ""; // clear out the message string

}   // end of sendLowTemperateAlarm()

// method to create and publish a high temperature alarm event to the Particle Cloud
void WLDAlarmProcessor::sendHighTemperatureAlarm(float theAlarmTemperature){

    if(_highTempAlarmArm == true) {  // alarm is armed, so publish the alarm event
        _alarmMsg += String(theAlarmTemperature);
        Particle.publish("WLDAlarmHighTemp", _alarmMsg);
        _highTempAlarmArm = false;   // disarm the alarm
        _highTempLastAlarm = millis();  // record the time of the alarm

    } else {    // see if enough time has passed to re-arm the alarm to send the alarm event
        if( (millis() - _highTempLastAlarm) >= ONE_DAY ) {   // hold off time has expired, send the alarm
            _alarmMsg += String(theAlarmTemperature);
            Particle.publish("WLDAlarmHighTemp", _alarmMsg);
            _highTempAlarmArm = false;   // disarm the alarm
            _highTempLastAlarm = millis();  // record the time of the alarm
        }
    }
    _alarmMsg = ""; // clear out the message string

}   // end of sendHighTemperatureAlarm()

// method to create and publish a water leak alarm event to the Particle Cloud
void WLDAlarmProcessor::sendWaterLeakAlarm() {
   if(_leakAlarmArm == true) {  // alarm is armed, so publish the alarm event
        _alarmMsg += String("Water leak Detected");
        Particle.publish("WLDAlarmWaterLeak", _alarmMsg);
        _leakAlarmArm = false;   // disarm the alarm
        _leakLastAlarm = millis();  // record the time of the alarm

    } else {    // see if enough time has passed to re-arm the alarm to send the alarm event
        if( (millis() - _leakLastAlarm) >= ONE_DAY ) {   // hold off time has expired, send the alarm
            _alarmMsg += String("Water leak Detected");
            Particle.publish("WLDAlarmWaterLeak", _alarmMsg);
            _leakAlarmArm = false;   // disarm the alarm
            _leakLastAlarm = millis();  // record the time of the alarm
        }
    }
    _alarmMsg = ""; // clear out the message string

}   // end of sendWaterLeakAlarm()

// method to create and publish a test alarm event to the Particle Cloud for field testing purposes
void WLDAlarmProcessor::sendTestAlarm() {

    _alarmMsg = "This is a test of the WLD alarm system";
    Particle.publish("WLDAlarmTest", _alarmMsg);

    _alarmMsg = ""; // clear out the message string

}   // end of sendTestAlarm()

// method to re-arm the low temperatre alarm so that a new alarm can be sent immediately
void WLDAlarmProcessor::armLowTempAlarm() {

    _lowTempAlarmArm = true;

}   // end of clearLowTempAlarmHoldoff()

// method to clear out the holdoff for the high temperatre alarm so that a new alarm can be sent immediately
void WLDAlarmProcessor::armHighTempAlarm() {

    _highTempAlarmArm = true;

}   // end of clearHighTempAlarmHoldoff()

// method to clear out the holdoff for the water leak alarm so that a new alarm can be sent immediately
void WLDAlarmProcessor::armLeakAlarm() {

   _leakAlarmArm = true;

}

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
