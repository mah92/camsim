#include "./nsrFilters.h"
#include "./nsrUtility.h"

#include <math.h>
#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif

void initIIR(IIRData* fD)
{
	fD->inited = 0;
	fD->data = 0;
}

ffloat doIIRfilter(IIRData* fD, ffloat data, ffloat weight)
{
	if(fD->inited == 0) {
		fD->inited = 1;
		fD->data = data;
		return data;
	}

	fD->data = weight * data + (1 - weight) * fD->data;
	return fD->data;
}

//////////////////////////////////////////////////

void initIIR2(IIRData2* fD, ffloat taw)
{
	fD->inited = 0;
	fD->data = 0;
	fD->pre_time_s = 0;
	fD->taw = taw;
}

ffloat doIIRfilter2(IIRData2* fD, ffloat data, ffloat time_s)
{
	ffloat weight, dt;
	if(fD->inited == 0) {
		fD->inited = 1;
		fD->data = data;
		fD->pre_time_s = time_s;
		return data;
	}

	dt = time_s - fD->pre_time_s;
    
    if(fD->taw <= 0.) { //filter disabled
        fD->data = data; 
        fD->pre_time_s = time_s;
        return fD->data;
    }

	weight = dt / (fD->taw + dt);
	fD->data = weight * data + (1 - weight) * fD->data;

	fD->pre_time_s = time_s;

	return fD->data;
}

/////////////////////////////////////////////////////

void initFIR(FIRData* fD, int datalen)
{
	int i;
	fD->turn = 0;
	fD->data_len = datalen;
	fD->data = (ffloat*)malloc(datalen * sizeof(ffloat));
	fD->weight = (ffloat*)malloc(datalen * sizeof(ffloat));
	for(i = 0; i < datalen; i++) {
		fD->data[i] = 0;
		fD->weight[i] = 0;
	}
}

ffloat doFIRfilter(FIRData* fD, ffloat data, float weight)
{
	int i;
	ffloat sum = 0;
	float weight_sum = 0;

	fD->data[fD->turn] = data;
	fD->weight[fD->turn] = weight;

	fD->turn++;
	if(fD->turn >= fD->data_len)
		fD->turn = 0;

	for(i = 0; i < fD->data_len; i++) {
		sum += fD->data[i] * fD->weight[i];
		weight_sum += fD->weight[i];
	}
	sum /= weight_sum;
	return sum;
}

void closeFIRFilter(FIRData* fD)
{
	free(fD->data);
}

/////////////////////////////////////////////////////

void initTotalAverager(TotalAveragerData* fD)
{
	fD->turn = 0;
	fD->data = 0;
}

ffloat doTotalAverage(TotalAveragerData* fD, ffloat data)
{
	float w;
	fD->turn++;
	w = 1. / fD->turn;
	fD->data = data * w + fD->data * (1 - w);
	return fD->data;
}

/////////////////////////////////////////////////

void initTickRate(TickRateData* fD)
{
	fD->inited = 0;
	fD->pre_t = 0;
#if tickRateFilterMethod == 0
	initIIR(&fD->IIR[0]);
	initIIR(&fD->IIR[1]);
#endif
#if tickRateFilterMethod == 1
	initFIR(&fD->FIR[0], 20);
	initFIR(&fD->FIR[1], 20);
#endif
#if tickRateFilterMethod == 2
	initTotalAverager(&fD->TAF[0]);
	initTotalAverager(&fD->TAF[1]);
#endif
}

ffloat tickRate(TickRateData* fD, int ticks)
{
	float dtmean, ticksmean, rate;
	double t, dt;

	t = myTime();

	//first time
	if(fD->inited == 0) {
		fD->inited = 1;
		fD->pre_t = t;

		return 0;
	}
	dt = t - fD->pre_t;

	if(dt <= 0) //divide by zero prevention
		dt = 0.01;

#if tickRateFilterMethod == 0
	dtmean = doIIRfilter(&fD->IIR[0], dt, 0.2);
	ticksmean = doIIRfilter(&fD->IIR[1], (float)(ticks), 0.2);
#endif
#if tickRateFilterMethod == 1
	dtmean = doFIRfilter(&fD->FIR[0], dt, 1);
	ticksmean = doFIRfilter(&fD->FIR[1], (float)(ticks), 1);
#endif
#if tickRateFilterMethod == 2
	dtmean = doTotalAverage(&fD->TAF[0], dt);
	ticksmean = doTotalAverage(&fD->TAF[1], (float)(ticks));
#endif
	rate = ticksmean / dtmean; //=(tickssum/dtsum)
	//printf("dtmean:%f//ticksmean:%f//tickrate:%f\n", dtmean, ticksmean, rate);
	fD->pre_t = t;
	return rate;
}

