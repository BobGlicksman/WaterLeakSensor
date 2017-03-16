/* SERVOCAL: program to calibrate the servo.
    The servo is connected to a 3.3 volt to 5 volt converter.  The converter
    input is connected to Photon pin D3.  A 5.6K ohm pulldown resistor is used
    to ensure that the servo is quiet while the Photon is being flashed/reset.

    The SIS debug client is used to specify positions for the servo.  In order
    to calibrate the servo, the servo with a pointer is mounted on to the meter
    scale.  The user specifies positions for the sero and observes where the
    servo pointer is on the meter scale.  The position values for the scale minimum
    and for the scale maximum are determined by trial and error and the final
    values are noted for use in the WaterLeakDetector program.

    date: 3/07/17, by Bob Glicksman

    (c) 2017 by Bob Glicksman, Jim Schrempp, and team Practical Projects
*/

Servo myservo;  // create servo object to control a servo

const int MIN_POS = 5;  // the minimum position value allowed
const int MAX_POS = 175;  // the maximum position value allowed

int pos = (MAX_POS - MIN_POS)/2;    // global variable to store the servo position

void setup() {
    Particle.function("Servo", servoCmd);

    myservo.attach(A5);  // attaches pin A5 to the servo object
    delay(2000);  // wait 2 seconds before continuing

} // end of setup

void loop() {
    myservo.write(pos);
} // end of loop

int servoCmd(String cmd) {
    int servoPosition = cmd.toInt(); // get the user's desired position for the servo

    if(servoPosition > MAX_POS)  {
        pos = MAX_POS;
    }
    else if(servoPosition < MIN_POS)  {
        pos = MIN_POS;
    }
    else  {
        pos = servoPosition;
    }

    return pos;
}  // end of servoCommand
