/*******************************************************************************
 * WLDAlarmProcessor:  class to generate and manage WLD alarms
 * See: "WLD V2 Concept Document" for further details:   
 * https://docs.google.com/document/d/1WXK0372C2H_zPN31xgyE5MKP1GyY8HaXpQrTsiYnKkg/edit?usp=sharing
 * 
 * By: Bob Glicksman, Jim Schrempp, Team Practical Projects
 * (c) 2022, Bob Glicksman, Jim Schrempp, Team Practical Projects
 * 
 * version 1.0: 10/22/22.  Initial release
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

    // XXX CODE GOES HERE
}

// Methods for handling alarms, holdoffs and filed testing

// method to create and publish a low temperature alarm event to the Particle Cloud
void sendLowTemperatureAlarm(float theAlarmTemperature) {

    // XXX CODE GOES HERE
}   // end of sendLowTemperateAlarm()

// method to create and publish a high temperature alarm event to the Particle Cloud
void WLDAlarmProcessor::sendHighTemperatureAlarm(float theAlarmTemperature){

    // XXX CODE GOES HERE
}   // end of snedHighTemperatureAlarm()

// method to create and publish a water leak alarm event to the Particle Cloud
void WLDAlarmProcessor::sendWaterLeakAlarm() {

    // XXX CODE GOES HERE
}   // end of sendWaterLeakAlarm()

// method to create and publish a test alarm event to the Particle Cloud for field testing purposes
void WLDAlarmProcessor::sendTestAlarm() {

    // XXX CODE GOES HERE
}   // end of sendTestAlarm()

// method to clear out the holdoff for the low temperatre alarm so that a new alarm can be sent immediately
void WLDAlarmProcessor::clearLowTempAlarmHoldoff() {

    // XXX CODE GOES HERE
}   // end of clearLowTempAlarmHoldoff()

// method to clear out the holdoff for the high temperatre alarm so that a new alarm can be sent immediately
void WLDAlarmProcessor::clearHighTempAlarmHoldoff() {

    // XXX CODE GOES HERE
}   // end of clearHighTempAlarmHoldoff()

// method to clear out the holdoff for the water leak alarm so that a new alarm can be sent immediately
void WLDAlarmProcessor::clearLeakAlarmHoldoff() {

    // XXX CODE GOES HERE
}

// Methods for debugging purposes

unsigned long WLDAlarmProcessor::get__lowTempAlarm_Holdoff() {

    // XXX CODE GOES HERE
}   // end of get__lowTempAlarm_Holdoff() 

unsigned long WLDAlarmProcessor::get__highTempAlarm_Holdoff() {

    // XXX CODE GOES HERE
}   // end of get__highTempAlarm_Holdoff() 


unsigned long WLDAlarmProcessor::get__leakAlarmHoldoff() {

    // XXX CODE GOES HERE
}   // end of get__leakAlarmHoldoff()
