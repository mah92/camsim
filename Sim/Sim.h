//ALLAH
#ifndef __NSRSIM_H__
#define __NSRSIM_H__

#include "./Init_Sim.h"
#include "./nsrPoseSim.h"

#ifdef __cplusplus
extern "C" {
#endif

extern SimStorage Sim;

int simInit();
int simLoop(double time_barrier);
int simClose();

void Sensors(SimStorage &Sim);
void addDynamic(Matrice &O, double T);

#ifdef __cplusplus
}
#endif

#endif
