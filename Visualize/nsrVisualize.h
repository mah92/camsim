//ALLAH
#ifndef __NSRVISUALIZE_H__
#define __NSRVISUALIZE_H__

#define _VISUALIZE_

#include "./nsrPlot.h"

#include "Sim/Init_Sim.h"

#ifdef __cplusplus
extern "C" {
#endif

void Visualize(SimStorage &Sim, double Tplot2);//, Con)

extern double point_alt_rms;
void nsrSaveErrors();

#ifdef __cplusplus
}
#endif

#endif
