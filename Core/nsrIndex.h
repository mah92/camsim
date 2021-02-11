#ifndef __NSRINDEX_H__
#define __NSRINDEX_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	//Common////////////////////////////////////
	int	Acc, Gyro, MMM, MMR, Eu,
		GPS_LLA, GPS_V, GPS_DOP, GPS_Other,
		Prs, PrsAlt, Tmp, QuExt, 
		RATE;

	//Added for filter///////////////////////////////
	int WExt, VExt, PrsAcc, Time;
	int GROUND_TRUTH_LLA, GROUND_TRUTH_EU;

	////////////////////////////////////////////
	int Total, Depth, Readers;
} SensorsNamespace;

//////////////////////////////////

typedef struct {
	SensorsNamespace Z;
} DataIndex;

extern DataIndex n;

void SetIndexes();
int createBuf(int* elem, int width);

extern int execution_turn;
extern char execution_desc[];
	
#ifdef __cplusplus
}
#endif

#endif /* __NSRINDEX_H__ */
