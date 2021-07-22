#include"/usr/include/time.h"

#include "nsrCore.h"
#include "View/nsrOsgView.h"
#include "View/nsrOsgCommonView.h"
#include "Visualize/nsrLinuxKeyBoard2.h"
#include "Visualize/nsrPlot.h"
#include "Sim/nsrRosInterface.h"

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

static volatile int programInterrupted = 0;

void finish()
{
    rosClose();

	nsrOsgPause();
	nsrOsgClose();
	LOGI(TAG, " 40 Closing All..\n");
	LOGDUMP();
	NativeClose();
	LOGI(TAG, " Destroyed!-\n");
    printf(" Destroyed!-\n");
}

void intHandler(int sig)
{
	programInterrupted = 1;
	LOGW(TAG, " Catched interrupt request(%i)!\n", sig);
	finish();
}

void linuxInit()
{
	signal(SIGINT, intHandler); //ctrl+c

	mySetTime(0);
	scrAutoDetect();

	char root_path[100] = "../../"; //It's relative to currrent build dir
	strcat(root_path, PROJECT_NAME);
	NativeOpen(root_path, myTime());
	setSharedBuffers();

	LOGI(TAG, " Initialization complete!-\n");

	nsrOsgInit();
	nsrOsgInitOsgWindow(0, 0, scr_width, scr_height);
}

static nsrKeyBoard selfkb;
static int last_read_index = 0;

void IOhandler()
{
	uint16_t ch = selfkb.getch(last_read_index);
	if(ch == KEY_ESC || ch == KEY_Q || (selfkb.ctrl_pressed && ch == KEY_C)) {
		LOGW(TAG, " Cached custom interrupt request!\n");
		programInterrupted = 1;
	}
}

int main(int argc, char *argv[])
{
	if(argc >= 2)
		execution_turn = atoi(argv[1]);

	/*cpu_set_t  mask;
	CPU_ZERO(&mask);
	CPU_SET(1, &mask);
	CPU_SET(2, &mask);
	int result = sched_setaffinity(0, sizeof(mask), &mask);
	*/

	//execution_turn = 0;
	double time_s = 0, last_time_s = -1;
	linuxInit();
	while(!_viewer->done() && !programInterrupted) {
		time_s = myTime();
		if(time_reached(0.25, 0., time_s, last_time_s) == 1)
			LOGDUMP();

		IOhandler();
		nsrOsgDraw();
		last_time_s = time_s;
	}

	refreshPlotsOnEnd();
	finish();

	return 0;
}
