#include "./plot3d.h"

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

static void onMouse(int event, int x, int y, int, void* data);
static void distanceCallback(int, void* data);

Plot3d::Plot3d(cv::String _win_name)
{
	//Settings/////////////////////////
#define AXES_LENGTH 10.
#define PYRAMID_HEIGHT 10.
#define PYRAMID_HALF_BASE 5.

	//phi, theta, psi
	//quViewCam.setEu(0.*M_PI/180., 0.*M_PI/180., 90.*M_PI/180.); //looking downwards, north is right
	quViewCam.setEu(0.*M_PI / 180., 0.*M_PI / 180., 90.*M_PI / 180.); //looking downwards, north is up

	fov_x = 10.; //in degrees
	distance = 5000;
	//width = 720; height = 576;
	width = 1280; height = 720;
	win_name = _win_name;

	origin.reinit(3, 1);
	origin.fill2(0., 0., -900.);
	///////////////////////////////////

	drag = false;
	last_x = 0;
	last_y = 0;
	last_draw_time = -100;

	image0 = cv::Mat(height, width, CV_8UC3);
	image0 = 0;

	f = ((width - 1.) / 2.) / tan(fov_x / 2.*M_PI / 180.);
	ox = (width - 1.) / 2.;
	oy = (height - 1.) / 2.;

	posQuCam.resize(3);
	posQuTempCam.resize(3);
	int i;
	for(i = 0; i < posQuTempCam.size(); i++)
		posQuTempCam[i][6] = -1000.; //init quat with unacceptable values
}

void Plot3d::insertCam(int cam_index, double time_stamp, double x, double y, double z, double e1, double e2, double e3, double et)
{
	std::array<double, 8> posQu;
	posQu[0] = x;
	posQu[1] = y;
	posQu[2] = z;

	posQu[3] = e1;
	posQu[4] = e2;
	posQu[5] = e3;
	posQu[6] = et;

	posQu[7] = time_stamp;

	posQuCam[cam_index].push_back(posQu);
}

void Plot3d::cutCamAfterTimeStamp(int cam_index, double time_stamp)
{
	int i;

	//Cut all
	if(time_stamp < 0) {
		posQuCam[cam_index].resize(0);
		return;
	}

	//Search start from end of queue
	for(i = posQuCam[cam_index].size() - 1; i >= 0; i--) {
		if(posQuCam[cam_index][i][7] <= time_stamp + 0.0001) {//found time_stamp
			posQuCam[cam_index].resize(i); //discard found time_stamp
			//posQuCam[cam_index].resize(i+1); //keep found time_stamp
			return;
		}
	}

	posQuCam[cam_index].resize(0);
}

void Plot3d::insertTempCam(int cam_index, double time_stamp, double x, double y, double z, double e1, double e2, double e3, double et)
{
	std::array<double, 8> posQu;
	posQu[0] = x;
	posQu[1] = y;
	posQu[2] = z;

	posQu[3] = e1;
	posQu[4] = e2;
	posQu[5] = e3;
	posQu[6] = et;

	posQu[7] = time_stamp;

	posQuTempCam[cam_index] = posQu;
}

void Plot3d::insertFeature(double x, double y, double z, cv::Scalar color)
{
	std::array<double, 6> point;
	point[0] = x;
	point[1] = y;
	point[2] = z;
	point[3] = color[0];
	point[4] = color[1];
	point[5] = color[2];
	pointProp.push_back(point);
}

