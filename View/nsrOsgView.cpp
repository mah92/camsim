/**
 * @file نمایش
 *
 * این فایل وظیفه فراهم سازی محتوای نمایش بر روی صفحه را بر عهده دارد.
 * نمایش به کمک کلاس های گرافیکی نوشته شده در این پروژه انجام می گیرد. این کلاس ها بر پایه کتابخانه
 * OSG(Open Scene Graph)
 * نوشته شده اند. این کتابخانه یک موتور گرافیکی و بازی سازی محبوب است که خود بر مبنای کتابخانه معروف
 * OpenGL(Open Graphics Library)
 * نوشته شده است.
 *
 * @since 1395
 */
#ifndef __ANDROID__
#include "/usr/include/time.h"
#endif

#include "Core/nsrCore.h"

#include "View/nsrOsgView.h"
#include "./nsrOsgView2.h"
#include "View/nsrImageDrawable.h"
#include "View/nsrOSGUtility.h"
#include "View/nsrOsgCommonView.h"

#include "nsrSimParamReader.h"
#include "nsrImageProc.h"

#include "Sim/Sim.h"

#include <osgDB/WriteFile>

#include "Sim/nsrRosInterface.h"

#undef TAG
#define TAG "Cpp:OsgViewer:"

osg::ref_ptr<CamSimScene> camSimScene1 = NULL;

osg::ref_ptr<osg::Camera> saveCamera = NULL;
osg::ref_ptr<osg::Camera> showCamera = NULL;

osg::ref_ptr<osg::Texture2D> saveTexture;//, shotTexture;

TickRateData renderTickRate;

#ifdef __cplusplus
extern "C" {
#endif

int sn;
osg::Vec2 *showScrDim, *fullScrDim, *saveScrDim;

//get video callback/////////////////////////////////////////////

int shotindex = 0;
static double _frame_timestamp_s = 0;
class ReadBufferCallback: public osg::Camera::DrawCallback
{
public:
	char address[300];

	osg::Texture2D* tex;
	osg::Image* image;
	osg::FrameBufferObject *fbo, *fbo2;
	osg::FrameBufferAttachment *fba, *fba2;
	int height;
	int width;
	ReadBufferCallback(osg::Texture2D *_tex)
	{
		tex = _tex;
		height = _tex->getTextureHeight();
		width = _tex->getTextureWidth();
		image = new osg::Image;
		fbo = new osg::FrameBufferObject();
		fba = new osg::FrameBufferAttachment(tex);
		fbo->setAttachment(osg::Camera::COLOR_BUFFER, *fba);//osg::Camera::DEPTH_BUFFER

		fbo2 = new osg::FrameBufferObject();
		fba2 = new osg::FrameBufferAttachment(tex);
		fbo2->setAttachment(osg::Camera::COLOR_BUFFER, *fba2);

		if(param_do_what == DO_IMAGE_PROC)
			nsrInitImageProc();
	}

	~ReadBufferCallback()
	{
		if(param_do_what == DO_IMAGE_PROC)
			nsrEndImageProc();
	}

	virtual void operator()(osg::RenderInfo &renderInfo) const
	{
		static int i = 0;
		i++;

		// very slow, 1fps, 2133x1600=3.412 M pixels = 13.648 MBs!!!
		//osg::ref_ptr<osg::Image> saveImage = new osg::Image();
		//saveImage->allocateImage(screen_width, screen_height, 32, GL_RGBA, GL_UNSIGNED_BYTE);
		//showCamera->attach(osg::Camera::COLOR_BUFFER, saveImage.get());

		//either double fbo or real data copy should be done to prevent slicing
		//if(i%2 == 1)
		fbo->apply(*(renderInfo.getState()), osg::FrameBufferObject::READ_FRAMEBUFFER);
		//else
		//	fbo2->apply(*(renderInfo.getState()), osg::FrameBufferObject::READ_FRAMEBUFFER);

		//reading GL_RGB is a quarter faster but is not supported everywhere(in phones)
        if(param_render_what == RENDER_LUMINANCE)
            image->readPixels(0, 0, width, height, GL_LUMINANCE, GL_UNSIGNED_BYTE);  //GL_LUMINANCE, GL_FLOAT);
        else    
            image->readPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE);  //GL_LUMINANCE, GL_FLOAT);
        
        int chans = (param_render_what==RENDER_LUMINANCE)? 1 : 4;

		if(param_do_what == DO_SAVE_BMP) {
			sprintf((char*)address, "%s/screenshot%05i.bmp", globals.savepath, shotindex);
			osgDB::writeImageFile(*image, std::string(address));
			//bmp bad resolution, works with both GL_RGB & GL_RGBA, 1.5GB for 30fpsx60s, 7fps
			//jpeg just works with GL_RGB in both image and save texture, 380MB for 30fpsx60s, 4fps, rms2bmp: 1.95
			//png saves blank screen
			//gif saves nothing
			//tiff causes lockup
			shotindex++;
		}

		if(param_do_what == DO_IMAGE_PROC) {
			image->flipVertical();//5 fps drop!
            nsrImageProc(image->data(), width, height, chans, _frame_timestamp_s);
		}
		
		if(param_do_what == DO_SAVE_ROS_BAG) {
            image->flipVertical();//5 fps drop!!
            registerRosImage(_frame_timestamp_s, image->data(), width, height, width*chans, chans);
            registerRosCamInfo(_frame_timestamp_s);
        }
	}
};

