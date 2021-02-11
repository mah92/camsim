/**
 * @file توابع چند منظوره
 * این فایل توابع همه منظوره ای را خصوصا برای کار با زمان مخصوص سیستم عامل آندروید در بر دارد: ا
 * @since 1391
 */

#include "./nsrUtility.h"
#include "./nsrMsgLog.h"

#include <sys/time.h>
#include <time.h>

#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

//math/////////////////////////////////////////////////////
//make bet [-pi, pi]
double normalize_angle(double angle)
{
	//return fmod( angle +pi, 2*pi ) - pi; //wrong
	while(angle > M_PI) angle -= 2 * M_PI;
	while(angle <= -M_PI) angle += 2 * M_PI;
	return angle;
}

double discretize(double val, double min, double interval, int down_round_up)
{
	//interval = (max - min)/steps;
	if(interval == 0)
		return val;
	double a = (val - min) / interval;
	int floor_of_a = nsrFloor(a);
	if(down_round_up < 0)
		return floor_of_a * interval + min;
	else if(down_round_up == 0)
		return nsrRound(a) * interval + min;
	else if(down_round_up > 0)
		return (floor_of_a + 1) * interval + min;
	return -1; //never happens
}

int nsrFloor2f(float var)
{
	return (int)((var >= 0) ? var : (var - 1));
}

int nsrRound2f(float var)
{
	float _floor = nsrFloor2f(var);
	return (((var - _floor) < 0.5) ? _floor : _floor + 1);
}

int nsrFloor2d(double var)
{
	return (int)((var >= 0) ? var : (var - 1));
}

int nsrRound2d(double var)
{
	double _floor = nsrFloor2d(var);
	return (((var - _floor) < 0.5) ? _floor : _floor + 1);
}

//normal random variate generator(box_muller method)
float normrnd(float mean, float _1sigma)
{
	float x1, x2, w, y1;
	static float y2;
	static int use_last = 0;

	if(use_last) {		         /* use value from previous call */
		y1 = y2;
		use_last = 0;
	} else {
		do {
			x1 = 2.0 * frand() - 1.0;
			x2 = 2.0 * frand() - 1.0;
			w = x1 * x1 + x2 * x2;
		} while(w >= 1.0);

		w = sqrt((-2.0 * log(w)) / w);
		y1 = x1 * w;
		y2 = x2 * w;
		use_last = 1;
	}

	return(mean + y1 * _1sigma);
}

//Time/////////////////////////////////////////////////////

//don't use wall clocks for filter - controllers(=real-time=RTC clocks!=system clocks) as they may jump due to time adjustments
//don't use clock(): it is cpu time used by program! & it overflows when CLOCKS_PER_SEC==1e6(android not win)(returns same every 72 minutes)
//clock_gettime has the flexibility of being both rtc & system clocks in its settings
/* clock_gettime:
 * int clock_gettime(clockid_t clk_id, struct timespec *tp);
 * CLOCK_REALTIME: System-wide real-time clock. Setting this clock requires appropriate privileges.
 * CLOCK_MONOTONIC: Clock that cannot be set and represents monotonic time since some unspecified starting point.
 * CLOCK_MONOTONIC_RAW (since Linux 2.6.28; Linux-specific): Similar to CLOCK_MONOTONIC, but provides access to a raw hardware-based time that is not subject to NTP adjustments.
 * CLOCK_PROCESS_CPUTIME_ID: High-resolution per-process timer from the CPU.
 * CLOCK_THREAD_CPUTIME_ID: Thread-specific CPU-time clock.
*/
/*#define CLOCK_REALTIME 0
#define CLOCK_MONOTONIC 1
#define CLOCK_PROCESS_CPUTIME_ID 2
#define CLOCK_THREAD_CPUTIME_ID 3*/

/*struct timeval {
  long tv_sec;
  long tv_usec;
};*/

/*struct timespec {
    time_t   tv_sec;        // seconds
    long     tv_nsec;       // nanoseconds
};*/

#ifdef __linux__
#define CLOCK_REALTIME 0
#define CLOCK_MONOTONIC 1
#endif

double getEpochTime()//May change & jump with network, don't rely on
{
#if __ANDROID__
	struct timespec tp;
	if(clock_gettime(CLOCK_REALTIME, &tp) == 0)
		return tp.tv_sec + tp.tv_nsec * 1e-9;
	else {
		//LOGE("Time","clock_gettime(CLOCK_REALTIME,...) error");
		return -1;
	}
#endif

#if __linux__
	struct timeval tv;
	gettimeofday(&tv, NULL);
	//time_t tv.tv_sec;
	//suseconds_t tv.tv_usec;
	return tv.tv_sec + tv.tv_usec * 1e-6;
#endif
}

double getMonoTime()
{
#ifdef __ANDROID__
	struct timespec tp;

	if(clock_gettime(CLOCK_MONOTONIC, &tp) == 0)
		return tp.tv_sec + tp.tv_nsec * 1e-9;// tv_sec & tv_nsec are both longs
	else {
		//LOGE("Time","clock_gettime(CLOCK_MONOTONIC,...) error");
		return -1;
	}
#endif
    return -1;
}

double _biasTime = 0;//in clock cycles, set once in init(single thread), after that read only
void mySetTime(double currentTime)
{
#ifdef ANDROID
	_biasTime = currentTime - getMonoTime();
#else
	_biasTime = currentTime - getEpochTime();
#endif
}
double myTime()
{
#ifdef ANDROID
	return getMonoTime() + _biasTime;
#else
	return getEpochTime() + _biasTime;
#endif
}

//اجرای برنامه را تا اختلاف زمانی مشخص شده، از اجرای قبلی این تابع به تاخیر می اندازد.
int synchronize_us(uint8_t chan, long int synchronize_time_us)
{
	double time_s;
	double static pre_time_s[SYNCHRONIZE_CHANNEL_NUM] = {-1};
	long int sleep_time_us;

	time_s = myTime();
	if(pre_time_s[chan] == -1)
		pre_time_s[chan] = time_s;

	sleep_time_us = synchronize_time_us - (time_s - pre_time_s[chan]) * 1000000;

	if(sleep_time_us > MIN_SLEEP_US)
		usleep(sleep_time_us);
	else
		usleep(MIN_SLEEP_US); //minimum sleep time required for program to be responsive

	pre_time_s[chan] = myTime();
	return 0;
}

int check_period_passed(uint8_t chan, float interval_s)
{
	static float last_time_s[PERIOD_CHANNEL_NUM] = {-1000000};

	double time_s = myTime();
	if(time_s > last_time_s[chan] + interval_s) {
		last_time_s[chan] = time_s;
		return 1;
	} else
		return 0;
	return -1; //never happens
}

//phase c [0., 1.]
int time_reached(double period, double phase, double t, double pre_tsim)
{
	if(period < 1e-6)
		return 1;

	if(nsrFloor2d(t / period - phase + 0.00001) > nsrFloor2d(pre_tsim / period - phase + 0.00001))
		return 1;

	return 0;
}

#ifdef __cplusplus
}
#endif
