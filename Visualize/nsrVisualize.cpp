#ifndef __ANDROID__
#include "/usr/include/time.h"
#endif

#include "./nsrVisualize.h"
#include "Core/RotLib/nsrRotLib.h"
#include "Core/GeoLib/nsrGeoLib2.h"
#include <math.h>
#include "./nsrPoseSim.h"
#include "Matlib/nsrQuat.h"

#include "./nsrMatShow.h"

#undef TAG
#define TAG "C:Visualize:"

#ifdef __cplusplus
extern "C" {
#endif

//current -> c part :: matlab
//Title:"3d flight", Real, GPS, Filtered, Command data, "x", "y", "z" focused on ("-z"/"x"): - > side2D: Real, GPS, Filtered, Command :: side2D: Real, GPS, Filtered, Command

class _3DFlightPlotView : public NsrFig
{
public:
	_3DFlightPlotView(bool enabled = true);
};

_3DFlightPlotView::_3DFlightPlotView(bool enabled) : NsrFig(1, 1, enabled)
{
	addSubWin(0, 2, //int sub_win, int axis_num=2,
			  "East(y, m)", "North(x, m)", "", "Up view"); //const char* xlable="", const char* ylable="", const char* zlable="", const char* title="",
	/*addSubWin(0, 2, //int sub_win, int axis_num=2,
		   "x", "-z", "", "3d flight", //const char* xlable="", const char* ylable="", const char* zlable="", const char* title="",
				false, true, true);//bool xReverse=false, bool yReverse=false, bool zReverse=false);
	*/
	//setAxis(0, -80.,80., -130.,0., -80.,80.);
	addSignal(0, "Real", PEN_LINE, 1, COL_BLUE, AUTOSCALE_AND_AXIS_EQUAL); //sig0
	addSignal(0, "GPS", PEN_POINT, '+', COL_MAGNETA, AUTOSCALE_AND_AXIS_EQUAL); //sig1
}
enum {line_flight3d_real, line_flight3d_gps};

///////////////////////////////////////////////////////////
//Title:"Positions"

class PositionView : public NsrFig
{
public:
	PositionView(bool enabled = true);
};

PositionView::PositionView(bool enabled) : NsrFig(3, 1, enabled)
{
	addSubWin(0, 2, //int sub_win, int axis_num=2,
			  "t(s)", "x(m)", "", "Position"); //const char* xlable="", const char* ylable="", const char* zlable="", const char* title="",
	setAxis(0, 0, 5, -100., 100., 0, 0);
	addSignal(0, "Real", PEN_LINE, 1, COL_BLUE);
	addSignal(0, "GPS", PEN_POINT, '+', COL_MAGNETA);

	//
	addSubWin(1, 2, //int sub_win, int axis_num=2,
			  "t(s)", "y(m)", "", ""); //const char* xlable="", const char* ylable="", const char* zlable="", const char* title="",
	setAxis(1, 0., 5., -100., 100., 0, 0);
	addSignal(1, "", PEN_LINE, 1, COL_BLUE); //Real
	addSignal(1, "", PEN_POINT, '+', COL_MAGNETA); //GPS

	//
	addSubWin(2, 2, //int sub_win, int axis_num=2,
			  "t(s)", "Alt(-z)(m)", "", ""); //const char* xlable="", const char* ylable="", const char* zlable="", const char* title="",
	setAxis(2, 0., 5., 100., 101., 0, 0);
	addSignal(2, "", PEN_LINE, 1, COL_BLUE); //Real
	addSignal(2, "", PEN_POINT, '+', COL_MAGNETA); //GPS
}
enum {
	line_position_x_real, line_position_x_gps,
	line_position_y_real, line_position_y_gps,
	line_position_z_real, line_position_z_gps,
};
	
///////////////////////////////////////////////////////////
//Title:"Velocity"

class VelocityView : public NsrFig
{
public:
	VelocityView(bool enabled = true);
};

VelocityView::VelocityView(bool enabled) : NsrFig(3, 1, enabled)
{

	addSubWin(0, 2, //int sub_win, int axis_num=2,
			  "t(s)", "Total Vel", "", "Velocities"); //const char* xlable="", const char* ylable="", const char* zlable="", const char* title="",
	setAxis(0, 0., 5., -2., 2., 0, 0);
	addSignal(0, "real", PEN_LINE, 1, COL_BLUE); //real

	//
	addSubWin(1, 2, //int sub_win, int axis_num=2,
			  "t(s)", "Horizontal Vel", "", ""); //const char* xlable="", const char* ylable="", const char* zlable="", const char* title="",
	setAxis(1, 0., 5., -2., 2., 0, 0);
	addSignal(1, "", PEN_LINE, 1, COL_BLUE); //real

	//
	addSubWin(2, 2, //int sub_win, int axis_num=2,
			  "t(s)", "Vertical Vel(DAlt)", "", ""); //const char* xlable="", const char* ylable="", const char* zlable="", const char* title="",
	setAxis(2, 0., 5., -2., 2., 0, 0);
	addSignal(2, "", PEN_LINE, 1, COL_BLUE); //real
}

enum {
	line_velocity_total_real,
	line_velocity_horizontal_real,
	line_velocity_vertical_real,
};

///////////////////////////////////////////////////////////
//Title:"Angles"

class AnglesView : public NsrFig
{
public:
	AnglesView(bool enabled = true);
};

AnglesView::AnglesView(bool enabled) : NsrFig(3, 1, enabled)
{

	addSubWin(0, 2, //int sub_win, int axis_num=2,
			  "t(s)", "roll/phi", "", "Angles"); //const char* xlable="", const char* ylable="", const char* zlable="", const char* title="",
	setAxis(0, 0., 5., -2., 2., 0, 0);
	addSignal(0, "real", PEN_LINE, 1, COL_BLUE); //real

	//
	addSubWin(1, 2, //int sub_win, int axis_num=2,
			  "t(s)", "pitch/theta", "", ""); //const char* xlable="", const char* ylable="", const char* zlable="", const char* title="",
	setAxis(1, 0., 5., -2., 2., 0, 0);
	addSignal(1, "", PEN_LINE, 1, COL_BLUE); //real

	//
	addSubWin(2, 2, //int sub_win, int axis_num=2,
			  "t(s)", "yaw/psi", "", ""); //const char* xlable="", const char* ylable="", const char* zlable="", const char* title="",
	setAxis(2, 0., 5., -2., 2., 0, 0);
	addSignal(2, "", PEN_LINE, 1, COL_BLUE); //real
}

enum {
	line_angles_x_real,
	line_angles_y_real,
	line_angles_z_real,
};

///////////////////////////////////////////////////////////
//Title:"Gyro"

class GyroView : public NsrFig
{
public:
	GyroView(bool enabled = true);
};

GyroView::GyroView(bool enabled) : NsrFig(3, 1, enabled)
{

	addSubWin(0, 2, //int sub_win, int axis_num=2,
			  "t(s)", "x", "", "Gyro"); //const char* xlable="", const char* ylable="", const char* zlable="", const char* title="",
	setAxis(0, 0., 5., -2., 2., 0, 0);
	addSignal(0, "real", PEN_LINE, 1, COL_BLUE); //real
	addSignal(0, "sensor", PEN_LINE, 2, COL_RED); //sensor

	//
	addSubWin(1, 2, //int sub_win, int axis_num=2,
			  "t(s)", "y", "", ""); //const char* xlable="", const char* ylable="", const char* zlable="", const char* title="",
	setAxis(1, 0., 5., -2., 2., 0, 0);
	addSignal(1, "", PEN_LINE, 1, COL_BLUE); //real
	addSignal(1, "", PEN_LINE, 2, COL_RED); //sensor

	//
	addSubWin(2, 2, //int sub_win, int axis_num=2,
			  "t(s)", "z", "", ""); //const char* xlable="", const char* ylable="", const char* zlable="", const char* title="",
	setAxis(2, 0., 5., -2., 2., 0, 0);
	addSignal(2, "", PEN_LINE, 1, COL_BLUE); //real
	addSignal(2, "", PEN_LINE, 2, COL_RED); //sensor
}

enum {
	line_gyro_x_real, line_gyro_x_sensor,
	line_gyro_y_real, line_gyro_y_sensor,
	line_gyro_z_real, line_gyro_z_sensor
};

///////////////////////////////////////////////////////////
//Title:"Acc"

class AccView : public NsrFig
{
public:
	AccView(bool enabled = true);
};

AccView::AccView(bool enabled) : NsrFig(3, 1, enabled)
{

	addSubWin(0, 2, //int sub_win, int axis_num=2,
			  "t(s)", "x", "", "Acc"); //const char* xlable="", const char* ylable="", const char* zlable="", const char* title="",
	setAxis(0, 0., 5., -2., 2., 0, 0);
	addSignal(0, "real", PEN_LINE, 1, COL_BLUE); //real
	addSignal(0, "sensor", PEN_LINE, 2, COL_RED); //sensor

	//
	addSubWin(1, 2, //int sub_win, int axis_num=2,
			  "t(s)", "y", "", ""); //const char* xlable="", const char* ylable="", const char* zlable="", const char* title="",
	setAxis(1, 0., 5., -2., 2., 0, 0);
	addSignal(1, "", PEN_LINE, 1, COL_BLUE); //real
	addSignal(1, "", PEN_LINE, 2, COL_RED); //sensor

	//
	addSubWin(2, 2, //int sub_win, int axis_num=2,
			  "t(s)", "z", "", ""); //const char* xlable="", const char* ylable="", const char* zlable="", const char* title="",
	setAxis(2, 0., 5., -2., 2., 0, 0);
	addSignal(2, "", PEN_LINE, 1, COL_BLUE); //real
	addSignal(2, "", PEN_LINE, 2, COL_RED); //sensor
}

enum {
	line_acc_x_real, line_acc_x_sensor,
	line_acc_y_real, line_acc_y_sensor,
	line_acc_z_real, line_acc_z_sensor
};

/////////////////////////////////////////////////////////////////

_3DFlightPlotView *_3DV = NULL;
PositionView *posV = NULL;
VelocityView *velV = NULL;
AnglesView *angV = NULL;
GyroView *gyroV = NULL;
AccView *accV = NULL;

static double last_t = -1000;

void Visualize(SimStorage &Sim, double Tplot2) //, Con)
{
	bool low_freq_draw = false;
	_LOCKCPP(Z_lock,);
	double t = cbLast(mZB, n.Z.Time);
	_UNLOCKCPP(Z_lock,);

    //return;
    
	if(time_reached(Tplot2 / param_speed_factor, 0., t, last_t) > 0.5)
		low_freq_draw = true;

	//If having ground truth, use real startup point as origin,
	//	otherwise use first gps point
	static int lla0_inited = 0;
	double Lat0, lon0, alt0;
	Lat0 = Sim.Lat0;
	lon0 = Sim.lon0;
	alt0 = Sim.alt0;
	lla0_inited = 1;

	double phi, theta, psi, locX, locY, locZ;
	osg::Vec3d lla, v_ac, a_ac, acc_ac, w_ac;
	osg::Quat acInNedQu;

	nsrPoseMakerExtract(t, 0,
						&lla, &v_ac, /*wrt. ned, in ned*/ &a_ac, /*wrt. ned, in ned*/ &acc_ac,/*accelerometer output*/
						&acInNedQu, /* ac/ned */ &w_ac, /* ac/ned, in ac */
						NULL, NULL, NULL);
	Spherical2Local(lla.x(), lla.y(), lla.z(),
					Lat0, lon0, alt0, //inputs
					locX, locY, locZ);
	Matrice loc(3, 1);
	loc.fill2(locX, locY, locZ);

	nsr::Quat(acInNedQu.x(), acInNedQu.y(), acInNedQu.z(), acInNedQu.w()).getEu(phi, theta, psi);

	//Flight3D plot//////////////////////////////////////
#undef ENABLE_PLOT
#define ENABLE_PLOT 1
	if(low_freq_draw) {

		if(_3DV == NULL)
			_3DV = new _3DFlightPlotView(ENABLE_PLOT && execution_turn < 0); //disable if execution_turn >=0

		_3DV->draw(line_flight3d_real, loc[1], loc[0]);

		double dx, dy, dz;
		if(lla0_inited == 1) {
			int ERRGPS;
			
			_LOCKCPP(Z_lock, -1);
			mZB->ERR = 0;
			Spherical2Local(cbLast(mZB, n.Z.GPS_LLA + 0), cbLast(mZB, n.Z.GPS_LLA + 1), cbLast(mZB, n.Z.GPS_LLA + 2),
							Lat0, lon0, alt0, //inputs
							dx, dy, dz); //output
			ERRGPS = mZB->ERR;
			_UNLOCKCPP(Z_lock, -1);

			if(ERRGPS == 0)
				_3DV->draw(line_flight3d_gps, dy, dx);
		}
	}

	//Position plot///////////////////////////////////
#undef ENABLE_PLOT
#define ENABLE_PLOT 1
	if(low_freq_draw) {
		int ERRGPS;
		double Lat, lon, alt;
		double gps_x, gps_y, gps_z;
		double x_, y_, z_;

		if(posV == NULL)
			posV = new PositionView(ENABLE_PLOT && execution_turn < 0); //disable if execution_turn >=0

		posV->draw(line_position_x_real, t, loc[0]);
		posV->draw(line_position_y_real, t, loc[1]);
		posV->draw(line_position_z_real, t, -loc[2]);

		if(n.Z.GPS_LLA != 0) {
			/////////////
			mZB->ERR = 0;
			Lat = cbPull(mZB, n.Z.GPS_LLA + 0, 2);
			lon = cbPull(mZB, n.Z.GPS_LLA + 1, 2);
			alt = cbPull(mZB, n.Z.GPS_LLA + 2, 2);
			ERRGPS = mZB->ERR;

			Spherical2Local(Lat, lon, alt,
							Lat0, lon0, alt0, //inputs
							gps_x, gps_y, gps_z); //output

			if(ERRGPS == 0) {
				posV->draw(line_position_x_gps, t, gps_x);
				posV->draw(line_position_y_gps, t, gps_y);
				posV->draw(line_position_z_gps, t, -gps_z);
			}

			//empty gps buffer
			mZB->ERR = 0;
			while(1) {
				cbPull(mZB, n.Z.GPS_LLA + 0, 2);
				cbPull(mZB, n.Z.GPS_LLA + 1, 2);
				cbPull(mZB, n.Z.GPS_LLA + 2, 2);
				if(mZB->ERR != 0) break;
			}
		}
	}

	//Velocity plot//////////////////////////////
#undef ENABLE_PLOT
#define ENABLE_PLOT 1
	if(low_freq_draw) {

		if(velV == NULL)
			velV = new VelocityView(ENABLE_PLOT && execution_turn < 0); //disable if execution_turn >=0

		double O_vned[3] = {v_ac.x(), v_ac.y(), v_ac.z()};
		
		velV->draw(line_velocity_total_real, t, sqrt(pow2(O_vned[0]) + pow2(O_vned[1]) + pow2(O_vned[2])));
		velV->draw(line_velocity_horizontal_real, t, sqrt(pow2(O_vned[0]) + pow2(O_vned[1])));
		velV->draw(line_velocity_vertical_real, t, -O_vned[2]); //DAlt not Vz
	}

	//Euler angles plot//////////////////////////////////////////////////////////////////////////
#undef ENABLE_PLOT
#define ENABLE_PLOT 1
	if(low_freq_draw) {
		if(angV == NULL)
			angV = new AnglesView(ENABLE_PLOT && execution_turn < 0); //disable if execution_turn >=0

		angV->draw(line_angles_x_real, t, phi * 180 / M_PI);
		angV->draw(line_angles_y_real, t, theta * 180 / M_PI);
		angV->draw(line_angles_z_real, t, psi * 180 / M_PI);
	}

	//Gyro plot//////////////////////////////////////////////////////////////////////////
#undef ENABLE_PLOT
#define ENABLE_PLOT 1
	if(gyroV == NULL)
		gyroV = new GyroView(ENABLE_PLOT && execution_turn < 0); //disable if execution_turn >=0

	gyroV->draw(line_gyro_x_real, t, w_ac.x() * 180 / M_PI);
	gyroV->draw(line_gyro_y_real, t, w_ac.y() * 180 / M_PI);
	gyroV->draw(line_gyro_z_real, t, w_ac.z() * 180 / M_PI);

	if(n.Z.Gyro != 0) {
		_LOCKCPP(Z_lock, -1);
		gyroV->draw(line_gyro_x_sensor, t, cbLast(mZB, n.Z.Gyro + 0) * 180 / M_PI);
		gyroV->draw(line_gyro_y_sensor, t, cbLast(mZB, n.Z.Gyro + 1) * 180 / M_PI);
		gyroV->draw(line_gyro_z_sensor, t, cbLast(mZB, n.Z.Gyro + 2) * 180 / M_PI);
		_UNLOCKCPP(Z_lock, -1);
	}

	//Acc plot//////////////////////////////////////////////////////////////////////////
#undef ENABLE_PLOT
#define ENABLE_PLOT 1
	if(accV == NULL)
		accV = new AccView(ENABLE_PLOT && execution_turn < 0); //disable if execution_turn >=0

	accV->draw(line_acc_x_real, t, a_ac.x());
	accV->draw(line_acc_y_real, t, a_ac.y());
	accV->draw(line_acc_z_real, t, a_ac.z());

	if(n.Z.Acc != 0) {
		_LOCKCPP(Z_lock, -1);
		accV->draw(line_acc_x_sensor, t, cbLast(mZB, n.Z.Acc + 0));
		accV->draw(line_acc_y_sensor, t, cbLast(mZB, n.Z.Acc + 1));
		accV->draw(line_acc_z_sensor, t, cbLast(mZB, n.Z.Acc + 2));
		_UNLOCKCPP(Z_lock, -1);
	}

	if(low_freq_draw)
		refreshPlots();

	if(time_reached(1., 0., t, last_t) > 0.5) {
		float render_frame_rate;
		_LOCKCPP(Z_lock, -1);
		render_frame_rate = cbLast(mZB, n.Z.RATE);
		_UNLOCKCPP(Z_lock, -1);
		LOGI(TAG, " exec:%i, t:%.1f->fps:%2.0f\n", execution_turn, t, render_frame_rate); fflush(stdout);
	}

	last_t = t;
}

#ifdef __cplusplus
}
#endif
