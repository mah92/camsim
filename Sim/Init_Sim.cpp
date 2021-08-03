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
    ///////////////////////////////////////////////////////////////////////////////////////
#define MEAN_ALT (123.825)

	/*if(execution_turn < 0) {
		//param_speed_factor = .2;
		param_speed_factor = .49; strcpy(execution_desc, "0160-half_speed(0.49)-noTimeBasedUpdate");
	}*/
	//param_speed_factor = .25; strcpy(execution_desc, "0155-0.25x_speed-noTimeBasedUpdate"); //diverge
	//param_speed_factor = .24; strcpy(execution_desc, "0154-0.24x_speed"); //diverge
	//param_speed_factor = .5; strcpy(execution_desc, "0101-half_speed");
	//{param_cam_in_ac_err_x += 1; strcpy(execution_desc, "0760-param_cam_in_ac_err_x+=1-compensated");}
	//{param_cam_in_ac_err_y += 1; strcpy(execution_desc, "0770-param_cam_in_ac_err_y+=1-compensated");}
	//{param_cam_in_ac_err_z += 1; strcpy(execution_desc, "0780-param_cam_in_ac_err_z+=1-compensated");}
	//execution_turn = 2;
	
	//param_speed_factor = .5; param_alt_offset += (8 - 1) * MEAN_ALT; param_map_max_dist *= 4; 
	//strcpy(execution_desc, "10080-8x_altitude-half_speed");

