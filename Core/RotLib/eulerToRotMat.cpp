#include "./nsrRotLib.h"
#include <math.h>

#undef TAG
#define TAG "C:eularToRotMat:"

#ifdef __cplusplus
extern "C" {
#endif

//ZYX euler to rotation matrix

void eulerToRotMat(Matrice &euler, //input
				   Matrice &CI2B) //output
{
	double phi, theta, psi,
		   SP, ST, SS, CP, CT, CS;
	CI2B.reinit(3, 3);

	phi = euler[0]; theta = euler[1]; psi = euler[2];
	SP = sin(phi); ST = sin(theta); SS = sin(psi);
	CP = cos(phi); CT = cos(theta); CS = cos(psi);
	CI2B.fill2(+CT * CS,         +CT * SS,   -ST,
			   SP * ST * CS - CP * SS, SP * ST * SS + CP * CS, SP * CT,
			   CP * ST * CS + SP * SS, CP * ST * SS - SP * CS, CP * CT); //Verified
}

#ifdef __cplusplus
}
#endif