This interface operates an az/el rotator built from a Pelco PT1250PP pan/tilt camera mount.

Each axis is driven by a 120VAC motor. Each motor is directional and has three-leads. One lead is connected to neutral, and each other lead corresponds to a direction. 

There is no brake.

Protocol is EASYCOMM II, tested with hamlib on Linux.

All files created by me are released under the GPL.

IMPORTANT NOTE:

The camera pan/tilt mount that I used has 120VAC motors. For maximum safety, my design includes
a Ground Fault Interrupter (GFI) and a fuse on the AC input. 

