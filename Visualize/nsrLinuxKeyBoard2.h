#ifndef __NSRLINUXKEYBOARD2_H__
#define __NSRLINUXKEYBOARD2_H__

#include <stdint.h>
#include <linux/input.h>

class nsrKeyBoard
{
private:

#define KEY_BUF_LEN 10
	static int active;
	static uint16_t ch[KEY_BUF_LEN];
	static int fd;
	static double shift_key_pressed_time_s, ctrl_key_pressed_time_s, alt_key_pressed_time_s, space_key_pressed_time_s;
	uint16_t fullgetch();

public:
	nsrKeyBoard();
	static int read_chars;

	uint16_t getch(int &last_read_times);

	static bool shift_pressed;
	static bool ctrl_pressed;
	static bool alt_pressed;
	static bool space_pressed;

	~nsrKeyBoard();
};

//returns -1 on 'q' or esc pressed
int waitKey(int wait_ms);

#endif
