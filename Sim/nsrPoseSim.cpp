#ifndef __ANDROID__
#include "/usr/include/time.h"
#endif
#include "./nsrPoseSim.h"

#include "Core/nsrCore.h"
#include "./nsrSimParamReader.h"
#include "View/nsrGeoLib.h"
#include "Matlib/nsrQuat.h"
#include "nsrLinuxKeyBoard2.h"
#include "nsrVisualize.h"

#undef TAG
#define TAG "Cpp:PoseSim:"

#ifdef __cplusplus
extern "C" {
#endif

void nsrPoseMakerStep();

int parsePathCSVLine(char* linebuf, double* ac_time, double* ac_lat, double* ac_lon, double* ac_alt, double* ac_roll, double* ac_pitch, double* ac_yaw)
{
	int linelen, foundlen;
	int foundFrom, foundTo;
	char tmpstr[20];

	linelen = strlen(linebuf);

#define GET_INT_IN_LINE(var) \
	{ \
		foundlen = findIntInLine(linebuf, foundTo+1, linelen-1, &foundFrom, &foundTo); \
		if(foundlen <= 0) return -1; \
		memcpy(tmpstr, &linebuf[foundFrom], foundTo-foundFrom+1); \
		tmpstr[foundTo-foundFrom+1] = '\0'; \
		var = atoi(tmpstr); \
	}
#define GET_FLOAT_IN_LINE(var) \
	{ \
		foundlen = findFloatInLine(linebuf, foundTo+1, linelen-1, &foundFrom, &foundTo); \
		if(foundlen <= 0) return -1; \
		memcpy(tmpstr, &linebuf[foundFrom], foundTo-foundFrom+1); \
		tmpstr[foundTo-foundFrom+1] = '\0'; \
		var = atof(tmpstr); \
	}

	foundTo = -1;
	if(ac_time != NULL) GET_FLOAT_IN_LINE(*ac_time);

	if(ac_lat != NULL) GET_FLOAT_IN_LINE(*ac_lat);
	if(ac_lon != NULL) GET_FLOAT_IN_LINE(*ac_lon);
	if(ac_alt != NULL) GET_FLOAT_IN_LINE(*ac_alt);

	if(ac_alt != NULL) {
		if(param_position_format == POSITION_FMT_XYZ)
			*ac_alt = - *ac_alt;
		*ac_alt += param_alt_offset;
		*ac_alt *= param_world_scale;
	}

	if(ac_lat != NULL && ac_lon != NULL
			&& (param_position_format == POSITION_FMT_XYA || param_position_format == POSITION_FMT_XYZ)) {
		*ac_lat *= param_world_scale;
		*ac_lon *= param_world_scale;
		osg::Vec3d lla = NED2LLA(osg::Vec3d(*ac_lat, *ac_lon, - *ac_alt), //dr,
								 osg::Vec3d(param_map_center_lat, param_map_center_lon, 0.));// lla0);
		*ac_lat = lla.x();
		*ac_lon = lla.y();
	}

	if(ac_roll != NULL) GET_FLOAT_IN_LINE(*ac_roll);
	if(ac_pitch != NULL) GET_FLOAT_IN_LINE(*ac_pitch);
	if(ac_yaw != NULL) GET_FLOAT_IN_LINE(*ac_yaw);

	/*LOGI(TAG, " csv line: %f, %f, %f, %f, %f, %f, %f\n",
	    ac_time != NULL ? *ac_time : NAN,
	    ac_lat  != NULL ? *ac_lat  : NAN,
	    ac_lon  != NULL ? *ac_lon  : NAN,
	    ac_alt  != NULL ? *ac_alt  : NAN,
	    ac_roll != NULL ? *ac_roll : NAN,
	    ac_pitch!= NULL ? *ac_pitch: NAN,
	    ac_yaw  != NULL ? *ac_yaw  : NAN);*/

	return 0;
}

///////////////////////////////////////////////////

static double path_start_time_s = 0;
static double pose_maker_time_s = 0;
static double pause_start_time_s = 0, pause_offset_s = 0;
static double last_t = 0;

static nsrKeyBoard * kb = NULL;
static int last_kb_read_index = 0, last_kb_read_index2 = 0;
static bool posemaker_mode = true;

static FILE* pathCsvFile = NULL, *sensorsCsvFile = NULL;
static SQRTController* pos2rate_cont[6];
static SQRTController* rate2acc_cont[6];
static SQRTController* acc2jerk_cont[6];

static IIRData2 path_filter[6], acc_filter[6];

static double pre_line_time_s = -1;
static double next_line_time_s = -1;

static double pre_ac[6];
static double next_ac[6];

static double ac_real[6] = {-DBL_MAX, -DBL_MAX, -DBL_MAX, -DBL_MAX, -DBL_MAX, -DBL_MAX};

static double ac_real_velocity[6] = {-DBL_MAX, -DBL_MAX, -DBL_MAX, -DBL_MAX, -DBL_MAX, -DBL_MAX};
static double ac_real_acc[6] = {-DBL_MAX, -DBL_MAX, -DBL_MAX, -DBL_MAX, -DBL_MAX, -DBL_MAX};

static double ac_command[6] = {0};

void nsrPoseMakerInit()
{
	int i;

	kb = new nsrKeyBoard();

	pathCsvFile = fopen(param_path_file, "r");	//"r":=read only string
	if(pathCsvFile == NULL) {
		LOGE(TAG, " Path file read failed in address %s! check settings file\n", param_path_file);
	}

	//read start time
	char linebuf[MAX_LINE_LENGTH];

	while(1) {
		if(fgets(linebuf, MAX_LINE_LENGTH, pathCsvFile) == NULL) { //EOF
			fclose(sensorsCsvFile);
			LOGE(TAG, " Exit due to empty Path csv\n");
			nsrExit(); //end program
		}

		if(linebuf[0] == '#') continue; //comment

		if(parsePathCSVLine(linebuf, &pre_line_time_s,
							&pre_ac[0], &pre_ac[1], &pre_ac[2],
							&pre_ac[3], &pre_ac[4], &pre_ac[5]) == 0) {
			path_start_time_s = pre_line_time_s;
			LOGI(TAG, "path_start_time_s: %f\n", path_start_time_s);
			break; //success
		}
	}

	pose_maker_time_s = path_start_time_s;

	////////////////////////////////////////////////////////////
	char sensorsCsvFilePath[MAX_PATH_LENGTH];
	strcpy(sensorsCsvFilePath, globals.savepath);
	strcat(sensorsCsvFilePath, "/log.csv");
	sensorsCsvFile = fopen(sensorsCsvFilePath, "w");	//"w":= write only
	if(sensorsCsvFile == NULL) {
		LOGE(TAG, " Sensors file opening failed in address %s! check settings file\n", sensorsCsvFilePath);
	}

	/////////////////////////////////////////////////////////////
	//a controller used for states to limit accelerations as in real aircraft

#define MAX_ACC (param_max_acc*param_world_scale)
#define MAX_JERK (100*param_world_scale)

#define MAX_ANGULAR_ACC (param_max_ang_acc)
#define MAX_ANGULAR_JERK (600)

#define TAW (0.5)
#define TAW2 (0.5)
#define TAW3 (0.05)

	pos2rate_cont[0] = new SQRTController(0, MAX_ACC * X2LAT(),
										  X2LAT()*param_world_scale * 10); //10m linear distance
	rate2acc_cont[0] = new SQRTController(MAX_ACC * X2LAT(), MAX_JERK * X2LAT(),
										  X2LAT()*param_world_scale * 3);//3m/s linear distance
	acc2jerk_cont[0] = new SQRTController(MAX_JERK * X2LAT(), 0,
										  X2LAT()*param_world_scale * 1);//1m/s2 linear distance

	pos2rate_cont[1] = new SQRTController(0, MAX_ACC * Y2LON(param_map_center_lat),
										  Y2LON(param_map_center_lat)*param_world_scale * 3);
	rate2acc_cont[1] = new SQRTController(MAX_ACC * Y2LON(param_map_center_lat), MAX_JERK * Y2LON(param_map_center_lat),
										  Y2LON(param_map_center_lat)*param_world_scale * 10);
	acc2jerk_cont[1] = new SQRTController(MAX_ACC * Y2LON(param_map_center_lat), 0,
										  Y2LON(param_map_center_lat)*param_world_scale * 1);

	pos2rate_cont[2] = new SQRTController(0, MAX_ACC, param_world_scale * 10);
	rate2acc_cont[2] = new SQRTController(MAX_ACC, MAX_JERK, param_world_scale * 10);
	acc2jerk_cont[2] = new SQRTController(MAX_JERK, 0, param_world_scale * 1);

	pos2rate_cont[3] = new SQRTController(0, MAX_ANGULAR_ACC, 0.01); //0.01deg linear distance
	rate2acc_cont[3] = new SQRTController(MAX_ANGULAR_ACC, MAX_ANGULAR_JERK, 1); //1deg/s linear distance
	acc2jerk_cont[3] = new SQRTController(MAX_ANGULAR_JERK, 0,  1); //1deg/s2 linear distance

	pos2rate_cont[4] = new SQRTController(0, MAX_ANGULAR_ACC, 0.01);
	rate2acc_cont[4] = new SQRTController(MAX_ANGULAR_ACC, MAX_ANGULAR_JERK, 1);
	acc2jerk_cont[4] = new SQRTController(MAX_ANGULAR_JERK, 0,  1);

	pos2rate_cont[5] = new SQRTController(0, MAX_ANGULAR_ACC, 1);
	rate2acc_cont[5] = new SQRTController(MAX_ANGULAR_ACC, MAX_ANGULAR_JERK, 1);
	acc2jerk_cont[5] = new SQRTController(MAX_ANGULAR_JERK, 0,  1);

	for(i = 0; i < 6; i++)
		initIIR2(&path_filter[i], TAW);
	for(i = 0; i < 3; i++)
		initIIR2(&acc_filter[i], TAW2);
	for(i = 3; i < 6; i++)
		initIIR2(&acc_filter[i], TAW3);

	//param_tracker_type = 3;
}

double nsrPoseMakerGetStartTime()
{
	return path_start_time_s;
}

#define SENSOR_DERIVATIVE_STEP_TIME (1./param_control_freq)

void nsrPoseMakerLoop(double time_barrier)
{
	//Try to reach time_barrier(if not reached yet)
	while(pose_maker_time_s <= time_barrier * param_speed_factor + SENSOR_DERIVATIVE_STEP_TIME) {
		//LOGI(TAG, "[[%f < %f]]\n", pose_maker_time_s , time_barrier + SENSOR_DERIVATIVE_STEP_TIME);

		nsrPoseMakerStep(); //pose_maker_time_s is input

		//Log sensor data
		/*nsrPoseMakerExtract((pose_maker_time_s - SENSOR_DERIVATIVE_STEP_TIME) / param_speed_factor, 1,
							NULL, NULL, NULL, NULL, NULL, NULL,
							NULL, NULL, NULL);*/
		pose_maker_time_s += SENSOR_DERIVATIVE_STEP_TIME;
	}
}

void nsrPoseMakerStep()
{
	int i, err;
	char linebuf[MAX_LINE_LENGTH];
	double ac[6];

	uint16_t ch;
	ch = kb->getch(last_kb_read_index);
	if(ch == KEY_M) {
		posemaker_mode = !posemaker_mode;
		printf("Pose Maker Mode Changed!\n");
		for(i = 0; i < 6; i++)
			ac_command[i] = ac_real[i];
		ac_command[3] = 0;	//roll
		ac_command[4] = 0;	//pitch
		pause_start_time_s = pose_maker_time_s - pause_offset_s; //pause_start_time_s is not rezl_start_time, just for calculating pause_offset_s
		last_t = pose_maker_time_s;
	}

	if(posemaker_mode) {
		//if needed, read a line in the future for interpolation
		while(pose_maker_time_s - pause_offset_s > next_line_time_s
				|| (pre_line_time_s < 0 || next_line_time_s < 0)//ensure reading two lines at first step
			 ) {
			//read a line, exit if unsuccessfull //don't use fscanf(), it can leave your file pointer in an unknown location on failure
			if(fgets(linebuf, MAX_LINE_LENGTH, pathCsvFile) == NULL
					|| linebuf[0] == '#') {
				fclose(sensorsCsvFile);
				LOGE(TAG, " Exit due to end of Path csv\n");
				nsrPoseMakerClose();
				nsrExit(); //end program
			}

			if(next_line_time_s >= 0) {
				pre_line_time_s = next_line_time_s;
				for(i = 0; i < 6; i++) pre_ac[i] = next_ac[i];
			}

			err = parsePathCSVLine(linebuf, &next_line_time_s, &next_ac[0], &next_ac[1], &next_ac[2], &next_ac[3], &next_ac[4], &next_ac[5]);
		}

		//interpolate commanded pose
		for(i = 0; i < 6; i++)
			ac_command[i] = rescale(pose_maker_time_s - pause_offset_s, pre_line_time_s, next_line_time_s, pre_ac[i], next_ac[i]);
	} else { //Pause mode
		double dt = pose_maker_time_s - last_t;
		//double* ac_lat, double* ac_lon, double* ac_alt, double* ac_roll, double* ac_pitch, double* ac_yaw

		//Keep button pressed, pressed key does not getch continuously
		static double last_key_time = 0;
		static int last_ch = 0;
		ch = kb->getch(last_kb_read_index2);
		if(ch != 0) {
			last_ch = ch;
			last_key_time = pose_maker_time_s;
		}
		if(ch == 0 && pose_maker_time_s < last_key_time + 0.1) //Keep button pressed
			ch = last_ch;
        
        double CY = cos(ac_command[5]*M_PI/180.);
        double SY = sin(ac_command[5]*M_PI/180.);

#define SPEED 30. //Manual Speed in m/s
        //Move in heading direction
		if(ch == KEY_UP) {
			ac_command[0] += SPEED * CY * X2LAT() * dt; //lat(deg)
			ac_command[1] += SPEED * SY * Y2LON(ac_command[0]) * dt; //lat(deg)
        }
		if(ch == KEY_DOWN) {
			ac_command[0] -= SPEED * CY * X2LAT() * dt; //lat(deg)
			ac_command[1] -= SPEED * SY * Y2LON(ac_command[0]) * dt; //lat(deg)
        }

		if(ch == KEY_EQUAL) {
            ac_command[0] -= SPEED * SY * X2LAT() * dt; //lat(deg)
			ac_command[1] += SPEED * CY * Y2LON(ac_command[0]) * dt; //lat(deg)
        }
		if(ch == KEY_MINUS) {
            ac_command[0] += SPEED * SY * X2LAT() * dt; //lat(deg)
			ac_command[1] -= SPEED * CY * Y2LON(ac_command[0]) * dt; //lat(deg)
        }

		if(ch == KEY_PAGEUP)
			ac_command[2] += +10 * dt; //alt(m)
		if(ch == KEY_PAGEDOWN)
			ac_command[2] += -10 * dt; //alt(m)

		if(ch == KEY_RIGHT) //==KEY_PLUS
			ac_command[5] += +30 * dt; //yaw(deg)
		if(ch == KEY_LEFT)
			ac_command[5] -= +30 * dt; //yaw(deg)

		//printf("((ch:%i, deg:%f,dt:%f, t:%f, dy:%f\n", ch, ac_command[5], dt, pose_maker_time_s, 30*dt);

		//if(ch != 0) printf("ch:0x%x(%i)\n", ch, ch);
		/////////////////////////////////////////////

		pause_offset_s = pose_maker_time_s - pause_start_time_s;
		last_t = pose_maker_time_s;
		//Do nothing
	}

	//correct
	//ac_command[5]+=2; //???? convert to real north
	//ac_command[2]-=47.5;// - N(undulation) to convert WGS-84 to EGM96 msl

	//LOGI(TAG, " p:%f, %f, %f, %f, %f, %f, %f\n", pose_maker_time_s, pre_ac[0], pre_ac[1], pre_ac[2], pre_ac[3], pre_ac[4], pre_ac[5]);
	//LOGI(TAG, " n:%f, %f, %f, %f, %f, %f, %f\n", pose_maker_time_s, next_ac[0], next_ac[1], next_ac[2], next_ac[3], next_ac[4], next_ac[5]);
	//LOGI(TAG, " s:%f, %f, %f, %f, %f, %f, %f\n", pose_maker_time_s, ac_command[0], ac_command[1], ac_command[2], ac_command[3], ac_command[4], ac_command[5]);

	//Initialize starting pose
	if(ac_real[0] < -1000) {
		for(i = 0; i < 6; i++)
			ac_real[i] = ac_command[i];

		for(i = 0; i < 6; i++)
			ac_real_velocity[i] = 0;

		for(i = 0; i < 6; i++)
			ac_real_acc[i] = 0;
	}
	
    //correct yaw command to follow real yaw with maximum 180 degrees
    //printf("pre_cmd:%f", ac_command[5]);
	while(ac_command[5] - ac_real[5] > 180.)
        ac_command[5]-=360.;
    while(ac_command[5] - ac_real[5] < -180.)
        ac_command[5]+=360.;
    //printf("ac_real:%f, post_cmd:%f", ac_real[5], ac_command[5]);

	//2 x controller loops to track linear interpolation and input/output 1st order filter for continuous acceleration///////
	if(param_tracker_type == TRACKER_TYPE_2xSQRT_INOUT_FILTER) {
		for(i = 0; i < 6; i++) {
			pos2rate_cont[i]->control_step(pose_maker_time_s, ac_real[i],
										   doIIRfilter2(&path_filter[i], ac_command[i], pose_maker_time_s));
			rate2acc_cont[i]->control_step(pose_maker_time_s, ac_real_velocity[i], pos2rate_cont[i]->goal_velocity);
			ac_real_velocity[i] += rate2acc_cont[i]->dt
								   * doIIRfilter2(&acc_filter[i], rate2acc_cont[i]->goal_velocity, pose_maker_time_s);
			ac[i] = ac_real[i] += pos2rate_cont[i]->dt * ac_real_velocity[i];
		}

        //printf(":::t:%f, cmd yaw:%f, yaw:%f, yaw vel:%f, goal_vel:%f, goal_acc:%f\n", pose_maker_time_s, ac_command[5], ac_real[5], ac_real_velocity[5], pos2rate_cont[5]->goal_velocity, rate2acc_cont[5]->goal_velocity);
	}

	//2 x controller loops to track linear interpolation and output 1st order filter for continuous acceleration///////
	//just minor filter can be used as it causes oscilation
	if(param_tracker_type == TRACKER_TYPE_2xSQRT_OUTPUT_FILTER) {
		for(i = 0; i < 6; i++) {
			pos2rate_cont[i]->control_step(pose_maker_time_s, ac_real[i], ac_command[i]);
			rate2acc_cont[i]->control_step(pose_maker_time_s, ac_real_velocity[i], pos2rate_cont[i]->goal_velocity);
			ac_real_acc[i] = doIIRfilter2(&acc_filter[i], rate2acc_cont[i]->goal_velocity, pose_maker_time_s);
			ac_real_velocity[i] += rate2acc_cont[i]->dt * ac_real_acc[i];
			ac[i] = ac_real[i] += pos2rate_cont[i]->dt * ac_real_velocity[i];
		}
	}

	//3 x controller loops to track linear interpolation with continuous acceleration///////
	//oscilates and diverges
	if(param_tracker_type == TRACKER_TYPE_3xSQRT) {
		for(i = 0; i < 6; i++) {
			pos2rate_cont[i]->control_step(pose_maker_time_s, ac_real[i], ac_command[i]);
			rate2acc_cont[i]->control_step(pose_maker_time_s, ac_real_velocity[i], pos2rate_cont[i]->goal_velocity);
			acc2jerk_cont[i]->control_step(pose_maker_time_s, ac_real_acc[i], rate2acc_cont[i]->goal_velocity, 0);
			ac_real_acc[i] += acc2jerk_cont[i]->dt * acc2jerk_cont[i]->goal_velocity;
			ac_real_velocity[i] += rate2acc_cont[i]->dt * ac_real_acc[i];
			ac[i] = ac_real[i] += pos2rate_cont[i]->dt * ac_real_velocity[i];
		}

	}

	//2 x controller loops to track linear interpolation///////
	if(param_tracker_type == TRACKER_TYPE_2xSQRT) {
		for(i = 0; i < 6; i++) {
			pos2rate_cont[i]->control_step(pose_maker_time_s, ac_real[i], ac_command[i]);
			rate2acc_cont[i]->control_step(pose_maker_time_s, ac_real_velocity[i], pos2rate_cont[i]->goal_velocity);
			ac_real_velocity[i] += rate2acc_cont[i]->dt * rate2acc_cont[i]->goal_velocity;
			ac[i] = ac_real[i] += pos2rate_cont[i]->dt * ac_real_velocity[i];
		}

		//if(pose_maker_time_s > 19.75){// && pose_maker_time_s < 20.25) {
		//    printf("t:%f, preA:%f, NexA:%f, ComA:%f, alt:%f, rV:%f, gV:%f, gAc:%f\n", pose_maker_time_s,
		//        (pre_ac[0]-pre_ac[0])/X2LAT(), (next_ac[0]-pre_ac[0])/X2LAT(), (ac_command[0]-pre_ac[0])/X2LAT(), (ac_real[0]-pre_ac[0])/X2LAT(),
		//           ac_real_velocity[0]/X2LAT(), pos2rate_cont[0]->goal_velocity/X2LAT(), rate2acc_cont[0]->goal_velocity/X2LAT());
		//	exit(10);
		// }
	}

	//1 controller loop to track linear interpolation///////
	if(param_tracker_type == TRACKER_TYPE_1xSQRT) {
		for(i = 0; i < 6; i++) {
			pos2rate_cont[i]->control_step(pose_maker_time_s, ac_real[i], ac_command[i]);
			ac[i] = ac_real[i] += pos2rate_cont[i]->dt * pos2rate_cont[i]->goal_velocity;
		}
	}

	//linear interpolation//////////
	if(param_tracker_type == TRACKER_TYPE_LINEAR) {
		for(i = 0; i < 6; i++)
			ac[i] = ac_command[i];
	}

	//stepwise(non-continuous)////////
	if(param_tracker_type == TRACKER_TYPE_DESCRETE) {
		for(i = 0; i < 6; i++)
			ac[i]   = pre_ac[i];
	}

	//debug////////
#if 0
	ac[i]   = 0;
	ac[1]   = 0;
	ac[2]   = 3000;
	ac[3]  = 0;
	ac[4] = 0;
	ac[5]   = 0;
#endif

	//LOGI(TAG, " pose kernel:%f, %f, %f, %f, %f, %f, %f\n", pose_maker_time_s, ac[0], ac[1], ac[2], ac[3], ac[4], ac[5]);

	_LOCKCPP(Z_lock,);
	cbPush(mZB, n.Z.GROUND_TRUTH_LLA + 0, ac[0], pose_maker_time_s / param_speed_factor);
	cbPush(mZB, n.Z.GROUND_TRUTH_LLA + 1, ac[1], pose_maker_time_s / param_speed_factor);
	cbPush(mZB, n.Z.GROUND_TRUTH_LLA + 2, ac[2], pose_maker_time_s / param_speed_factor);
	cbPush(mZB, n.Z.GROUND_TRUTH_EU + 0, ac[3]*M_PI / 180, pose_maker_time_s / param_speed_factor); //in rad
	cbPush(mZB, n.Z.GROUND_TRUTH_EU + 1, ac[4]*M_PI / 180, pose_maker_time_s / param_speed_factor);
	cbPush(mZB, n.Z.GROUND_TRUTH_EU + 2, ac[5]*M_PI / 180, pose_maker_time_s / param_speed_factor);
	_UNLOCKCPP(Z_lock,);
}

void nsrPoseMakerExtract(double time_s, int do_log,
						 osg::Vec3d* _lla,  //

						 osg::Vec3d* _v_ac, //wrt. ned, in ned
						 osg::Vec3d* _a_ac, //wrt. ned, in ned
						 osg::Vec3d* _acc_ac,//accelerometer output
						 osg::Quat* _acInNedQu, //ac/ned
						 osg::Vec3d* _w_ac, //ac/ned, in ac

						 osg::Vec3d* _v_cam, //wrt. ned, in cam
						 osg::Quat* _camInNedQu,//cam/ned
						 osg::Vec3d* _w_cam) //cam/ac, in cam
{
	int ERRZ;
	double interp_time_s, ac_x, ac_y, ac_z;
	double ac_lat_n, ac_lon_n, ac_alt_n, ac_lat, ac_lon, ac_alt, ac_lat_p, ac_lon_p, ac_alt_p;
	double ac_roll_n, ac_pitch_n, ac_yaw_n, ac_roll, ac_pitch, ac_yaw, ac_roll_p, ac_pitch_p, ac_yaw_p;
	osg::Quat acInNedQu_n, acInNedQu, acInNedQu_p,
		camInAcQu, camInNedQu;
	osg::Vec3d lla_n, lla, lla_p,
		v_ac, a_ac, acc_ac, w_ac, w_cam_in_ac, xyz;
	osg::Vec3d w_cam, v_cam;

	///AC//////////
	_LOCKCPP(Z_lock,);
	mZB->ERR = 0;

	if(_v_ac != NULL || _a_ac != NULL || _acc_ac != NULL || _v_cam != NULL || do_log == 1) {
		//interp_time_s = time_s - SENSOR_DERIVATIVE_STEP_TIME;
		interp_time_s = time_s - 2 * SENSOR_DERIVATIVE_STEP_TIME;

		ac_lat_n = cbInterpolate(mZB, n.Z.GROUND_TRUTH_LLA + 0, interp_time_s);
		ac_lon_n = cbInterpolate(mZB, n.Z.GROUND_TRUTH_LLA + 1, interp_time_s);
		ac_alt_n = cbInterpolate(mZB, n.Z.GROUND_TRUTH_LLA + 2, interp_time_s);

		//interp_time_s = time_s + SENSOR_DERIVATIVE_STEP_TIME;
		interp_time_s = time_s;
		ac_lat_p = cbInterpolate(mZB, n.Z.GROUND_TRUTH_LLA + 0, interp_time_s);
		ac_lon_p = cbInterpolate(mZB, n.Z.GROUND_TRUTH_LLA + 1, interp_time_s);
		ac_alt_p = cbInterpolate(mZB, n.Z.GROUND_TRUTH_LLA + 2, interp_time_s);

		lla_n = osg::Vec3d(ac_lat_n, ac_lon_n, ac_alt_n);
		lla_p = osg::Vec3d(ac_lat_p, ac_lon_p, ac_alt_p);
	}

	if(_lla != NULL || _a_ac != NULL || _acc_ac != NULL || do_log == 1) {
		//interp_time_s = time_s;
		interp_time_s = time_s - SENSOR_DERIVATIVE_STEP_TIME;
		ac_lat = cbInterpolate(mZB, n.Z.GROUND_TRUTH_LLA + 0, interp_time_s);
		ac_lon = cbInterpolate(mZB, n.Z.GROUND_TRUTH_LLA + 1, interp_time_s);
		ac_alt = cbInterpolate(mZB, n.Z.GROUND_TRUTH_LLA + 2, interp_time_s);

		lla   = osg::Vec3d(ac_lat, ac_lon, ac_alt);
		if(_lla != NULL) *_lla = lla;
	}

	if(_v_ac != NULL || _v_cam != NULL || do_log == 1) {
		v_ac  = calcVfromLLA(lla_n, lla_p, 2 * SENSOR_DERIVATIVE_STEP_TIME); //wrt. ned, in ned
		if(_v_ac != NULL) *_v_ac = v_ac;
	}

	if(_a_ac != NULL || _acc_ac != NULL || do_log == 1) {
		a_ac  = calcAfromLLA(lla_n, lla, lla_p, SENSOR_DERIVATIVE_STEP_TIME); //wrt. ned, in ned
		if(_a_ac != NULL) *_a_ac = a_ac;

		/*static double vv=0, aa=0, vv2=0;
		printf("t:%f, (%f, %f, %f)", time_s, -lla_n.z(), -lla.z(), -lla_p.z());
		printf("v:%f, v2:%f, a:%f\n", vv=(-lla_p.z() - -lla.z())/SENSOR_DERIVATIVE_STEP_TIME, vv2+=aa*SENSOR_DERIVATIVE_STEP_TIME, aa=(-lla_n.z() +2*lla.z() -lla_p.z())/SENSOR_DERIVATIVE_STEP_TIME/SENSOR_DERIVATIVE_STEP_TIME);*/
	}

	if(_w_ac != NULL || _w_cam != NULL || _camInNedQu != NULL || _v_cam != NULL || do_log == 1) {
		//interp_time_s = time_s - SENSOR_DERIVATIVE_STEP_TIME;
		interp_time_s = time_s - 2 * SENSOR_DERIVATIVE_STEP_TIME;
		ac_roll_n  = cbInterpolate(mZB, n.Z.GROUND_TRUTH_EU + 0, interp_time_s); //in rad
		ac_pitch_n = cbInterpolate(mZB, n.Z.GROUND_TRUTH_EU + 1, interp_time_s);
		ac_yaw_n   = cbInterpolate(mZB, n.Z.GROUND_TRUTH_EU + 2, interp_time_s);

		//interp_time_s = time_s + SENSOR_DERIVATIVE_STEP_TIME;
		interp_time_s = time_s;
		ac_roll_p  = cbInterpolate(mZB, n.Z.GROUND_TRUTH_EU + 0, interp_time_s); //in rad
		ac_pitch_p = cbInterpolate(mZB, n.Z.GROUND_TRUTH_EU + 1, interp_time_s);
		ac_yaw_p   = cbInterpolate(mZB, n.Z.GROUND_TRUTH_EU + 2, interp_time_s);

		acInNedQu_n = osg::Quat(ac_roll_n, osg::X_AXIS)
					  * osg::Quat(ac_pitch_n, osg::Y_AXIS)
					  * osg::Quat(ac_yaw_n, osg::Z_AXIS);

		acInNedQu_p = osg::Quat(ac_roll_p, osg::X_AXIS)
					  * osg::Quat(ac_pitch_p, osg::Y_AXIS)
					  * osg::Quat(ac_yaw_p, osg::Z_AXIS);
	}

	if(_acInNedQu != NULL || _acc_ac != NULL || _camInNedQu != NULL || _v_cam != NULL || do_log == 1) {
		//interp_time_s = time_s;
		interp_time_s = time_s - SENSOR_DERIVATIVE_STEP_TIME;
		ac_roll  = cbInterpolate(mZB, n.Z.GROUND_TRUTH_EU + 0, interp_time_s); //in rad
		ac_pitch = cbInterpolate(mZB, n.Z.GROUND_TRUTH_EU + 1, interp_time_s);
		ac_yaw   = cbInterpolate(mZB, n.Z.GROUND_TRUTH_EU + 2, interp_time_s);

		acInNedQu = osg::Quat(ac_roll, osg::X_AXIS)
					* osg::Quat(ac_pitch, osg::Y_AXIS)
					* osg::Quat(ac_yaw, osg::Z_AXIS);
		if(_acInNedQu != NULL) *_acInNedQu = acInNedQu;
	}

	ERRZ = mZB->ERR;

	int buffer_full = mZB->Wrtn[n.Z.GROUND_TRUTH_EU - 1] >= mZB->no_Depth ? 1 : 0;
	double lastTime = cbLastTime(mZB, n.Z.GROUND_TRUTH_EU);

	_UNLOCKCPP(Z_lock,);

	if(ERRZ == NUMBUFFER_ERR_INTERPOLATE_TIMEUNREACHED)
		LOGW(TAG, "WARNING: Interpolate time unreached, increase camera time delay(%f > %f)!...\n", time_s + SENSOR_DERIVATIVE_STEP_TIME, lastTime);

	if(ERRZ == NUMBUFFER_ERR_INTERPOLATE_TIMEPASSED && buffer_full)
		LOGW(TAG, "WARNING: Interpolate time passed, increase buffer depth or decrease step size!...\n");

	if(_w_ac != NULL || _w_cam != NULL || do_log == 1) {
		w_ac = calcWfromQu(acInNedQu_n, acInNedQu_p, 2 * SENSOR_DERIVATIVE_STEP_TIME); //ac/ned, in ac
		if(_w_ac != NULL) *_w_ac = w_ac;
	}

	if(_acc_ac != NULL || do_log == 1) {
		/*nsr::Quat qu; qu.set(acInNedQu.x(), acInNedQu.y(), acInNedQu.z(), acInNedQu.w());
		Matrice CI2B;
		qu.getRotMat(CI2B);
		Matrice aac = CI2B*colmat(a_ac.x(), a_ac.y(), a_ac.z() - 9.81);
		//acc_ac.x()=aac.x(); acc_ac.y()=aac.y(); acc_ac.z()=aac.z();*/

		acc_ac = osg::Matrixd::rotate(acInNedQu) * (a_ac - osg::Vec3d(0, 0, 9.81)); //accelerometer
		//printf("(%f, %f, %f)(%f, %f, %f)\n", acc_ac.x(), acc_ac.y(), acc_ac.z(), aac.x(), aac.y(), aac.z());
		if(_acc_ac != NULL) *_acc_ac = acc_ac;
	}

	///CAM/////////
	if(_w_cam != NULL || _camInNedQu != NULL || _v_cam != NULL || do_log == 1) {
		camInAcQu =
			//2)Second putting camera front (z direction) on x direction
			osg::Quat(osg::inDegrees(90.), osg::X_AXIS)
			* osg::Quat(osg::inDegrees(90.), osg::Z_AXIS)
			//1)First normal euler rotations, to put x direction on desired vector, ZYX == PanTiltRoll
			* osg::Quat(osg::inDegrees(param_cam_in_ac_roll), osg::X_AXIS)
			* osg::Quat(osg::inDegrees(param_cam_in_ac_pitch), osg::Y_AXIS)
			* osg::Quat(osg::inDegrees(param_cam_in_ac_yaw), osg::Z_AXIS)
			//0)Little installation error on root mount
			* osg::Quat(osg::inDegrees(param_cam_in_ac_err_x), osg::X_AXIS)
			* osg::Quat(osg::inDegrees(param_cam_in_ac_err_y), osg::Y_AXIS)
			* osg::Quat(osg::inDegrees(param_cam_in_ac_err_z), osg::Z_AXIS);
	}

	if(_camInNedQu != NULL || _v_cam != NULL || do_log == 1) {
		camInNedQu = camInAcQu * acInNedQu;
		if(_camInNedQu != NULL) *_camInNedQu = camInNedQu;
		//LOGI(TAG, " q:%f, %f, %f, %f, %f\n", time_s, camInNedQu.x(), camInNedQu.y(), camInNedQu.z(), camInNedQu.w());
	}

	if(_v_cam != NULL || do_log == 1) {
		v_cam = osg::Matrixd::rotate(camInNedQu) * v_ac; //wrt. ned, in cam
		if(_v_cam != NULL) *_v_cam = v_cam;
		//LOGI(TAG, " v:%f, %f, %f, %f\n", time_s, v_cam.x(), v_cam.y(), v_cam.z());
	}
	if(_w_cam != NULL || do_log == 1) {
		w_cam_in_ac = osg::Vec3d(0., 0., 0.); //cam/ac, in cam
		w_cam = osg::Matrixd::rotate(camInAcQu) * w_ac + w_cam_in_ac; //cam/ned, in cam
		if(_w_cam != NULL) *_w_cam = w_cam;
	}

	///////////

	if(do_log) {
		static bool title_printed = false;
		if(title_printed == false) {
			fprintf(sensorsCsvFile, "time_s,lla,,,xyz,,,v_in_ac,,,v_in_cam,,,a_in_ac,,,acc,,,"
					"eu_ac,,,qu_ac,,,,qu_cam,,,,w_ac,,,w_cam,,,\n");
			title_printed = true;
		}

		xyz = LLA2NED(osg::Vec3d(ac_lat, ac_lon, ac_alt), //dr
					  osg::Vec3d(param_map_center_lat, param_map_center_lon, 0.));// lla0
		ac_x = xyz.x();
		ac_y = xyz.y();
		ac_z = xyz.z();

		fprintf(sensorsCsvFile,
				" %f,"
				" %f, %f, %f," //lla
				" %f, %f, %f," //xyz
				" %f, %f, %f," //v in ac
				" %f, %f, %f," //v in cam
				" %f, %f, %f," //a in ac
				" %f, %f, %f," //acc
				" %f, %f, %f," //eu ac
				" %f, %f, %f, %f," //qu ac
				" %f, %f, %f, %f," //qu cam
				" %f, %f, %f," //pqr ac
				" %f, %f, %f\n", //pqr cam
				time_s,
				ac_lat, ac_lon, ac_alt,
				ac_x, ac_y, ac_z,
				v_ac.x(), v_ac.y(), v_ac.z(),
				v_cam.x(), v_cam.y(), v_cam.z(),
				a_ac.x(), a_ac.y(), a_ac.z(),
				acc_ac.x(), acc_ac.y(), acc_ac.z(),
				ac_roll * 180 / M_PI, ac_pitch * 180 / M_PI, ac_yaw * 180 / M_PI,
				acInNedQu.x(), acInNedQu.y(), acInNedQu.z(), acInNedQu.w(),
				camInNedQu.x(), camInNedQu.y(), camInNedQu.z(), camInNedQu.w(),
				w_ac.x() * 180 / M_PI, w_ac.y() * 180 / M_PI, w_ac.z() * 180 / M_PI,
				w_cam.x() * 180 / M_PI, w_cam.y() * 180 / M_PI, w_cam.z() * 180 / M_PI
			   );
	}
}

void nsrPoseMakerClose()
{
	if(sensorsCsvFile != NULL) fclose(sensorsCsvFile);
	if(pathCsvFile != NULL) fclose(pathCsvFile);
	if(kb != NULL) delete kb;
}

#ifdef __cplusplus
}
#endif
