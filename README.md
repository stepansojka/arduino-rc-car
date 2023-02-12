# Arduino RC Car

Controls an RC car using [RC Joystick Controll app](https://play.google.com/store/apps/details?id=com.andico.control.joystick).

The app connects to the Arduino via a [HC-06 bluetooth module](https://components101.com/wireless/hc-06-bluetooth-module-pinout-datasheet).
The Arduino reads commands from the app and controls the car's motor and servo
via [L298P H-bridhge motor driver](https://electropeak.com/learn/interfacing-l298p-h-bridge-motor-driver-shield-with-arduino/).
