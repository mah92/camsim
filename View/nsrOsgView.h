#ifndef _NSROSGVIEW_H_
#define _NSROSGVIEW_H_

#ifdef __cplusplus
extern "C" {
#endif

//Initialization function
void nsrOsgInit();
int nsrOsgInitOsgWindow(int x, int y, int screen_width, int screen_height);
//Draw
int nsrOsgDraw();
void nsrOsgPause();
//Finish, just called on program total exit
void nsrOsgClose();

#ifdef __cplusplus
}
#endif

#endif /* _NSROSGVIEW_H_ */
