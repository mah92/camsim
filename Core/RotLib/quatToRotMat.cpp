#include "./nsrRotLib.h"
#include "Core/nsrCore.h"

#undef TAG
#define TAG "C:quatToRotMat:"

#ifdef __cplusplus
extern "C" {
#endif

int quatToRotMat(Matrice &quat, //input
				 Matrice &CI2B) //output
{
	//
	// Converts a quaternion into a 3x3 rotation matrix(CI2B)
	// using the {i,j,k,1} convention
	//

	if(size(quat, 1) != 4 || size(quat, 2) != 1)  {
		LOGE(TAG, "Input quaternion must be 4x1\n");
		return -1;
	}

	CI2B.reinit(3, 3);

	//if( fabs(norm(quat) - 1) > 1e-15 ) {
	if(fabs(norm(quat) - 1) > 0.1) {
		LOGW(TAG, "Input quaternion is strongly not unit-length. norm(q) = %f. Re-normalizing.\n", norm(quat));
	}
	quat = quat / norm(quat);
	//}

	//both solutions found to be exactly equal, but second is simpler
	//1)
	//R = quatRightComp(quat).t() * quatLeftComp(quat);
	//CI2B = renormalizeRotMat( R(1:3,1:3) );

	//2)
	double et, e1, e2, e3,
		   et_2, e1_2, e2_2, e3_2;

	e1 = quat[0]; e2 = quat[1]; e3 = quat[2];
	et = quat[3];

	et_2 = et * et; e1_2 = e1 * e1; e2_2 = e2 * e2; e3_2 = e3 * e3;
	CI2B.fill2(
		+e1_2 - e2_2 - e3_2 + et_2, +2 * e1 * e2 + 2 * e3 * et, +2 * e1 * e3 - 2 * e2 * et, //Verified
		+2 * e1 * e2 - 2 * e3 * et, -e1_2 + e2_2 - e3_2 + et_2, +2 * e2 * e3 + 2 * e1 * et,
		+2 * e1 * e3 + 2 * e2 * et, +2 * e2 * e3 - 2 * e1 * et, -e1_2 - e2_2 + e3_2 + et_2);

	return 0;
}

#ifdef __cplusplus
}
#endif
