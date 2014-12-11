
     =============================================================================
    |                                                                             |
    |                      readme.txt  -  by team megalodon                       |
    |Casey O’Rourke (caseyso) ⋅Jonathan Hirokawa (johiro) ⋅ Mike DiNicola (mdinic)|
    |                                                                             |
     =============================================================================

      How to implement the manual & automatic Syma S107G Gumstix flight system!

---------------------------------------------------------------------------------------

                            ######## Kinect ########
The Kinect portion of the program have the following requirements:
1. Processing: https://processing.org/
2. Installing SimpleOpenNI (requires root access): https://code.google.com/p/simple-openni/wiki/Installation

Once these items are install, you can use the Processing IDE to load the kinect_code/closestObject/closest_object_tracker.pde file. Assuming the Bluetooth connection is already setup, the setup() function in the program needs to find the proper serial port to write to. As of this writing, it is currently hardcoded. If you run the program, will print the list of ports on your machine you and you will have to change line 21 to specify the proper port to write to. 

NOTE: It is best to have the Gumstix user level program running before running the Processing program.


                            ######## Gumstix ########

                                **** User ****
To run the user level program a Bluetooth connection between the Gumstix and PC with the Kinect must already be established and bound to /dev/rfcomm0. The user will run the /gumstix_code/bluetooth/rfcomm_list executable before running the Processing program. This allows for the user level program hold in a loop while you setup the Processing program on the PC.

                                **** Kernel ****

                              **** Controller ****

The seven files included in the gumstix_code/controller directory (including Makefile) are required to compile the GUI binary. Source code is written in C, and there is an additional .pro file which is created by Qt and has not been modified. Running make creates a binary simply called “controller.” This binary must be transferred to the gumstix via zmodem, as well as the following: "fonts libQtCore.so.4 libQtGui.so.4 libQtNetwork.so.4 ld-uClibc.so.0 libc.so.0 libm.so.0 libstdc++.so.6” from folder: $EC535/gumstix/oe/qt/lib to /usr/lib on the gumstix. Note that they will likely be too large and will need to be copied to an SD card (/media/card/lib). Create library links to these (excluding /fonts). Follow this example: ln -s /media/card/lib/libQtCore.so.4 libQtCore.so.4. Finally, export some variables in gumstix:

    export QWS_MOUSE_PROTO='tslib:/dev/input/touchscreen0'
    export TSLIB_CONFFILE=/etc/ts.conf
    export TSLIB_PLUGINDIR=/usr/lib
    export TSLIB_TSDEVICE=/dev/input/event0
    export TSLIB_FBDEVICE=/dev/fb0
    export TSLIB_CONSOLEDEVICE=/dev/tty
    export QT_QWS_FONTDIR=/media/card/lib/fonts
    export TSLIB_PLUGINDIR=/usr/lib/ts

You can now run the binary (./controller -qws). If needed, calibrate the touchscreen first (ts_calibrate).

                            ######## Arduino ########

                                **** Driver ****

                                **** Circuit ****
See https://github.com/tab262/ec535/blob/master/ArduinoCircuit.png for diagram of circuit. Pins to connect between the Arduion and Gumstix are as follows:

Arduino   |    Gumstix
13        |    29
12        |    30
11        |    101 
10        |    17  
9         |    113 
8         |    28  
6         |    31







