#ifndef _NSROSGCOMMONVIEW_H_
#define _NSROSGCOMMONVIEW_H_

#include "View/nsrImageDrawableParams.h"

#include <osgViewer/Viewer>

#define MAX_GRAPHICAL_PARAM_COUNT 100

extern bool _first_draw_done;

extern osg::ref_ptr<osgViewer::Viewer> _viewer;
extern osg::ref_ptr<osg::Group> _root;

extern ImageDrawableParams** prms;

#ifndef __ANDROID__
extern osg::ref_ptr<osg::Camera> showCamera;
#endif

#ifdef __cplusplus
extern "C" {
#endif

void commonOsgInit();
ImageDrawableParamsReader* commonOsgInitParams(std::string filename);
void commonOsgInitOsgWindow1(int x, int y, int _screen_width, int _screen_height);
float commonOsgInitOsgWindowCorrectAR(int &x, int &y, int &_screen_width, int &_screen_height, float dstAR);
void commonOsgInitOsgWindow2();
void commonOsgView();
void commonOsgPause();
void commonOsgClose();

#ifdef __cplusplus
}
#endif

#endif /* _NSROSGCOMMONVIEW_H_ */
