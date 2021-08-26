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

//#include "nsrLinuxKeyBoard2.h"

#include "Core/nsrUtility.h" //for time_reached
#include "Visualize/nsrVisualize.h"
#include "Matlib/nsrQuat.h"
#include "Sim/nsrRosInterface.h"

//#include <unistd.h> //usleep

#undef TAG
#define TAG "Cpp:Main:"

#ifdef __cplusplus
extern "C" {
#endif

SimStorage Sim;

int simInit()
{
    if(param_do_what == DO_SAVE_ROS_BAG)
        rosOpen(param_rosbag_compression);
    
	Sim.t = nsrPoseMakerGetStartTime();
    return 0;
}

int simLoop(double time_barrier)
{

	while(Sim.t <= time_barrier) {

		//Simulate Sensors//////////////////////////////////////
		Sensors(Sim);
		
		//Show//////////////////////////////////////////////////
		if(time_reached(Sim.Tplot1 / param_speed_factor, 0., Sim.t, Sim.t - Sim.Tenv) == 1)
			Visualize(Sim, Sim.Tplot2);

		//end program//////////////////////////////////////////
			//while((pose_maker_time_s - path_start_time_s) <= (time_barrier - path_start_time_s)* param_speed_factor + SENSOR_DERIVATIVE_STEP_TIME) {

		if(Sim.t - nsrPoseMakerGetStartTime() >= param_end_time/param_speed_factor) {
			LOGE(TAG, " Exit due to end of filter time...\n");
			ask_for_finish(); 
			return -1;
		}

		Sim.i = Sim.i + 1;
		Sim.t = Sim.t + Sim.Tenv;
	}

	return 0;
}

int simClose()
{
    rosClose();
    return 0;
}

#ifdef __cplusplus
}
#endif
