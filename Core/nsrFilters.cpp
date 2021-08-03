#include "./nsrFilters.h"
#include "./nsrUtility.h"

#include <math.h>
#include <assert.h>
#include <nsrGeoLib.h>
#include "nsrUtility.h"

#ifdef __cplusplus
extern "C" {
#endif

int debug_flag = 0;
    
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


//////////////////////////////////////////////////
void initMaxSlope(MaxSlopeData* fD, ffloat max_slope)
{
	fD->inited = 0;
	fD->data = 0;
	fD->pre_time_s = 0;
	fD->max_slope = max_slope;
}

ffloat doMaxSlope(MaxSlopeData* fD, ffloat data, ffloat time_s)
{
	ffloat dt;
	if(fD->inited == 0) {
		fD->inited = 1;
		fD->data = data;
		fD->pre_time_s = time_s;
		return data;
	}

	dt = time_s - fD->pre_time_s;
    
    if(EQUALS(fD->max_slope, 0., 1e-8) || EQUALS(dt, 0., 1e-8)) { //filter disabled
        fD->data = data; 
        fD->pre_time_s = time_s;
        return fD->data;
    }

	ffloat slope = (data - fD->data)/dt;
    if(slope > fD->max_slope)
        data = fD->data + fD->max_slope * dt;
    else if(slope < -fD->max_slope)
        data = fD->data - fD->max_slope * dt;
    
    fD->data = data;
	fD->pre_time_s = time_s;

	return fD->data;
}

//////////////////////////////////////////////////
void initMax3Slope(Max3SlopeData* fD, ffloat max_speed, ffloat max_acc, ffloat max_jerk)
{
	fD->inited = 0;
	fD->data1 = 0;
    fD->data2 = 0;
    fD->data3 = 0;
	fD->pre_time_s1 = 0;
    fD->pre_time_s2 = 0;
    fD->pre_time_s3 = 0;
	fD->max_speed = max_speed;
    fD->max_acc = max_acc;
    fD->max_jerk = max_jerk;
}

ffloat doMax3Slope(Max3SlopeData* fD, ffloat data, ffloat time_s)
{
	ffloat dt1, dt2, dt3;
	if(fD->inited == 0) {
		fD->inited = 1;
        fD->data1 = data;
		fD->pre_time_s1 = time_s;
		return data;
	}
	
	dt1 = time_s - fD->pre_time_s1;
    
    if(EQUALS(dt1, 0., 1e-8)) { //reject data, it can not change in zero time
        return fD->data1;
    }
    
    //1st check max_speed
    if(!(EQUALS(fD->max_speed, 0., 1e-8))) { //enabled
        ffloat speed = (data - fD->data1)/dt1;
        if(speed > fD->max_speed)
            data = fD->data1 + fD->max_speed * dt1;
        
        if(speed < -fD->max_speed)
            data = fD->data1 - fD->max_speed * dt1;
    }

    ///////////////////////////////////////
    
    if(fD->inited == 1) {
		fD->inited = 2;
		fD->data2 = fD->data1;
        fD->data1 = data;
        fD->pre_time_s2 = fD->pre_time_s1;
		fD->pre_time_s1 = time_s;
		return data;
	}
	    
    dt2 = fD->pre_time_s1 - fD->pre_time_s2;
    
    if(!(EQUALS(fD->max_acc, 0., 1e-8))) { //enabled
        ffloat v1 = (data - fD->data1)/dt1;
        ffloat v2 = (fD->data1 - fD->data2)/dt2;
        ffloat acc = (v1 - v2)/((dt1+dt2)/2);
        
        /*acc = fD->max_acc
        (v1 - v2)/((dt1+dt2)/2) = fD->max_acc
        (v1 - v2) = fD->max_acc*((dt1+dt2)/2)
        v1 = fD->max_acc*((dt1+dt2)/2) + v2
        (data - fD->data1)/dt1 = fD->max_acc*((dt1+dt2)/2) + v2
        data - fD->data1 = (fD->max_acc*((dt1+dt2)/2) + v2)*dt1
        data = (fD->max_acc*((dt1+dt2)/2) + v2)*dt1 + fD->data1
        */
        if(acc > fD->max_acc)
            //data = fD->data2 + 0.5*fD->max_acc * pow2(dt1+dt2) + v2*(dt1+dt2);
            data = fD->data1 + (fD->max_acc*((dt1+dt2)/2) + v2)*dt1;
        
        if(acc < -fD->max_acc)
            //data = fD->data2 - (0.5*fD->max_acc * pow2(dt1+dt2) + v2*(dt1+dt2));
            data = fD->data1 + (-fD->max_acc*((dt1+dt2)/2) + v2)*dt1;
    }
    
    ////////////////////////////////////
    
    if(fD->inited == 2) {
		fD->inited = 3;
        fD->data3 = fD->data2;
        fD->data2 = fD->data1;
		fD->data1 = data;
        fD->pre_time_s3 = fD->pre_time_s2;
        fD->pre_time_s2 = fD->pre_time_s1;
		fD->pre_time_s1 = time_s;
		return data;
	}

    dt3 = fD->pre_time_s2 - fD->pre_time_s3;
    
    if(!(EQUALS(fD->max_jerk, 0., 1e-8))) { //enabled
        ffloat v1 = (data - fD->data1)/dt1;
        ffloat v2 = (fD->data1 - fD->data2)/dt2;
        ffloat v3 = (fD->data2 - fD->data3)/dt3;
        
        ffloat acc1 = (v1 - v2)/((dt1+dt2)/2);
        ffloat acc2 = (v2 - v3)/((dt2+dt3)/2);
        ffloat jerk = (acc1 - acc2)/((dt1+dt3)/2); //t4  t3 t2 t1,(t1-t3)/2 - (t2-t4)/2 = (dt1)/2 + (dt3)/2 = (dt1+dt3)/2
        
        /*
        jerk == fD->max_jerk
        (acc1 - acc2)/((dt1+dt3)/2) = fD->max_jerk
        (acc1 - acc2) = fD->max_jerk*((dt1+dt3)/2)
        acc1 = fD->max_jerk*((dt1+dt3)/2) + acc2
        (v1 - v2)/((dt1+dt2)/2) = fD->max_jerk*((dt1+dt3)/2) + acc2
        (v1 - v2) = (fD->max_jerk*((dt1+dt3)/2) + acc2)*((dt1+dt2)/2)
        v1 = (fD->max_jerk*((dt1+dt3)/2) + acc2)*((dt1+dt2)/2) + v2
        v1 = (fD->max_jerk*((dt1+dt3)/2) + acc2)*((dt1+dt2)/2) + v2
        (data - fD->data1)/dt1 = (fD->max_jerk*((dt1+dt3)/2) + acc2)*((dt1+dt2)/2) + v2
        data - fD->data1 = ((fD->max_jerk*((dt1+dt3)/2) + acc2)*((dt1+dt2)/2) + v2)*dt1
        data = fD->data1 + ((fD->max_jerk*((dt1+dt3)/2) + acc2)*((dt1+dt2)/2) + v2)*dt1
        */
        if(jerk > fD->max_jerk)
            //data = fD->data3 + 0.16666666*fD->max_jerk*pow3(dt1+dt2+dt3);    //1*v*t -> 0.5*a*t2 -> /0.5*j*t3/3
            data = fD->data1 + ((fD->max_jerk*((dt1+dt3)/2) + acc2)*((dt1+dt2)/2) + v2)*dt1;
        
        if(jerk < -fD->max_jerk)
            //data = fD->data3 - 0.16666666*fD->max_jerk*pow3(dt1+dt2+dt3);
            data = fD->data1 + ((-fD->max_jerk*((dt1+dt3)/2) + acc2)*((dt1+dt2)/2) + v2)*dt1;
    }
    
    ////////////////////////////////////////////

	fD->pre_time_s3 = fD->pre_time_s2;
    fD->pre_time_s2 = fD->pre_time_s1;
    fD->pre_time_s1 = time_s;
    fD->data3 = fD->data2;
    fD->data2 = fD->data1;
    fD->data1 = data;
	return data;
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
SQRTController::SQRTController(double _max_velocity, double _max_relative_acceleration, double _linear_dist) : pre_time(-1), pre_value(0), pre_target(0)
{
	//properties
	max_velocity = _max_velocity;
	max_acceleration = _max_relative_acceleration;
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
//not inputing target_velocity means automatic calculation
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
	if(max_velocity > 0) { //if max_velocity is not disabled
		goal_velocity = saturate(goal_velocity, -max_velocity, max_velocity);
        if(debug_flag==2) printf("v_t:%f\n", max_velocity/X2LAT());
    }

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
                                   double _linear_dist_position, double _linear_dist_velocity, double _linear_dist_acc,
                                   double _in_filter_taw, double _out_filter_taw)
{
    if(_max_velocity > 0.|| _max_accel > 0.)
        pos2rate_cont = new SQRTController(_max_velocity, _max_accel, _linear_dist_position);
    if(_max_accel > 0.|| _max_jerk > 0.)
        rate2acc_cont = new SQRTController(_max_accel, _max_jerk, _linear_dist_velocity>0?_linear_dist_velocity:pos2rate_cont->linear_max_command);
    if(_max_jerk > 0.)
        acc2jerk_cont = new SQRTController(_max_jerk, 0., _linear_dist_acc>0?_linear_dist_acc:rate2acc_cont->linear_max_command);
    
    /*if(_max_velocity>0 || _max_accel >0) {
        printf("linear_dist:%f, %f, %f\n\n", _linear_dist_position, pos2rate_cont->linear_max_command, rate2acc_cont->linear_max_command);
    }*/
    
    initMax3Slope(&slope_filter, _max_velocity, _max_accel, _max_jerk);
    initIIR2(&path_filter, _in_filter_taw);
    initIIR2(&acc_filter, _out_filter_taw);
    
    max_velocity = _max_velocity;
    max_accel = _max_accel;
    max_jerk = _max_jerk;
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
//Ensure position continuity, and limit max speed and relative acceleration
//Has remaining error without feedforward
double SmoothPathFollower::step2(double time_s, double command_position)
{
    //feedforward (target) acceleration is not limited in sqrt, so we disabled feedforward
    pos2rate_cont->control_step(time_s, real_position, command_position, 0.);
    real_position += pos2rate_cont->dt * pos2rate_cont->goal_velocity;
    
    if(debug_flag) printf("e:%f, v:%f\n", (command_position - real_position)/X2LAT(), pos2rate_cont->goal_velocity/X2LAT());
    
    return real_position;
}

//2 x controller loops to track linear interpolation and input/output 1st order filter for continuous acceleration///////
//By 2 sqrt loops, one is able to ensure position+velocity continuity and limit max speed, acceleration, and relative jerk
double SmoothPathFollower::step3(double time_s, double command_position)
{
    pos2rate_cont->control_step(time_s, real_position, command_position);
    rate2acc_cont->control_step(time_s, real_velocity, pos2rate_cont->goal_velocity, 0.); //feedforward in inner loops causes oscillations, get feedback from real object acceleration
    real_velocity += rate2acc_cont->dt * rate2acc_cont->goal_velocity;
    real_position += pos2rate_cont->dt * real_velocity;
    
    if(debug_flag) printf("e:%f, gv:%f, tv:%f, r/ga:%f\n", (command_position - real_position)/X2LAT(), pos2rate_cont->goal_velocity/X2LAT(), real_velocity/X2LAT(), rate2acc_cont->goal_velocity/X2LAT());

    return real_position;
}

//3 x sqrt
//the 3rd inner loop causes oscillations
//remember no to feed _max_jerk as zero(automatic)
double SmoothPathFollower::step4(double time_s, double command_position)
{
    pos2rate_cont->control_step(time_s, real_position, command_position);
    rate2acc_cont->control_step(time_s, real_velocity, pos2rate_cont->goal_velocity, 0.); //feedforward in inner loops causes oscillations, get feedback from real object acceleration
    acc2jerk_cont->control_step(time_s, real_acc, rate2acc_cont->goal_velocity, 0.); //feedforward in inner loops causes oscillations, get feedback from real object acceleration
    real_acc += rate2acc_cont->dt * acc2jerk_cont->goal_velocity;
    real_velocity += rate2acc_cont->dt * real_acc;
    real_position += pos2rate_cont->dt * real_velocity;
    
    if(debug_flag) printf("t:%f, e:%f, r/gv:%f,:%f, r/ga:%f\n", time_s, (command_position - real_position)/X2LAT(), real_velocity/X2LAT(), pos2rate_cont->goal_velocity/X2LAT(), rate2acc_cont->goal_velocity/X2LAT());

    return real_position;
}

    
//2 x controller loops to track linear interpolation and input/output 1st order filter for continuous acceleration///////
//By 2 sqrt loops, one is able to ensure position+velocity continuity and limit max speed, acceleration, and relative jerk
//By 2 1st order filters in input desired position and output command, one is able to finetune acceleration vibrations, but it causes remenant error
double SmoothPathFollower::step6(double time_s, double command_position)
{
    double filtered_input = doIIRfilter2(&path_filter, command_position, time_s);
    pos2rate_cont->control_step(time_s, real_position, filtered_input);
    if(debug_flag>0) debug_flag++;
    rate2acc_cont->control_step(time_s, real_velocity, pos2rate_cont->goal_velocity, 0.); //feedforward in inner loops causes oscillations, get feedback from real object acceleration
    double filtered_command = doIIRfilter2(&acc_filter, rate2acc_cont->goal_velocity, time_s);
    
    //filtered_command = doMaxSlope(&slope_filter, filtered_command, time_s);
    
    real_velocity += rate2acc_cont->dt * filtered_command;
    real_position += pos2rate_cont->dt * real_velocity;
    
    if(debug_flag) printf("t;%f, e:%f, r/gv:%f,:%f, r/ga:%f\n", time_s, (command_position - real_position)/X2LAT(), real_velocity/X2LAT(), pos2rate_cont->goal_velocity/X2LAT(), rate2acc_cont->goal_velocity/X2LAT());

    return real_position;
}


//Diverges
double SmoothPathFollower::step7(double time_s, double command_position)
{
    real_position = doMax3Slope(&slope_filter, command_position, time_s);

    if(debug_flag) printf("t;%f, e:%f\n", time_s, (command_position - real_position)/X2LAT());

    return real_position;
}

#ifdef __cplusplus
}
#endif
