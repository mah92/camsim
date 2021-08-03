#include"/usr/include/time.h"

#include "nsrCore.h"
#include "View/nsrOsgView.h"
#include "View/nsrOsgCommonView.h"
#include "Visualize/nsrLinuxKeyBoard2.h"
#include "Visualize/nsrPlot.h"
#include "Sim/Sim.h"
#include "Sim/nsrRosInterface.h"
#include "Sim/nsrSimParamReader.h"

#include <X11/Xlib.h> //for getting screen resolution
#include <signal.h>

//for cpu controll//////
#define _GNU_SOURCE
#include <sched.h>
////////////////////////

#undef TAG
#define TAG "cpp:Main: "

#define SCR_DEFAULT_WIDTH 1024
#define SCR_DEFAULT_HEIGHT 768

int scr_width = SCR_DEFAULT_WIDTH,
	scr_height = SCR_DEFAULT_HEIGHT;

void scrAutoDetect()
{
	Display* d = XOpenDisplay(NULL);
	Screen*  s = DefaultScreenOfDisplay(d);
	scr_width = s->width;
	scr_height = s->height;
	if(scr_width == 0 || scr_height == 0)
		printf("\nWarning Screen resolution autodetect failed \nusing default resolutions (1024X768)\ncheck HDMI connections \n");
	XCloseDisplay(d);
}

void intHandler(int sig);

void linuxInit()
{
	signal(SIGINT, intHandler); //ctrl+c

	mySetTime(0);
	scrAutoDetect();

	char root_path[100] = "../../"; //It's relative to currrent build dir
	strcat(root_path, PROJECT_NAME);
	NativeOpen(root_path, myTime());
	setSharedBuffers();

	simInit();
	nsrOsgInit();
	nsrOsgInitOsgWindow(0, 0, scr_width, scr_height);
	LOGI(TAG, " Initialization complete!-\n");
}


static volatile int programInterrupted = 0;

void finish()
{
	LOGDUMP();	
	LOGI(TAG, " Destroyed0!-\n");
    simClose();
	LOGI(TAG, " Destroyed1!-\n");
	nsrOsgPause();
	LOGI(TAG, " Destroyed2!-\n");
	nsrOsgClose();
	LOGI(TAG, " Destroyed3!-\n");
	NativeClose();
	LOGI(TAG, " Destroyed4!-\n");
}

void ask_for_finish()
{
	programInterrupted++;
	if(programInterrupted >=3)
		exit(110);
	LOGW(TAG, " Catched interrupt request for %s time!\n", programInterrupted==1?"1st":"2nd");
}

void intHandler(int sig) //CTRL+C
{
	ask_for_finish();
	//finish();
}

static nsrKeyBoard selfkb;
static int last_read_index = 0;

void exitIOhandler()
{
	uint16_t ch = selfkb.getch(last_read_index);
	if(ch == KEY_ESC || ch == KEY_Q) {
		ask_for_finish();
	}
}

int pauseTrap()
{
	uint16_t ch = selfkb.getch(last_read_index);
	if(ch == KEY_P) {
		ch = 0;
		while(ch != KEY_P && ch != KEY_ESC && ch != KEY_Q) {
			refreshPlotsOnEnd();
			usleep(100000);
			ch = selfkb.getch(last_read_index);
		}
	}
	return 0;
}

int main(int argc, char *argv[])
{
	if(argc >= 2)
		execution_turn = atoi(argv[1]); //can be overriden by the .xml if present

	/*cpu_set_t  mask;
	CPU_ZERO(&mask);
	CPU_SET(1, &mask);
	CPU_SET(2, &mask);
	int result = sched_setaffinity(0, sizeof(mask), &mask);
	*/

	//execution_turn = 0;
	double last_time_s = -1, frame_timestamp_s = -1;
	frame_timestamp_s = nsrPoseMakerGetStartTime() + param_camera_phase_percent * (1. / param_camera_fps); ////first frame start from an offset from 1st row time

	linuxInit();
	while(!_viewer->done() && !programInterrupted) {
		if(time_reached(0.25, 0., frame_timestamp_s, last_time_s) == 1) //dump logs every 0.25 seconds
			LOGDUMP();

		frame_timestamp_s += (1. / param_camera_fps);

		if(simLoop(frame_timestamp_s) < 0) //Loop until next frame
			break;
	
		if(nsrOsgDraw(frame_timestamp_s) < 0) //Render next frame
			break;
		
		pauseTrap();
		exitIOhandler();
		last_time_s = frame_timestamp_s;
	}

	while(programInterrupted == 1) { //Wait for a second interrupt
		exitIOhandler();
		refreshPlotsOnEnd();
	}
	finish();

	return 0;
}
