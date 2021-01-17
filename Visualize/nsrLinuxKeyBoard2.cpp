//ALLAH
/**
 * SDL, NCURSES & Boost not used, because of odroid support problem
 * ref: https://github.com/kernc/logkeys
 */

#ifdef __linux__
#include"/usr/include/time.h"
#endif

#include "nsrLinuxKeyBoard2.h"

#undef TAG
#define TAG "cpp:KeyBoard2"

///////////////////////
#include <sstream> //stringstream
//#include <cerrno>
#include <assert.h>
#include <error.h>
#include <vector>
#include <string.h> //strcpy
#include <cstdlib> //atoi
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <linux/input.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/time.h>
#include <termios.h>

#include <stdint.h>

void perror_exit(const char *error_str)
{
	printf("%s\n", error_str);
	exit(0);
}

#define INPUT_EVENT_PATH "/dev/input/" // standard path
#define EXE_GREP "/bin/grep"
#define EXIT_FAILURE 1
// executes cmd and returns string ouput
std::string execute(const char* cmd)
{
	FILE* pipe = popen(cmd, "r");
	if(!pipe)
		perror_exit(" Pipe error\n");
	char buffer[128];
	std::string result = "";
	while(!feof(pipe))
		if(fgets(buffer, 128, pipe) != NULL)
			result += buffer;
	pclose(pipe);
	return result;
}

void determine_input_device(char * keyboard_device)
{
	// better be safe than sory: while running other programs, switch user to nobody
	//if(seteuid(65534) != 0 || setegid(65534) != 0)
	;//perror_exit(" Unable to set user as unknown...\n");
	// extract input number from /proc/bus/input/devices (I don't know how to do it better. If you have an idea, please let me know.)
	// The compiler automatically concatenates these adjacent strings to a single string.
	const char* cmd = EXE_GREP " -E 'Handlers|EV=' /proc/bus/input/devices | "
					  EXE_GREP " -B1 'EV=1[02]001[3Ff]' | "
					  EXE_GREP " -Eo 'event[0-9]+' ";
	std::stringstream output(execute(cmd));

	std::vector<std::string> results;
	std::string line;

	while(std::getline(output, line)) {
		std::string::size_type i = line.find("event");
		if(i != std::string::npos) i += 5;  // "event".size() == 5
		if(i < line.size()) {
			int index = atoi(&line.c_str()[i]);

			if(index != -1) {
				std::stringstream input_dev_path;
				input_dev_path << INPUT_EVENT_PATH;
				input_dev_path << "event";
				input_dev_path << index;

				results.push_back(input_dev_path.str());
			}
		}
	}

	if(results.size() == 0) {
		perror_exit("Couldn't determine keyboard device. :/\n");
		perror_exit("Please post contents of your /proc/bus/input/devices file as a new bug report. Thanks!\n");
	}

	strcpy(keyboard_device, results[0].c_str());  // for now, use only the first found device

	// now we reclaim those root privileges
	if(seteuid(0) != 0 || setegid(0) != 0)
		printf("Couldn't set user to root :/\n");
}

/*inline int to_char_keys_index(unsigned int keycode)
{
  if (keycode >= KEY_1 && keycode <= KEY_EQUAL)  // keycodes 2-13: US keyboard: 1, 2, ..., 0, -, =
	return keycode - 2;
  if (keycode >= KEY_Q && keycode <= KEY_RIGHTBRACE)  // keycodes 16-27: q, w, ..., [, ]
	return keycode - 4;
  if (keycode >= KEY_A && keycode <= KEY_GRAVE)  // keycodes 30-41: a, s, ..., ', `
	return keycode - 6;
  if (keycode >= KEY_BACKSLASH && keycode <= KEY_SLASH)  // keycodes 43-53: \, z, ..., ., /
	return keycode - 7;

  if (keycode == KEY_102ND) return 47;  // key right to the left of 'Z' on US layout

  return -1;  // not character keycode
}*/

/////////////////////////////////
int nsrKeyBoard::active = 0;
uint16_t nsrKeyBoard::ch[KEY_BUF_LEN] = {0};
int nsrKeyBoard::read_chars = 0;
int nsrKeyBoard::fd = 0;
bool nsrKeyBoard::shift_pressed = false;
bool nsrKeyBoard::ctrl_pressed = false;
bool nsrKeyBoard::alt_pressed = false;
bool nsrKeyBoard::space_pressed = false;

double nsrKeyBoard::shift_key_pressed_time_s = 0;
double nsrKeyBoard::ctrl_key_pressed_time_s = 0;
double nsrKeyBoard::alt_key_pressed_time_s = 0;
double nsrKeyBoard::space_key_pressed_time_s = 0;

