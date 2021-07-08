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

#include "./nsrOsgCamSimScene.h"

#include "Core/nsrCore.h"
//#include "Image/nsrImage.h"

#include "View/nsrOSGUtility.h"
#include "View/nsrOsgCommonView.h"
#include "View/nsrGeoLib.h"

#include "./nsrOsgView2.h"
#include "View/nsrOSGShaders.h"

#include "Sim/Sim.h"

#include <opencv2/highgui/highgui.hpp>

#undef TAG
#define TAG "Cpp:CamSimScene:"
#ifdef __cplusplus
extern "C" {
#endif

////////////////////////////////////////

///Color Camera Textures///////////////////////////
void createTextureCamera(osg::ref_ptr<osg::Camera> &cam, osg::ref_ptr<osg::Texture2D> &texture,
						 int x, int y, int width, int height, double AR, double fovy, const char* name, int render_depth, int extra_param = 0)
{
	cam = createCamera(false, false); //draw_on_others, transparent

	cam->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	cam->setClearColor(osg::Vec4(1., 1., 1., 0.)); //transparent white as infinity is easily recognized by the shader

	cam->setClearDepth(1.0);
	cam->setViewport(x, y, width, height);

	cam->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR); //used to make setProjectionMatrix effective in setting far, near..., far&near used for normalizing DEPTH_BUFFER
	cam->setProjectionMatrix(osg::Matrix::perspective(
								 fovy,//double fovy(deg), calculated for height not width
								 AR,//double aspectRatio,
								 10.,//double zNear, //is really effective, both on camera & on depth
								 65e3)); //double zFar, //more than 65e3 is not supported in depth camera output

	osg::ref_ptr<osg::StateSet> ss = cam->getOrCreateStateSet();
	osg::ref_ptr<osg::Program> program = new osg::Program;

	if(extra_param >= 0) {
		if(render_depth == 0) {
			program->addShader(new osg::Shader(osg::Shader::VERTEX, imageVertexShader));
			program->addShader(new osg::Shader(osg::Shader::FRAGMENT, imageFragmentShader));
		} else {
			program->addShader(new osg::Shader(osg::Shader::VERTEX, depthVertexShader));
			program->addShader(new osg::Shader(osg::Shader::FRAGMENT, depthFragmentShader));
		}
	} else {
		program->addShader(new osg::Shader(osg::Shader::VERTEX, whiteVertexShader));
		program->addShader(new osg::Shader(osg::Shader::FRAGMENT, whiteFragmentShader));
	}

	ss->setAttributeAndModes(program.get(), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE | osg::StateAttribute::PROTECTED);

	// tell the camera to use OpenGL frame buffer object where supported.
	cam->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT, osg::Camera::FRAME_BUFFER);

	texture = new osg::Texture2D;
	texture->setTextureSize(width, height);
	texture->setInternalFormat(GL_RGBA);
	if(render_depth == 0) {
		texture->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
		texture->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);
	} else {
		//so that depth channels(bytes) is not merged in edges
		texture->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::NEAREST);
		texture->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::NEAREST);
	}

	//CLAMP_TO_EDGE maybe needed for non size of two textures
	//Practically just for making a difference so that textures are not overrode!!!
	if(extra_param == 1 || extra_param == 3) {
		texture->setWrap(osg::Texture2D::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
		texture->setWrap(osg::Texture2D::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
	}

	//Support for 24-bit or 32-bit depth buffers is driver dependent. Some will fallback on 24-bit if 32-bit is not supported.
	//So we use color buffer for both color and depth
	cam->attach(osg::Camera::COLOR_BUFFER, texture.get()); //works //hides rendering, guides it to texture
}

////////////////////////////////////////

CamSimScene::CamSimScene() : prmReader(NULL),
	idealImageTexture1(NULL), idealImageTexture2(NULL),
	idealDepthTexture1(NULL), idealDepthTexture2(NULL),
	vignetTexture(NULL), distortedDrawable(NULL),
	idealImageCam1(NULL), idealImageCam2(NULL),
	idealDepthCam1(NULL), idealDepthCam2(NULL)
{
	prmReader = commonOsgInitParams("/positions-camsim-v1.xml");
}

//Initialization function
//Also called onResume()
//help: for transformations: x: right, y: up, z:towards eye
//x, y: down-left of screen
int CamSimScene::InitOsgWindow(int x, int y, int _screen_width, int _screen_height)
{
	int i;
	int extended_width, extended_height;
	osg::Vec2 scrDim(_screen_width, _screen_height);

	LOGI(TAG, " Initializing cam scene...\n");

	_viewer->setThreadingModel(osgViewer::ViewerBase::SingleThreaded);
	_viewer->getDatabasePager()->setIncrementalCompileOperation(new osgUtil::IncrementalCompileOperation());
	_viewer->getDatabasePager()->setDoPreCompile(true);
	_viewer->getDatabasePager()->setTargetMaximumNumberOfPageLOD(0);
	_viewer->getDatabasePager()->setUnrefImageDataAfterApplyPolicy(true, true);

#define DOUBLE_FRAME 0
	//Not works as all 5 textures are not transfered to shader correctly

	//Calculating image input camera, with a threshold
	//idealImageCam texture should be available in more resolution & fov than distorted final one
	extended_width = param_width * param_extra_margin;
	extended_height = param_height * param_extra_margin;

	//see http://forum.openscenegraph.org/viewtopic.php?t=7283
	//double fy = (_screen_height/2.)/tan(param_fov_y/2.*M_PI/180.);
	double extended_fovy = 2 * atan((extended_height / 2.) / (param_fy + param_f_err)) * 180. / M_PI;
	double aspectRatio = param_width / (float)param_height * (param_fy + param_f_err) / (param_fx + param_f_err); //used for pixels, checked once experimentally
	LOGE(TAG, " idealExtendedCam (%ix%i), fovy(deg): %f, aspectRatio:%f\n", extended_width, extended_height, extended_fovy, aspectRatio);

	///Maps////////////////////////////////////////////////
	setEarthScale(param_world_scale);
	idealMapDrawable = new MapDrawable2(true);//use dem
	idealMapDrawable->setCoordCenter(param_map_center_lat, param_map_center_lon, 0);

	//x, y, zoom
	if(param_show_what == SHOW_MAP) {
		idealMapDrawable->addSeenMapRegion(param_map_center_lat, param_map_center_lon, param_map_max_alt, param_map_max_dist, param_map_zoom);
	}

	if(param_show_what == SHOW_PATTERN) {
		idealMapDrawable->addImageToMap(param_pattern_file, param_map_center_lat, param_map_center_lon, 0, param_world_scale * param_pattern_metric_width);
	}

	///Color Camera Textures///////////////////////////
	createTextureCamera(idealImageCam1, idealImageTexture1,
						x, y, extended_width, extended_height, aspectRatio, extended_fovy, "ImageShader1", 0, 0);

	if(DOUBLE_FRAME) {
		createTextureCamera(idealImageCam2, idealImageTexture2,
							x, y, extended_width, extended_height, aspectRatio, extended_fovy, "ImageShader2", 0, 1);
	}

	///Depth Camera Textures//////////////////
	createTextureCamera(idealDepthCam1, idealDepthTexture1,
						x, y, extended_width, extended_height, aspectRatio, extended_fovy, "DepthShader1", 1, 2);

	if(DOUBLE_FRAME) {
		createTextureCamera(idealDepthCam2, idealDepthTexture2,
							x, y, extended_width, extended_height, aspectRatio, extended_fovy, "DepthShader2", 1, 3);
	}

	idealImageCam1->addChild(idealMapDrawable);
	if(DOUBLE_FRAME) idealImageCam2->addChild(idealMapDrawable);
	idealDepthCam1->addChild(idealMapDrawable);
	if(DOUBLE_FRAME) idealDepthCam2->addChild(idealMapDrawable);

	//DEBUG
	/*if(DOUBLE_FRAME) {
		idealImageCam2->setClearColor(osg::Vec4(1.,0.,0., 1.)); //white
		idealDepthCam2->setClearColor(osg::Vec4(1.,0.,0., 1.)); //white
	}*/

	_root->addChild(idealImageCam1); //so that the camera is rendered
	if(DOUBLE_FRAME) _root->addChild(idealImageCam2); //so that the camera is rendered
	_root->addChild(idealDepthCam1); //so that the camera is rendered
	if(DOUBLE_FRAME) _root->addChild(idealDepthCam2); //so that the camera is rendered

	/// Static Vignet Texture//////////////////////////////////
	{
		vignetTexture = new osg::Texture2D;
		vignetTexture->setTextureSize(param_width, param_height);
		vignetTexture->setInternalFormat(GL_RGB);
		vignetTexture->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
		vignetTexture->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);

		osg::ref_ptr<osg::Image> pImage;
		if(strlen(param_vignet_file) != 0) {
			std::string vignet_path(param_vignet_file);
			pImage = osgDB::readImageFile(vignet_path);
			if(!pImage.valid())
				LOGE(TAG, " Couldn't find vignet.bmp!\n");
		}

		if(!pImage.valid()) {
			LOGI(TAG, " Creating Empty Vignet!\n");
			pImage = new osg::Image();
			unsigned char *data = new unsigned char[3 * param_width * param_height];
			memset(data, 0xff, 3 * param_width * param_height);
			pImage->setImage(param_width, param_height,
							 3, //channels num
							 GL_RGB,//Specifies the internal format of the texture, values:GL_ALPHA, GL_LUMINANCE, GL_LUMINANCE_ALPHA, GL_RGB, GL_RGBA.
							 GL_RGB,//Must match internalformat.
							 GL_UNSIGNED_BYTE,//Specifies the data type(depth) of the each texel data. values: GL_UNSIGNED_BYTE(8UCX), GL_UNSIGNED_SHORT(16UCX), GL_FLOAT(32UCX)
							 data,
							 osg::Image::USE_NEW_DELETE,//NO_DELETE,USE_NEW_DELETE,USE_MALLOC_FREE
							 1);
		}

		vignetTexture->setImage(pImage.get());
	}

	///////////////////////////////////////////////////////////////
	distortedDrawable = new ImageDrawable();
	prmReader->findParamByName("FullView", prms[0]);

#define RENDER_DEBUG 0
#if RENDER_DEBUG == 1
	distortedDrawable->setImageFromTexture(prms[0], &scrDim, idealImageTexture1.get()); //works, if uniform setting is disabled in draw
#elif RENDER_DEBUG == 2
	distortedDrawable->setImageFromTexture(prms[0], &scrDim, idealDepthTexture1.get());
#elif RENDER_DEBUG == 3
	distortedDrawable->setImageFromTexture(prms[0], &scrDim, idealImageTexture2.get());
#elif RENDER_DEBUG == 4
	distortedDrawable->setImageFromTexture(prms[0], &scrDim, idealDepthTexture2.get());
#elif RENDER_DEBUG == 5
	distortedDrawable->setImageFromTexture(prms[0], &scrDim, vignetTexture.get());
#elif RENDER_DEBUG == 0
	if(!DOUBLE_FRAME) {
		distortedDrawable->setLensFromTexture1(prms[0], &scrDim,
											   idealImageTexture1.get(),
											   idealDepthTexture1.get(),
											   vignetTexture.get(), param_render_what == RENDER_DEPTH ? true : false);
	} else {
		distortedDrawable->setLensFromTexture2(prms[0], &scrDim,
											   idealImageTexture1.get(), idealImageTexture2.get(),
											   idealDepthTexture1.get(), idealDepthTexture2.get(),
											   vignetTexture.get(), param_render_what == RENDER_DEPTH ? true : false);
	}

#endif
	this->addChild(distortedDrawable);

#if RENDER_DEBUG == 0
	distortedDrawable->inVecCallback->setElement(UNIFORM_LENS_ITERATIONS, param_max_lens_iterations);
	distortedDrawable->inVecCallback->setElement(UNIFORM_PIX_ERR, param_max_pix_error);

	distortedDrawable->inVecCallback->setElement(UNIFORM_K1, param_k1 + param_k1_err);
	distortedDrawable->inVecCallback->setElement(UNIFORM_K2, param_k2 + param_k2_err);
	distortedDrawable->inVecCallback->setElement(UNIFORM_T1, param_t1 + param_t1_err);
	distortedDrawable->inVecCallback->setElement(UNIFORM_T2, param_t2 + param_t2_err);
	distortedDrawable->inVecCallback->setElement(UNIFORM_K3, param_k3 + param_k3_err);

	distortedDrawable->inVecCallback->setElement(UNIFORM_K4, param_k4);
	distortedDrawable->inVecCallback->setElement(UNIFORM_K5, param_k5);
	distortedDrawable->inVecCallback->setElement(UNIFORM_K6, param_k6);

	distortedDrawable->inVecCallback->setElement(UNIFORM_VIGNET_THRESH1, param_vignet_thresh1);
	distortedDrawable->inVecCallback->setElement(UNIFORM_VIGNET_THRESH2, param_vignet_thresh2);
	distortedDrawable->inVecCallback->setElement(UNIFORM_TD, param_td + param_td_err); //neutralized in shader itself
	distortedDrawable->inVecCallback->setElement(UNIFORM_TR, param_tr + param_tr_err);
	distortedDrawable->inVecCallback->setElement(UNIFORM_TE, param_te);
	//distortedDrawable->inVecCallback->setElement(UNIFORM_SAMPLING_POINTS, 2.);
	distortedDrawable->inVecCallback->setElement(UNIFORM_BLUR_EXTRA_SAMPLES, param_motion_blur_extra_samples);
	distortedDrawable->inVecCallback->setElement(UNIFORM_TI, param_ti);

	distortedDrawable->inVecCallback->setElement(UNIFORM_WIDTH, param_width);
	distortedDrawable->inVecCallback->setElement(UNIFORM_HEIGHT, param_height);

	distortedDrawable->inVecCallback->setElement(UNIFORM_FX, param_fx + param_f_err);
	distortedDrawable->inVecCallback->setElement(UNIFORM_FY, param_fy + param_f_err);
	distortedDrawable->inVecCallback->setElement(UNIFORM_OX, param_ox + param_ox_err);
	distortedDrawable->inVecCallback->setElement(UNIFORM_OY, param_oy + param_oy_err);

	distortedDrawable->inVecCallback->setElement(UNIFORM_EXTRA_MARGIN, param_extra_margin);
	distortedDrawable->inVecCallback->setElement(UNIFORM_EXTRA_ZOOM, param_extra_zoom);

	distortedDrawable->inVecCallback->setElement(UNIFORM_DAY_LIGHT, param_day_light);
	distortedDrawable->inVecCallback->setElement(UNIFORM_NOISE_AMPLITUDE_DYNAMIC, param_noise_amp_dynamic);
	distortedDrawable->inVecCallback->setElement(UNIFORM_NOISE_AMPLITUDE_STATIC1, param_noise_amp_static1);
	distortedDrawable->inVecCallback->setElement(UNIFORM_NOISE_AMPLITUDE_STATIC2, param_noise_amp_static2);
	distortedDrawable->inVecCallback->setElement(UNIFORM_MAX_FOG_DISTANCE, param_max_fog_distance);
#endif

	/*osgUtil::Optimizer optimizer;
	//optimizer.optimize( root.get() );
	//optimizer.optimize( idealMapDrawable.get() );//has bad effect!!!
	optimizer.optimize( this );//has bad effect!!!
	 */

	LOGI(TAG, " Added earth scenegraph...\n");
    return 0;
}

