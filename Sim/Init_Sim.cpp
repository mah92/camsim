#include "./Init_Sim.h"
#include "./nsrImageProc.h" //for controlling image proc settings

#include "Core/nsrCore.h"

#undef TAG
#define TAG "C:InitSim:"

#ifdef __cplusplus
extern "C" {
#endif
	
void Init_Sim(SimStorage &Sim)
{
	nsrReadSimParams("Parameters.xml");

	//Configure//////////////////////////////////////////
	Sim.DynamicInTheLoop = 0;
	srand(param_seed);

	Sim.Lat0 = param_map_center_lat; Sim.lon0 = param_map_center_lon; Sim.alt0 = 0.;
	//Sim.Lat0=35, Sim.lon0=53, Sim.alt0=500; //origin of local coordinates
	/*double Rx0=0, Ry0=0, Rz0=-300; //first position in local coordinates
	double Vx0=0, Vy0=0, Vz0=0;
	double phai0=0, theta0=0, psi0=0;
	double Wx0=0, Wy0=0, Wz0=0;
	*/
	Sim.MMR = zeros(3, 1);
	//Sim.MMR.fill2(1.,0.15,1.); //Magnetometer Reference(Normalized)
	//Sim.MMR.fill2(1.,0.,1.); //Magnetometer Reference(Normalized)
	Sim.MMR.fill2(1., 0., 0.); //Magnetometer Reference(Normalized)
	Sim.MMR = Sim.MMR / norm(Sim.MMR);

	//Init Simulation////////////////////////////////////////
	Sim.t = 0;
	Sim.i = 0;
	Sim.Texec = 1. / 200.;    //Autopilot freq, has real effect on accuracy, can be tested by safe_mode divergance
	Sim.Tenv = 1. / param_control_freq; //Sim.Texec/4.; //800Hz simulation freq.
	Sim.Tplot1 = 1 / 5.;   // Visualize1 interval, used for whole Visualize function call
	Sim.Tplot2 = 1 / 1.;   // Visualize2 interval for more sensitive material, should be less than Visualize1

	/*    //Object Outputs Config//////////////////////////////////
	    //[u,v,w p,q,r phi,theta,psi, x,y,z]
	    //n.O.V=1;	n.O.Qu=0;   n.O.W=4;   n.O.Eu=7;  n.O.Loc=10;
	    n.O.Eu=7;
	    n.O.Qu=0;
	    n.O.Q3=0;
	    n.O.V=1;
	    n.O.Loc=10;
	    n.O.W=4;
	    n.O.A=0;
	    n.O.Bf=0;
	    n.O.Bg=0;
	    n.O.BGPS=0;
	    n.O.Total=12;
	    //n.O.Depth=100; //Filter & Vehicle Should Match (2 days)
	    //n.O.Readers=1; //Readers is number of buffer users, 1 page readers enough

	    //Init O////////////////////////////////////////////////
	    //States:[u,v,w, p,q,r, phi,theta,psi, x,y,z]
	    Sim.O = zeros(n.O.Total,1);
		Sim.O.p->matrix_indices_start_by_one=true;
		Sim.O.fill2(Vx0,Vy0,Vz0, Wx0,Wy0,Wz0, phai0,theta0,psi0, Rx0,Ry0,Rz0);
	*/
	Sim.P_Loc = zeros(3, 1);
	Sim.P_Angle = zeros(3, 1);

	//Camera params//////////////////////////////////////////
	double phaicam = 0 * M_PI / 180;
	double thetacam = 0 * M_PI / 180;
	double psicam = 90 * M_PI / 180; //Camera in body frame
	double xcam = 0;  //Camera in body frame
	double ycam = 0;
	double zcam = 0;

	//calibration
	double pixels_x = 240;
	double fov_x = 24.5 * 2;
	double pixels_y = 320;
	double fov_y = 31.7 * 2;
	double fx = ((pixels_x - 1) / 2) / tan(fov_x / 2 * M_PI / 180);
	double cx = (pixels_x - 1) / 2;
	double fy = ((pixels_y - 1) / 2) / tan(fov_y / 2 * M_PI / 180);
	double cy = (pixels_y - 1) / 2;

	//Sensors Noise//////////////////////////////////////////
	Sim.Freq  = zeros(n.Z.Total, 1);
	Sim.Freq.p->matrix_indices_start_by_one = true;

	Sim.Noise = zeros(n.Z.Total, 1); //Vector for tracking sensors noise
	Sim.Noise.p->matrix_indices_start_by_one = true;

	Sim.RMS   = zeros(n.Z.Total, 1);
	Sim.RMS.p->matrix_indices_start_by_one = true;

	Sim.Bias  = zeros(n.Z.Total, 1);
	Sim.Bias.p->matrix_indices_start_by_one = true;

	//Accelerometer
	if(n.Z.Acc != 0) {
		double g = 9.81;
		Sim.RMS(n.Z.Acc + 0) = 0.005 * g;
		Sim.RMS(n.Z.Acc + 1) = 0.005 * g;
		Sim.RMS(n.Z.Acc + 2) = 0.005 * g;
		Sim.Bias(n.Z.Acc + 0) = -0.003 * g; //0.03
		Sim.Bias(n.Z.Acc + 1) = +0.003 * g;
		Sim.Bias(n.Z.Acc + 2) = -0.003 * g;
		Sim.Freq(n.Z.Acc) = 1. / Sim.Texec; //200Hz Acc
	}
	//gyro
	if(n.Z.Gyro != 0) {
		Sim.RMS(n.Z.Gyro + 0) = 0.15 / 180 * M_PI;
		Sim.RMS(n.Z.Gyro + 1) = 0.15 / 180 * M_PI;
		Sim.RMS(n.Z.Gyro + 2) = 0.15 / 180 * M_PI;
		Sim.Bias(n.Z.Gyro + 0) = -0.3 / 180 * M_PI; //0.0052
		Sim.Bias(n.Z.Gyro + 1) = +0.3 / 180 * M_PI;
		Sim.Bias(n.Z.Gyro + 2) = -0.3 / 180 * M_PI;
		Sim.Freq(n.Z.Gyro) = 1. / Sim.Texec; //200Hz Gyro
	}
	//Magnetometer
	if(n.Z.MMM != 0) {
		Sim.RMS(n.Z.MMM + 0) = .1 / 180 * M_PI;
		Sim.RMS(n.Z.MMM + 1) = .1 / 180 * M_PI;
		Sim.RMS(n.Z.MMM + 2) = .1 / 180 * M_PI;
		Sim.Bias(n.Z.MMM + 0) = 0 / 180 * M_PI;
		Sim.Bias(n.Z.MMM + 1) = 0 / 180 * M_PI;
		Sim.Bias(n.Z.MMM + 2) = 0 / 180 * M_PI;
		Sim.Freq(n.Z.MMM) = 10; //10Hz Magnetometer
	}
	//Magnetometer Reference
	if(n.Z.MMR != 0) {
		Sim.RMS(n.Z.MMR + 0) = 0; //(300/29000/3); //WMM error 1 sigma
		Sim.RMS(n.Z.MMR + 1) = 0; //(300/29000/3);
		Sim.RMS(n.Z.MMR + 2) = 0; //(300/29000/3);
		Sim.Bias(n.Z.MMR + 0) = 0;
		Sim.Bias(n.Z.MMR + 1) = 0;
		Sim.Bias(n.Z.MMR + 2) = 0;
		//Sim.Freq(n.Z.MMR)=1/100000.; //sync with gps
	}
	//GPS
	if(n.Z.GPS_LLA != 0) {
		Sim.RMS(n.Z.GPS_LLA + 0) = 2.5;
		Sim.RMS(n.Z.GPS_LLA + 1) = 2.5;
		Sim.RMS(n.Z.GPS_LLA + 2) = 10;
		Sim.Bias(n.Z.GPS_LLA + 0) = 0;
		Sim.Bias(n.Z.GPS_LLA + 1) = 0;
		Sim.Bias(n.Z.GPS_LLA + 2) = 0;
		Sim.Freq(n.Z.GPS_LLA) = 5.; //5Hz GPS
	}
	//Prs
	if(n.Z.PrsAlt != 0) {
		Sim.RMS(n.Z.PrsAlt) = 0.2;
		Sim.Bias(n.Z.PrsAlt) = 0;
		Sim.Freq(n.Z.PrsAlt) = 10; //10Hz Prs
	}
	//APrs
	if(n.Z.PrsAcc != 0) {
		Sim.RMS(n.Z.PrsAcc) = .1;
		Sim.Bias(n.Z.PrsAcc) = 0;
		//Sim.Freq(n.Z.PrsAlt)=10; //sync with pressure
	}

	//WExt(Table)
	if(n.Z.WExt != 0) {
		Sim.RMS(n.Z.WExt + 0) = 0;
		Sim.RMS(n.Z.WExt + 1) = 0;
		Sim.RMS(n.Z.WExt + 2) = 0;
		Sim.Bias(n.Z.WExt + 0) = 0;
		Sim.Bias(n.Z.WExt + 1) = 0;
		Sim.Bias(n.Z.WExt + 2) = 0;
	}
	//VExt(Table)
	if(n.Z.VExt != 0) {
		Sim.RMS(n.Z.VExt + 0) = 0;
		Sim.RMS(n.Z.VExt + 1) = 0;
		Sim.RMS(n.Z.VExt + 2) = 0;
		Sim.Bias(n.Z.VExt + 0) = 0;
		Sim.Bias(n.Z.VExt + 1) = 0;
		Sim.Bias(n.Z.VExt + 2) = 0;
	}

}

#ifdef __cplusplus
}
#endif
