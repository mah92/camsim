# بسم اله الرحمن الرحیم - هست کلید در گنج حکیم

# Nasir project for distorted airborne camera simulation

## Preface

This is an extended airborne camera simulation including geometric, photometric and temporal distortions.

It can be run online (capable of generating over 30frames/second) or store the images for offline usage.

This code is the result of less than a decade of need, experience, and coding on the field of robotics navigation.

It can be used for evaluating different image-processing algorithms under these kinds of distortions and parameters:

* Any Installation angle under aircraft
* Pinhole camera model
* Geometric distortions including Brown-Conrady 5 parameter distortion
* Photometric distortions including gaussian noise, vignetting, and a total gain(as time of day)
* Temporal distortions including delay, motion blur, rolling shutter and interlacing

The earth surface is assumed to have an arbitrary shape, formed by satellite and digital elevation maps; Or it can include just a calibration pattern.

The aircraft moves on an input path, determined by a descrete flight log of positions and angles in any frequency (which can consist of two to thousands of points)

The path generator algorithm, makes an eligible high frequency path with constrained velocities and accelerations.

A simple bias/noise simulation of usual airborne sensors including accelerometer, gyroscope, magnetometer(compass), pressure altimeter and gps is also included.

Hopefully, this simulator will be used in the proper hands, as it's name suggests.

## References
This simulator is based on the following article. Please cite the paper when using this toolbox or parts of it in an academic publication.

Mahmoudi, A., Sabzehparvar, M., & Mortazavi, M. (2021). A virtual environment for evaluation of computer vision algorithms under general airborne camera imperfections. Journal of Navigation, 1-21. doi:10.1017/S0373463321000060

## Installation

It is tested on ubuntu 16.04 and 18.04 LTS, although it also used to work on Android before simplifications.

Ensure that the proper GPU driver is installed to prevent bad behaviour like system-wide lockups.

A good graphic card ensures the speed of execution.

$sudo apt-get install cmake build-essential

$sudo apt-get install xdotool

$sudo apt-get install libplplot-dev

$sudo apt-get install libopenscenegraph-dev openscenegraph

$sudo apt-get install libopencv-dev (optional)

It is also possible to install libosgearth-dev, openscenegraph-plugin-osgearth if the OpenSceneGraph does not get installed correctly.

After installations, go to the program folder and run:

$mkdir build

$cd build

$cmake ..

$make -j4

remember 

## Typcal Errors
In Ubuntu 20.04 I also needed to do the following commands to prevent some complier linker errors:

sudo ln -s /usr/lib/x86_64-linux-gnu/libSM.so.6 /usr/lib/x86_64-linux-gnu/libSM.so

sudo ln -s /usr/lib/x86_64-linux-gnu/libICE.so.6 /usr/lib/x86_64-linux-gnu/libICE.so

sudo ln -s /usr/lib/x86_64-linux-gnu/libXext.so.6 /usr/lib/x86_64-linux-gnu/libXext.so

On run-time faults, try replacing between wxwidgets and qtwidget in plplots library (nsrPlot.cpp).

On error: "assertion 'G_IS_DBUS_CONNECTION (connection)' failed", solve by:

sudo apt purge fcitx-module-dbus

## Demos
This is a simulated flight on the FSR2015-ASL dataset path. 

Average height over ground is 110m, and the speed is run at 10x averaging to 100m/s.

The camera is set to 10fps, with a 15 degrees tilt angle.

Simulation Steps:

