/* SERVOCAL: program to calibrate the servo.
    The servo is connected to a 3.3 volt to 5 volt converter.  The converter
    input is connected to Photon pin A5.  A 4.7K ohm pulldown resistor is used
    to ensure that the servo is quiet while the Photon is being flashed/reset.

    The Particle Console is used to specify positions for the servo.  Complete 
    instructions can be found in the document:
    
    https://github.com/TeamPracticalProjects/WaterLeakSensor/blob/master/Documentation/Servo_Meter_Calibration.pdf
    
    In order to calibrate the servo, the servo with a pointer is mounted on to the meter
    scale.  The user specifies positions for the servo and observes where the
    servo pointer is on the meter scale.  The position values for the scale minimum
    and for the scale maximum are determined by trial and error and the final
    values are noted for use in the WaterLeakDetector program.

    date: 3/07/17, by Bob Glicksman; updated 3/20/17 by Jim Schrempp; comments
    updated 9/14/17 by Bob Glicksman.

    (c) 2017 by Bob Glicksman, Jim Schrempp, and Team Practical Projects
*/

Servo myservo;  // create servo object to control a servo

const int MIN_POS = 5;  // the minimum position value allowed
const int MAX_POS = 175;  // the maximum position value allowed

int mg_position = (MAX_POS - MIN_POS)/2;    // global variable to store the servo position

void setup() {
	Particle.function("Servo", servoCmd);
	Particle.function("ServoPlus5", servoPlus5);
	Particle.function("ServoMinus2", servoMinus2);

	myservo.attach(A5);  // attaches pin A5 to the servo object
	delay(2000);  // wait 2 seconds before continuing

} // end of setup

void loop() {
	myservo.write(mg_position);
} // end of loop

int servoCmd(String cmd) {
	int servoPosition = cmd.toInt(); // get the user's desired position for the servo

	if(servoPosition > MAX_POS)  {
		mg_position = MAX_POS;
	}
	else if(servoPosition < MIN_POS)  {
		mg_position = MIN_POS;
	}
	else  {
		mg_position = servoPosition;
	}

	return mg_position;
}  // end of servoCommand


int servoPlus5(String cmd) {

	int new_position = mg_position + 5;
	servoCmd(String(new_position));
	return mg_position;

}

int servoMinus2(String cmd) {

	int new_position = mg_position - 2;
	servoCmd(String(new_position));
	return mg_position;

}
