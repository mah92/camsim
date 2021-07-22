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

#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>

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

std::string exec(const char* cmd) 
{
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

//path is input/output
int make_savepath_dir(char* path)
{
    int i;
    DIR *dpdf;
    struct dirent *epdf;
    dpdf = opendir(path);
    if(dpdf == NULL) {  //root log directory does not exist
        if(mkdir_p(path) != 0) { //make log directory in specified path
            LOGW(TAG, " Could not make root savepath directory in(%s)!\n", path); //specified path unaccessible
            return -1;
        } else {
            LOGW(TAG, " Made root savepath directory in(%s)!\n", path);
        }
    }
            
    int len;
    int last_num = 0, pure_num;

    last_num = 0;
    while((epdf = readdir(dpdf))) {
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

    LOGW(TAG, " log: %i\n", last_num + 1);

    sprintf(path, "%s/%i", path, last_num + 1);
    
    if(mkdir_p(path) != 0) {
        return -1;
	} else {
        LOGW(TAG, " final savepath: %s\n", path);
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

	if(mkdir_p(globals.path) != 0) {
		sleep(5); //wait maybe sdcard loads
		if(mkdir_p(globals.path) != 0) {
			sleep(5); //wait maybe sdcard loads
			if(mkdir_p(globals.path) != 0) {
				LOGE(TAG, " Could not make path directory(%s)!\n", globals.path);
				strcpy(globals.path, globals.rootpath);
			}
		}
	}
	//datapath//////////////////////////

	//strcpy(globals.datapath, globals.rootpath);
	strcpy(globals.datapath, "../assets/");
	strcat(globals.datapath, PROJECT_NAME);

	if(mkdir_p(globals.datapath) != 0) {
		LOGE(TAG, " Could not make data directory(%s)!\n", globals.datapath);
		strcpy(globals.datapath, globals.path);
	}

	//savepath//////////////////////////
	//whenever the program is run, it build a new folder in this address and stores used setting(.xml) files, simulation output, log files, ...
    //Preferably use a flash drive so your Harddrive does not grow old, Use "Disks" to mount your flash at this point 
    //
    
	std::string flash_path = exec("lsblk -o NAME,HOTPLUG,MOUNTPOINT | grep -w 1 | grep -vE \"sr|loop\" | awk \'{print $3}\'");
    flash_path.erase(std::remove(flash_path.begin(), flash_path.end(), '\n'), flash_path.end()); //erase new lines
    strcpy(globals.savepath, flash_path.c_str());
    //strcpy(globals.savepath, "/mnt/usb"); 
    
	if(make_savepath_dir(globals.savepath)!=0) {
        sprintf(globals.savepath, "%s/log", globals.path); //change to default path(project name/log)
        if(make_savepath_dir(globals.savepath)!=0) {
            LOGW(TAG, " Could not make final savepath directory!\n");
            exit(110);
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