void Plot3d::drawCam(int cam_color, Matrice &posCam, nsr::Quat &quCam)
{
	Matrice pos(3, 1);
	cv::Scalar color;
	float pix_x[5], pix_y[5];
	
	if(cam_color == 0)
		color = cv::Scalar(0, 255, 0); //real, green
	else if(cam_color == 1)
		color = cv::Scalar(0, 0, 255); //estim, red
	else //cam_color==2
		color = cv::Scalar(0, 128, 128); //

	pos.fill2(0., 0., 0.); //local pose
	pos = posCam + quCam.conj() * pos; //quat is CI2B, global pose
	pos2pix(pos.x(), pos.y(), pos.z(), pix_x[0], pix_y[0]);

	pos.fill2(+PYRAMID_HALF_BASE, +PYRAMID_HALF_BASE, PYRAMID_HEIGHT); //local pose
	pos = posCam + quCam.conj() * pos; //quat is CI2B, global pose
	pos2pix(pos.x(), pos.y(), pos.z(), pix_x[1], pix_y[1]);

	pos.fill2(+PYRAMID_HALF_BASE, -PYRAMID_HALF_BASE, PYRAMID_HEIGHT); //local pose
	pos = posCam + quCam.conj() * pos; //quat is CI2B, global pose
	pos2pix(pos.x(), pos.y(), pos.z(), pix_x[2], pix_y[2]);

	pos.fill2(-PYRAMID_HALF_BASE, -PYRAMID_HALF_BASE, PYRAMID_HEIGHT); //local pose
	pos = posCam + quCam.conj() * pos; //quat is CI2B, global pose
	pos2pix(pos.x(), pos.y(), pos.z(), pix_x[3], pix_y[3]);

	pos.fill2(-PYRAMID_HALF_BASE, +PYRAMID_HALF_BASE, PYRAMID_HEIGHT); //local pose
	pos = posCam + quCam.conj() * pos; //quat is CI2B, global pose
	pos2pix(pos.x(), pos.y(), pos.z(), pix_x[4], pix_y[4]);

	cv::line(image0, cv::Point2f(pix_x[0], pix_y[0]), cv::Point2f(pix_x[1], pix_y[1]), color, 1);
	cv::line(image0, cv::Point2f(pix_x[0], pix_y[0]), cv::Point2f(pix_x[2], pix_y[2]), color, 1);
	cv::line(image0, cv::Point2f(pix_x[0], pix_y[0]), cv::Point2f(pix_x[3], pix_y[3]), color, 1);
	cv::line(image0, cv::Point2f(pix_x[0], pix_y[0]), cv::Point2f(pix_x[4], pix_y[4]), color, 1);

	cv::line(image0, cv::Point2f(pix_x[1], pix_y[1]), cv::Point2f(pix_x[2], pix_y[2]), color, 1);
	cv::line(image0, cv::Point2f(pix_x[2], pix_y[2]), cv::Point2f(pix_x[3], pix_y[3]), color, 1);
	cv::line(image0, cv::Point2f(pix_x[3], pix_y[3]), cv::Point2f(pix_x[4], pix_y[4]), color, 1);
	cv::line(image0, cv::Point2f(pix_x[4], pix_y[4]), cv::Point2f(pix_x[1], pix_y[1]), color, 1);
}

