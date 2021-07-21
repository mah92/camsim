/**
 * @file ماکرو ها و سرتیتر توابع چند منظوره
 * این فایل ماکرو ها و سرتیتر توابع همه منظوره ای را مخصوص سیستم عامل آندروید در بر دارد: ا
 * @version 2.1
 * @since 1391
 */

#ifndef __NSRUTILITY_H__
#define __NSRUTILITY_H__

#include <stdbool.h> //for bool in c
#include <stdint.h> //for uintx_t, ...
#include<unistd.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

//math/////////////////////////////////////////////////////
#ifndef max
#define nsrMax(a,b) (a>b?a:b)
#define nsrMin(a,b) (a>b?b:a)
#endif

//#ifndef pi
//#define pi 3.14159265358979
//#endif

#define equals(variable, value, thresh)	\
	((((variable) - (value)) <= thresh && ((value) - (variable)) <= thresh)?1:0)

#define saturate(variable, minimum, maximum) \
	((variable < minimum)? minimum:((variable > maximum)? maximum: variable))

//also check min & max
#define saturate2(variable, minimum, maximum) \
	saturate(variable, nsrMin(minimum, maximum), nsrMax(minimum, maximum))

#define rescale(var, min, max, newmin, newmax) \
	((((newmax) - (newmin))/((max) - (min)))*((var) - (min)) + (newmin))
//y=((y1-y0)/(x1-x0))*(x-x0)+y0;

#define sgn(var) \
	((var > 0)? 1:((var < 0)? -1: 0))
#define nsrFloor(var) \
	((int)((var>=0)?var:(var-1)))
#define nsrRound(var) \
	(((var - nsrFloor(var)) < 0.5)?nsrFloor(var):nsrFloor(var)+1)

int nsrFloor2f(float var);
int nsrRound2f(float var);
int nsrFloor2d(double var);
int nsrRound2d(double var);

#define getBit(byte,bit) ((byte >> bit)  & 0x01)
#define setBit(byte,bit,val) byte = ((val)==1?((0x01 << bit) | byte):~((0x01 << bit) | ~byte))
#define setBit64(byte,bit,val) byte = ((val)==1?(((uint64_t)0x01 << bit) | byte):~(((uint64_t)0x01 << bit) | ~byte))
#define getBitRange(byte,totalbits,lowerbit,higherbit) ((byte << (totalbits-1-higherbit)) >> (totalbits-1-higherbit+lowerbit))

//note: shifting by more than word size(32 bits for arm) is undefined
//note2: shifting does -not- change inputs

double normalize_angle(double angle);

inline double pow2(double var) {return var * var;}
inline double pow3(double var) {return var * var * var;}

//down_round_up<0: floor, down_round_up=0 round, down_round_up>0 ceil
double discretize(double val, double min, double interval, int down_round_up);

//Random number generation/////////////////////////////////
#include <stdlib.h>

//set seed with srand(unsigned int seed)

//uniform distribution
#define frand() ((double) rand() / (RAND_MAX+1.0))
#define RAND(min, max) ((min) + (float)rand()/(RAND_MAX) * ((max)-(min)))

//normal distribution
float normrnd(float mean, float _1sigma);

//Time/////////////////////////////////////////////////////
double getEpochTime();
double getMonoTime();

extern double _biasTime;
void mySetTime(double currentTime);
double myTime();

//minimum time sleep required for program to be responsive
#define MIN_SLEEP_US 5000
enum {SYNCHRONIZE_CHANNEL_CONTROL, SYNCHRONIZE_CHANNEL_DECODER, SYNCHRONIZE_CHANNEL_ENCODER, SYNCHRONIZE_CHANNEL_LOGGER, SYNCHRONIZE_CHANNEL_DRAWER};
#define SYNCHRONIZE_CHANNEL_NUM 5
int synchronize_us(uint8_t channel, long int synchronize_time_us);

//
enum {PERIOD_CHANNEL_LOSS, PERIOD_CHANNEL_HEARTBEAT, PERIOD_CHANNEL_STATUS, PERIOD_CHANNEL_RC_GIMBAL, PERIOD_CHANNEL_RC_MAINBOARD, PERIOD_CHANNEL_PARAM, PERIOD_CHANNEL_STREAMER_MAXFILEPERIOD, PERIOD_CHANNEL_CAMSCENE_LOG, PERIOD_CHANNEL_MAPSCENE_LOGTRACE};
#define PERIOD_CHANNEL_NUM 10
//note: always do the work if period checking result is true
int check_period_passed(uint8_t chan, float interval_s);

int time_reached(double period, double phase, double t, double pre_tsim);

//LOCKS////////////////////////////////////////////////////
#include <pthread.h>

typedef pthread_mutex_t* NativeLock;

#define _LOCKNATIVE(Key,err)\
	pthread_mutex_lock(Key);

#define _UNLOCKNATIVE(Key,err)\
	pthread_mutex_unlock(Key);

#define _LOCK_DEFINE_NATIVE(Key)\
	pthread_mutex_t *Key, _##Key; \
	//pthread_mutex_t _##Key = PTHREAD_MUTEX_INITIALIZER;
//"Key = PTHREAD_MUTEX_INITIALIZER" in c++ 11 is equal to "pthread_mutex_init( Key, NULL);"

#define _LOCK_FIND_NATIVE(Key)\
	extern pthread_mutex_t* Key;

#define _LOCK_INIT_NATIVE(Key,err)\
	pthread_mutex_init( &_##Key, NULL);\
	Key = &_##Key ;

#define _LOCK_DESTROY_NATIVE(Key,err)\
	pthread_mutex_destroy( Key);

#define Lock NativeLock
#define _LOCKCPP(Key,err) _LOCKNATIVE(Key,err)
#define _UNLOCKCPP(Key,err) _UNLOCKNATIVE(Key,err)

//error reporting////////////////////////
extern char errstr[];
extern int mline;
#define PRINT_LINE() LOGI(TAG, "l:%i", __LINE__);
#define PRINT_LINE2() sprintf(errstr, "(%s, %d)\n", __func__, __LINE__);
#define PRINT_LINE3() printf("(%s, %d)\n", __func__, __LINE__);

#ifdef __cplusplus
}
#endif

#endif /* __NSRUTILITY_H__ */
