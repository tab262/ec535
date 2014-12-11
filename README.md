
     =============================================================================
    |                                                                             |
    |                      readme.txt  -  by team megalodon                       |
    |Casey O’Rourke (caseyso) ⋅Jonathan Hirokawa (johiro) ⋅ Mike DiNicola (mdinic)|
    |                                                                             |
     =============================================================================

      How to implement the manual & automatic Syma S107G Gumstix flight system!

---------------------------------------------------------------------------------------

                            ######## Kinect ########

                            ######## Gumstix ########

                                **** User ****

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







