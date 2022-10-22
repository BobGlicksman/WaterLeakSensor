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
#ifndef wldap
#define wldap

#include "application.h"

class WLDAlarmProcessor  {
    private:
        // Constants
        const unsigned int ONE_MINUTE = 60000; // one minute = 60000 milliseconds
        const unsigned int ONE_DAY = ONE_MINUTE * 60 * 24;  // 60 minutes per hour, 24 hours per day

        // Variables
        float _temperature;  // holds the reported temperature that caused the alarm
        unsigned long _lowTempAlarm_Holdoff;   // holdoff time for next low temp alarm
        unsigned long _highTempAlarm_Holdoff;   // holdoff time for next high temp alarm
        unsigned long _leakAlarmHoldoff;    // holdoff time for next leak alarm
        
        // Private methods (internal use only)
    
    public:
        // Constructor
        WLDAlarmProcessor();

        // Initialization
        void begin();
        
        // Methods for handling alarms, holdoffs and filed testing
        void sendLowTemperatureAlarm(float theAlarmTemperature);
        void sendHighTemperatureAlarm(float theAlarmTemperature);
        void sendWaterLeakAlarm();
        void sendTestAlarm();      // for field testing purposes

        void clearLowTempAlarmHoldoff();    // clear the one day holdoff between new alarms and for testing
        void clearHighTempAlarmHoldoff();   // clear the one day holdoff between new alarms and for testing
        void clearLeakAlarmHoldoff();       // clear the one day holdoff between new alarms and for testing

        // Methods for debugging purposes
        unsigned long get__lowTempAlarm_Holdoff();
        unsigned long get__highTempAlarm_Holdoff();
        unsigned long get__leakAlarmHoldoff();
};

#endif
