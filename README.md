# cmsr_new_member_opti
ECE Integration into Carnegie Mellon Solar Racing's New Member Initiative

## Overview
This repository contains the code and wiring schematics for a bi-directional bluetooth communication system between a user and a racing boat with Arduino's and nRF2401 bluetooth modules. The user controls a single, 2-axis joystick to send commands to the boat, while the boat sends back data to the user. This data varies in practicality, ranging from less useful luminance(photoresistor) to very important acceleration and velocity(IMU). 

## Communication  
Timing is key in this system since the entire bi-directional communication can break if both endpoints attempt to send or receive data at the same time. Rather, one endpoint must transmit data as the other one waits for this message. Our system has the boat constantly send data and listen in periodic cycles until it receives a response from the user. The boat will not drive until receiving commands. The user will not send any commands until it receives a signal from the boat.

Occasionally, one endpoint will "disconnect" from the other and the system will fall out of synch. # In this case, simply power off the user controls by disconnecting the Arduino from its USB power source. The builtin LED on the Arduino will light up when communication is regained. 

## Controls
This system features a single 2-axis joystick, which unfortunately has poor sensitivity and only registers intermediate(not 0 or max of 1023) values near the forward axis. Driving forward simply spins both motors, whereas turning left or right turns off one motor and drives the other at max speed to create a moment. In this case, our system has the boat drive the right motor at max speed when the joystick is jammed forward anywhere in the 2nd Quadrant of a unit circle. The left motor drives at max speed in the 1st Quadrant. Only near the positive y-axis will the inactive motor slowly begin to power on, which allows for stability. Backwards driving is not allowed. We define this axis according to the orientation of the pin headings on the joystick itself with upright characters pointing up towards the y-axis. This means the cables point left when you hold the joystick upright.