void Plot3d::updateView()
{
	int i, k, cam_color;
	double time_s = myTime();

	if(last_draw_time < 0) {//first time
		last_draw_time = time_s;
		cv::namedWindow(win_name, 0); //By adding delay in cv::namedWindow in this phase, a plplot bug in nsrPlot.cpp is suppressed
		cv::setMouseCallback(win_name, onMouse, this);
        cv::createTrackbar("distance", win_name, &distance, 10000, distanceCallback, this); ///TODO: change position of origin point
	}

	if(!drag && execution_turn >= 0) //Don't update in automatic runs when mouse not exists
		return;

	if(drag) {
		if(!time_reached(0.10, 0., time_s, last_draw_time)) {
			last_draw_time = time_s;
			return;
		}
	} else {
		if(!time_reached(0.2, 0., time_s, last_draw_time)) {
			last_draw_time = time_s;
			return;
		}
	}
	last_draw_time = time_s;

	//first assume to look at origin=(0,0,0)
	//posViewCam is view vector in Inertia
	posViewCam = (quViewCam.conj() //quat is CI2B, we want to go from Body to NED
				  *colmat(0., 0., 1.)) //camera view vector looking at z direction
				 *((double) - distance) //minus because our position is opposite to view vector
				 + origin; //center of looking
	//printf("%i\n", distance); fflush(stdout);

	//////////////////////////
	//image0 = 0; //black background
	image0 = cv::Scalar(255, 255, 255); //white background

	//Draw axes///////////////
	float pix_x[4], pix_y[4];

	pos2pix(0., 0., 0., pix_x[0], pix_y[0]);
	pos2pix(AXES_LENGTH, 0., 0., pix_x[1], pix_y[1]);
	pos2pix(0., AXES_LENGTH, 0., pix_x[2], pix_y[2]);
	pos2pix(0., 0., AXES_LENGTH, pix_x[3], pix_y[3]);

	cv::arrowedLine(image0, cv::Point2f(pix_x[0], pix_y[0]), cv::Point2f(pix_x[1], pix_y[1]), cv::Scalar(255, 0, 0), 2);
	cv::arrowedLine(image0, cv::Point2f(pix_x[0], pix_y[0]), cv::Point2f(pix_x[2], pix_y[2]), cv::Scalar(0, 255, 0), 2);
	cv::arrowedLine(image0, cv::Point2f(pix_x[0], pix_y[0]), cv::Point2f(pix_x[3], pix_y[3]), cv::Scalar(0, 0, 255), 2);

	//Draw points//////////////
	//color = cv::Scalar(0,255,0);
	for(i = 0; i < pointProp.size(); i++) {
		pos2pix(pointProp[i][0], pointProp[i][1], pointProp[i][2], pix_x[0], pix_y[0]);
		circle(image0, cv::Point2f(pix_x[0], pix_y[0]), 2, cv::Scalar(pointProp[i][3], pointProp[i][4], pointProp[i][5]), 2);
	}

	//Draw cam positions///////
	nsr::Quat quCam;
	Matrice posCam(3, 1);

	for(k = 0; k < posQuCam.size(); k++)
		for(i = 0; i < posQuCam[k].size(); i++) {
			posCam.fill2(posQuCam[k][i][0], posQuCam[k][i][1], posQuCam[k][i][2]);
			quCam.set(posQuCam[k][i][3], posQuCam[k][i][4], posQuCam[k][i][5], posQuCam[k][i][6]);
			cam_color = k;
			drawCam(cam_color, posCam, quCam);
		}

	for(k = 0; k < posQuTempCam.size(); k++) {
		if(posQuTempCam[k][6] < - 2.) continue;
		posCam.fill2(posQuTempCam[k][0], posQuTempCam[k][1], posQuTempCam[k][2]);
		quCam.set(posQuTempCam[k][3], posQuTempCam[k][4], posQuTempCam[k][5], posQuTempCam[k][6]);
		cam_color = k;
		drawCam(cam_color, posCam, quCam);
	}

	//Draw image///////////////

	cv::imshow(win_name, image0);
}

static void onMouse(int event, int x, int y, int, void* data)
{
	Plot3d* p = (Plot3d*)data;

	if(event == cv::EVENT_LBUTTONDOWN) {
		p->drag = true;
		p->last_x = x;
		p->last_y = y;
	}
	if(event == cv::EVENT_LBUTTONUP) {
		p->drag = false;
	}
	if(event != cv::EVENT_MOUSEMOVE || p->drag != true)
		return;

	//cv::Point seed = cv::Point(x,y);
	//cv::Scalar color = cv::Scalar(255, 0, 255);
	//cv::circle(image0, seed, 5, color);
	//printf("%i, %i\n", x-last_x, y-last_y);

#define GAIN 1.
	p->quViewCam = nsr::Quat((x - p->last_x) * M_PI / 180.*GAIN, nsr::Y_AXIS)
				   * nsr::Quat(-(y - p->last_y) * M_PI / 180.*GAIN, nsr::X_AXIS)
				   * p->quViewCam; //CI2B

	p->updateView();

	p->last_x = x;
	p->last_y = y;
}

static void distanceCallback(int, void* data)
{
	Plot3d* p = (Plot3d*)data;
	p->updateView();
}

void plot3dtest()
{

	Plot3d _plot3d("image");

	int i;
	for(i = 0; i < 50000; i++)
		_plot3d.insertFeature(RAND(-500, 500), RAND(-500, 500), RAND(-1, 1));
	nsr::Quat quCam;
	quCam.setEu(0., 0., 0.);
	for(i = 0; i < 5000; i++)
		_plot3d.insertCam(0, 0, 10., i * 2 * 0.1, 10., quCam.e1, quCam.e2, quCam.e3, quCam.et);

	_plot3d.updateView();

	cv::waitKey(0);
}

