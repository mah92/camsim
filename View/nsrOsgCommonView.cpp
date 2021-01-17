#ifndef __ANDROID__

#include"/usr/include/time.h"
#endif

#include "./nsrOsgCommonView.h"
#include "Core/nsrCore.h"

#include "View/nsrOSGUtility.h"
#include "View/sharing_textures.h"

//osg
#include <osg/Node>
#include <osg/Notify>
#include <osgViewer/Viewer>

#undef TAG
#define TAG "Cpp:OsgCommonViewer:"

bool _first_draw_done = false;
static double _first_draw_time = -1;

#ifdef __ANDROID__
osg::ref_ptr<osgViewer::Viewer> _viewer = NULL;
#else
osg::ref_ptr<osgViewer::Viewer> _viewer = _viewer = new osgViewer::Viewer() ;// its Just Miracle
#endif

osg::ref_ptr<osg::Group> _root = NULL;

ImageDrawableParams** prms = NULL;

#ifdef __ANDROID__
#include "View/OsgAndroidNotifyHandler.hpp"
osg::ref_ptr<OsgAndroidNotifyHandler> _notifyHandler = NULL;
#else
class OsgLinuxNotifyHandler : public osg::NotifyHandler
{
public:
	OsgLinuxNotifyHandler() { }
	virtual ~OsgLinuxNotifyHandler() { }
	virtual void notify(osg::NotifySeverity severity, const char* msg)
	{
		LOGI("OsgSelfNotification", " %s\n", msg);
	}
};
#endif

#ifdef __cplusplus
extern "C" {
#endif

void commonOsgInit()
{
	int i;
#ifdef __ANDROID__
	//is flooded with unavoidable errors
	///TODO: use apitrace in android
	/*_notifyHandler = new OsgAndroidNotifyHandler();
	_notifyHandler->setTag("OsgSelfNotification");
	osg::setNotifyHandler(_notifyHandler);
	osg::setNotifyLevel(osg::WARN);*/
#else
	osg::setNotifyHandler(new OsgLinuxNotifyHandler());
	osg::setNotifyLevel(osg::WARN);
	//osg::setNotifyLevel( osg::INFO );//cli is flooded
	//better: use api trace
	//sudo apt-get install apitrace
	//apitrace trace --api gl ./CamSim
	//qapitrace CamSim.2.trace
#endif

	prms = new ImageDrawableParams*[MAX_GRAPHICAL_PARAM_COUNT];
	for(i = 0; i < MAX_GRAPHICAL_PARAM_COUNT; i++)
		prms[i] = new ImageDrawableParams();

	LOGI(TAG, " Osg init done...\n");
}

ImageDrawableParamsReader* commonOsgInitParams(std::string filename)
{
	return new ImageDrawableParamsReader((std::string(globals.datapath) + filename).c_str());
}

void commonOsgInitOsgWindow1(int x, int y, int _screen_width, int _screen_height)
{
	LOGI(TAG, " Osg init 1...\n");

	share_textures_init();
#ifdef __ANDROID__
	_viewer = new osgViewer::Viewer();// each view has a separate draw() func, smaller multiple views were tested successfully...
	_viewer->setUpViewerAsEmbeddedInWindow(x, y, _screen_width, _screen_height);
#endif

#ifndef __ANDROID__
	_viewer->setThreadingModel(osgViewer::ViewerBase::SingleThreaded);
#endif
	//viewer::setUpViewerAsEmbeddedInWindow:once
	//view::setUpViewInWindow
	//camera::setViewport
	//_viewer->setThreadingModel(osgViewer::ViewerBase::ThreadPerCamera);
	_root = new osg::Group;

	//Every Node(so every Group) can be fed to setSceneData()
	_viewer->setSceneData(_root);
}

//x, y: down-left of screen
float commonOsgInitOsgWindowCorrectAR(int &x, int &y, int &_screen_width, int &_screen_height, float dstAR)
{
	//prmReader->correctParametersForAspectRatio((float)_screen_width/(float)_screen_height);

	//correct for AR
	float AR = (float)_screen_width / (float) _screen_height;

	if(dstAR > AR) {
		y += _screen_height * (1 - AR / dstAR) / 2.;
		_screen_height = _screen_height * AR / dstAR;
	}	else	{
		x += _screen_width * (1 - dstAR / AR) / 2.;
		_screen_width = _screen_width * dstAR / AR;
	}

	return dstAR;
}

void commonOsgInitOsgWindow2()
{
	share_textures(_root.get());
	//realize() is equivalent to {return viewer.run}, except that run also implements: {while (!viewer.done()) viewer.frame();}
	_viewer->realize();
	_viewer->getViewerStats()->collectStats("scene", true);

	LOGI(TAG, " Initializing geometry done(%f)!\n", myTime());
}

void commonOsgView()
{
	///////////////////////////////////////////////////////
	_viewer->frame();// at this line all renderings are done, this line takes time...
	/*if (!_viewer->done()) {
		_viewer->advance(USE_REFERENCE_TIME);
		_viewer->eventTraversal();
		_viewer->updateTraversal();
		_LOCKNATIVE(decoderLock,-1);
		_viewer->renderingTraversals();
	    _UNLOCKNATIVE(decoderLock,-1);
	}*/

	if(_first_draw_done == false) {
		LOGI(TAG,  " First draw done(%f)!\n", myTime());
		_first_draw_done = true;
		_first_draw_time = myTime();
	}

}

void commonOsgPause()
{
	_root = NULL;
	_viewer = NULL;

	//_viewer->sync(); // wait for all cull and draw threads to complete before exit.
	LOGI(TAG, " Osg paused!...\n");
}

void commonOsgClose()
{
	int i;

	if(prms != NULL) {
		for(i = 0; i < MAX_GRAPHICAL_PARAM_COUNT; i++)
			delete prms[i];
		delete[] prms;
		prms = NULL;
	}

	LOGI(TAG, " Osg closed!\n");
}

#ifdef __cplusplus
}
#endif
