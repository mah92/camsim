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

#include <dirent.h>

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

	//savepath//////////////////////////
	//whenever the program is run, it build a new folder in this address and stores used setting(.xml) files, simulation output, log files, ...
	
    strcpy(globals.savepath, globals.path);
    strcat(globals.savepath, "/log");
                
    DIR *dpdf;
    struct dirent *epdf;
    dpdf = opendir(globals.savepath);
    if(dpdf == NULL) {  //log directory does not exist
        if(mkdir_p(globals.savepath) != 0) { //make log directory in specified path
            LOGW(TAG, " Could not make directory in(%s)!\n", globals.savepath); //specified path unaccessible
            sprintf(globals.savepath, "%s/log", globals.path); //change to default path(project name/log)
            if(mkdir_p(globals.savepath) != 0) { //make directory in default path
                LOGE(TAG, " Could not make directory in(%s)!\n", globals.savepath); //default path unaccessible!!!
                sprintf(globals.savepath, "mnt/sdcard/log"); //change to root path(mnt/sdcard/log)
            }
        }
    }
            
    int len;
    int last_num = 0, pure_num;

    dpdf = opendir(globals.savepath);
    if(dpdf == NULL) {
        LOGE(TAG, " No savepath!\n");
    } else {
        last_num = 0;
        while(epdf = readdir(dpdf)) {
            //refusing "." & ".."
            if(0 == strcmp(epdf->d_name, "."))
                continue;
            if(0 == strcmp(epdf->d_name, ".."))
                continue;

            pure_num = 1;
            len = strlen(epdf->d_name);
            for(i = 0; i < len; i++)
                if(isNum(epdf->d_name[i]) == 0) //not a number
                    pure_num = 0;
            if(pure_num == 0 || len <= 0) //not a pure number
                continue;
            //LOGI(TAG, " aa: %s\n", epdf->d_name);

            //if pure number
            last_num = nsrMax(last_num, atoi(epdf->d_name));
        }
    }

    LOGW(TAG, " log: %i\n", last_num + 1);

    sprintf(globals.savepath, "%s/%i", globals.savepath, last_num + 1);
    LOGI(TAG, " savepath: %s\n", globals.savepath);
    
    if(mkdir_p(globals.savepath) != 0) {
		LOGE(TAG, " Could not make directory(%s)!\n", globals.savepath);
		strcpy(globals.savepath, globals.path);
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

	LOGOPEN(globals.savepath, "messagelog.txt");

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