[![](http://img.youtube.com/vi/FHoJwCSKPYY/0.jpg)](http://www.youtube.com/watch?v=FHoJwCSKPYY "Simulation Steps")

Temporal distortions:

[![](http://img.youtube.com/vi/HQmMbud8VcA/0.jpg)](http://www.youtube.com/watch?v=HQmMbud8VcA "Temporal Distortions")

Notice the camera outage at extremely high speeds.

The input ideal images are rendered bigger, as temporal(and lens) distortions need data from outside the normal field-of-view. For extremely high speeds, this margin might not be enough. To prevent this problem, increase the extraMargin parameter (with the cost of slower runs).

Lens(geometric+photometric) distortions:

[![](http://img.youtube.com/vi/u17KExoSsLo/0.jpg)](http://www.youtube.com/watch?v=u17KExoSsLo "Lens Distortions")

Photometric distortions:

[![](http://img.youtube.com/vi/-Q92ijIwtIU/0.jpg)](http://www.youtube.com/watch?v=-Q92ijIwtIU "Photometric Distortions")

Installation angle errors:

[![](http://img.youtube.com/vi/-k1eqQOB8QE/0.jpg)](http://www.youtube.com/watch?v=-k1eqQOB8QE "Installation angle errors")

## Usage

1. Set addresses.txt for input maps - output log addresses

2. (Optional) vignet.bmp should be a 3 channel(RGB) bitmap, having the same resolution with output, each channel multiplies to corresponding channel

3. path.csv: aircraft path flight log

 A descrete flight path is input and the aircraft is controlled on it with bound velocity-accelerations.
 
 The frequency is arbitrary and even two points can be used.
 
 manual mode can be entered/exited at anytime by pressing 'm'

 columns:
 
 frame time(s, can start from any positive number),
 
 lat(deg)/x(m) towards north,
 
 lon(deg)/y(m) towards east,
 
 alt(m)/z(m) towards down,
 
 ac roll(deg),
 
 ac pitch(deg),
 
 ac yaw(deg, wrt. real north)
	
4. Parameters.xml (and included files in it)

	These files are read from top to bottom, bottom lines override upper ones
	
	Other xml files "with the same structure" can be included
	
	All meaningfull tags should be defined inside the "simParams" tag

	* mainParams
	
	renderWhat = "IMAGE" or "DEPTH"
	
	showWhat = "MAP" or "PATTERN"
	
	doWhat = "SAVE_BMP" or "IMAGE_PROC"

	* mapParams
	
	Rendered distance is minimum of maxDist and max horizon seen at maxAlt(spherical earth)
	
	when rendering depth, motion bluring & noise are deactivated(but not lens effect or rolling shutter or delay)
	
	depth format: vec4(floor(floor(Z)/255.)/255., fract(floor(Z)/255.), fract(Z), 1.);
	
	depth map (texCoord_realResMean)get problematic(a glosbe line) when camera looks at out of map
	
	zoom: if not set, is set automatically
	
	using dem in zooms equal or lower than 10x is canceled

	* patternParams
	
	location is laid horizontally on the zero ground at centerLat, centerLon of mapParams
	
	metricWidth: how much in meters the pattern width should be stretched

	* pathParams
	
	Input data format
	
	csvPositionFmt(for path.csv): "LLA"(Lat, Lon, Alt(wgs84-m)) as default, "XYA"(X, Y, Alt(wgs84-m) or "XYZ")
	
	trackerType: "0"-> descrete(hold), "1"-> linear interpolation, "2"-> linear interpolation + 1 controller loop, "3"-> linear interpolation + 2 controller loops	
	
	trackerType: "4"-> linear interpolation + 3 controller loops, "5"-> linear interpolation + 2 controller loops + output filter, "6"-> linear interpolation + input filter + 2 controller loops + output filter(works best)	
	
	accMax: max linear acceleration(m/s2) applied to reach interpolated path specified in csv file
	
	angAccMax: max angular acceleration(deg/s2) applied to reach interpolated path specified in csv file
	
	controlFreq: path follower controller frequency, also used for advancing time in calculating velocities and sensor data log, should be a multiple of cameraFps
	
	* pathParamsExtra
	speedFactor: the speed at which the flight log would be passes, default: 1x
	
	worldScale: scale every metric thing, including earth radius, dem elevations, aircraft path
	
	as scale is unobservable, scaling should theoretically not change camera output, (just scales metric sensors output)
	
	but is beneficial if you want to see things nearer than 1meter or farther than 65km
	
	altOffset: altitude added to data log

	* sensorParams
	
	Sensors output data format
	
	cameraFps: should be dividable by controlFreq, otherwise jitter occurs 
	
	phasePercent: time offset for first frame, between 0. and 1.
	
	first_frame_time == first row time in path.csv + phasePercent/cameraFps 

	* dynamicParams
	
	dynamic intrinsic parameters(move effect)
	
	dynamic distortion approximated by a linear model
	
	td: Center row delay of camera (usually positive)
	
	tr: Up2down(rolling shutter) delay of each screen(usually positive meaning upper rows be sooner)-->
	
	te: In-row(in-pixel) delay, can be tr/rows (usually positive)-->
	
	motionBlurExtraSamples: extra sample points used for creating motion blur(caused by in-row delay), is added to DIRECT_SAMPLING_POINTS (defined in frag. shader), increase if better motion blur wanted
	
	ti: odd(including 1st) rows are got from previous ti time (usually positive)
	
	tdErr: error in td in seconds over above nominal value
	
	trErr: error in tr in seconds over above nominal value
	
	* marginParams
	extraMargin: Ideally rendered image width and height multiplied by extraMargin before applying distortions so that static-dynamic distortions does not cause void regions
			
	Increase if void(black) regions in the borders are more than what expected by vignetting (static effects of lens) or if void regions apear in hard movements(by dynamic effects like delay, rolling shutter,...)
			
	extraZoom: Just for debug, zooms out

	* signalParams
	dayLight between[0,1]
	
	noiseAmpDynamic between[0,1], is 1 sigma of added white gaussian noise for each pixel in each channel, notice that if the output image is ment to be a grayscale image, the 3 channel averaging reduces the noise by 1/sqrt(3)~=0.57 factor.
	
	noiseAmpStatic1 between[0,1], is 1 sigma of added static gaussian noise for each pixel in gray channel, this is the typical signal caused by lens dirt
	
	noiseAmpStatic2 between[0,1], is 1 sigma of added gaussian noise for each col/row in gray channel, this is the typical noise present in night cameras
	
	final pixel = DayLight*pix + noise

	* camInAcEu
	
	ZYX Euler angles of camera in aircraft frame(in deg)
	
	Note that as engineers are more familiar with zyx euler angles for aircraft (and pan and tilt for camera),
	
	just here, camera front axis is assumed on its x axis, 
	
	so yaw==pan, pitch==-tilt, roll==roll
	
	(roll= 0 , pitch= 0 , yaw=0) means looking to aircraft x(nose) direction, top of camera in upward direction
	
	(roll= 0 , pitch=-90, yaw=0) means looking to aircraft z(down) direction, top of camera in nose direction,
	
	(roll=-90, pitch=-90, yaw=0) means looking to aircraft z(down) direction, top of camera on left wing,
	
	err: the default installation angle error in degrees for euler rotations,
	
	if not set directly in below fields, the final errors would be applied with this amplitude in random directions
	
	xErr: installation error angle (over above nominal values)
	
	yErr: installation error angle (over above nominal values)
	
	zErr: installation error angle (over above nominal values)

	* Ideal camera intrisic parameters
	
	scaleFactor: scales parameters(width, height, fx, fy, ox, oy) for when you need higher, lower resolution with same lens
	
	width, height: should match settings.render2stream_width, settings.render2stream_height
	
	You should provide {ox, oy} or {oxOffset, oyOffset}
	
	oxOffset, oyOffset: image center offsets in pixels(default:0)
			
	ox = oxOffset + width /2.
	
	oy = oyOffset + height/2.

	ox, oy are found in 3x3 cam matrix(K)
	
	ox, oxOffset increase means shift from left to right
	
	oy, oyOffset increase means shift from up to down
	
	You should provide one of these 3 parameters: fovX & fovY(in ideal lens camera), just f, or fx & fy
	
	fovX: Field of view in x(left-right) direction, fx := ((width /2.)/tan(fov_x/2.*pi/180.))
	        
	fovY: Field of view in y(up  -down ) direction, fy := ((height/2.)/tan(fov_y/2.*pi/180.))
	        
	f: (=fx=fy) is equal canonical length in pixels(found in 3x3 cam matrix)
	        
	fx, fy: is x,y canonical length in pixels(found in 3x3 cam matrix)
	
	fErr : in pixels
	
	oxErr: in pixels
	
	oyErr: in pixels

	* 5-8 param lens distortion intrinsic parameters (used for lens correction in OpenCV)
	
	As we use a fragment shader to make the lens model fill every pixel,...
	
	the inverse(undistortion) model is needed for making the distortion;...
	
	As there is no complete analytical solution to inverse model,...
	
	We use an iterative solution
	
	k1Err: typical value: 0.03
	
	k2Err: typical value: 0.03
	
	t1Err: typical value: 0.001
	
	t2Err: typical value: 0.001
	
	k3Err: typical value: 0.03

	* Vignetting is the effect of camera lens holder ring or other objects darkening the frame in the borders
	
	vignetFile="" means no vignetting
	
	Even if no vigneeting is used, the view will be limited under strong lens effect,
	
	as our rendering is limited, see extraMargin parameter
	
	vignetThresh: maximum light at which the vignetTexture is assumed opaque
	
	vignetThresh2: minimum light at which the vignetTexture is assumed transparent
	
	between these thresholds, the vignet is multiplied by output seperately in each channel
	
	If you mean simple grayscale vignet, remember to make the vignet gray scale before usage
	
	In Gimp->Right click on image->image->mode->grayscale, File->Export As-> use .bmp-->
	
	* includeParams
	
	Included Camera file, note that the included file has the same format(and possible tags) with this file.
	
	Note that the main xml file and all the included files will be copied into the log folder so that it can be checked in a later phase

## Libraries

Remember to have enough patience if you want to understand the whole program. Although I have tried hard to make the program readable, years of working cannot be understood in a glance.

These independant libraries can be reviewed before reviewing the main program:

* pugixml
* libtiff
* geoid
* MathLib
* numbuffer
* plot3d

Impatient readers can just check the glsl shaders under View/nsrOSGShaders.cpp

## Shortkeys

Remember: Run the program with sudo if you want keyboard inputs to work

### Manual piloting shortkeys
Working with manual aircraft path is possible using short keys described as follows:
* 'm':  entering/exiting  manual path mode
* right/left/up/down arrow keys: moving aircraft to east/west/north/south respectively (manual mode)
* page up/down: increasing/decreasing aircraft  (manual mode)
* '+'/'-': increasing/decreasing aircraft yawaltitude angle (manual mode)

### Investigating through diagrams
Being used to matlab plots, I was searching for a similar ploting toolbox in C/C++ with no success, so I wrote a similar library based on plplot.

It is not very nice but is usable. Navigating through diagrams is possible using short keys described as follows:
* Alt+right/left   arrows: Navigate between different figures
* Alt+up/down      arrows: Navigate between different windows inside a figure
* Ctrl+right/left  arrows: Zoom In/Out Horizontally  
* Ctrl+up/down     arrows: Zoom In/Out Vertically
* Shift+right/left arrows: Shift Horizontally
* Shift+up/down    arrows: Shift Vertically

### Other Shortkeys
* 'c'/'q': will stop the simulation and stop for final review by user
* 'Esc': will close the simulation instantly

## Notes
* If shaking is seen in the shown image(or calib pattern), make it bigger

	Assume you have rendered a 2mx2m image on the earth surface. 
	
	As the earth surface is placed in ECEF coordinates, objects have very big coordinates, meaning 6,400km+
	
	The openscenegraph engine can not render such big coordinates without numerical errors in order of tens of centimeters.
	
	To eliminate these errors, use bigger tiles/calibration patterns, in order of 1000 meters.
	
	TODO: use a coordinate center near to earth surface

* If flickering is seen on far mountains or objects farther than 65Km are needed to be seen, decrease worldScale in Parameters.xml or increase both zNear, zFar in nsrOsgCamScene.cpp. Openscenegraph has limited resolution for depth.

* Mountains shape seem unsusual

	Increase TILES_DEM_RES_FACTOR in View/nsrMapDrawable2.h file

* High or low accelerometer/ gyro rms

    change 1st order filter constants: TAW, TAW2, ... in nsrPoseSim.cpp

* Opencv circular or chessboard patterns -cannot- calibrate fisheye lens,

 as whole pattern become non-detectable near the edges. 
 
 so near the edges calibration(near vignetts) become unusable
 
 use opencv "charuco", kalibr "april", or better, agisoft-lens "wide chess" pattern(if focus in close range is not a problem)

* Remember to correct alt and angle offsets in nsrPoseSim.cpp line 194
