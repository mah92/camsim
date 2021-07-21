//For ALLAH
//Acronyms:
//Sim.Env,n.e: Environment states
//Sim.O,n.O: Object's true states | Output states
//Sim.Z,n.Z: Sensors vector
//Flt.Z,n.Z: Sensors vector
#ifndef __ANDROID__
#include "/usr/include/time.h"
#endif

#include "Sim/Sim.h"

#include "nsrLinuxKeyBoard2.h"
#include "nsrPoseSim.h"

#include "Core/nsrUtility.h" //for time_reached
#include "Visualize/nsrVisualize.h"
#include "Matlib/nsrQuat.h"
#include "Sim/nsrRosInterface.h"

#include <unistd.h> //usleep

#undef TAG
#define TAG "Cpp:Main:"

#ifdef __cplusplus
extern "C" {
#endif

SimStorage Sim;

int simInit()
{
	Init_Sim(Sim);
	nsrPoseMakerInit();
    
    if(param_do_what == DO_SAVE_ROS_BAG)
        rosOpen();
    
    return 0;
}

int simLoop(double time_barrier)
{
	nsrPoseMakerLoop(nsrMax(time_barrier, //sensors needs until next frame
							time_barrier - (param_td + param_td_err) + 0.5 * (param_tr + param_tr_err) - 0. + 0.5 * param_te) //next camera frame needs
					);

	while(Sim.t <= time_barrier) {
		/*if(Sim.DynamicInTheLoop==1) {
		    //Simulate Environment//////////////////////////////
		    Sim.Env = Environment(Sim.O, Sim.Tenv);

		    //Simulate Aircraft//////////////////////////////////
		    //outputs Sim.O
		    Aircraft2D(Sim.O, Sim.n, Con_U, Sim.Env, Sim.t);
		    //Aircraft3D(Sim.O, Sim.n, Con_U, Sim.Env, Sim.t);
		} else {
		    Command(Sim.O, Sim.t, Sim.O); //also called in Autopilot.m
		    addDynamic(Sim.O, Sim.Tenv);
		}*/

		//Simulate Sensors//////////////////////////////////////
		Sensors(Sim);

		if(time_reached(Sim.Texec, 0., Sim.t, Sim.t - Sim.Tenv) == 1) {    //AutoPilot freq, ran from second step
			//AutpPilot//////////////////////////////////////////
			//Filter(Flt);
		}
		
		//Show//////////////////////////////////////////////////
		if(time_reached(Sim.Tplot1 / param_speed_factor, 0., Sim.t, Sim.t - Sim.Tenv) == 1)
			Visualize(Sim, Sim.Tplot2);//, Con);

		/////////////////////////////////////
		pauseTrap();
		if(Sim.t > param_end_time/param_speed_factor) {
			if(execution_turn < 0) //Don't stop is an automatic run is in progress
				refreshPlotsOnEnd();
			LOGE(TAG, " Exit due to end of filter time...\n");
			simClose();
			nsrExit(); //end program
		}

		Sim.i = Sim.i + 1;
		Sim.t = Sim.t + Sim.Tenv;
	}

	return 0;
}

int simClose()
{
    rosClose();
	nsrPoseMakerClose();
    return 0;
}

#ifdef __cplusplus
}
#endif
