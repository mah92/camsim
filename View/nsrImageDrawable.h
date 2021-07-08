#ifndef __nsrImageDrawable__
#define __nsrImageDrawable__

#include "nsrImageDrawableParams.h"
#include <osg/PositionAttitudeTransform>
//#include <osg/ImageStream>
#include <osg/Texture2D>

#include <vector>

#include "Core/nsrCore.h"

#ifdef __cplusplus
extern "C" {
#endif

class AnimateCallback: public osg::Uniform::Callback
{
	float transparency;
	_LOCK_DEFINE_NATIVE(Key)

public:
	void setTransparency(float _transparency)
	{
		//LOGI("Mola", "111");
		_LOCKNATIVE(Key,);
		transparency = _transparency;
		_UNLOCKNATIVE(Key,);
		//LOGI("Mola", "222");
	}
	AnimateCallback()
	{
		transparency = 1.;
		_LOCK_INIT_NATIVE(Key,);
	}
	virtual void operator()(osg::Uniform* uniform, osg::NodeVisitor* nv)
	{
		_LOCKNATIVE(Key,);
		uniform->set((float)transparency);
		_UNLOCKNATIVE(Key,);
	}
private:
};

//Everything updated here should also be updated in the shader
enum {UNIFORM_RENDER_DEPTH, UNIFORM_SEED, UNIFORM_TIME,
	  UNIFORM_WX, UNIFORM_WY, UNIFORM_WZ,
	  UNIFORM_VX, UNIFORM_VY, UNIFORM_VZ,

	  UNIFORM_LENS_ITERATIONS, UNIFORM_PIX_ERR,
	  UNIFORM_K1, UNIFORM_K2, UNIFORM_T1, UNIFORM_T2, UNIFORM_K3, UNIFORM_K4, UNIFORM_K5, UNIFORM_K6,
	  UNIFORM_VIGNET_THRESH1, UNIFORM_VIGNET_THRESH2,
	  UNIFORM_TD, UNIFORM_TR, UNIFORM_TE,
	  UNIFORM_BLUR_EXTRA_SAMPLES, UNIFORM_TI,
	  UNIFORM_WIDTH, UNIFORM_HEIGHT, UNIFORM_FX, UNIFORM_FY, UNIFORM_OX, UNIFORM_OY,
	  UNIFORM_EXTRA_MARGIN, UNIFORM_EXTRA_ZOOM,
	  UNIFORM_DAY_LIGHT, UNIFORM_NOISE_AMPLITUDE_DYNAMIC, UNIFORM_NOISE_AMPLITUDE_STATIC1, UNIFORM_NOISE_AMPLITUDE_STATIC2, UNIFORM_MAX_FOG_DISTANCE,
	  UNIFORM_DOUBLE_INPUT, UNIFORM_NUM /* Total number*/
	 };

class InputVectorCallback: public osg::Uniform::Callback
{
	_LOCK_DEFINE_NATIVE(Key)
	float inputVec[UNIFORM_NUM];

public:
	InputVectorCallback()
	{
		_LOCK_INIT_NATIVE(Key,);
		int i;
		for(i = 0; i < UNIFORM_NUM; i++)
			inputVec[i] = 0.;
	}
	void setElement(int elem, float data)
	{
		_LOCKNATIVE(Key,);
		inputVec[elem] = data;
		_UNLOCKNATIVE(Key,);
	}
	virtual void operator()(osg::Uniform* uniform, osg::NodeVisitor* nv)
	{
		_LOCKNATIVE(Key,);
		int i;
		for(i = 0; i < UNIFORM_NUM; i++)
			uniform->setElement(i, (float)inputVec[i]);
		_UNLOCKNATIVE(Key,);
	}
};

class ImageDrawable: public osg::PositionAttitudeTransform
{
public:
	bool hidden;
	osg::Vec3d hideposition;
	osg::ref_ptr<osg::Image> im;
	//osg::ref_ptr<osg::ImageStream> im2;

	float transparency;

	ImageDrawable() : PositionAttitudeTransform() { transparency = 1.; ac = NULL;}

	//offset is distance(in pixels)images center(center of rotation) goes to positive axis
	int setImage(const std::string &texfile, osg::Vec3 _pivot,
				 osg::Vec3 _pos, float _width, float _height);
	//int setImageIndirect(const char* texName, float _drawable_center_x, float _drawable_center_y,
	//		float _screen_position_x, float _screen_position_y, float _width_scale, float _height_scale);
	int setImageIndirect2(ImageDrawableParams* prm, osg::Vec2* scrDim);

	int setImageRaw(ImageDrawableParams* prm, osg::Vec2* scrDim,
					int height, int width, int imtype, unsigned char *frame);

	//DynamicImageCallback* imc;
	AnimateCallback* ac;
	int setImageRawDynamic(ImageDrawableParams* prm, osg::Vec2* scrDim,
						   int height, int width, int imtype, unsigned char *frame, bool has_transparency = false, osg::Uniform::Callback* imc = NULL);
	//int setImageRawDynamicFaded(ImageDrawableParams* prm, osg::Vec2* scrDim,
	//									int height, int width, int imtype, unsigned char *frame1, unsigned char *frame2);
	int setImageFromTexture(ImageDrawableParams* prm, osg::Vec2* scrDim, osg::Texture2D* texture);

	InputVectorCallback* inVecCallback;
	int setLensFromTexture1(ImageDrawableParams* prm, osg::Vec2* scrDim, osg::Texture2D* baseTexture, osg::Texture2D* depthTexture, osg::Texture2D* vignetTexture, bool render_depth = false);
	int setLensFromTexture2(ImageDrawableParams* prm, osg::Vec2* scrDim,
							osg::Texture2D* baseTexture1, osg::Texture2D* baseTexture2,
							osg::Texture2D* depthTexture1, osg::Texture2D* depthTexture2,
							osg::Texture2D* vignetTexture, bool render_depth = false);

	void setTransparency(float transparency);

	void hide();
	void show();

protected:
	virtual ~ImageDrawable() { if(ac != NULL); delete ac; ac = NULL;}

};

#ifdef __cplusplus
}
#endif

#endif