///////////////////////////////////////////////////////

void nsrOsgInit()
{
	commonOsgInit();
	camSimScene1 = new CamSimScene();
}

//Initialization function
//Also called onResume()
//help: for transformations: x: right, y: up, z:towards eye
//x, y: down-left of screen
int nsrOsgInitOsgWindow(int x, int y, int _full_screen_width, int _full_screen_height)
{
	///Init sizes///////////////////////////////////////////////
	int _show_screen_width, _show_screen_height;
	fullScrDim = new osg::Vec2(_full_screen_width, _full_screen_height);
	commonOsgInitOsgWindow1(x, y, fullScrDim->x(), fullScrDim->y());
	LOGI(TAG, " Initial full geometry...origin(%ix%i)...size(%ix%i)\n", x, y, (int)fullScrDim->x(), (int)fullScrDim->y());

	camSimScene1->prmReader->findParamByName("FullView", prms[0]);
	//prms[0]->posX-=1;//1 screen left
	//prms[0]->width*=2; prms[0]->height*=1;
	_show_screen_width = _full_screen_width; _show_screen_height = _full_screen_height;
	commonOsgInitOsgWindowCorrectAR(x, y, _show_screen_width, _show_screen_height, prms[0]->dstAR);
	showScrDim = new osg::Vec2(_show_screen_width, _show_screen_height);
	LOGI(TAG, " Corrected geometry...origin(%ix%i)...size(%ix%i)\n", x, y, (int)showScrDim->x(), (int)showScrDim->y());

	//4/3 AR: 576x432, 768x576, 1024x768
	//16/9 AR: 640x360, 720x405, 848x480, 960x540, 1024x576, 1280x720, 1366x768

	saveScrDim = new osg::Vec2(param_width, param_height);
	LOGI(TAG, " Save dimension:(%ix%i)\n", (int)saveScrDim->x(), (int)saveScrDim->y());

	///Init main scene graph///////////////////////////////////////////////
	if(camSimScene1 != NULL) {
		camSimScene1->setPosition(osg::Vec3d(0, 0, 0));
		camSimScene1->InitOsgWindow(0, 0, showScrDim->x(), showScrDim->y());
	}

	///Init show camera///////////////////////////////////////////////////
	if(execution_turn < 0) { //disable if execution_turn >=0
		showCamera = createCamera(true, true); //draw_on_others, transparent
		showCamera->setViewport(x, y, showScrDim->x(), showScrDim->y());
		showCamera->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //needed for showCamera to show sky after looking at earth
		showCamera->setClearColor(osg::Vec4(0.0f, 0.0f, 0.0f, 1.0f)); //black, cleared by setClearColor
		_root->addChild(showCamera);

		if(camSimScene1 != NULL)
			showCamera->addChild(camSimScene1);

		setCameraToViewForm(showCamera.get(), prms, showScrDim);
	}

	///Init save camera///////////////////////////////////////////////////
	saveCamera = createCamera(true, true); //draw_on_others, transparent
	saveCamera->setViewport(0, 0, saveScrDim->x(), saveScrDim->y());
	saveCamera->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);//needed for saveCamera to show sky after looking at earth
	saveCamera->setClearColor(osg::Vec4(0.0f, 0.0f, 0.0f, 1.0f)); //black, cleared by setClearColor
	//saveCamera->addChild(showCamera); //add saveCamera childs as a group, 3fps speed up
	_root->addChild(saveCamera);

	if(camSimScene1 != NULL)
		saveCamera->addChild(camSimScene1);

	setCameraToViewForm(saveCamera.get(), prms, showScrDim);

	///Attach textue
	saveTexture = new osg::Texture2D;
	saveTexture->setTextureSize(saveScrDim->x(), saveScrDim->y());
    
    if(param_render_what == RENDER_LUMINANCE)
        saveTexture->setInternalFormat(GL_LUMINANCE);
    else
        saveTexture->setInternalFormat(GL_RGBA); //GL_RGB is three/forth faster but is not supported in all phones...
	saveTexture->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
	saveTexture->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);

	saveCamera->attach(osg::Camera::COLOR_BUFFER, saveTexture.get(), 0, 0, false, 0, 0);//works, high speed
	//osg::Camera::FRAME_BUFFER_OBJECT hides camera output and causes higher speed(at least 3 fps), other options has no effect
	saveCamera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT); //FRAME_BUFFER_OBJECT(the only one that has effect), PIXEL_BUFFER_RTT, PIXEL_BUFFER, FRAME_BUFFER
	saveCamera->setFinalDrawCallback(new ReadBufferCallback(saveTexture));
    
	initTickRate(&renderTickRate);

	commonOsgInitOsgWindow2();

	LOGI(TAG, " Initialized scenes!\n");
    return 0;
}

