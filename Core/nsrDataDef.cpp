//ALLAH
/**
 * @file تعریفات مشترک
 *
 * این فایل محا تعریف داده های مشترک در کد سی و داده های مشترک بین سی و جاواست
 *
 */
#ifndef __ANDROID__
#include "/usr/include/time.h"
#endif

#include "./nsrCore.h"

#include <stddef.h> //NULL keyword
#include <stdio.h>
#include <string.h>
#include <time.h>

//for mkdir()
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h> //usleep
#include <limits.h>     // PATH_MAX
#include <sys/stat.h> //for stat

#include <exception>      // std::exception, std::terminate

#undef TAG
#define TAG "C:DataDef:"

#ifdef __cplusplus
extern "C" {
#endif

//This structure just contains global variables which are shared by more than 1 file
GlobalsStruct globals;

//Set once in init(single thread), after that read only
//Lock Z_lock, X_lock, Y_lock, U_lock;
NumBuffer mZB_; //Set once in init(single thread), after that read only
NumBuffer *mZB = NULL; //Protected by locks ///TODO: check

_LOCK_DEFINE_NATIVE(Z_lock)

int mkdir_p(const char *path)
{
	/* Adapted from http://stackoverflow.com/a/2336245/119527 */
	const size_t len = strlen(path);
	char _path[PATH_MAX];
	char *p;

	errno = 0;

	/* Copy string so its mutable */
	if(len > sizeof(_path) - 1) {
		errno = ENAMETOOLONG;
		return -1;
	}
	strcpy(_path, path);

	/* Iterate the string */
	for(p = _path + 1; *p; p++) {
		if(*p == '/') {
			/* Temporarily truncate */
			*p = '\0';

			if(mkdir(_path, S_IRWXU) != 0) {
				if(errno != EEXIST)
					return -1;
			}

			*p = '/';
		}
	}

	if(mkdir(_path, S_IRWXU) != 0) {
		if(errno != EEXIST)
			return -1;
	}

	return 0;
}

int NativeOpen(const char* rootpath,
			   double starttime)
{
	LOGOPEN(NULL, NULL);
	memset(&globals, 0, sizeof(GlobalsStruct));

	//setting time/////////////////////
	mySetTime(starttime);

	//fix paths//////////////////////
	//Root path(/mnt/sdcard)////////////////////////

	strcpy(globals.rootpath, rootpath);

	strcpy(globals.path, globals.rootpath);
	LOGI(TAG, " Project Name: %s , globals.rootpath(%s)\n", PROJECT_NAME, globals.rootpath);

	int i;

	if(mkdir_p(globals.path) != 0) {
		sleep(5); //wait maybe sdcard loads
		if(mkdir_p(globals.path) != 0) {
			sleep(5); //wait maybe sdcard loads
			if(mkdir_p(globals.path) != 0) {
				LOGE(TAG, " Could not make directory(%s)!\n", globals.path);
				strcpy(globals.path, globals.rootpath);
			}
		}
	}
	//datapath//////////////////////////

	//strcpy(globals.datapath, globals.rootpath);
	strcpy(globals.datapath, "../assets/");
	strcat(globals.datapath, PROJECT_NAME);

	if(mkdir_p(globals.datapath) != 0) {
		LOGE(TAG, " Could not make directory(%s)!\n", globals.datapath);
		strcpy(globals.datapath, globals.path);
	}

	return 0;
}

//settings should be read before it
int setDynamicAddresses()
{
	//savepath/////////////////////////
	if(mkdir_p(settings.savepath) != 0) {
		LOGE(TAG, " Could not make directory(%s)!\n", settings.savepath);
		strcpy(settings.savepath, globals.path);
	}

	//mappath//////////////////////////
	struct stat info;
	if(stat(settings.mappath, &info) != 0) {
		sleep(5); //wait maybe sdcard loads
		if(stat(settings.mappath, &info) != 0) {
			sleep(5); //wait maybe sdcard loads
		}
	}

	return 0;
}

int setSharedBuffers()
{
	SetIndexes();

	//Init Message Logger//////////////////////////////////
	time_t mtime;
	struct tm* tp;
	mtime = time(NULL);
	//tp=gmtime(&mtime);
	tp = localtime(&mtime);

	LOGOPEN(settings.savepath, "messagelog.txt");

	LOGI(TAG, " -------------------------------------------------\n");
	LOGI(TAG, " Just for ALLAH\n");
	LOGI(TAG, " Ya Zahra...\n");

	_LOCK_INIT_NATIVE(Z_lock, -1)
	mZB = &mZB_;
	cbInitNumBuffer(mZB, n.Z.Total, n.Z.Depth, n.Z.Readers);
	cbReset(mZB);

	LOGDUMP(); //Dump all direct writings to file

	LOGI(TAG, " Time:%s", asctime(tp));//includes a newline

	return 0;
}

int NativeClose()
{
	//Exit Message Logger//////////////////////////////////
	LOGI(TAG, " LOG Finished!\n");
	LOGDUMP(); //Should be done before last message
	LOGCLOSE(); //Should be done before blocking errors

	return 0;
}

int nsrExit()
{
	exit(0);
	//std::terminate(); //causes SIGABRT
}

#ifdef __cplusplus
}
#endif
