# WaterLeakDetector - Version 2
The Water Leak Detector (WLD) is an IoT device that can monitor two specific locations for the presence of water. 
When water is detected an alarm will sound and an SMS text alert will be sent to the user's mobile phone. 

A typical use case is to deploy the WLD in a laundry room where it can monitor water leaks that might occur from a washing machine, 
hot water heater, or sewer backup.

The WLD also includes a temperature and humidity sensor. These values are displayed on a dial and also in a smartphone app.  The
smartphone app (Android only at this time) is included in this project.  High and low temperature limits may be set via the smartphone
app.  When either limit is hit, an sms text alert will be sent to the user's mobile phone.  The smartphone app allows the user to set the
high and low temperature limits.  The app displays the current temperature, relative humidity, and the status of all three alarms (low
temperature, high temperature, water leak).

This version (version 2) of the WLD eliminates Blynk and IFTTT integrations, in favor or our current standard techniques for app development,
alarms and alerts, and for logging to Google cloud spreadsheets.  For details, see: 

https://github.com/TeamPracticalProjects/Connectivity_Tools_with_Particle_Devices

Begin with the file WLD Read Me First.pdf

![front view completed project](Photos/Finished%20pointerB.JPG?raw=true "Completed Project")

