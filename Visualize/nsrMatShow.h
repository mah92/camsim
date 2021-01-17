#ifndef __NSR_MAT_SHOW_H__
#define __NSR_MAT_SHOW_H__

#include "Matlib/nsrMathLib.h"
#include <opencv2/core/core.hpp>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_DIM 60

class MatShow
{
	int name_count;
	char** names;
	int mode;
	cv::String winname;

public:
	MatShow(const char* winname_ = "mat");
	void registerNames(int name_count_ = 0, char** names_ = NULL, int mode_ = 0);
	void matShow(Matrice &mat, int square_size = 30);
};

#ifdef __cplusplus
}
#endif

#endif
