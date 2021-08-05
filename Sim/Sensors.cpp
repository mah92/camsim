#ifndef __ANDROID__
#include "/usr/include/time.h"
#endif

#include "./Sim.h"
#include "Core/RotLib/nsrRotLib.h"
#include "View/nsrGeoLib.h"
#include "./nsrPoseSim.h"
#include "Matlib/nsrQuat.h"
#include "Sim/nsrSimParamReader.h"
#include "Sim/nsrRosInterface.h"

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

    double senx, seny, senz;
	double t = Sim.t;

	//printf("sen:%f\n", t);
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
	nsr::Quat q(acInNedQu.x(), acInNedQu.y(), acInNedQu.z(), acInNedQu.w());
    q.getRotMat(CI2B);
    
    //registerRosGroundTruth(t, lla.x(), lla.y(), lla.z(), q.e1, q.e2, q.e3, q.et);

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

            senx = acc_ac.x() + Noise(n.Z.Acc + 0); //Acc
			seny = acc_ac.y() + Noise(n.Z.Acc + 1);
			senz = acc_ac.z() + Noise(n.Z.Acc + 2);
            
			_LOCKCPP(Z_lock, -1);
			cbPush(mZB, n.Z.Acc + 0, senx, t); cbPush(mZB, n.Z.Acc + 1, seny, t); cbPush(mZB, n.Z.Acc + 2, senz, t);
			_UNLOCKCPP(Z_lock, -1);
            
            registerRosAcc(t, senx, seny, senz,
                SAFE_MODE == 1 ? 0 : RMS(n.Z.Acc + 0), SAFE_MODE == 1 ? 0 : RMS(n.Z.Acc + 1), SAFE_MODE == 1 ? 0 : RMS(n.Z.Acc + 2)
            );
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

			senx = w_ac.x() + Noise(n.Z.Gyro + 0); //Gyro
			seny = w_ac.y() + Noise(n.Z.Gyro + 1);
			senz = w_ac.z() + Noise(n.Z.Gyro + 2);
			
			_LOCKCPP(Z_lock, -1);
			cbPush(mZB, n.Z.Gyro + 0, senx, t); cbPush(mZB, n.Z.Gyro + 1, seny, t); cbPush(mZB, n.Z.Gyro + 2, senz, t);
			_UNLOCKCPP(Z_lock, -1);
            
            registerRosGyro(t, senx, seny, senz,
                SAFE_MODE == 1 ? 0 : RMS(n.Z.Gyro + 0), SAFE_MODE == 1 ? 0 : RMS(n.Z.Gyro + 1), SAFE_MODE == 1 ? 0 : RMS(n.Z.Gyro + 2)
            );

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

            senx = mmm[0]; seny = mmm[1]; senz = mmm[2];
            
			_LOCKCPP(Z_lock, -1);
			cbPush(mZB, n.Z.MMM + 0, senx, t); cbPush(mZB, n.Z.MMM + 1, seny, t); cbPush(mZB, n.Z.MMM + 2, senz, t);
			_UNLOCKCPP(Z_lock, -1);
            
            registerRosMag(t, senx, seny, senz,
                SAFE_MODE == 1 ? 0 : RMS(n.Z.MMM + 0), SAFE_MODE == 1 ? 0 : RMS(n.Z.MMM + 1), SAFE_MODE == 1 ? 0 : RMS(n.Z.MMM + 2)
            );
		}

	//GPS////////////////////////////////////////////////////
	if(n.Z.GPS_LLA!=0 && Freq(n.Z.GPS_LLA) > 1e-6
				&& (Sim.t < (10. / param_speed_factor) /*&& SAFE_START==0*/)) //On 1st 10seconds, if SAFE_START procedure is present, gps is not needed
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

                double EarthRadius = getEarthRadius(); //6378137
                senx = lla.x() + Noise(n.Z.GPS_LLA + 0) * (180 / M_PI) / EarthRadius;
                seny = lla.y() + Noise(n.Z.GPS_LLA + 1) * (180 / M_PI) / (EarthRadius * cos(lla.x()*M_PI / 180));
                senz = lla.z() + /*param_world_scale?? * */ Noise(n.Z.GPS_LLA + 2);
                
				_LOCKCPP(Z_lock, -1);
				cbPush(mZB, n.Z.GPS_LLA + 0, senx, t); cbPush(mZB, n.Z.GPS_LLA + 1, seny, t); cbPush(mZB, n.Z.GPS_LLA + 2, senz, t);
				_UNLOCKCPP(Z_lock, -1);
                
                registerRosGPS(t, senx, seny, senz,
                    SAFE_MODE == 1 ? 0 : RMS(n.Z.GPS_LLA + 0), SAFE_MODE == 1 ? 0 : RMS(n.Z.GPS_LLA + 1), SAFE_MODE == 1 ? 0 : RMS(n.Z.GPS_LLA + 2)
                );

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

                senx = mmr[0];
                seny = mmr[1];
                senz = mmr[2];
                
				_LOCKCPP(Z_lock, -1);
				cbPush(mZB, n.Z.MMR + 0, senx, t); cbPush(mZB, n.Z.MMR + 1, seny, t); cbPush(mZB, n.Z.MMR + 2, senz, t);
				_UNLOCKCPP(Z_lock, -1);
                
                registerRosMagRef(t, senx, seny, senz,
                    SAFE_MODE == 1 ? 0 : RMS(n.Z.MMR + 0), SAFE_MODE == 1 ? 0 : RMS(n.Z.MMR + 1), SAFE_MODE == 1 ? 0 : RMS(n.Z.MMR + 2)
                );
			}

			if(n.Z.GPS_V != 0) {
                senx = 180 / M_PI * atan2(v_ac.y(), v_ac.x());
                seny = sqrt(v_ac.x()*v_ac.x() + v_ac.y()*v_ac.y());
                senz = -v_ac.z();
                
				_LOCKCPP(Z_lock, -1);
				cbPush(mZB, n.Z.GPS_V + 0, senx, t); cbPush(mZB, n.Z.GPS_V + 1, seny, t); cbPush(mZB, n.Z.GPS_V + 2, senz, t);
				_UNLOCKCPP(Z_lock, -1);
                
                registerRosGPSVel(t, v_ac.x(), v_ac.y(), v_ac.z(),
                    0,0,0
                );
			}
		}

	//Prs////////////////////////////////////////////////////
	if(n.Z.PrsAlt != 0 && Freq(n.Z.PrsAlt) > 1e-6)
		if(time_reached(1. / Freq(n.Z.PrsAlt), 0., Sim.t, tPre) == 1) {
			Noise(n.Z.PrsAlt) = normrnd(Bias(n.Z.PrsAlt), RMS(n.Z.PrsAlt)); //Prs
			if(SAFE_MODE == 1) {
				Noise(n.Z.PrsAlt) = 0;
			}
			
			senz = lla.z() + /*param_world_scale?? * */ Noise(n.Z.PrsAlt);
            
			_LOCKCPP(Z_lock, -1);
			cbPush(mZB, n.Z.PrsAlt, senz, t);//Prs
			_UNLOCKCPP(Z_lock, -1);
            
            registerRosPrsAlt(t, senz, SAFE_MODE == 1 ? 0 : RMS(n.Z.PrsAlt));
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
