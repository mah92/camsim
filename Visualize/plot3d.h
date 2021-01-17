#ifndef __PLOT3D_H__
#define __PLOT3D_H__

#include <opencv2/core/core.hpp>
#include <vector>
#include <array>

#include "Core/Matlib/nsrMathLib.h"
#include "Core/Matlib/nsrQuat.h"

class Plot3d
{
public:
	Plot3d(cv::String _win_name);

	//Settings//////////////
	nsr::Quat quViewCam; //looking downwards, north is up
	double fov_x; //in degrees
	int distance; //initial and current distance
	int width, height;
	cv::String win_name;

	std::vector<std::vector<std::array<double, 8> > > posQuCam;
	std::vector<std::array<double, 8> > posQuTempCam;
	std::vector<std::array<double, 6> > pointProp;

	float f, ox, oy;
	cv::Mat image0;
	bool drag;
	int last_x, last_y;
	double last_draw_time;

	Matrice posViewCam, origin, featurePos;

	inline void pos2pix(double x, double y, double z, float &pix_x, float &pix_y)
	{
		featurePos.reinit(3, 1);
		featurePos.fill2(x, y, z);
		featurePos = quViewCam * (featurePos - posViewCam); //quat is CI2B
		pix_x = ox + f * (featurePos.x() / featurePos.z());
		pix_y = oy + f * (featurePos.y() / featurePos.z());
	}

	void drawCam(int cam_color, Matrice &posCam, nsr::Quat &quCam);

public:
	void insertCam(int cam_index, double time_stamp, double x, double y, double z, double e1, double e2, double e3, double et);

	//Negative timestamp to clear all
	void cutCamAfterTimeStamp(int cam_index, double time_stamp);
	void insertTempCam(int cam_index, double time_stamp, double x, double y, double z, double e1, double e2, double e3, double et);
	void insertFeature(double x, double y, double z, cv::Scalar color = cv::Scalar(0, 255, 0));

	void updateView();
};

void plot3dtest();

#endif
