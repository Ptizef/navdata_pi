# navdata_pi

The purpose is to show more informations about the route followed that can be seen in the console but also a global view of the trip.
There are two parts in the main dialog
** The first part shows for all active route points BRG, RNG, TTG, ETA mean speed at VMG or SOG in a grid.
The dialog can't be opened without an active route.
Once a route is activated, the plugin shows the informations above for all route points.
At the start, the active point is always the first in the left grid column  and is marked by a small blinking red flag.
The number of route points visible (thus the size of the dialog) can be choosen by dragging the right side of the dialog.
If a point in particularity has to be followed, a left click (or touch) on this point in the canvas will mark it by a blinking green circle and in hits grid column by a small blinking green flag and make it always visible.

** The second part shows a summary of the current trip. The data are based on the current active track. At any moment when a track is active and the dialog is opened, the data shows start date-time, time spend, distance since departure and mean spead since depature.
Off course if there is no track opened, the data are empty.

There are very few settings:
Clicking on the settings button (upper left) will open a small dialog where it is possible to hide or show the trip part of the dialog and the choice to use SOG or VMG as base for TTG and ETA.

installation
============


remark : in any case the OpenCPN installation must be a standard official intall
            (intaller for Windows and from the ppa from linus. see opencpn.org/ downloads)
** Windows
    use the installer navdata_pi-x.x-win32.exe 
    it was tested on W10 only but should work on all recent Windows.

** Linux(Ubuntu and derived)
    install with navdata_pi_x.x-x_amd64
    
****    warning
            It was Compiled and tested on a Linux 

Compiling
=========
Get the sources
* git clone https://github.com/Ptizef/navdata_pi.git

The pugin is compiled as standalone

** Windows
    perequisites (Visual studio 2017 only)
        Be sure you are able to compile windows including building the apckage
        Eventually read and follow instructions in opencpn.org manual/ Developer manual/ Developer guide/ Compliling on Windows
        then in /compile plugins and build package

    compile OpenCPN then

*    $ cd sources directory
*    $ mkdir build
*    $ cd build
        in this build directory, copy the opecpn.lib from OpenCPN\build\release\opencpn.lib
*    cmake -T v141_xp ..
*    cmake --build .                  (only for debugging)
*    cmake --build . --config release
*    cmake --build . --config release --target package

** Linux(Ubuntu and derived)
    perequisites
        the same as to compile OpenCPN
        Eventually read and follow instructions in opencpn.org manual/ Developer manual/ Developer guide/ Compliling on Linux

*    $ cd sources directory
*    $ mkdir build
*    $ cd build
*    $ cmake ../
*    $ make
*    $ make install
    or
*    $ make package (you may have to install the rpm package)