nsrKeyBoard::nsrKeyBoard()
{
	active++;
	if(active == 1) {
		read_chars = 0;

		char name[256] = "Unknown";
		char keyboard_device[200];// = "/dev/input/event4";

		if((getuid()) != 0)
			printf("You are not root! This may not work...\n");

		determine_input_device(keyboard_device);
		printf("Keyboard device:%s...\n", keyboard_device);

		//Open Device
		if((fd = open(keyboard_device, O_RDONLY | O_NONBLOCK)) == -1)
			printf("%s is not a valid device.\n", keyboard_device);

		//Print Device Name
		ioctl(fd, EVIOCGNAME(sizeof(name)), name);
		printf("Reading From : %s (%s)\n", keyboard_device, name);

		printf("Keyboard inited\n");
	}
}

uint16_t nsrKeyBoard::fullgetch()
{
	struct input_event ev[64];
	int rd, value, size = sizeof(struct input_event);
	double time_s = clock() / (double)CLOCKS_PER_SEC;

	if(fd == 0)
		return 0;

	//No key input event
	if((rd = read(fd, ev, size * 64)) >= size) {

		value = ev[0].value;

#define EV_MAKE   1  // when key pressed
#define EV_BREAK  0  // when key released
#define EV_REPEAT 2  // when key switches to repeating after short delay

		if(value != ' ' && ev[1].type == EV_KEY) {
			if(ev[1].value == EV_BREAK) { //key up event
				if(ev[1].code == KEY_LEFTSHIFT || ev[1].code == KEY_RIGHTSHIFT)
					shift_pressed = false;
				if(ev[1].code == KEY_LEFTCTRL || ev[1].code == KEY_RIGHTCTRL)
					ctrl_pressed = false;
				if(ev[1].code == KEY_LEFTALT || ev[1].code == KEY_RIGHTALT) {
					alt_pressed = false;
					//printf("AltUp\n");
				}
				if(ev[1].code == KEY_SPACE)
					space_pressed = false;
			}

			if(ev[1].value == EV_MAKE || ev[1].value == EV_REPEAT) { // key press or repeat event
				if(ev[1].code == KEY_LEFTSHIFT || ev[1].code == KEY_RIGHTSHIFT) {
					shift_key_pressed_time_s = time_s;
					shift_pressed = true;
				}
				if(ev[1].code == KEY_LEFTCTRL || ev[1].code == KEY_RIGHTCTRL) {
					ctrl_key_pressed_time_s = time_s;
					ctrl_pressed = true;
				}
				if(ev[1].code == KEY_LEFTALT || ev[1].code == KEY_RIGHTALT) {
					//printf("AltPress\n");
					alt_key_pressed_time_s = time_s;
					alt_pressed = true;
				}
				if(ev[1].code == KEY_SPACE) {
					space_key_pressed_time_s = time_s;
					space_pressed = true;
				}

				//printf ("Code(%i)[%d]\n", read_chars, (ev[1].code)); fflush(stdout);
				return ev[1].code;
			}
		}
	}

	//In case we lose key up event
	//Not works, shift/ctrl/alt/space pressed event comes periodically unless another key is pressed/unpressed
	/*if(shift_pressed && time_s - shift_key_pressed_time_s > 0.4)
		shift_pressed = false;
	if(ctrl_pressed && time_s - ctrl_key_pressed_time_s > 0.4)
		ctrl_pressed = false;
	if(alt_pressed && time_s - alt_key_pressed_time_s > 0.4) {
		printf("AltTimeout\n");
		alt_pressed = false;
	}
	if(space_pressed && time_s - space_key_pressed_time_s > 0.4)
		space_pressed = false;
	*/

	//printf("Alt:%i\n", alt_pressed?1:0);
	return 0;
}
uint16_t nsrKeyBoard::getch(int &last_read_times)
{

	if(last_read_times < read_chars - KEY_BUF_LEN)
		last_read_times = read_chars - KEY_BUF_LEN;
	assert(last_read_times >= 0);
	assert(last_read_times <= read_chars);

	if(last_read_times < read_chars) {
		last_read_times++;
		return ch[(last_read_times - 1) % KEY_BUF_LEN];
	}

	//Reader is up-to-date, try to read a new char
	uint16_t temp_char;
	temp_char = fullgetch();
	if(temp_char != 0) {
		ch[(read_chars) % KEY_BUF_LEN] = temp_char;
		read_chars++;
		last_read_times = read_chars;
		return temp_char;
	}

	//no new character
	return 0;
}

nsrKeyBoard::~nsrKeyBoard()
{
	active--;
	if(active == 0) {

		printf("Keyboard disabled\n");
	}
}

nsrKeyBoard selfkb;
static int last_read_index = 0;
int waitKey(int total_wait_ms)
{
	int wait_ms = 0;
	uint16_t ch = 0;

	while(total_wait_ms == 0 || wait_ms < total_wait_ms) {
		ch = selfkb.getch(last_read_index);
		if(ch == KEY_ESC || ch == KEY_Q) {
			printf("got!\n"); fflush(stdout);
			return -1;
		}
		usleep(1000);
		wait_ms++;
	}

	return 0;
}