int CamSimScene::Draw(double frame_timestamp_s)
{
	osg::Vec3d lla1, lla2, lla3;
	osg::Quat camInNedQu1, camInNedQu2, camInNedQu3;
	osg::Vec3d w_cam1, v_cam1, w_cam2, v_cam2, w_cam3, v_cam3;

	//Frame Timings
	//min: -td - 0.5*tr - ti - 0.5*te
	//max: -td + 0.5*tr - 0. + 0.5*te
	//mean:-td - 0.5*ti
	//deviation: 0.5*(tr + te + ti)

	//at mean time
	nsrPoseMakerExtract(frame_timestamp_s - (param_td + param_td_err) - 0.5 * param_ti, 0,
						&lla2, NULL, NULL, NULL, NULL, NULL,
						&v_cam2, &camInNedQu2, &w_cam2);

	double _ecef_center_x, _ecef_center_y, _ecef_center_z;
	idealMapDrawable->getCoordCenter(_ecef_center_x, _ecef_center_y, _ecef_center_z);

	osg::Vec3d ecef_center;
	ecef_center.x() = _ecef_center_x; ecef_center.y() = _ecef_center_y; ecef_center.z() = _ecef_center_z;

	if(!DOUBLE_FRAME) {
		setCamera_LLA_QU(idealImageCam1.get(), lla2, camInNedQu2, ecef_center);
		setCamera_LLA_QU(idealDepthCam1.get(), lla2, camInNedQu2, ecef_center);
	} else {
		//at min time
		nsrPoseMakerExtract(frame_timestamp_s - (param_td + param_td_err) - 0.5 * (param_tr + param_tr_err) - param_ti - 0.5 * param_te, 0,
							&lla1, NULL, NULL, NULL, NULL, NULL,
							&v_cam1, &camInNedQu1, &w_cam1);
		setCamera_LLA_QU(idealImageCam1.get(), lla1, camInNedQu1, ecef_center);
		setCamera_LLA_QU(idealDepthCam1.get(), lla1, camInNedQu1, ecef_center);

		//at max time
		nsrPoseMakerExtract(frame_timestamp_s - (param_td + param_td_err) + 0.5 * (param_tr + param_tr_err) - 0. + 0.5 * param_te, 0,
							&lla3, NULL, NULL, NULL, NULL, NULL,
							&v_cam3, &camInNedQu3, &w_cam3);
		setCamera_LLA_QU(idealImageCam2.get(), lla3, camInNedQu3, ecef_center);
		// 		setCamera_LLA_QU(idealDepthCam2.get(), lla3, camInNedQu3, ecef_center);
	}

#if RENDER_DEBUG == 0
	distortedDrawable->inVecCallback->setElement(UNIFORM_SEED, param_seed); //just used for creating a random number
	distortedDrawable->inVecCallback->setElement(UNIFORM_TIME, frame_timestamp_s); //also used for creating a random number

	distortedDrawable->inVecCallback->setElement(UNIFORM_WX, w_cam2.x());
	distortedDrawable->inVecCallback->setElement(UNIFORM_WY, w_cam2.y());
	distortedDrawable->inVecCallback->setElement(UNIFORM_WZ, w_cam2.z());

	distortedDrawable->inVecCallback->setElement(UNIFORM_VX, v_cam2.x());
	distortedDrawable->inVecCallback->setElement(UNIFORM_VY, v_cam2.y());
	distortedDrawable->inVecCallback->setElement(UNIFORM_VZ, v_cam2.z());
#endif
    return 0;
}

void CamSimScene::Pause()
{
	idealImageCam1->detach(osg::Camera::COLOR_BUFFER);
	idealImageCam2->detach(osg::Camera::COLOR_BUFFER);
	idealDepthCam1->detach(osg::Camera::COLOR_BUFFER);
	idealDepthCam2->detach(osg::Camera::COLOR_BUFFER);

	idealImageTexture1 = NULL;
	idealImageTexture2 = NULL;
	idealDepthTexture1 = NULL;
	idealDepthTexture2 = NULL;
	vignetTexture = NULL;
}

CamSimScene::~CamSimScene()
{
	if(prmReader != NULL) {
		delete prmReader;
		prmReader = NULL;
	}

	LOGI(TAG, " CamSimScene deleted!\n");
}

#ifdef __cplusplus
}
#endif
