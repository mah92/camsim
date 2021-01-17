#ifndef __NSR_IMAGE_PROC_H__
#define __NSR_IMAGE_PROC_H__

#include <stdint.h>
#include "Visualize/plot3d.h"

#define STRATEGY_ALLFRAMES 0
#define STRATEGY_INTERVAL 1
#define STRATEGY_DISTANCE 2
#define STRATEGY_HORIZONTAL_DISTANCE 3
#define STRATEGY_FEATURES_MOVE 4

#define KEYFRAME_STRATEGY STRATEGY_HORIZONTAL_DISTANCE

#ifdef __cplusplus
extern "C" {
#endif
	
extern double keyframe_interval, keyframe_distance;

int nsrInitImageProc();
int nsrImageProc(uint8_t* data, int width, int height, int channels, double frame_timestamp_s);
void nsrEndImageProc();

extern Plot3d* plt3d;

#ifdef __cplusplus
}
#endif

#endif