int nsrOsgDraw(double frame_timestamp_s)
{
	if(_viewer == NULL)
		return -1;

	_frame_timestamp_s = frame_timestamp_s; //keep last time for use in above callback
	
	//LOGI(TAG, "Frame Time:%f\n", frame_timestamp_s);
	if(camSimScene1 != NULL) camSimScene1->Draw(frame_timestamp_s);

	_LOCKCPP(Z_lock, -1);
	cbPush(mZB, n.Z.RATE, tickRate(&renderTickRate, 1), myTime());
	_UNLOCKCPP(Z_lock, -1);

	commonOsgView();

	//after return, image is generated,
	//  and a callback is called which calls nsrInitImageProc()

	return 0;
}

void nsrOsgPause()
{
	LOGI(TAG, "osg Pause1\n");
	
	if(saveCamera != NULL) saveCamera->detach(osg::Camera::COLOR_BUFFER);
	if(camSimScene1 != NULL) camSimScene1->Pause();

	commonOsgPause();
	saveCamera = NULL;
	showCamera = NULL;
	
	LOGI(TAG, "osg Pause2\n");
}

void nsrOsgClose()
{
	//every global ref_ptr should also be dereferenced by itself
	//if defined locally, ref_ptr will be dereferenced in the local scope but not available here

	//LOGI(TAG, "*close11(%i)", camSimScene1.valid()?camSimScene1->referenceCount():0);
	camSimScene1 = NULL;
	//LOGI(TAG, "*close12(%i)", camSimScene1.valid()?camSimScene1->referenceCount():0);

	commonOsgClose();
}

#ifdef __cplusplus
}
#endif
