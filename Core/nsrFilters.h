#ifndef __NSRFILTERS_H__
#define __NSRFILTERS_H__

#include <stdint.h>
#include <float.h>

#define ffloat double

typedef struct {
	int inited;
	ffloat data;
} IIRData;

typedef struct {
	int inited;
	ffloat data;
	ffloat pre_time_s;
	ffloat taw;
} IIRData2;

typedef struct {
	int turn;
	int data_len;
	ffloat* data;
	ffloat* weight;
} FIRData;

typedef struct {
	int inited;
	ffloat data;
	ffloat pre_time_s;
	ffloat max_slope;
} MaxSlopeData;

typedef struct {
	int inited;
	ffloat data1;
    ffloat data2;
    ffloat data3;
	ffloat pre_time_s1;
	ffloat pre_time_s2;
	ffloat pre_time_s3;
	ffloat max_speed;
    ffloat max_acc;
    ffloat max_jerk;
} Max3SlopeData;

typedef struct _TotalData {
	uint32_t turn;
	ffloat data;
} TotalAveragerData;

#define tickRateFilterMethod 0
typedef struct _TickRateData {
	int inited;
	float pre_t;
#if tickRateFilterMethod == 0
	IIRData IIR[2];
#endif
#if tickRateFilterMethod == 1
	FIRData FIR[2];
#endif
#if tickRateFilterMethod == 2
	TotalAveragerData TAF[2];
#endif
} TickRateData;

//ref: http://ardupilot.org/dev/docs/apmcopter-programming-attitude-control-2.html
//This controller puts system on the max relative speed with target so that we reach target (with zero relative velocity) with max braking acceleration
//this policy leads to a controller realtive to sqrt of error(rather than linear to error)
//A linear region is considered near error=0 so that noise does not cause chattering(+-linear_dist)
//As this region should be continues & slope continues, the sqrt parts are shifted, this shift(x0) is half of linear_dist

class SQRTController
{
public:

	//properties
	double max_acceleration, max_velocity;
	double linear_dist;

	//derived parameters
	double linear_max_command;

	//memory
	double pre_time, pre_value, pre_target;

	//outputs
	double goal_velocity;

	//other data
	double dt;

public:
    
    //just the relative to target acceleration is bound(feedforward target acceleration is not limited)
    //if you want to directly limit total acceleration, disable feefoward by passing target_vel = 0.,
    //  it only effectively limits total acceleration if the target is not moving
	SQRTController(double _max_velocity, double _max_relative_acceleration, double _linear_dist);
	double control_step(double time_s, double value, double target, double target_vel = DBL_MAX);
	double simulate_dynamic_step(double value);
};

/////////////////////////////////////////
//a controller used for states to limit accelerations as in real aircraft

class SmoothPathFollower
{
    double max_velocity, max_accel, max_jerk;
    double pre_time;
    IIRData2 path_filter, acc_filter;
    Max3SlopeData slope_filter;
    SQRTController* pos2rate_cont, *rate2acc_cont, *acc2jerk_cont;
    //memory
	double real_position, real_velocity, real_acc;

public:
	SmoothPathFollower() : 
	pre_time(-1),
        pos2rate_cont(0), rate2acc_cont(0), acc2jerk_cont(0), //NULL
            real_position(0.), real_velocity(0.), real_acc(0.)
	{};
    
    ~SmoothPathFollower()
    {
        if(pos2rate_cont!=0) delete pos2rate_cont;
        if(rate2acc_cont!=0) delete rate2acc_cont;
        if(acc2jerk_cont!=0) delete acc2jerk_cont;
    }
    
    void setParams(double _max_velocity, double _max_accel, double _max_jerk,
              double _linear_dist_position, double _linear_dist_velocity,  double _linear_dist_acc, 
              double _in_filter_taw, double _out_filter_taw //negative means disabled
    );
    
    void init(double current_value); //set current value
    double get() {return real_position; }
    
    double step0(double time_s, double command_position); //Just a simple 1st order filter
	double step2(double time_s, double command_position); //1xsqrt control loop 
	double step3(double time_s, double command_position); //2xsqrt control loop 
    double step4(double time_s, double command_position); //3xsqrt control loop 
	double step6(double time_s, double command_position); //2xsqrt control loops + 2 filters
	double step7(double time_s, double command_position); //speed, acc, jerk saturation
};

#ifdef __cplusplus
extern "C" {
#endif
    
extern int debug_flag;

void initIIR2(IIRData2* fD, ffloat taw);
ffloat doIIRfilter2(IIRData2* fD, ffloat data, ffloat time_s);

void initFIR(FIRData* fD, int datalen);
ffloat doFIRfilter(FIRData* fD, ffloat data, float weight);
void closeFIRFilter(FIRData* fD);

void initMaxSlope(MaxSlopeData* fD, ffloat max_slope);
ffloat doMaxSlope(MaxSlopeData* fD, ffloat data, ffloat time_s);

void initMax3Slope(Max3SlopeData* fD, ffloat max_speed, ffloat max_acc, ffloat max_jerk);
ffloat doMax3Slope(Max3SlopeData* fD, ffloat data, ffloat time_s);

void initTotalAverager(TotalAveragerData* fD);
ffloat doTotalAverage(TotalAveragerData* fD, ffloat data);

void initTickRate(TickRateData* fD);
ffloat tickRate(TickRateData* fD, int ticks);

#ifdef __cplusplus
}
#endif

#endif /* __NSRFILTERS_H__ */

