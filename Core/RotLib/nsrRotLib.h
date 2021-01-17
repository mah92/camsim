#ifndef __NSR_ROT_LIB_H__
#define __NSR_ROT_LIB_H__

#include "Matlib/nsrMathLib.h"

#ifdef __cplusplus
extern "C" {
#endif

double calcEuError(double phi1, double theta1, double psi1,
				   double phi2, double theta2, double psi2);

int crossMat(Matrice &vec, //input
			 Matrice &vecCross); //output

void eulerToRotMat(Matrice &euler, //input
				   Matrice &CI2B); //output

int quatToRotMat(Matrice &quat, //input
				 Matrice &CI2B); //output

#ifdef __cplusplus
}
#endif

#endif