//////////////////////////////////////////////////////
//linear_dist is half the size of total linear region
//max_velocity <= 0 means disabled
//max_acceleration <= 0 means disabled
//You can not disable both max_velocity, max_acceleration
SQRTController::SQRTController(double _max_velocity, double _max_acceleration, double _linear_dist) : pre_time(-1), pre_value(0), pre_target(0)
{
	//properties
	max_velocity = _max_velocity;
	max_acceleration = _max_acceleration;
	linear_dist = _linear_dist;

    assert(max_acceleration > 0 || max_velocity > 0);
    
	if(max_acceleration > 0) {//if max_acceleration is not disabled
		linear_max_command = sqrt(2 * max_acceleration * linear_dist / 2.);
		if(max_velocity > 0) {//if max_velocity is not disabled
			if(max_velocity < linear_max_command) {//if acceleration is not a limiting factor, control becomes just linear(with saturation)
				linear_max_command = max_velocity;
				max_acceleration = -1; //disable acceleration check
			}
		}
	} else { //max_acceleration is disabled
		linear_max_command = max_velocity;
	}
}

//value & vel are outer defined static input-output
double SQRTController::control_step(double time_s, double value, double target, double target_vel)
{
	double target_velocity;
	if(pre_time < 0) {//first time
		pre_time = time_s;
		pre_value = value;
		pre_target = target;

		//zero command
		goal_velocity = 0;
		return 0;
	}

	dt = time_s - pre_time;

	if(dt <= 1e-4) //last control command
		return goal_velocity;

	if(target_vel > 1e50) //no data for last entry causes DBL_MAX, and automatic feedforward
		target_velocity = (target - pre_target) / dt;
	else
		target_velocity = target_vel;

	pre_time = time_s;
	pre_value = value;
	pre_target = target;

	//sqrt controller
	//max relative speed with target so that we reach target (with zero relative velocity) with max braking acceleration
	double goal_relative_velocity;
	if(value < target - linear_dist) {
		if(max_acceleration > 0)  //if max_acceleration is not disabled
			goal_relative_velocity = sqrt(2 * max_acceleration * (fabs(target - value) - linear_dist / 2.));
		else
			goal_relative_velocity = max_velocity;
	} else if(value > target + linear_dist) {
		if(max_acceleration > 0)  //if max_acceleration is not disabled
			goal_relative_velocity = -sqrt(2 * max_acceleration * (fabs(target - value) - linear_dist / 2.));
		else
			goal_relative_velocity = -max_velocity;
	} else { //linear region
		goal_relative_velocity = rescale(target - value, 0, linear_dist, 0, linear_max_command);
	}

	//final goal velocity, feed forward converting relative velocity to absolute velocity
	goal_velocity = goal_relative_velocity + target_velocity;
	if(max_velocity > 0) //if max_velocity is not disabled
		goal_velocity = saturate(goal_velocity, -max_velocity, max_velocity);

	return goal_velocity;
}

//may be called right after
double SQRTController::simulate_dynamic_step(double value)
{
	value += goal_velocity * dt;
	return value;
}

////////////////////////////////////////////////////

void SmoothPathFollower::setParams(double _max_velocity, double _max_accel, double _max_jerk,
                                   double _linear_dist_position, double _linear_dist_velocity,
                                   double _in_filter_taw, double _out_filter_taw)
{
    if(_max_velocity > 0.|| _max_accel > 0.)
        pos2rate_cont = new SQRTController(_max_velocity, _max_accel, _linear_dist_position);
    if(_max_accel > 0.|| _max_jerk > 0.)
        rate2acc_cont = new SQRTController(_max_accel, _max_jerk, _linear_dist_velocity);
    initIIR2(&path_filter, _in_filter_taw);
    initIIR2(&acc_filter, _out_filter_taw);
}

void SmoothPathFollower::init(double current_value) //set current value
{
    real_position = current_value;
    real_velocity = 0;
    real_acc = 0;
}

//Just a simple 1st order filter
//Ensure position continuity
double SmoothPathFollower::step0(double time_s, double command_position)
{
    real_position = doIIRfilter2(&path_filter, command_position, time_s);
    return real_position;
}

//1 controller loop to track linear interpolation///////
//Ensure position continuity, and limit max speed and acceleration
double SmoothPathFollower::step1(double time_s, double command_position)
{
    pos2rate_cont->control_step(time_s, real_position, command_position);
    real_position += pos2rate_cont->dt * pos2rate_cont->goal_velocity;    
    return real_position;
}
    
//2 x controller loops to track linear interpolation and input/output 1st order filter for continuous acceleration///////
//By 2 sqrt loops, one is able to ensure position+velocity continuity and limit max speed, acceleration, and jerk
//By 2 1st order filters in input desired position and output command, one is able to finetune acceleration vibrations
double SmoothPathFollower::step2(double time_s, double command_position)
{
    double filtered_input = doIIRfilter2(&path_filter, command_position, time_s);
    pos2rate_cont->control_step(time_s, real_position, filtered_input);
    rate2acc_cont->control_step(time_s, real_velocity, pos2rate_cont->goal_velocity);
    double filtered_command = doIIRfilter2(&acc_filter, rate2acc_cont->goal_velocity, time_s);
    real_velocity += rate2acc_cont->dt * filtered_command;
    real_position += pos2rate_cont->dt * real_velocity;

    //printf(":::t:%f, cmd yaw:%f, yaw:%f, yaw vel:%f, goal_vel:%f, goal_acc:%f\n", time_s, ac_command[5], ac_real[5], ac_real_velocity[5], pos2rate_cont[5]->goal_velocity, rate2acc_cont[5]->goal_velocity);
    return real_position;
}

#ifdef __cplusplus
}
#endif
