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
#ifndef wldap
#define wldap

#include "application.h"

class WLDAlarmProcessor  {
    private:
        // Constants
        const unsigned int ONE_MINUTE = 60000; // one minute = 60000 milliseconds
        // const unsigned int ONE_DAY = ONE_MINUTE * 60 * 24;  // 60 minutes per hour, 24 hours per day
        const unsigned int ONE_DAY = ONE_MINUTE;    // FOR TESTING ONLY
        
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
