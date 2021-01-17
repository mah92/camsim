//ALLAH
/**
 * @file
 * این فایل خواندن فایل تنظیمات اولیه(تنظیمات عموما ثابت) را به عهده دارد
 *
 * @see settings.txt
 * @since 1391
 */

#include <stdio.h>
#include <string.h>//strcpy
#include <errno.h>

#include <dirent.h>
#include "./nsrCore.h"

#undef TAG
#define TAG "C:Settings:"

#ifdef __cplusplus
extern "C" {
#endif

SettingsStruct settings;//Set once in init(single thread), after that read only

int findnextparam(FILE* file, char* buf)
{
	if(file == NULL) {
		buf = NULL;
		return -1;
	}

	while(1) {
		if(EOF == fscanf(file, "%s", buf)) { //read one word
			buf = NULL;
			return -2;
		}

		if(buf[0] == '@' && buf[0] != '\r' && buf[0] != '\n') {
			sscanf(buf, "@%s", buf); //ignore seperator
			return strlen(buf);
		}
	}

	return 0;
}

int init_settings()
{
	LOGI(TAG, " Initializing Settings...\n");

	memset(&settings, 0, sizeof(SettingsStruct));

	return 0;
}

int read_settings()
{
	//Read Settings////////////////////////////////////////
	FILE *setfile;
	char buf[300];
	char filepath[MAX_PATH_LENGTH];
	int i, len;

	LOGI(TAG, " Reading Settings...\n");

	strcpy(filepath, globals.datapath);
	strcat(filepath, "/addresses.txt");

	setfile = fopen(filepath, "r");	//"r":=read only
	if(setfile == NULL) {
		LOGE(TAG, " Settings file read failed in address %s! check settings file\n", filepath);
		exit(10);
	}
	
	//savepath//////////////////////////////////////////////
	len = findnextparam(setfile, buf);
	strcpy(settings.savepath, globals.path);
	if(len > 0)
		strcat(settings.savepath, buf);
	else
		strcat(settings.savepath, "/log");

	DIR *dpdf;
	struct dirent *epdf;
	dpdf = opendir(settings.savepath);
	if(dpdf == NULL)  //log directory does not exist
		if(mkdir_p(settings.savepath) != 0) { //make log directory in specified path
			LOGW(TAG, " Could not make directory in(%s)!\n", settings.savepath); //specified path unaccessible
			sprintf(settings.savepath, "%s/log", globals.path); //change to default path(project name/log)
			if(mkdir_p(settings.savepath) != 0) { //make directory in default path
				LOGE(TAG, " Could not make directory in(%s)!\n", settings.savepath); //default path unaccessible!!!
				sprintf(settings.savepath, "mnt/sdcard/log"); //change to root path(mnt/sdcard/log)
			}
		}

	int last_num = 0, pure_num;

	dpdf = opendir(settings.savepath);
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

	sprintf(settings.savepath, "%s/%i", settings.savepath, last_num + 1);
	LOGI(TAG, " savepath: %s\n", settings.savepath);

	//mappath//////////////////////////////////////////////
	len = findnextparam(setfile, buf);
	if(len > 0) {
		if(strncmp(buf, "http", 4) == 0)
			strcpy(settings.mappath, buf);
		else {
			strcpy(settings.mappath, globals.path);
			strcat(settings.mappath, buf);
		}
	}
	LOGI(TAG, " mappath: %s\n", settings.mappath);

	//dempath//////////////////////////////////////////////
	len = findnextparam(setfile, buf);
	strcpy(settings.dempath, globals.path);
	if(len > 0)
		strcat(settings.dempath, buf);
	else
		strcat(settings.dempath, "/map");
	LOGI(TAG, " dempath: %s\n", settings.dempath);

	//---------------------------------------
	if(setfile != NULL)
		fclose(setfile);

	LOGI(TAG, " Read settings.txt file!\n");

	return 0;
}

#ifdef __cplusplus
}
#endif
