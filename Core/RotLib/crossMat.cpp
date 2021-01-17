#include "./nsrRotLib.h"
#include "Core/nsrCore.h"

#undef TAG
#define TAG "C:crossMat:"

#ifdef __cplusplus
extern "C" {
#endif

int crossMat(Matrice &vec, //input
			 Matrice &vecCross) //output
{
	//
	// Computes the cross-product matrix of a 3x1 vector
	//
	vecCross.reinit(3, 3);

	if(size(vec, 1) != 3 || size(vec, 2) != 1) {
		LOGE(TAG, "Input vector must be 3x1\n");
		return -1;
	}

	vecCross.fill2(0.,  -vec[2],   vec[1],
				   vec[2],	    0.,  -vec[0],
				   -vec[1],	vec[0],		  0.);
	return 0;
}

#ifdef __cplusplus
}
#endif
