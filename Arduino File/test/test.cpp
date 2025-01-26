//rduino Code - Move from A to B by a specific number of steps with set a accelerationArduino
//Arduino Code - Move from A to B by a specific number of steps with set speed and acceleration
 
// Include the Arduino Stepper.h library:
#include <AccelStepper.h> //Include the AccelStepper library
 
// Define the motor pins:
#define MP1  9 // IN1 on the ULN2003
#define MP2  10 // IN2 on the ULN2003
#define MP3  11 // IN3 on the ULN2003
#define MP4  12 // IN4 on the ULN2003
 
#define MotorInterfaceType 8 // Define the interface type as 8 = 4 wires * step factor (2 for half step)
AccelStepper stepper = AccelStepper(MotorInterfaceType, MP1, MP3, MP2, MP4);//Define the pin sequence (IN1-IN3-IN2-IN4)
const int SPR = 2048;//Steps per revolution
 
void setup() {
  stepper.setMaxSpeed(1000);//Set the maximum motor speed in steps per second
  stepper.setAcceleration(1000);//Set the maximum acceleration in steps per second^2
}
 
void loop() {
  stepper.moveTo(3*SPR); //Set the target motor position (i.e. turn motor for 3 full revolutions)
  stepper.runToPosition(); // Run the motor to the target position 
  delay(1000);
  stepper.moveTo(-3*SPR);//Same as above: Set the target motor position (i.e. turn motor for 3 full revolutions)
  stepper.runToPosition(); // Run the motor to the target position 
  delay(1000);
}