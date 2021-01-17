//ALLAH
/**
 * @file بافر تصویر
 *
 * این بافر وظیفه نگه داری فریم های تصویر را به عهده دارد
 * ورژن دوم از این بافر به صورت بافر چندگانه پیاده سازی شده
 * هر چند به صورت تک بافر قفل دار نیز قابل استفاده است.
 * در صورت پر بودن قفل نمیکند بلکه خطا می دهد(نان بلاکینگ است)ا
 *
 * @see nftSimple sample
 */

#include "./nsrImageBuffer2.h"

#include "./nsrCore.h"
#include <float.h>

#include <math.h> //for fabs

using namespace std;
#ifdef HAVE_OPENCV
using namespace cv;
#endif

#undef TAG
#define TAG "Cpp:FrameBuffer:"

#ifdef __cplusplus
extern "C" {
#endif

ImageBuffer2::ImageBuffer2(int _width, int _height, int _frame_type, int _buffer_num)
{
	int i;

	_inited = false;

	buffer_num = _buffer_num;

	width = _width;
	height = _height;
	frame_type = _frame_type;

	readIndex = 0;
	writeIndex = 0;

	//gPixFormat = AR_PIXEL_FORMAT_NV21;
	//if(frame_type == YUV420p)
	//	frameSize = (sizeof(uint8_t)*(_width*_height + 2*_width/2*_height/2));
	if(frame_type == CV_8UC1)
		frameSize = (sizeof(uint8_t) * (_width * _height));
	else if(frame_type == CV_8UC3)
		frameSize = (sizeof(uint8_t) * (_width * _height * 3));
	else if(frame_type == CV_8UC4)
		frameSize = (sizeof(uint8_t) * (_width * _height * 4));
	else {
		LOGE(TAG, " Unknown frame_Type!\n");
		return;
	}

	quantums = (imageQuantum*) malloc(buffer_num * sizeof(imageQuantum));
	for(i = 0; i < buffer_num; i++) {
		quantums[i].frame0 = (uint8_t*) malloc(frameSize);
		quantums[i].frame = quantums[i].frame0;
		quantums[i].time_s = -1;
		quantums[i].writing = false;
		quantums[i].reading = 0;
#ifdef HAVE_OPENCV
		quantums[i].cvFrame = new cv::Mat(_height, _width, frame_type, (unsigned char *)quantums[i].frame0);
#endif
	}

	_LOCK_INIT_NATIVE(lock, -1)

	LOGI(TAG, "Image Buffer Init, size:%dx%d , %d-byte buffer.", _width, _height, frameSize);

	_inited = true;
}

ImageBuffer2::~ImageBuffer2()
{
	int i;

	if(!_inited) {
		LOGW(TAG, "ibClosecalled before ibInit!\n");
		return;
	}

	_inited = false;

	for(i = 0; i < buffer_num; i++) {
		free(quantums[i].frame0);
#ifdef HAVE_OPENCV
		quantums[i].cvFrame->release(); //does not release external data
#endif
	}

	free(quantums);

	frameSize = 0;

	_LOCK_DESTROY_NATIVE(lock, -1);
}

//Write functions////////////////////////////////////////////
//always done on oldest
int ImageBuffer2::ibGetANDLockBufferForWrite()
{
	int i, index = -1;
	double tmin = DBL_MAX;

	if(!_inited) {
		LOGW(TAG, " called before nsrIBInit!\n");
		return -2;
	}

	//_LOCKNATIVE(lock, -1);
	for(i = 0; i < buffer_num; i++)
		if(quantums[i].writing == false && quantums[i].reading == 0)  //image not in any use
			if(tmin > quantums[i].time_s) {//has minimum(oldest) time
				tmin = quantums[i].time_s;
				index = i;
			}
	//_UNLOCKNATIVE(lock, -1);

	if(index >= 0) //found an option
		quantums[index].writing = true;

	return index;
}

void ImageBuffer2::ibUnLockBufferInWrite(int index)
{
	//_LOCKNATIVE(lock, );
	writeIndex++;
	//_UNLOCKNATIVE(lock, );

	quantums[index].writing = false;
}

int ImageBuffer2::ibSetRefFrame(uint8_t *_frame, double _time)
{
	int index;
	index = ibGetANDLockBufferForWrite();
	if(index < 0) return -1;

	quantums[index].time_s = _time;
	quantums[index].frame = _frame;
#ifdef HAVE_OPENCV
	quantums[index].cvFrame->data = (unsigned char*)_frame;
#endif

	ibUnLockBufferInWrite(index);

	return 0;
}

//Read functions////////////////////////////////////////////
//always done on newest
int ImageBuffer2::ibGetANDLockBufferForRead()
{
	int i, index = -1;
	double tmax = 0;

	if(!_inited) {
		LOGW(TAG, " called before nsrIBInit!\n");
		return -2;
	}

	//_LOCKNATIVE(lock, -1);
	for(i = 0; i < buffer_num; i++)
		if(quantums[i].writing == false //image not in writing use, reading is not important
				&& quantums[i].time_s >= 0) { //image written at least once
			//search and find minimum(oldest) time
			if(tmax < quantums[i].time_s) {
				tmax = quantums[i].time_s;
				index = i;
			}
		}
	//_UNLOCKNATIVE(lock, -1);

	if(index >= 0) //found an option
		quantums[index].reading++;

	return index;
}

void ImageBuffer2::ibUnLockBufferInRead(int index)
{
	//_LOCKNATIVE(lock, );
	readIndex++;
	quantums[index].reading--;
	//_UNLOCKNATIVE(lock, );
}

uint8_t *ImageBuffer2::ibGetRefFrame()
{
	int index;
	index = ibGetANDLockBufferForRead(); //newest
	if(index < 0) return NULL;
	ibUnLockBufferInRead(index);

	return quantums[index].frame;
}

#ifdef __cplusplus
}
#endif
