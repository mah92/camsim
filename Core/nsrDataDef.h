//ALLAH
/**
 * @file سربرگ تعريف داده ها و حافظه مشترک با جاوا
 *
 * اين فايل سربرگ تعريف داده هاي مشترک کد سي و داده هاي مشترک سي-جاوا است
 *
 */

#ifndef __NSRDATADEF_H__
#define __NSRDATADEF_H__

#include "./nsrIndex.h"
#include "./nsrUtility.h"
#include "./nsrStrUtility.h"
#include "./nsrNumBuffer.h"
#include "./nsrImageBuffer2.h"

#ifdef __cplusplus
extern "C" {
#endif

//This structure just contains global variables which are shared by more than 1 file

extern DataIndex n;

typedef struct GlobalsStruct_ {
	//Initialization in structs impossible, all variables are initialized to zero
	//nsrDataDef.cpp

	/** آدرس پوشه ریشه */
	char rootpath[MAX_PATH_LENGTH];

	/** آدرس پوشه برنامه */
	char path[MAX_PATH_LENGTH];

	/** آدرس پوشه داده ها و فایل تنظیمات  */
	char datapath[MAX_PATH_LENGTH];
    
    char savepath[MAX_PATH_LENGTH];
} GlobalsStruct;

extern GlobalsStruct globals;

extern Lock Z_lock;
extern NumBuffer mZB_;
extern NumBuffer *mZB;

/**
 * مقدار دهی اولیه در محیط سی
 * @return شماره خطا
 */
int NativeOpen(const char* rootpath, double starttime);

/**
 * گشودن محیط سی با تعریف داده و مقدار دهی اولیه ساختارهای اصلی داده
 * @return شماره خطا
 */
int setSharedBuffers();

/**
 * بستن محیط سی با آزادسازی حافظه ساختارهای اصلی داده
 * @return شماره خطا
 */
int NativeClose();
int nsrExit();

int mkdir_p(const char *path);

#ifdef __cplusplus
}
#endif

#endif /*__NSRDATADEF_H__*/
