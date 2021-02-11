#ifndef __ANDROID__
#include "/usr/include/time.h"
#endif

#include "./Sim.h"
#include "Core/RotLib/nsrRotLib.h"
#include "Core/GeoLib/nsrGeoLib2.h"
#include "./nsrPoseSim.h"
#include "Matlib/nsrQuat.h"
#include "Sim/nsrSimParamReader.h"

#ifdef __cplusplus
extern "C" {
#endif

//local
static Matrice CI2B(3, 3), MMR(3, 1), MM(3, 1), mmr(3, 1), mmm(3, 1);
static double Lat0, lon0, alt0;
static double T;

//local camera
static Matrice RefI(3, 1), SBinI(3, 1), dR(3, 1), MB(3, 1), MCam(3, 1);

//memory
static int tpre_inited = 0;
static double tPre;

void Sensors(SimStorage &Sim)
{
	Matrice &RMS = Sim.RMS;
	Matrice &Bias = Sim.Bias;
	Matrice &Freq = Sim.Freq;
	Matrice &Noise = Sim.Noise;

	osg::Vec3d lla, v_ac, a_ac, acc_ac, w_ac;
	osg::Quat acInNedQu;

	double t = Sim.t;

	nsrPoseMakerExtract(t, 0,
						&lla, &v_ac, /*wrt. ned, in ned*/ &a_ac, /*wrt. ned, in ned*/ &acc_ac,/*accelerometer output*/
						&acInNedQu, /* ac/ned */ &w_ac, /* ac/ned, in ac */
						NULL, NULL, NULL);

	////////////////////////////////////////////////////////
	Lat0 = Sim.Lat0;
	lon0 = Sim.lon0;
	alt0 = Sim.alt0;

	//Calculate time step////////////////////////////////////
	if(tpre_inited == 0) {
		tPre = t;
		tpre_inited = 1;
	}

	if(t - tPre > 1e-6) T = t - tPre;
	else T = 1;

	//Calculate CI2B (Change to Body Rotation Matrix)////////
	nsr::Quat(acInNedQu.x(), acInNedQu.y(), acInNedQu.z(), acInNedQu.w()).getRotMat(CI2B);

	//Accelerometers////////////////////////////////////////
	if(n.Z.Acc != 0 && Freq(n.Z.Acc) > 1e-6)  //40Hz Acc
		if(time_reached(1. / Freq(n.Z.Acc), 0., Sim.t, tPre) == 1) {  //40Hz Acc

			Noise(n.Z.Acc + 0) = normrnd(Bias(n.Z.Acc + 0), RMS(n.Z.Acc + 0)); //Acc
			Noise(n.Z.Acc + 1) = normrnd(Bias(n.Z.Acc + 1), RMS(n.Z.Acc + 1));
			Noise(n.Z.Acc + 2) = normrnd(Bias(n.Z.Acc + 2), RMS(n.Z.Acc + 2));
			if(SAFE_MODE == 1) {
				Noise(n.Z.Acc + 0) = 0;
				Noise(n.Z.Acc + 1) = 0;
				Noise(n.Z.Acc + 2) = 0;
			}

			_LOCKCPP(Z_lock, -1);
			cbPush(mZB, n.Z.Acc + 0, acc_ac.x() + Noise(n.Z.Acc + 0), t); //Acc
			cbPush(mZB, n.Z.Acc + 1, acc_ac.y() + Noise(n.Z.Acc + 1), t);
			cbPush(mZB, n.Z.Acc + 2, acc_ac.z() + Noise(n.Z.Acc + 2), t);
			_UNLOCKCPP(Z_lock, -1);
		}

	//Gyro//////////////////////////////////////////////////
	if(n.Z.Gyro != 0 && Freq(n.Z.Gyro) > 1e-6)
		if(time_reached(1. / Freq(n.Z.Gyro), 0., Sim.t, tPre) == 1) {  //40Hz Acc
			//Memorial: there was a lethal error which for days made you
			//miserable, just ALLAH saved you at the very miserable moment

			Noise(n.Z.Gyro + 0) = normrnd(Bias(n.Z.Gyro + 0), RMS(n.Z.Gyro + 0)); //Gyro
			Noise(n.Z.Gyro + 1) = normrnd(Bias(n.Z.Gyro + 1), RMS(n.Z.Gyro + 1));
			Noise(n.Z.Gyro + 2) = normrnd(Bias(n.Z.Gyro + 2), RMS(n.Z.Gyro + 2));
			if(SAFE_MODE == 1) {
				Noise(n.Z.Gyro + 0) = 0;
				Noise(n.Z.Gyro + 1) = 0;
				Noise(n.Z.Gyro + 2) = 0;
			}

			_LOCKCPP(Z_lock, -1);
			cbPush(mZB, n.Z.Gyro + 0, w_ac.x() + Noise(n.Z.Gyro + 0), t); //Gyro
			cbPush(mZB, n.Z.Gyro + 1, w_ac.y() + Noise(n.Z.Gyro + 1), t);
			cbPush(mZB, n.Z.Gyro + 2, w_ac.z() + Noise(n.Z.Gyro + 2), t);
			_UNLOCKCPP(Z_lock, -1);

			//printf("%f, %f, %f\n", w_ac.x(), w_ac.y(), w_ac.z());
		}

	//Magnetometer////////////////////////////////////////S////
	MMR = Sim.MMR;
	if(n.Z.MMM != 0 && Freq(n.Z.MMM) > 1e-6) 
		if(time_reached(1. / Freq(n.Z.MMM), 0., Sim.t, tPre) == 1) {

			MM = CI2B * MMR; //Magnetometer(Normalized)

			Noise(n.Z.MMM + 0) = normrnd(Bias(n.Z.MMM + 0), RMS(n.Z.MMM + 0));
			Noise(n.Z.MMM + 1) = normrnd(Bias(n.Z.MMM + 1), RMS(n.Z.MMM + 1));
			Noise(n.Z.MMM + 2) = normrnd(Bias(n.Z.MMM + 2), RMS(n.Z.MMM + 2));
			if(SAFE_MODE == 1) {
				Noise(n.Z.MMM + 0) = 0;
				Noise(n.Z.MMM + 1) = 0;
				Noise(n.Z.MMM + 2) = 0;
			}

			mmm.fill2(MM[0] + Noise(n.Z.MMM + 0), MM[1] + Noise(n.Z.MMM + 1), MM[2] + Noise(n.Z.MMM + 2));
			mmm = mmm / norm(mmm);

			_LOCKCPP(Z_lock, -1);
			cbPush(mZB, n.Z.MMM + 0, mmm[0], t);
			cbPush(mZB, n.Z.MMM + 1, mmm[1], t);
			cbPush(mZB, n.Z.MMM + 2, mmm[2], t);
			_UNLOCKCPP(Z_lock, -1);
		}

	//GPS////////////////////////////////////////////////////
	if(n.Z.GPS_LLA!=0 && Freq(n.Z.GPS_LLA) > 1e-6)
		if(time_reached(1. / Freq(n.Z.GPS_LLA), 0., Sim.t, tPre) == 1) {  // && (t<30 || t>130))
			if(n.Z.GPS_LLA != 0) {
				Noise(n.Z.GPS_LLA + 0) = normrnd(Bias(n.Z.GPS_LLA + 0), RMS(n.Z.GPS_LLA + 0));
				Noise(n.Z.GPS_LLA + 1) = normrnd(Bias(n.Z.GPS_LLA + 1), RMS(n.Z.GPS_LLA + 1));
				Noise(n.Z.GPS_LLA + 2) = normrnd(Bias(n.Z.GPS_LLA + 2), RMS(n.Z.GPS_LLA + 2));
				if(SAFE_MODE == 1) {
					Noise(n.Z.GPS_LLA + 0) = 0;
					Noise(n.Z.GPS_LLA + 1) = 0;
					Noise(n.Z.GPS_LLA + 2) = 0;
				}

#define EarthRadius 6378137
				_LOCKCPP(Z_lock, -1);
				cbPush(mZB, n.Z.GPS_LLA + 0, lla.x() + Noise(n.Z.GPS_LLA + 0) * (180 / M_PI) / EarthRadius, t); //GPS
				cbPush(mZB, n.Z.GPS_LLA + 1, lla.y() + Noise(n.Z.GPS_LLA + 1) * (180 / M_PI) / (EarthRadius * cos(lla.x()*M_PI / 180)), t);
				cbPush(mZB, n.Z.GPS_LLA + 2, lla.z() + /*param_world_scale?? * */ Noise(n.Z.GPS_LLA + 2), t);
				_UNLOCKCPP(Z_lock, -1);

				//Magnetometer Reference////////////////////////////////
				//Put in the n.Z.MM if for MMR to be defined
				//if(n.Z.MMR!=0 && time_reached(1./Freq(n.Z.MMR), 0., Sim.t, tPre) == 1){
				Noise(n.Z.MMR + 0) = normrnd(Bias(n.Z.MMR + 0), RMS(n.Z.MMR + 0));
				Noise(n.Z.MMR + 1) = normrnd(Bias(n.Z.MMR + 1), RMS(n.Z.MMR + 1));
				Noise(n.Z.MMR + 2) = normrnd(Bias(n.Z.MMR + 2), RMS(n.Z.MMR + 2));
				if(SAFE_MODE == 1) {
					Noise(n.Z.MMR + 0) = 0;
					Noise(n.Z.MMR + 1) = 0;
					Noise(n.Z.MMR + 2) = 0;
				}

				mmr.fill2(MMR[0] + Noise(n.Z.MMR + 0), MMR[1] + Noise(n.Z.MMR + 1), MMR[2] + Noise(n.Z.MMR + 2));
				mmr = mmr / norm(mmr);

				_LOCKCPP(Z_lock, -1);
				cbPush(mZB, n.Z.MMR + 0, mmr[0], t);
				cbPush(mZB, n.Z.MMR + 1, mmr[1], t);
				cbPush(mZB, n.Z.MMR + 2, mmr[2], t);
				_UNLOCKCPP(Z_lock, -1);
				//}
			}

			if(n.Z.GPS_V != 0) {
				_LOCKCPP(Z_lock, -1);
				cbPush(mZB, n.Z.GPS_V + 0, 180 / M_PI * atan2(v_ac.y(), v_ac.x()), t);
				cbPush(mZB, n.Z.GPS_V + 1, sqrt(v_ac.x()*v_ac.x() + v_ac.y()*v_ac.y()), t);
				cbPush(mZB, n.Z.GPS_V + 2, -v_ac.z(), t);
				_UNLOCKCPP(Z_lock, -1);
			}
		}

	//Prs////////////////////////////////////////////////////
	if(n.Z.PrsAlt != 0 && Freq(n.Z.PrsAlt) > 1e-6)
		if(time_reached(1. / Freq(n.Z.PrsAlt), 0., Sim.t, tPre) == 1) {
			Noise(n.Z.PrsAlt) = normrnd(Bias(n.Z.PrsAlt), RMS(n.Z.PrsAlt)); //Prs
			if(SAFE_MODE == 1) {
				Noise(n.Z.PrsAlt) = 0;
			}

			_LOCKCPP(Z_lock, -1);
			cbPush(mZB, n.Z.PrsAlt, lla.z() + /*param_world_scale?? * */ Noise(n.Z.PrsAlt), t);//Prs
			//cbPush(mZB, n.Z.PrsAlt, -Loc[1] + Noise(n.Z.PrsAlt), t);//Prs
			_UNLOCKCPP(Z_lock, -1);

			//APrs////////////////////////////////////////////////////
			Noise(n.Z.PrsAcc) = normrnd(Bias(n.Z.PrsAcc), RMS(n.Z.PrsAcc)); //APrs
			if(SAFE_MODE == 1) {
				Noise(n.Z.PrsAcc) = 0;
			}

			//APrs=-(V[2]-Vpre[2])/T;
			_LOCKCPP(Z_lock, -1);
			cbPush(mZB, n.Z.PrsAcc, a_ac.z() + Noise(n.Z.PrsAcc), t);//APrs
			_UNLOCKCPP(Z_lock, -1);
		}

	//Time//////////////////////////////////////////////////
	_LOCKCPP(Z_lock, -1);
	cbPush(mZB, n.Z.Time, t, t); //time
	_UNLOCKCPP(Z_lock, -1);

	//Save Previous States//////////////////////////////////
	tPre = t;
}

#ifdef __cplusplus
}
#endif
