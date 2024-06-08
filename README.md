# autoBot
Arduino Sketch projects for embedded AI courses

1. Summary

   This project is to build a simple robotic car that is able to understand voice commands and take action to follow the commands.
   The commands are: stop, go forward, turn left, turn right, go backward
   Also, the robot car can avoid object in front of it.
2. Hardware

   The autobot is build upon robot car kit with UNO R3,  L298N driver module, V5 Sensor shielding, Ultrasonic Module, and two motors plus power module, etc.
   To have a voice command feature, an Arduino Nano 33 BE Sense V2 board is used with loaded deep learning network model for voice understanding.
   Nano 33 BLE board is equipped with microphone which takes in the command signal, and the voice model understands the signal and controls the UNO board for driving direction changes.
   The gyro sensor build-in with the Nano board also provide additional navagation controls, e.g. when turn left, the Nano board controls turn 90 deg only.
3. Software
   
   The programming is based on Arduino IDE with programming language C/C++.
   The voice model is pre-trained using Picovoice library (English). Programming of the voice model can be found in the link https://picovoice.ai/
4. Links

   Video for test run the autoBot.


