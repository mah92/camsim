#ifndef __NSRIMAGEBUFFER2_H__
#define __NSRIMAGEBUFFER2_H__

#include "./nsrCore.h"
#ifdef HAVE_OPENCV
#include <opencv2/core/core.hpp>
#else
enum CVTypes {CV_8UC1 = 1, CV_8UC2, CV_8UC3, CV_8UC4};
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct imageQuantum_ {
	uint8_t *frame;
	uint8_t *frame0; //first really allocated memory
	double time_s;
	bool writing;
	uint8_t reading; //may be mopre than one
#ifdef HAVE_OPENCV
	cv::Mat* cvFrame;
#endif
} imageQuantum;

class ImageBuffer2
{
	bool _inited;
	int buffer_num;
	imageQuantum* quantums;
	_LOCK_DEFINE_NATIVE(lock)

public:
	int frameSize, frame_type;
	int width, height;
	int readIndex, writeIndex;

	ImageBuffer2(int width, int height, int frame_type, int buffer_num = 2);
	~ImageBuffer2();

	//Write functions////////////////////////////////////////////
private:
	int ibGetANDLockBufferForWrite();
	void ibUnLockBufferInWrite(int index);
public:
	int ibSetRefFrame(uint8_t *frame, double _time); //prev:ibSetRawFrameUnprotected

	//Read functions////////////////////////////////////////////
private:
	int ibGetANDLockBufferForRead();
	void ibUnLockBufferInRead(int index);
public:

	uint8_t* ibGetRefFrame(); //prev:ibGetRawFrameUnprotected
};

#ifdef __cplusplus
}
#endif

#endif /*__NSRIMAGEBUFFER2_H__*/
