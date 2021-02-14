//ALLAH
#ifndef __ANDROID__
#include "/usr/include/time.h"
#endif

#include "./nsrImageProc.h"

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <iostream>
#include <fstream>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/opencv.hpp>

#include "nsrCore.h"
#include "Core/Matlib/nsrQuat.h"
#include "nsrIndex.h"

#include "nsrSimParamReader.h"
#include "nsrPoseSim.h"
#include "Visualize/plot3d.h"
#include "Visualize/nsrVisualize.h"
#include "View/nsrGeoLib.h"

#include <osg/PositionAttitudeTransform>

#undef TAG
#define TAG "C:ImageProc:"

using namespace cv;
using namespace std;

#ifdef __cplusplus
extern "C" {
#endif

Plot3d *plt3d = NULL;
cv::Ptr<cv::VideoWriter> camcorder = NULL;

int nsrInitImageProc()
{
	plt3d = new Plot3d("image");

	return 0;
}

static Mat colorInput;

int nsrImageProc(uint8_t *data, int width, int height, int channels, double frame_timestamp_s)
{
	int i;
	int type;
	static double last_time = 0;
	
	//Make matrix///////////////////////////////////////////////
	if(channels == 1) type = CV_8UC1;
	if(channels == 3) type = CV_8UC3;
	if(channels == 4) type = CV_8UC4;

	colorInput = Mat(height, width, type, data);//, size_t step=AUTO_STEP);

    cv::Mat colorInput2;
    cvtColor(colorInput, colorInput2, cv::COLOR_RGBA2BGR);
     
	imshow("colorInput", colorInput2);
	cv::waitKey(5);
	
	//Save vid//////////////////////////////////////////////////////
#if 1
	if(camcorder==NULL) {
		//camcorder = new cv::VideoWriter("./test.mp4", cv::CAP_ANY, VideoWriter::fourcc('m','p','4','v'), (int)param_camera_fps, cv::Size(width, height), true /*bool isColor*/ );
        camcorder = new cv::VideoWriter("appsrc ! autovideoconvert ! x264enc ! matroskamux ! filesink location=test.mkv sync=false", 0, (int)param_camera_fps, cv::Size(width, height), true /*bool isColor*/ );
	}
	assert(colorInput2.channels() == 3); //otherwise video saving would not work
	
	//if(camcorder) *(camcorder) << colorInput2;
    if(camcorder) camcorder->write(colorInput2);
    //if(myTime() > 10) camcorder->release();
    

#endif	
	
	//calc position////////////////////////////////////////
	osg::Vec3d lla, lla0, xyz;
	lla0 = osg::Vec3d(param_map_center_lat, param_map_center_lon, 0.);

	//Have ground truth, plot3d just for visualization
	osg::Vec3d llaGT, xyzGT;
	osg::Quat qu_cam_gt_;
	Matrice locGT(3, 1);
	nsr::Quat qu_cam_gt;
	nsrPoseMakerExtract(frame_timestamp_s, 0, &llaGT, NULL, NULL, NULL, NULL /*&qu_ac_*/, NULL, NULL, &qu_cam_gt_, NULL); //cam/ac, in cam
	xyzGT =  LLA2NED(llaGT, lla0); //OK
	locGT.fill2(xyzGT.x(), xyzGT.y(), xyzGT.z());
	//qu_ac_gt.set(qu_ac_gt_.x(),qu_ac_gt_.y(),qu_ac_gt_.z(),qu_ac_gt_.w());
	qu_cam_gt.set(qu_cam_gt_.x(), qu_cam_gt_.y(), qu_cam_gt_.z(), qu_cam_gt_.w());
	//printf("))%f, %f, %f,, %f, %f, %f, %f\n", locGT.x(), locGT.y(), locGT.z(), qu_cam_gt.e1,qu_cam_gt.e2,qu_cam_gt.e3,qu_cam_gt.et); fflush(stdout);
	if(time_reached(.1, 0., frame_timestamp_s, last_time))
		plt3d->insertCam(0, 0, locGT.x(), locGT.y(), locGT.z(), qu_cam_gt.e1, qu_cam_gt.e2, qu_cam_gt.e3, qu_cam_gt.et);

	plt3d->updateView(); //time reached contained
	
	last_time = frame_timestamp_s;
    
    return 0;
}

void nsrEndImageProc()
{
	if(plt3d != NULL)
		delete plt3d;
	
	if(camcorder!=NULL) {
        camcorder->release();
		delete camcorder;
    }
}

#ifdef __cplusplus
}
#endif