#define REPEAT_COUNT 10
	if(execution_turn >= 0) {
		int cc = 0;
		param_seed += execution_turn + 150;
              
/*
		/////////////////
		if((execution_turn / REPEAT_COUNT) == cc++) {keyframe_distance = 20; param_speed_factor = 1.; param_alt_offset += (16 - 1) * MEAN_ALT; param_map_max_dist *= 4; 
			strcpy(execution_desc, "10161-16x_altitude-1x_speed-20m");}
		if((execution_turn / REPEAT_COUNT) == cc++) {keyframe_distance = 20; param_speed_factor = .5; param_alt_offset += (16 - 1) * MEAN_ALT; param_map_max_dist *= 4; 
			strcpy(execution_desc, "10160-16x_altitude-half_speed-20m");}
		if((execution_turn / REPEAT_COUNT) == cc++) {keyframe_distance = 20; param_speed_factor = 2.; param_alt_offset += (16 - 1) * MEAN_ALT; param_map_max_dist *= 4; 
			strcpy(execution_desc, "10162-16x_altitude-2x_speed-20m");}
		if((execution_turn / REPEAT_COUNT) == cc++) {keyframe_distance = 20; param_speed_factor = 4.; param_alt_offset += (16 - 1) * MEAN_ALT; param_map_max_dist *= 4; 
			strcpy(execution_desc, "10164-16x_altitude-4x_speed-20m");}
		if((execution_turn / REPEAT_COUNT) == cc++) {keyframe_distance = 20; param_speed_factor = 8.; param_alt_offset += (16 - 1) * MEAN_ALT; param_map_max_dist *= 4; 
			strcpy(execution_desc, "10168-16x_altitude-8x_speed-20m");}
			
		if((execution_turn / REPEAT_COUNT) == cc++) {keyframe_distance = 40; param_speed_factor = 1.; param_alt_offset += (16 - 1) * MEAN_ALT; param_map_max_dist *= 4; 
			strcpy(execution_desc, "10161-16x_altitude-1x_speed-40m");}
		if((execution_turn / REPEAT_COUNT) == cc++) {keyframe_distance = 40; param_speed_factor = .5; param_alt_offset += (16 - 1) * MEAN_ALT; param_map_max_dist *= 4; 
			strcpy(execution_desc, "10160-16x_altitude-half_speed-40m");}
		if((execution_turn / REPEAT_COUNT) == cc++) {keyframe_distance = 40; param_speed_factor = 2.; param_alt_offset += (16 - 1) * MEAN_ALT; param_map_max_dist *= 4; 
			strcpy(execution_desc, "10162-16x_altitude-2x_speed-40m");}
		if((execution_turn / REPEAT_COUNT) == cc++) {keyframe_distance = 40; param_speed_factor = 4.; param_alt_offset += (16 - 1) * MEAN_ALT; param_map_max_dist *= 4; 
			strcpy(execution_desc, "10164-16x_altitude-4x_speed-40m");}
		if((execution_turn / REPEAT_COUNT) == cc++) {keyframe_distance = 40; param_speed_factor = 8.; param_alt_offset += (16 - 1) * MEAN_ALT; param_map_max_dist *= 4; 
			strcpy(execution_desc, "10168-16x_altitude-8x_speed-40m");}
			
		if((execution_turn / REPEAT_COUNT) == cc++) {keyframe_distance = 80; param_speed_factor = 1.; param_alt_offset += (16 - 1) * MEAN_ALT; param_map_max_dist *= 4; 
			strcpy(execution_desc, "10161-16x_altitude-1x_speed-80m");}
		if((execution_turn / REPEAT_COUNT) == cc++) {keyframe_distance = 80; param_speed_factor = .5; param_alt_offset += (16 - 1) * MEAN_ALT; param_map_max_dist *= 4; 
			strcpy(execution_desc, "10160-16x_altitude-half_speed-80m");}
		if((execution_turn / REPEAT_COUNT) == cc++) {keyframe_distance = 80; param_speed_factor = 2.; param_alt_offset += (16 - 1) * MEAN_ALT; param_map_max_dist *= 4; 
			strcpy(execution_desc, "10162-16x_altitude-2x_speed-80m");}
		if((execution_turn / REPEAT_COUNT) == cc++) {keyframe_distance = 80; param_speed_factor = 4.; param_alt_offset += (16 - 1) * MEAN_ALT; param_map_max_dist *= 4; 
			strcpy(execution_desc, "10164-16x_altitude-4x_speed-80m");}
		if((execution_turn / REPEAT_COUNT) == cc++) {keyframe_distance = 80; param_speed_factor = 8.; param_alt_offset += (16 - 1) * MEAN_ALT; param_map_max_dist *= 4; 
			strcpy(execution_desc, "10168-16x_altitude-8x_speed-80m");}

		///////////////
		if((execution_turn / REPEAT_COUNT) == cc++) {param_fx *= 2; param_fy *= 2; param_speed_factor = 1.; param_alt_offset += (16 - 1) * MEAN_ALT; param_map_max_dist *= 4; strcpy(execution_desc, "10161-16x_altitude-1x_speed-f2x");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_fx *= 2; param_fy *= 2; param_speed_factor = .5; param_alt_offset += (16 - 1) * MEAN_ALT; param_map_max_dist *= 4; strcpy(execution_desc, "10160-16x_altitude-half_speed-f2x");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_fx *= 2; param_fy *= 2; param_speed_factor = 2.; param_alt_offset += (16 - 1) * MEAN_ALT; param_map_max_dist *= 4; strcpy(execution_desc, "10162-16x_altitude-2x_speed-f2x");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_fx *= 2; param_fy *= 2; param_speed_factor = 4.; param_alt_offset += (16 - 1) * MEAN_ALT; param_map_max_dist *= 4; strcpy(execution_desc, "10164-16x_altitude-4x_speed-f2x");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_fx *= 2; param_fy *= 2; param_speed_factor = 8.; param_alt_offset += (16 - 1) * MEAN_ALT; param_map_max_dist *= 4; strcpy(execution_desc, "10168-16x_altitude-8x_speed-f2x");}			
		
		if((execution_turn / REPEAT_COUNT) == cc++) {param_fx *= 4; param_fy *= 4; param_speed_factor = 1.; param_alt_offset += (16 - 1) * MEAN_ALT; param_map_max_dist *= 4; strcpy(execution_desc, "10161-16x_altitude-1x_speed-f4x");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_fx *= 4; param_fy *= 4; param_speed_factor = .5; param_alt_offset += (16 - 1) * MEAN_ALT; param_map_max_dist *= 4; strcpy(execution_desc, "10160-16x_altitude-half_speed-f4x");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_fx *= 4; param_fy *= 4; param_speed_factor = 2.; param_alt_offset += (16 - 1) * MEAN_ALT; param_map_max_dist *= 4; strcpy(execution_desc, "10162-16x_altitude-2x_speed-f4x");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_fx *= 4; param_fy *= 4; param_speed_factor = 4.; param_alt_offset += (16 - 1) * MEAN_ALT; param_map_max_dist *= 4; strcpy(execution_desc, "10164-16x_altitude-4x_speed-f4x");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_fx *= 4; param_fy *= 4; param_speed_factor = 8.; param_alt_offset += (16 - 1) * MEAN_ALT; param_map_max_dist *= 4; strcpy(execution_desc, "10168-16x_altitude-8x_speed-f4x");}			
		
		if((execution_turn / REPEAT_COUNT) == cc++) {param_fx *= 8; param_fy *= 8; param_speed_factor = 1.; param_alt_offset += (16 - 1) * MEAN_ALT; param_map_max_dist *= 4; strcpy(execution_desc, "10161-16x_altitude-1x_speed-f8x");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_fx *= 8; param_fy *= 8; param_speed_factor = .5; param_alt_offset += (16 - 1) * MEAN_ALT; param_map_max_dist *= 4; strcpy(execution_desc, "10160-16x_altitude-half_speed-f8x");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_fx *= 8; param_fy *= 8; param_speed_factor = 2.; param_alt_offset += (16 - 1) * MEAN_ALT; param_map_max_dist *= 4; strcpy(execution_desc, "10162-16x_altitude-2x_speed-f8x");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_fx *= 8; param_fy *= 8; param_speed_factor = 4.; param_alt_offset += (16 - 1) * MEAN_ALT; param_map_max_dist *= 4; strcpy(execution_desc, "10164-16x_altitude-4x_speed-f8x");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_fx *= 8; param_fy *= 8; param_speed_factor = 8.; param_alt_offset += (16 - 1) * MEAN_ALT; param_map_max_dist *= 4; strcpy(execution_desc, "10168-16x_altitude-8x_speed-f8x");}			

		////////////////
		if((execution_turn / REPEAT_COUNT) == cc++) {default_max_augmentation_units = 20; param_speed_factor = 1.; param_alt_offset += (16 - 1) * MEAN_ALT; param_map_max_dist *= 4; strcpy(execution_desc, "10161-16x_altitude-1x_speed-aug20");}
		if((execution_turn / REPEAT_COUNT) == cc++) {default_max_augmentation_units = 20; param_speed_factor = .5; param_alt_offset += (16 - 1) * MEAN_ALT; param_map_max_dist *= 4; strcpy(execution_desc, "10160-16x_altitude-half_speed-aug20");}
		if((execution_turn / REPEAT_COUNT) == cc++) {default_max_augmentation_units = 20; param_speed_factor = 2.; param_alt_offset += (16 - 1) * MEAN_ALT; param_map_max_dist *= 4; strcpy(execution_desc, "10162-16x_altitude-2x_speed-aug20");}
		if((execution_turn / REPEAT_COUNT) == cc++) {default_max_augmentation_units = 20; param_speed_factor = 4.; param_alt_offset += (16 - 1) * MEAN_ALT; param_map_max_dist *= 4; strcpy(execution_desc, "10164-16x_altitude-4x_speed-aug20");}
		if((execution_turn / REPEAT_COUNT) == cc++) {default_max_augmentation_units = 20; param_speed_factor = 8.; param_alt_offset += (16 - 1) * MEAN_ALT; param_map_max_dist *= 4; strcpy(execution_desc, "10168-16x_altitude-8x_speed-aug20");}
		
		if((execution_turn / REPEAT_COUNT) == cc++) {default_max_augmentation_units = 40; param_speed_factor = 1.; param_alt_offset += (16 - 1) * MEAN_ALT; param_map_max_dist *= 4; strcpy(execution_desc, "10161-16x_altitude-1x_speed-aug40");}
		if((execution_turn / REPEAT_COUNT) == cc++) {default_max_augmentation_units = 40; param_speed_factor = .5; param_alt_offset += (16 - 1) * MEAN_ALT; param_map_max_dist *= 4; strcpy(execution_desc, "10160-16x_altitude-half_speed-aug40");}
		if((execution_turn / REPEAT_COUNT) == cc++) {default_max_augmentation_units = 40; param_speed_factor = 2.; param_alt_offset += (16 - 1) * MEAN_ALT; param_map_max_dist *= 4; strcpy(execution_desc, "10162-16x_altitude-2x_speed-aug40");}
		if((execution_turn / REPEAT_COUNT) == cc++) {default_max_augmentation_units = 40; param_speed_factor = 4.; param_alt_offset += (16 - 1) * MEAN_ALT; param_map_max_dist *= 4; strcpy(execution_desc, "10164-16x_altitude-4x_speed-aug40");}
		if((execution_turn / REPEAT_COUNT) == cc++) {default_max_augmentation_units = 40; param_speed_factor = 8.; param_alt_offset += (16 - 1) * MEAN_ALT; param_map_max_dist *= 4; strcpy(execution_desc, "10168-16x_altitude-8x_speed-aug40");}
		
		if((execution_turn / REPEAT_COUNT) == cc++) {default_max_augmentation_units = 80; param_speed_factor = 1.; param_alt_offset += (16 - 1) * MEAN_ALT; param_map_max_dist *= 4; strcpy(execution_desc, "10161-16x_altitude-1x_speed-aug80");}
		if((execution_turn / REPEAT_COUNT) == cc++) {default_max_augmentation_units = 80; param_speed_factor = .5; param_alt_offset += (16 - 1) * MEAN_ALT; param_map_max_dist *= 4; strcpy(execution_desc, "10160-16x_altitude-half_speed-aug80");}
		if((execution_turn / REPEAT_COUNT) == cc++) {default_max_augmentation_units = 80; param_speed_factor = 2.; param_alt_offset += (16 - 1) * MEAN_ALT; param_map_max_dist *= 4; strcpy(execution_desc, "10162-16x_altitude-2x_speed-aug80");}
		if((execution_turn / REPEAT_COUNT) == cc++) {default_max_augmentation_units = 80; param_speed_factor = 4.; param_alt_offset += (16 - 1) * MEAN_ALT; param_map_max_dist *= 4; strcpy(execution_desc, "10164-16x_altitude-4x_speed-aug80");}
		if((execution_turn / REPEAT_COUNT) == cc++) {default_max_augmentation_units = 80; param_speed_factor = 8.; param_alt_offset += (16 - 1) * MEAN_ALT; param_map_max_dist *= 4; strcpy(execution_desc, "10168-16x_altitude-8x_speed-aug80");}
	*/	
		/*
		if((execution_turn / REPEAT_COUNT) == cc++) {param_speed_factor = 1.; param_alt_offset -= 0.5 * MEAN_ALT; param_map_zoom = 18;       
			strcpy(execution_desc, "10001-half_altitude_2x_zoom-1x_speed");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_speed_factor = 1.; param_alt_offset += (1 - 1) * MEAN_ALT;                        
			strcpy(execution_desc, "10011-1x_altitude-1x_speed");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_speed_factor = 1.; param_alt_offset += (2 - 1) * MEAN_ALT;                        
			strcpy(execution_desc, "10021-2x_altitude-1x_speed");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_speed_factor = 1.; param_alt_offset += (4 - 1) * MEAN_ALT; param_map_max_dist *= 2; 
			strcpy(execution_desc, "10041-4x_altitude-1x_speed");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_speed_factor = 1.; param_alt_offset += (8 - 1) * MEAN_ALT; param_map_max_dist *= 4; 
			strcpy(execution_desc, "10081-8x_altitude-1x_speed");}
		
		if((execution_turn / REPEAT_COUNT) == cc++) {param_speed_factor = .5; param_alt_offset -= 0.5 * MEAN_ALT; param_map_zoom = 18;       
			strcpy(execution_desc, "10000-half_altitude_2x_zoom-half_speed");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_speed_factor = .5; param_alt_offset += (1 - 1) * MEAN_ALT;                        
			strcpy(execution_desc, "10010-1x_altitude-half_speed");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_speed_factor = .5; param_alt_offset += (2 - 1) * MEAN_ALT;                        
			strcpy(execution_desc, "10020-2x_altitude-half_speed");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_speed_factor = .5; param_alt_offset += (4 - 1) * MEAN_ALT; param_map_max_dist *= 2; 
			strcpy(execution_desc, "10040-4x_altitude-half_speed");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_speed_factor = .5; param_alt_offset += (8 - 1) * MEAN_ALT; param_map_max_dist *= 4; 
			strcpy(execution_desc, "10080-8x_altitude-half_speed");}
		
		if((execution_turn / REPEAT_COUNT) == cc++) {param_speed_factor = 2.; param_alt_offset -= 0.5 * MEAN_ALT; param_map_zoom = 18;       
			strcpy(execution_desc, "10002-half_altitude_2x_zoom-2x_speed");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_speed_factor = 2.; param_alt_offset += (1 - 1) * MEAN_ALT;                        
			strcpy(execution_desc, "10012-1x_altitude-2x_speed");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_speed_factor = 2.; param_alt_offset += (2 - 1) * MEAN_ALT;                        
			strcpy(execution_desc, "10022-2x_altitude-2x_speed");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_speed_factor = 2.; param_alt_offset += (4 - 1) * MEAN_ALT; param_map_max_dist *= 2; 
			strcpy(execution_desc, "10042-4x_altitude-2x_speed");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_speed_factor = 2.; param_alt_offset += (8 - 1) * MEAN_ALT; param_map_max_dist *= 4; 
			strcpy(execution_desc, "10082-8x_altitude-2x_speed");}
		
		if((execution_turn / REPEAT_COUNT) == cc++) {param_speed_factor = 1.; param_alt_offset += (16 - 1) * MEAN_ALT; param_map_max_dist *= 4; 
			strcpy(execution_desc, "10161-16x_altitude-1x_speed");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_speed_factor = .5; param_alt_offset += (16 - 1) * MEAN_ALT; param_map_max_dist *= 4; 
			strcpy(execution_desc, "10160-16x_altitude-half_speed");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_speed_factor = 2.; param_alt_offset += (16 - 1) * MEAN_ALT; param_map_max_dist *= 4; 
			strcpy(execution_desc, "10162-16x_altitude-2x_speed");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_speed_factor = 4.; param_alt_offset += (16 - 1) * MEAN_ALT; param_map_max_dist *= 4; 
			strcpy(execution_desc, "10164-16x_altitude-4x_speed");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_speed_factor = 8.; param_alt_offset += (16 - 1) * MEAN_ALT; param_map_max_dist *= 4; 
			strcpy(execution_desc, "10168-16x_altitude-8x_speed");}
		*/

		//if((execution_turn / REPEAT_COUNT) == cc++) {param_cam_in_ac_roll = 0.; param_cam_in_ac_pitch = -90.; param_cam_in_ac_yaw = -90.; param_tr_err += 0.03; ysync=1;
		//	strcpy(execution_desc, "xcamOnxac-ysync-UnknownTr=0.030");} //left-right

		//if((execution_turn / REPEAT_COUNT) == cc++) {param_cam_in_ac_roll = 0.; param_cam_in_ac_pitch = -90.; param_cam_in_ac_yaw = -90.; param_td_err += 0.1; ysync=1;
		//	strcpy(execution_desc, "xcamOnxac-ysync-UnknownTd=0.1");} //left-right
/*
		if((execution_turn / REPEAT_COUNT) == cc++) {
			param_cam_in_ac_roll = 0.; param_cam_in_ac_pitch = -90.; param_cam_in_ac_yaw = -90.;
			strcpy(execution_desc, "xcamOnxac-xsync");
		} //forward-backward

		if((execution_turn / REPEAT_COUNT) == cc++) {
			param_cam_in_ac_roll = 0.; param_cam_in_ac_pitch = -90.; param_cam_in_ac_yaw = -90.; param_tr_err += 0.03;
			strcpy(execution_desc, "xcamOnxac-xsync-UnknownTr=0.030");
		} //forward-backward

		if((execution_turn / REPEAT_COUNT) == cc++) {
			param_cam_in_ac_roll = 0.; param_cam_in_ac_pitch = -90.; param_cam_in_ac_yaw = -90.; param_td_err += 0.1;
			strcpy(execution_desc, "xcamOnxac-xsync-UnknownTd=0.1");
		} //forward-backward
*/
/*		if((execution_turn / REPEAT_COUNT) == cc++) {param_cam_in_ac_pitch = -80.; param_map_max_dist = 11100; strcpy(execution_desc, "0400-param_cam_in_ac_pitch=-80.");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_cam_in_ac_pitch = -70.; param_map_max_dist = 11100; strcpy(execution_desc, "0410-param_cam_in_ac_pitch=-70.");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_cam_in_ac_pitch = -60.; param_map_max_dist = 11100; strcpy(execution_desc, "0420-param_cam_in_ac_pitch=-60.");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_cam_in_ac_pitch = -50.; param_map_max_dist = 11100; strcpy(execution_desc, "0430-param_cam_in_ac_pitch=-50.");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_cam_in_ac_pitch = -40.; param_map_max_dist = 11100; strcpy(execution_desc, "0440-param_cam_in_ac_pitch=-40.");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_cam_in_ac_pitch = -30.; param_map_max_dist = 11100; strcpy(execution_desc, "0450-param_cam_in_ac_pitch=-30.");}
*/
		/*		if((execution_turn / REPEAT_COUNT) == cc++) {timed_update_interval = 0.1; cov_time_update_type = 0; features_in_timed_updates = 1; param_speed_factor = .12; strcpy(execution_desc, "0098-1/8x(0.12)_speed-DSCRT-1f2c");}
				if((execution_turn / REPEAT_COUNT) == cc++) {timed_update_interval = 0.1; cov_time_update_type = 3; features_in_timed_updates = 1; disable_pressure_after_startup = 1; disable_mag_after_startup = 1; param_speed_factor = .12; strcpy(execution_desc, "0098-1/8x(0.12)_speed-RK4-NoMag-NoPrs-1f2c");}
				if((execution_turn / REPEAT_COUNT) == cc++) {timed_update_interval = 2.; cov_time_update_type = 0; features_in_timed_updates = 1; param_speed_factor = .12; strcpy(execution_desc, "0098-1/8x(0.12)_speed-DSCRT-1f2d");}
				if((execution_turn / REPEAT_COUNT) == cc++) {timed_update_interval = 2.; cov_time_update_type = 3; features_in_timed_updates = 1; disable_pressure_after_startup = 1; disable_mag_after_startup = 1; param_speed_factor = .12; strcpy(execution_desc, "0098-1/8x(0.12)_speed-RK4-NoMag-NoPrs-1f2d");}
		*/

		/*if((execution_turn / REPEAT_COUNT) == cc++) {features_in_timed_updates = 0; param_speed_factor = .12; strcpy(execution_desc, "0098-1/8(0.12)_speed-DSCRT-0f");}
		if((execution_turn / REPEAT_COUNT) == cc++) {features_in_timed_updates = 1; param_speed_factor = .12; strcpy(execution_desc, "0098-1/8(0.12)_speed-DSCRT-1f2d");}
		if((execution_turn / REPEAT_COUNT) == cc++) {features_in_timed_updates = 1000; param_speed_factor = .12; strcpy(execution_desc, "0098-1/8(0.12)_speed-DSCRT-nf");}

		if((execution_turn / REPEAT_COUNT) == cc++) {features_in_timed_updates = 0; param_speed_factor = 1; strcpy(execution_desc, "0000-ideal-DSCRT");}
		*/

		/*if((execution_turn / REPEAT_COUNT) == cc++) {cov_time_update_type = 1; features_in_timed_updates = 1000; param_speed_factor = 1; strcpy(execution_desc, "0000-ideal-Rk1");}*/
		/*if((execution_turn / REPEAT_COUNT) == cc++) {cov_time_update_type = 2; features_in_timed_updates = 1000; param_speed_factor = 1; strcpy(execution_desc, "0000-ideal-Rk2");}
		if((execution_turn / REPEAT_COUNT) == cc++) {cov_time_update_type = 3; features_in_timed_updates = 1000; param_speed_factor = 1; strcpy(execution_desc, "0000-ideal-Rk4");}

		if((execution_turn / REPEAT_COUNT) == cc++) {cov_time_update_type = 3; features_in_timed_updates = 0; param_speed_factor = .12; strcpy(execution_desc, "0098-1/8(0.12)_speed-Rk4-0f");}
		if((execution_turn / REPEAT_COUNT) == cc++) {cov_time_update_type = 3; features_in_timed_updates = 1; param_speed_factor = .12; strcpy(execution_desc, "0098-1/8(0.12)_speed-Rk4-1f2d");}
		if((execution_turn / REPEAT_COUNT) == cc++) {cov_time_update_type = 3; features_in_timed_updates = 1000; param_speed_factor = .12; strcpy(execution_desc, "0098-1/8(0.12)_speed-Rk4-nf");}

		///
		if((execution_turn / REPEAT_COUNT) == cc++) {cov_time_update_type = 1; features_in_timed_updates = 1000; param_speed_factor = 1; strcpy(execution_desc, "0000-ideal-Rk1");}*/
		/*
			if((execution_turn / REPEAT_COUNT) == cc++) {cov_time_update_type = 2; features_in_timed_updates = 1000; param_speed_factor = 1; disable_mag_after_startup = 1; disable_pressure_after_startup = 1; strcpy(execution_desc, "0000-ideal-RK2-NoMag-NoPrs");}
			if((execution_turn / REPEAT_COUNT) == cc++) {cov_time_update_type = 3; features_in_timed_updates = 1000; param_speed_factor = 1; disable_mag_after_startup = 1; disable_pressure_after_startup = 1; strcpy(execution_desc, "0000-ideal-RK4-NoMag-NoPrs");}
			if((execution_turn / REPEAT_COUNT) == cc++) {cov_time_update_type = 5; features_in_timed_updates = 1000; param_speed_factor = 1; disable_mag_after_startup = 1; disable_pressure_after_startup = 1; strcpy(execution_desc, "0000-ideal-CLOSED-DSCRT2-NoMag-NoPrs");}

			if((execution_turn / REPEAT_COUNT) == cc++) {cov_time_update_type = 2; features_in_timed_updates = 1000; param_speed_factor = 1; disable_mag_after_startup = 1; strcpy(execution_desc, "0000-ideal-RK2-NoMag");}
			if((execution_turn / REPEAT_COUNT) == cc++) {cov_time_update_type = 3; features_in_timed_updates = 1000; param_speed_factor = 1; disable_mag_after_startup = 1; strcpy(execution_desc, "0000-ideal-RK4-NoMag");}
			if((execution_turn / REPEAT_COUNT) == cc++) {cov_time_update_type = 5; features_in_timed_updates = 1000; param_speed_factor = 1; disable_mag_after_startup = 1; strcpy(execution_desc, "0000-ideal-CLOSED-DSCRT2-NoMag");}

			if((execution_turn / REPEAT_COUNT) == cc++) {cov_time_update_type = 5; features_in_timed_updates = 1000; param_speed_factor = 1; strcpy(execution_desc, "0000-ideal-CLOSED-DSCRT2");}
		*/
		/////////////////
		//if((execution_turn / REPEAT_COUNT) == cc++) {cov_time_update_type = 5; features_in_timed_updates = 1; param_speed_factor = .12; strcpy(execution_desc, "0098-1/8(0.12)_speed-CLOSED-DSCRT2-1f2d");}

		/*		if((execution_turn / REPEAT_COUNT) == cc++) {cov_time_update_type = 3; features_in_timed_updates = 1; param_speed_factor = .12; strcpy(execution_desc, "0098-1/8(0.12)_speed-Rk4-1f2d");}
				if((execution_turn / REPEAT_COUNT) == cc++) {cov_time_update_type = 2; features_in_timed_updates = 1; param_speed_factor = .12; strcpy(execution_desc, "0098-1/8(0.12)_speed-Rk2-1f2d");}
				if((execution_turn / REPEAT_COUNT) == cc++) {cov_time_update_type = 0; features_in_timed_updates = 1; param_speed_factor = .12; strcpy(execution_desc, "0098-1/8(0.12)_speed-DSCRT-1f2d");}

				if((execution_turn / REPEAT_COUNT) == cc++) {cov_time_update_type = 3; features_in_timed_updates = 1; param_speed_factor = .12; strcpy(execution_desc, "0098-1/8(0.12)_speed-Rk4-1f2d");}
				if((execution_turn / REPEAT_COUNT) == cc++) {cov_time_update_type = 2; features_in_timed_updates = 1; param_speed_factor = .12; strcpy(execution_desc, "0098-1/8(0.12)_speed-Rk2-1f2d");}
				if((execution_turn / REPEAT_COUNT) == cc++) {cov_time_update_type = 0; features_in_timed_updates = 1; param_speed_factor = .12; strcpy(execution_desc, "0098-1/8(0.12)_speed-DSCRT-1f2d");}

				if((execution_turn / REPEAT_COUNT) == cc++) {cov_time_update_type = 3; features_in_timed_updates = 1; param_speed_factor = .12; strcpy(execution_desc, "0098-1/8(0.12)_speed-Rk4-1f2d");}
				if((execution_turn / REPEAT_COUNT) == cc++) {cov_time_update_type = 2; features_in_timed_updates = 1; param_speed_factor = .12; strcpy(execution_desc, "0098-1/8(0.12)_speed-Rk2-1f2d");}
				if((execution_turn / REPEAT_COUNT) == cc++) {cov_time_update_type = 0; features_in_timed_updates = 1; param_speed_factor = .12; strcpy(execution_desc, "0098-1/8(0.12)_speed-DSCRT-1f2d");}

				//if((execution_turn / REPEAT_COUNT) == cc++) {cov_time_update_type = 3; features_in_timed_updates = 1; param_speed_factor = .12; strcpy(execution_desc, "0098-1/8(0.12)_speed-Rk4-1f2d");}
				if((execution_turn / REPEAT_COUNT) == cc++) {cov_time_update_type = 2; features_in_timed_updates = 1; param_speed_factor = .12; strcpy(execution_desc, "0098-1/8(0.12)_speed-Rk2-1f2d");}
				if((execution_turn / REPEAT_COUNT) == cc++) {cov_time_update_type = 0; features_in_timed_updates = 1; param_speed_factor = .12; strcpy(execution_desc, "0098-1/8(0.12)_speed-DSCRT-1f2d");}

				//if((execution_turn / REPEAT_COUNT) == cc++) {cov_time_update_type = 3; features_in_timed_updates = 1; param_speed_factor = .12; strcpy(execution_desc, "0098-1/8(0.12)_speed-Rk4-1f2d");}
				if((execution_turn / REPEAT_COUNT) == cc++) {cov_time_update_type = 2; features_in_timed_updates = 1; param_speed_factor = .12; strcpy(execution_desc, "0098-1/8(0.12)_speed-Rk2-1f2d");}
				if((execution_turn / REPEAT_COUNT) == cc++) {cov_time_update_type = 0; features_in_timed_updates = 1; param_speed_factor = .12; strcpy(execution_desc, "0098-1/8(0.12)_speed-DSCRT-1f2d");}
		*/

		/*if((execution_turn / REPEAT_COUNT) == cc++) {cov_time_update_type = 2; features_in_timed_updates = 1; param_speed_factor = .12; strcpy(execution_desc, "0098-1/8(0.12)_speed-Rk2-1f2d");} //6

		if((execution_turn / REPEAT_COUNT) == cc++) {cov_time_update_type = 2; features_in_timed_updates = 0; param_speed_factor = .12; strcpy(execution_desc, "0098-1/8(0.12)_speed-Rk2-0f2d");} //4
		*/

		//////////////
		/*		if((execution_turn / REPEAT_COUNT) == cc++) {cov_time_update_type = 5; features_in_timed_updates = 0; param_speed_factor = .12; strcpy(execution_desc, "0098-1/8(0.12)-CLOSED-DSCRT2-0f2d");}
				if((execution_turn / REPEAT_COUNT) == cc++) {cov_time_update_type = 3; features_in_timed_updates = 0; param_speed_factor = .12; strcpy(execution_desc, "0098-1/8(0.12)_speed-Rk4-0f2d");}
				if((execution_turn / REPEAT_COUNT) == cc++) {cov_time_update_type = 2; features_in_timed_updates = 0; param_speed_factor = .12; strcpy(execution_desc, "0098-1/8(0.12)_speed-Rk2-0f2d");}
				if((execution_turn / REPEAT_COUNT) == cc++) {cov_time_update_type = 0; features_in_timed_updates = 0; param_speed_factor = .12; strcpy(execution_desc, "0098-1/8(0.12)_speed-DSCRT-0f2d");}

				if((execution_turn / REPEAT_COUNT) == cc++) {cov_time_update_type = 5; features_in_timed_updates = 0; disable_mag_after_startup = 1; param_speed_factor = .12; strcpy(execution_desc, "0098-1/8(0.12)-CLOSED-DSCRT2-0f2d-NoMag");}
				if((execution_turn / REPEAT_COUNT) == cc++) {cov_time_update_type = 3; features_in_timed_updates = 0; disable_mag_after_startup = 1; param_speed_factor = .12; strcpy(execution_desc, "0098-1/8(0.12)_speed-Rk4-0f2d-NoMag");}
				if((execution_turn / REPEAT_COUNT) == cc++) {cov_time_update_type = 2; features_in_timed_updates = 0; disable_mag_after_startup = 1; param_speed_factor = .12; strcpy(execution_desc, "0098-1/8(0.12)_speed-Rk2-0f2d-NoMag");}
				if((execution_turn / REPEAT_COUNT) == cc++) {cov_time_update_type = 0; features_in_timed_updates = 0; disable_mag_after_startup = 1; param_speed_factor = .12; strcpy(execution_desc, "0098-1/8(0.12)_speed-DSCRT-0f2d-NoMag");}

				if((execution_turn / REPEAT_COUNT) == cc++) {cov_time_update_type = 5; features_in_timed_updates = 0; disable_pressure_after_startup = 1; disable_mag_after_startup = 1; param_speed_factor = .12; strcpy(execution_desc, "0098-1/4(0.12)-CLOSED-DSCRT2-0f2d-NoMag-NoPrs");}
				if((execution_turn / REPEAT_COUNT) == cc++) {cov_time_update_type = 3; features_in_timed_updates = 0; disable_pressure_after_startup = 1; disable_mag_after_startup = 1; param_speed_factor = .12; strcpy(execution_desc, "0098-1/4(0.12)_speed-Rk4-0f2d-NoMag-NoPrs");}
				if((execution_turn / REPEAT_COUNT) == cc++) {cov_time_update_type = 2; features_in_timed_updates = 0; disable_pressure_after_startup = 1; disable_mag_after_startup = 1; param_speed_factor = .12; strcpy(execution_desc, "0098-1/4(0.12)_speed-Rk2-0f2d-NoMag-NoPrs");}
				if((execution_turn / REPEAT_COUNT) == cc++) {cov_time_update_type = 0; features_in_timed_updates = 0; disable_pressure_after_startup = 1; disable_mag_after_startup = 1; param_speed_factor = .12; strcpy(execution_desc, "0098-1/4(0.12)_speed-DSCRT-0f2d-NoMag-NoPrs");}
		*/
#if 0

		if((execution_turn / REPEAT_COUNT) == cc++) {strcpy(execution_desc, "0000-ideal");}

		//Flight profile///////////////////////////////////////
		/*if((execution_turn / REPEAT_COUNT) == cc++) {param_speed_factor = .5; strcpy(execution_desc, "0100-half_speed");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_speed_factor = 2;  strcpy(execution_desc, "0110-2x_speed");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_speed_factor = 4;  strcpy(execution_desc, "0120-4x_speed");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_speed_factor = 8;  strcpy(execution_desc, "0130-8x_speed");}*/

		//new config: gps is cut in a fixed point, so that pre gps-cut manuevers remain intact,
		//	and fixed percent of flight has gps, otherwise, low speed scenario gets diverged
		if((execution_turn / REPEAT_COUNT) == cc++) {param_speed_factor = .12; strcpy(execution_desc, "0098-1/8x(0.12)_speed");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_speed_factor = .24; strcpy(execution_desc, "0099-quarter(0.24)_speed");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_speed_factor = .49; strcpy(execution_desc, "0101-half(0.49)_speed");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_speed_factor = .5; strcpy(execution_desc, "0101-half_speed");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_speed_factor = 2;  strcpy(execution_desc, "0111-2x_speed");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_speed_factor = 4;  strcpy(execution_desc, "0121-4x_speed");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_speed_factor = 8;  strcpy(execution_desc, "0131-8x_speed");}

		if((execution_turn / REPEAT_COUNT) == cc++) {param_speed_factor = .12; strcpy(execution_desc, "0150-0.12x_speed-noTimeBasedUpdate");}

		if((execution_turn / REPEAT_COUNT) == cc++) {param_speed_factor = .25; strcpy(execution_desc, "0155-0.25x_speed-noTimeBasedUpdate");}

		if((execution_turn / REPEAT_COUNT) == cc++) {param_speed_factor =  .5; strcpy(execution_desc, "0160-half_speed-noTimeBasedUpdate");}

		if((execution_turn / REPEAT_COUNT) == cc++) {param_alt_offset -= 0.5 * MEAN_ALT; param_map_zoom = 18;       strcpy(execution_desc, "0200-half_altitude_2x_zoom");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_alt_offset += (2 - 1) * MEAN_ALT;                        strcpy(execution_desc, "0210-2x_altitude");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_alt_offset += (4 - 1) * MEAN_ALT; param_map_max_dist *= 2; strcpy(execution_desc, "0220-4x_altitude");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_alt_offset += (8 - 1) * MEAN_ALT; param_map_max_dist *= 4; strcpy(execution_desc, "0230-8x_altitude");}

		////
		if((execution_turn / REPEAT_COUNT) == cc++) {param_alt_offset += (2 - 1) * MEAN_ALT; strcpy(execution_desc, "0211-2x_altitude-20m");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_alt_offset += (2 - 1) * MEAN_ALT; strcpy(execution_desc, "0212-2x_altitude-20depth-10m");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_alt_offset += (4 - 1) * MEAN_ALT; param_map_max_dist *= 2; strcpy(execution_desc, "0222-4x_altitude-40depth-10m");}

		//Noise(flight time)///////////////////////////////////
		if((execution_turn / REPEAT_COUNT) == cc++) {param_noise_amp = 1 / 255.;    strcpy(execution_desc, "0300-1x_noise");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_noise_amp = 5 / 255.;    strcpy(execution_desc, "0310-5x_noise");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_noise_amp = 10 * 1 / 255.; strcpy(execution_desc, "0320-10x_noise");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_noise_amp = 15 * 1 / 255.; strcpy(execution_desc, "0330-15x_noise");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_noise_amp = 30 * 1 / 255.; strcpy(execution_desc, "0340-30x_noise");}

		//Known errors/////////////////////////////////////////
		if((execution_turn / REPEAT_COUNT) == cc++) {param_cam_in_ac_pitch = -80.; param_map_max_dist = 11100; strcpy(execution_desc, "0400-param_cam_in_ac_pitch=-80.");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_cam_in_ac_pitch = -70.; param_map_max_dist = 11100; strcpy(execution_desc, "0410-param_cam_in_ac_pitch=-70.");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_cam_in_ac_pitch = -60.; param_map_max_dist = 11100; strcpy(execution_desc, "0420-param_cam_in_ac_pitch=-60.");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_cam_in_ac_pitch = -50.; param_map_max_dist = 11100; strcpy(execution_desc, "0430-param_cam_in_ac_pitch=-50.");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_cam_in_ac_pitch = -40.; param_map_max_dist = 11100; strcpy(execution_desc, "0440-param_cam_in_ac_pitch=-40.");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_cam_in_ac_pitch = -30.; param_map_max_dist = 11100; strcpy(execution_desc, "0450-param_cam_in_ac_pitch=-30.");}

		/*			<staticParams maxLensIterations="50" maxPixError="0.1" k1="-0.390024" k2="0.111124" t1="-0.0012744" t2="-0.0081517" k3="-0.00842763" k4="0.000000" k5="0.000000" k6="0.000000" desc = "calibrated by agisoft-lens" />
				if((execution_turn / REPEAT_COUNT) == cc++) {param_k1 +=-0.4;  strcpy(execution_desc, "0500-param_k1+=-0.4");}
				if((execution_turn / REPEAT_COUNT) == cc++) {param_k2 += 0.1;  strcpy(execution_desc, "0510-param_k2+=0.01");}
				if((execution_turn / REPEAT_COUNT) == cc++) {param_t1 += 0.01; strcpy(execution_desc, "0520-param_t1+=0.01");}
				if((execution_turn / REPEAT_COUNT) == cc++) {param_t2 += 0.01; strcpy(execution_desc, "0530-param_t2+=0.01");}
				if((execution_turn / REPEAT_COUNT) == cc++) {param_k3 += 0.01; strcpy(execution_desc, "0540-param_k3+=0.01");}
		*/

		if((execution_turn / REPEAT_COUNT) == cc++) {param_td += 0.1;   strcpy(execution_desc, "0600-param_td+=0.1");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_td += 0.2;   strcpy(execution_desc, "0610-param_td+=0.2");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_tr += 0.015; strcpy(execution_desc, "0620-param_tr+=0.015");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_tr += 0.030; strcpy(execution_desc, "0630-param_tr+=0.030");}

		//if((execution_turn / REPEAT_COUNT) == cc++) {param_fx *= 2; param_fy *= 2; strcpy(execution_desc, "0650-param_f*=2");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_fx /= 2; param_fy /= 2; strcpy(execution_desc, "0660-param_f/=2");}

		//Unknown errors/////////////////////////////////////////
		if((execution_turn / REPEAT_COUNT) == cc++) {param_cam_in_ac_err_x += 1; strcpy(execution_desc, "0700-param_cam_in_ac_err_x+=1");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_cam_in_ac_err_y += 1; strcpy(execution_desc, "0710-param_cam_in_ac_err_y+=1");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_cam_in_ac_err_z += 1; strcpy(execution_desc, "0720-param_cam_in_ac_err_z+=1");}

		//Use: Qu-Loc-VNED-Bg-Bf-InstE+Aug10
		//if((execution_turn / REPEAT_COUNT) == cc++) {param_cam_in_ac_err_x += 0; strcpy(execution_desc, "0750-param_cam_in_ac_err=0-compensated");}
		//if((execution_turn / REPEAT_COUNT) == cc++) {param_cam_in_ac_err_x += 1; strcpy(execution_desc, "0760-param_cam_in_ac_err_x+=1-compensated");}
		//if((execution_turn / REPEAT_COUNT) == cc++) {param_cam_in_ac_err_y += 1; strcpy(execution_desc, "0770-param_cam_in_ac_err_y+=1-compensated");}
		//if((execution_turn / REPEAT_COUNT) == cc++) {param_cam_in_ac_err_z += 1; strcpy(execution_desc, "0780-param_cam_in_ac_err_z+=1-compensated");}

		/////
		if((execution_turn / REPEAT_COUNT) == cc++) {param_f_err  += 1; strcpy(execution_desc, "0800-param_f_err +=1");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_ox_err += 1; strcpy(execution_desc, "0810-param_ox_err+=1");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_oy_err += 1; strcpy(execution_desc, "0820-param_oy_err+=1");}

		//Use: Qu-Loc-VNED-Bg-Bf-fxy+Aug10
		if((execution_turn / REPEAT_COUNT) == cc++) {param_f_err  += 0; strcpy(execution_desc, "0845-param_f_err +=0-compensated");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_f_err  += 1; strcpy(execution_desc, "0850-param_f_err +=1-compensated");}

		//Use: Qu-Loc-VNED-Bg-Bf-o12+Aug10
		if((execution_turn / REPEAT_COUNT) == cc++) {param_ox_err += 0; strcpy(execution_desc, "0855-param_oxy_err+=0-compensated");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_ox_err += 1; strcpy(execution_desc, "0860-param_ox_err+=1-compensated");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_oy_err += 1; strcpy(execution_desc, "0870-param_oy_err+=1-compensated");}

		if((execution_turn / REPEAT_COUNT) == cc++) {param_k1_err += 0.01 ; strcpy(execution_desc, "0900-param_k1_err+=0.01");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_k2_err += 0.01 ; strcpy(execution_desc, "0910-param_k2_err+=0.01");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_t1_err += 0.001; strcpy(execution_desc, "0920-param_t1_err+=0.001");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_t2_err += 0.001; strcpy(execution_desc, "0930-param_t2_err+=0.001");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_k3_err += 0.01 ; strcpy(execution_desc, "0940-param_k3_err+=0.01");}

		//Use: Qu-Loc-VNED-Bg-Bf-k123+Aug10
		if((execution_turn / REPEAT_COUNT) == cc++) {param_k1_err += 0.0 ; strcpy(execution_desc, "0945-param_k123_err+=0.0-compensated");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_k1_err += 0.01 ; strcpy(execution_desc, "0950-param_k1_err+=0.01-compensated");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_k2_err += 0.01 ; strcpy(execution_desc, "0960-param_k2_err+=0.01-compensated");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_k3_err += 0.01 ; strcpy(execution_desc, "0970-param_k3_err+=0.01-compensated");}

		//Use: Qu-Loc-VNED-Bg-Bf-t12+Aug10
		if((execution_turn / REPEAT_COUNT) == cc++) {param_t1_err += 0.000; strcpy(execution_desc, "0975-param_t12_err+=0.000-compensated");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_t1_err += 0.001; strcpy(execution_desc, "0980-param_t1_err+=0.001-compensated");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_t2_err += 0.001; strcpy(execution_desc, "0990-param_t2_err+=0.001-compensated");}

		/////
		if((execution_turn / REPEAT_COUNT) == cc++) {param_td_err += 0.1  ; strcpy(execution_desc, "1000-param_td_err+=0.1");}
		//Use: Qu-Loc-VNED-Bg-Bf-Td+Aug10
		if((execution_turn / REPEAT_COUNT) == cc++) {param_td_err += 0.0  ; strcpy(execution_desc, "1040-param_td_err+=0.-compensated");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_td_err += 0.1  ; strcpy(execution_desc, "1050-param_td_err+=0.1-compensated");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_td_err += 0.2  ; strcpy(execution_desc, "1060-param_td_err+=0.2-compensated");}

		/////
		if((execution_turn / REPEAT_COUNT) == cc++) {param_tr_err += 0.001; strcpy(execution_desc, "1010-param_tr_err+=0.001");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_tr_err += 0.005; strcpy(execution_desc, "1020-param_tr_err+=0.005");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_tr_err += 0.030; strcpy(execution_desc, "1025-param_tr_err+=0.030");}
		//Use: Qu-Loc-VNED-Bg-Bf-Tr+Aug10
		if((execution_turn / REPEAT_COUNT) == cc++) {param_tr_err += 0.000; strcpy(execution_desc, "1075-param_tr_err+=0.000-compensated");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_tr_err += 0.005; strcpy(execution_desc, "1080-param_tr_err+=0.005-compensated");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_tr_err += 0.030; strcpy(execution_desc, "1090-param_tr_err+=0.030-compensated");}

		//Use: idealCam-90yaw.xml/////
		if((execution_turn / REPEAT_COUNT) == cc++) {param_tr_err += 0.001; strcpy(execution_desc, "1011-param_tr_err+=0.001-xDirSyncLines");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_tr_err += 0.005; strcpy(execution_desc, "1021-param_tr_err+=0.005-xDirSyncLines");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_tr_err += 0.030; strcpy(execution_desc, "1026-param_tr_err+=0.030-xDirSyncLines");}
		//Use: Qu-Loc-VNED-Bg-Bf-Tr+Aug10, Use: idealCam-90yaw.xml/////
		//if((execution_turn / REPEAT_COUNT) == cc++) {param_tr_err += 0.000; strcpy(execution_desc, "1076-param_tr_err+=0.000-xDirSyncLines-compensated");}
		//if((execution_turn / REPEAT_COUNT) == cc++) {param_tr_err += 0.005; strcpy(execution_desc, "1081-param_tr_err+=0.005-xDirSyncLines-compensated");}
		// 		//if((execution_turn / REPEAT_COUNT) == cc++) {param_tr_err += 0.030; strcpy(execution_desc, "1091-param_tr_err+=0.030-xDirSyncLines-compensated");}

		//Camera fps/resolution////////////////////////////////
		if((execution_turn / REPEAT_COUNT) == cc++) {param_camera_fps = 20; strcpy(execution_desc, "1100-param_camera_fps=20");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_camera_fps = 5; strcpy(execution_desc, "1110-param_camera_fps=5");}

		if((execution_turn / REPEAT_COUNT) == cc++) {
			param_resolution_scale *= 2; strcpy(execution_desc, "1200-param_resolution_scale*=2");
			param_width *= param_resolution_scale;
			param_height *= param_resolution_scale;
			param_ox *= param_resolution_scale;
			param_oy *= param_resolution_scale;
			param_fx *= param_resolution_scale;
			param_fy *= param_resolution_scale;
			param_f_err *= param_resolution_scale;
			param_ox_err *= param_resolution_scale;
			param_oy_err *= param_resolution_scale;
		}
		if((execution_turn / REPEAT_COUNT) == cc++) {
			param_resolution_scale /= 2; strcpy(execution_desc, "1210-param_resolution_scale/=2");
			param_width *= param_resolution_scale;
			param_height *= param_resolution_scale;
			param_ox *= param_resolution_scale;
			param_oy *= param_resolution_scale;
			param_fx *= param_resolution_scale;
			param_fy *= param_resolution_scale;
			param_f_err *= param_resolution_scale;
			param_ox_err *= param_resolution_scale;
			param_oy_err *= param_resolution_scale;
		}
#endif

/*        if((execution_turn / REPEAT_COUNT) == cc++) {param_te = 0.030; param_motion_blur_extra_samples = 20; strcpy(execution_desc, "2020-param_te=0.030");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_te = 0.000; strcpy(execution_desc, "2000-param_te=0.000");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_te = 0.005; param_motion_blur_extra_samples = 20; strcpy(execution_desc, "2010-param_te=0.005");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_te = 0.1; param_motion_blur_extra_samples = 50; strcpy(execution_desc, "2030-param_te=0.1");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_te = 0.5; param_motion_blur_extra_samples = 50; strcpy(execution_desc, "2030-param_te=0.5");}
		
        if((execution_turn / REPEAT_COUNT) == cc++) {param_te = 0.030; param_motion_blur_extra_samples = 20; strcpy(execution_desc, "2020-param_te=0.030");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_te = 0.000; strcpy(execution_desc, "2000-param_te=0.000");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_te = 0.005; param_motion_blur_extra_samples = 20; strcpy(execution_desc, "2010-param_te=0.005");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_te = 0.1; param_motion_blur_extra_samples = 50; strcpy(execution_desc, "2030-param_te=0.1");}
		if((execution_turn / REPEAT_COUNT) == cc++) {param_te = 0.5; param_motion_blur_extra_samples = 50; strcpy(execution_desc, "2030-param_te=0.5");}
*/

		//if((execution_turn / REPEAT_COUNT) == cc++) {strcpy(execution_desc, "1205-gyro_noise=(0.15deg/s)/5");}
		//if((execution_turn / REPEAT_COUNT) == cc++) {strcpy(execution_desc, "1206-gyro_noise=(0.15deg/s)*5");}
		//if((execution_turn / REPEAT_COUNT) == cc++) {strcpy(execution_desc, "1211-gyro_bias(0.3deg/s)-nonestim");}
		//if((execution_turn / REPEAT_COUNT) == cc++) {strcpy(execution_desc, "1212-gyro_bias=(0.3deg/s)/5");}
		//if((execution_turn / REPEAT_COUNT) == cc++) {strcpy(execution_desc, "1213-gyro_bias=(0.3deg/s)*5");}
		//if((execution_turn / REPEAT_COUNT) == cc++) {strcpy(execution_desc, "1501-acc_noise=(0.005g)/5");}
		//if((execution_turn / REPEAT_COUNT) == cc++) {strcpy(execution_desc, "1502-acc_noise=(0.005g)*5");}
		//if((execution_turn / REPEAT_COUNT) == cc++) {strcpy(execution_desc, "1511-acc_bias(0.003g)-nonestim");}
		//if((execution_turn / REPEAT_COUNT) == cc++) {strcpy(execution_desc, "1521-acc_bias=(0.003g)/5");}
		//if((execution_turn / REPEAT_COUNT) == cc++) {strcpy(execution_desc, "1522-acc_bias=(0.003g)*5");}
		//if((execution_turn / REPEAT_COUNT) == cc++) {strcpy(execution_desc, "1621-pressure_noise=(0.2m)/5");}
		//if((execution_turn / REPEAT_COUNT) == cc++) {strcpy(execution_desc, "1622-pressure_noise=(0.2m)*5");}
		//if((execution_turn / REPEAT_COUNT) == cc++) {strcpy(execution_desc, "1651-pressure_bias=5m");}
		//if((execution_turn / REPEAT_COUNT) == cc++) {strcpy(execution_desc, "1652-pressure_bias=10m");}

		//if((execution_turn / REPEAT_COUNT) == cc++) {strcpy(execution_desc, "1701-mag_noise=(.1deg)/5");}
        //if((execution_turn / REPEAT_COUNT) == cc++) {strcpy(execution_desc, "1702-mag_noise=(.1deg)*5");}
        
        //if((execution_turn / REPEAT_COUNT) == cc++) {strcpy(execution_desc, "1731-mag_bias=(1deg)-filter:.1deg");}
        //if((execution_turn / REPEAT_COUNT) == cc++) {strcpy(execution_desc, "1732-mag_bias=(1deg)-filter:1deg");}
        //if((execution_turn / REPEAT_COUNT) == cc++) {strcpy(execution_desc, "1733-mag_bias=(1deg)-filter:2deg");}
        //if((execution_turn / REPEAT_COUNT) == cc++) {strcpy(execution_desc, "1734-mag_bias=(1deg)-filter:5deg");}
        //if((execution_turn / REPEAT_COUNT) == cc++) {strcpy(execution_desc, "1735-mag_bias=(1deg)-filter:10deg");}
        
        //if((execution_turn / REPEAT_COUNT) == cc++) {strcpy(execution_desc, "1751-mag_ref_bias=(1deg)-filter:.1deg");}
        //if((execution_turn / REPEAT_COUNT) == cc++) {strcpy(execution_desc, "1752-mag_ref_bias=(1deg)-filter:1deg");}
        //if((execution_turn / REPEAT_COUNT) == cc++) {strcpy(execution_desc, "1753-mag_ref_bias=(1deg)-filter:2deg");}
        //if((execution_turn / REPEAT_COUNT) == cc++) {strcpy(execution_desc, "1754-mag_ref_bias=(1deg)-filter:5deg");}
        //if((execution_turn / REPEAT_COUNT) == cc++) {strcpy(execution_desc, "1755-mag_ref_bias=(1deg)-filter:10deg");}
        /*if((execution_turn / REPEAT_COUNT) == cc++) {param_noise_amp_static1 = 1./255; strcpy(execution_desc, "2001-noise_amp_static1 = 1./255");}

        if((execution_turn / REPEAT_COUNT) == cc++) {param_noise_amp_static1 = 2./255; strcpy(execution_desc, "2002-noise_amp_static1 = 2./255");}

        if((execution_turn / REPEAT_COUNT) == cc++) {param_noise_amp_static1 = 3./255; strcpy(execution_desc, "2003-noise_amp_static1 = 3./255");}

        if((execution_turn / REPEAT_COUNT) == cc++) {param_noise_amp_static1 = 4./255; strcpy(execution_desc, "2004-noise_amp_static1 = 4./255");}

        if((execution_turn / REPEAT_COUNT) == cc++) {param_noise_amp_static1 = 5./255; strcpy(execution_desc, "2005-noise_amp_static1 = 5./255");}

        if((execution_turn / REPEAT_COUNT) == cc++) {param_noise_amp_static1 = 10./255; strcpy(execution_desc, "2006-noise_amp_static1 = 10./255");}


        
        if((execution_turn / REPEAT_COUNT) == cc++) {param_noise_amp_static2 = 1./255; strcpy(execution_desc, "2011-noise_amp_static2 = 1./255");}

        if((execution_turn / REPEAT_COUNT) == cc++) {param_noise_amp_static2 = 2./255; strcpy(execution_desc, "2012-noise_amp_static2 = 2./255");}

        if((execution_turn / REPEAT_COUNT) == cc++) {param_noise_amp_static2 = 3./255; strcpy(execution_desc, "2013-noise_amp_static2 = 3./255");}

        if((execution_turn / REPEAT_COUNT) == cc++) {param_noise_amp_static2 = 4./255; strcpy(execution_desc, "2014-noise_amp_static2 = 4./255");}

        if((execution_turn / REPEAT_COUNT) == cc++) {param_noise_amp_static2 = 5./255; strcpy(execution_desc, "2015-noise_amp_static2 = 5./255");}

        if((execution_turn / REPEAT_COUNT) == cc++) {param_noise_amp_static2 = 10./255; strcpy(execution_desc, "2016-noise_amp_static2 = 10./255");}
*/
        //if((execution_turn / REPEAT_COUNT) == cc++) {strcpy(execution_desc, "1751-mag_ref_bias=(1deg)-filter:.1deg");}
        //if((execution_turn / REPEAT_COUNT) == cc++) {strcpy(execution_desc, "1752-mag_ref_bias=(1deg)-filter:1deg");}
        //if((execution_turn / REPEAT_COUNT) == cc++) {strcpy(execution_desc, "1753-mag_ref_bias=(1deg)-filter:2deg");}
        //if((execution_turn / REPEAT_COUNT) == cc++) {strcpy(execution_desc, "1754-mag_ref_bias=(1deg)-filter:5deg");}
        //if((execution_turn / REPEAT_COUNT) == cc++) {strcpy(execution_desc, "1755-mag_ref_bias=(1deg)-filter:10deg");}
        
        //if((execution_turn / REPEAT_COUNT) == cc++) {strcpy(execution_desc, "1762-mag_ref_bias=(0deg)-filter:10deg");}
        
		if((execution_turn / REPEAT_COUNT) >= cc) exit(110);
		LOGI(TAG, " !!!!!!!!!!!!!!!!!!!Breaking news!!!!!!!!!!!!!!!!!!!!\n Override command(%i): %s\n", execution_turn, execution_desc);
	}
	
	
	//Use instead of copying .xml files to apply changed parameters
	//These paramers cannot be saved inside the rosbag as they are needed at the beginning
	nsrSaveSimParams("finalParameters.xml");
	
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
