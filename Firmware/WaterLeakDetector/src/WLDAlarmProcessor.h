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
#ifndef wldap
#define wldap

#include "application.h"

class WLDAlarmProcessor  {
    private:
        // Constants
        const unsigned int ONE_MINUTE = 60000; // one minute = 60000 milliseconds
        const unsigned int ONE_DAY = ONE_MINUTE * 60 * 24;  // 60 minutes per hour, 24 hours per day

        // Variables
        bool _lowTempAlarmArm;      // indicate alarm arming
        bool _highTempAlarmArm;     // indicate alarm arming
        bool _leakAlarmArm;         // indicate alarm arming

        unsigned long _lowTempLastAlarm;    // time of the last alarm
        unsigned long _highTempLastAlarm;   // time of the last alarm
        unsigned long _leakLastAlarm;       // time of the last alarm
        
        // Private methods (internal use only)
        unsigned long diff(unsigned long current, unsigned long last);
    
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

        void armLowTempAlarm();     // forced arming of the alarm
        void armHighTempAlarm();    // forced arming of the alarm
        void armLeakAlarm();        // forced arming of the alarm

        // Methods for debugging purposes
        unsigned long get_lowTempLastAlarm();
        unsigned long get_highTempLastAlarm();
        unsigned long get_leakLastAlarm();
};

#endif
