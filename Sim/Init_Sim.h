#ifndef __INIT_SIM_H__
#define __INIT_SIM_H__

#include "Matlib/nsrMathLib.h"
#include "Core/nsrNumBuffer.h"
#include "Sim/nsrSimParamReader.h"
#include "nsrIndex.h"
#include "nsrDataDef.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef DataIndex DataIndex3;

typedef struct {

	int DynamicInTheLoop;

	double t;
	int i;

	double Tenv; 		 //320Hz simulation freq.
	double Tplot1;       //1 Hz Visualize1 freq.
	double Tplot2;       //0.3 Hz Visualize2 freq.

	Matrice O, P_Loc, P_Angle;
	double Lat0, lon0, alt0;

	Matrice Noise, Freq, RMS, Bias;
	Matrice MMR;
} SimStorage;

void Init_Sim(SimStorage &Sim);

#ifdef __cplusplus
}
#endif

#endif
