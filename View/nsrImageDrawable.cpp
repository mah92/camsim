#ifndef __ANDROID__
#include "/usr/include/time.h"
#endif

#include "nsrImageDrawable.h"
#include "Core/nsrCore.h"

#include "nsrOSGShaders.h"
#include "nsrImageDrawableParams.h"

#include <osg/BlendFunc>
#include <osgDB/ReadFile>
#include <osgText/Text>
#include <osg/ImageStream>

#undef TAG
#define TAG "Cpp:OsgImageDrawable:"

#ifdef __cplusplus
extern "C" {
#endif

//Notes:
//1)Reference frame:
//screen reference frame starts from bottom-left
//but image reference is center of image as defined in this function
//2)image types:
//png reading cause SIGSEGV )-:
//tiff returns null(Image not found at(*.tif)!) )-:
//bmp shown but no transparency(even in 32-bit version saved by photoshop) (-:
//jpg shown but has no transparency at all (-:
//jpeg-2000 (jp2) not compiled )-:
//gif shown successfully and has transparency :-)(-:
//4)image names -can- have spaces

int ImageDrawable::setImage(const std::string &texfile, osg::Vec3 _pivot,
							osg::Vec3 _pos, float _width, float _height)
{

	if(texfile != "") {
		im = osgDB::readImageFile(texfile);

		if(im == NULL) {
			LOGW(TAG, " Image not found at (%s)!\n", texfile.c_str());
			//return -1;
		}
	} else
		im = NULL;

	osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
	texture->setImage(im);

	osg::ref_ptr<osg::Drawable> quad = osg::createTexturedQuadGeometry(
										   osg::Vec3(-_width * 0.5f, -_height * 0.5f, 0.0f), //corner
										   osg::Vec3(_width, 0.0f, 0.0f), //widthVec
										   osg::Vec3(0.0f, _height, 0.0f),  //heightVec
										   0,//left, of used input image
										   0,//bottom, of used input image
										   1,//right, of used input image
										   1);//top, of used input image

	osg::ref_ptr<osg::Geode> geode = new osg::Geode;
	geode->addDrawable(quad.get());

	osg::ref_ptr<osg::StateSet> _state;
	_state = geode->getOrCreateStateSet();
	_state->setMode(GL_BLEND, osg::StateAttribute::ON);
	_state->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);   //sorts objects by distance, so multiplexing occurs

#ifdef OSG_GL_FIXED_FUNCTION_AVAILABLE
	_state->setTextureAttributeAndModes(0, texture.get());
	_state->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
#else
	_state->setTextureAttribute(0, texture.get());
	osg::Uniform* baseTextureSampler = new osg::Uniform("baseTexture", 0);
	_state->addUniform(baseTextureSampler);

	osg::Program * prog = new osg::Program;
	prog->addShader(new osg::Shader(osg::Shader::VERTEX, imageVertexShader));
	prog->addShader(new osg::Shader(osg::Shader::FRAGMENT, imageFragmentShader));
	_state->setAttribute(prog);
#endif

	removeChild(0, 1); //added to remove memory leak when recreating an object
	addChild(geode.release());   //release on node does not remove object data! just some data used in the creation process

	setPivotPoint(_pivot);
	setPosition(_pos);

	return 0;
}

//extern std::vector<osg::ref_ptr<osg::Image> > textureImages;
//screen reference frame starts from bottom-left
//Image->Texture2D->Drawable->Geode->PositionAttitudeTransform
int ImageDrawable::setImageIndirect2(ImageDrawableParams* prm, osg::Vec2* scrDim)
{
	float left, down, right, up;
	float width, height;
	float rotCenter_x, rotCenter_y;
	float scale_x, scale_y;

	if(prm == NULL)
		return -1;

	/*if(textureImages[prm->imagefile_index] == NULL)	return -1;
	osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
	texture->setImage(textureImages[prm->imagefile_index]);*/

	std::string datapath(globals.datapath);
	std::string file(prm->resource_name);
	std::string fullpath(datapath + "/" + file);

	if(strlen(prm->resource_name) != 0) {
		im = osgDB::readImageFile(fullpath);

		if(im == NULL) {
			LOGW(TAG, " Image(%s) not found at (%s)!!\n", prm->name, fullpath.c_str());
			//return -1;
		}
	} else
		im = NULL;

	osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
	texture->setImage(im);

	left = prm->left;
	down = prm->down;
	right = prm->right;
	up = prm->up;

	width = prm->width * scrDim->x(); //textureImagesWidth*fabs(right - left);
	height = prm->height * scrDim->y(); //textureImagesHeight*fabs(up - down);

	//LOGI(TAG, "%s, l:%f,d:%f, r:%f,u%f\n", prm->name, left, down, right, up);
	//LOGI(TAG, "%s, w:%f,h:%f\n", prm->name, prm->width, prm->height);

	osg::ref_ptr<osg::Drawable> quad = osg::createTexturedQuadGeometry(
										   osg::Vec3(-width * 0.5f, -height * 0.5f, +0.0f), //corner
										   osg::Vec3(width, 0.0f, 0.0f), //widthVec
										   osg::Vec3(0.0f, height, 0.0f),  //heightVec
										   left,//left, of used input image
										   down,//bottom, of used input image
										   right,//right, of used input image
										   up);//top, of used input image

	osg::ref_ptr<osg::Geode> geode = new osg::Geode;
	geode->addDrawable(quad.get());

	osg::ref_ptr<osg::StateSet> _state;
	_state = geode->getOrCreateStateSet();
	_state->setMode(GL_BLEND, osg::StateAttribute::ON);
	_state->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);   //sorts objects by distance, so multiplexing occurs

#ifdef OSG_GL_FIXED_FUNCTION_AVAILABLE
	_state->setTextureAttributeAndModes(0, texture.get());
	_state->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
#else
	_state->setTextureAttribute(0, texture.get());
	osg::Uniform* baseTextureSampler = new osg::Uniform("baseTexture", 0);
	_state->addUniform(baseTextureSampler);

	osg::Program * prog = new osg::Program;
	prog->addShader(new osg::Shader(osg::Shader::VERTEX, imageVertexShader));
	prog->addShader(new osg::Shader(osg::Shader::FRAGMENT, imageFragmentShader));
	_state->setAttribute(prog);
#endif

	removeChild(0, 1); //added to remove memory leak when recreating an object
	addChild(geode.release());   //release on node does not remove object data! just some data used in the creation process

	setPivotPoint(osg::Vec3(width * prm->rotCenterX, height * prm->rotCenterY, 0));
	setPosition(osg::Vec3d(scrDim->x()*prm->posX, scrDim->y()*prm->posY, prm->posZ));
	//setPosition(osg::Vec3d(0,0,0));

	hideposition = getPosition();
	hidden = false;

	return 0;
}

int ImageDrawable::setImageRaw(ImageDrawableParams* prm, osg::Vec2* scrDim,
							   int height, int width, int imtype, unsigned char *frame)
{
	im = new osg::Image;

	if(imtype == 0) {
		im->setImage(width, height,
					 1, //channels num
					 GL_LUMINANCE,//Specifies the internal format(channels) of the texture, values:GL_ALPHA, GL_LUMINANCE, GL_LUMINANCE_ALPHA, GL_RGB, GL_RGBA.
					 GL_LUMINANCE,//Must match internalformat.
					 GL_UNSIGNED_BYTE,//Specifies the data type(depth) of the each texel data. values: GL_UNSIGNED_BYTE(8UCX), GL_UNSIGNED_SHORT(16UCX), GL_FLOAT(32UCX)
					 frame,
					 osg::Image::NO_DELETE,//NO_DELETE,USE_NEW_DELETE,USE_MALLOC_FREE
					 1);
	} else {
		im->setImage(width, height,
					 3, //channels num
					 GL_RGB,//Specifies the internal format of the texture, values:GL_ALPHA, GL_LUMINANCE, GL_LUMINANCE_ALPHA, GL_RGB, GL_RGBA.
					 GL_RGB,//Must match internalformat.
					 GL_UNSIGNED_BYTE,//Specifies the data type(depth) of the each texel data. values: GL_UNSIGNED_BYTE(8UCX), GL_UNSIGNED_SHORT(16UCX), GL_FLOAT(32UCX)
					 frame,
					 osg::Image::NO_DELETE,//NO_DELETE,USE_NEW_DELETE,USE_MALLOC_FREE
					 1);
	}

	osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
	texture->setImage(im);

	osg::ref_ptr<osg::Drawable> quad = osg::createTexturedQuadGeometry(
										   osg::Vec3(-scrDim->x() * prm->width * 0.5f, +scrDim->y() * prm->height * 0.5f, 0.0f), //second term made negetive to make pic upside-down
										   osg::Vec3(scrDim->x() * prm->width, 0.0f, 0.0f),
										   osg::Vec3(0.0f, -scrDim->y() * prm->height, 0.0f));	//second term made negetive to make pic upside-down

	osg::ref_ptr<osg::Geode> geode = new osg::Geode;
	geode->addDrawable(quad.get());

	osg::ref_ptr<osg::StateSet> _state;
	_state = geode->getOrCreateStateSet();
	_state->setMode(GL_BLEND, osg::StateAttribute::ON);
	_state->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);   //sorts objects by distance, so multiplexing occurs

#ifdef OSG_GL_FIXED_FUNCTION_AVAILABLE
	_state->setTextureAttributeAndModes(0, texture.get());
	_state->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
#else
	_state->setTextureAttribute(0, texture.get());
	osg::Uniform* baseTextureSampler = new osg::Uniform("baseTexture", 0);
	_state->addUniform(baseTextureSampler);

	osg::Program * prog = new osg::Program;
	prog->addShader(new osg::Shader(osg::Shader::VERTEX, imageVertexShader));
	prog->addShader(new osg::Shader(osg::Shader::FRAGMENT, imageFragmentShader));
	_state->setAttribute(prog);
#endif

	removeChild(0, 1); //added to remove memory leak when recreating an object
	addChild(geode.release());   //release on node does not remove object data! just some data used in the creation process

	setPivotPoint(osg::Vec3(prm->width * prm->rotCenterX, prm->height * prm->rotCenterY, 0));
	setPosition(osg::Vec3d(scrDim->x()*prm->posX, scrDim->y()*prm->posY, prm->posZ));
	//setPosition(osg::Vec3d(0,0,0));

	hideposition = getPosition();
	hidden = false;

	return 0;
}

////////////////////////////////////////////////

//for movies or changing frames
//note1: for performance, frame should be of size 2^nx2^m(e.g. 512x512)
//note2: after every change, run im->dirty();
int ImageDrawable::setImageRawDynamic(ImageDrawableParams* prm, osg::Vec2* scrDim,
									  int height, int width, int imtype, unsigned char *frame, bool has_transparency, osg::Uniform::Callback* imc)
{
	im = new osg::Image();
	im->setDataVariance(DYNAMIC);
#if !defined(ANDROID) && !defined(__APPLE__)
	//makes performance better
	im->setPixelBufferObject(new osg::PixelBufferObject(im));
#endif

	int glimtype;
	int glchannels;
	if(imtype == 0) {
		glchannels = 1;
		glimtype = GL_LUMINANCE;
	} else if(imtype == 1) {
		glchannels = 3;
		glimtype = GL_RGB;
	} else if(imtype == 2) {
		glchannels = 4;
		glimtype = GL_RGBA;
	}

	im->setImage(width, height,
				 glchannels, //channels num
				 glimtype,//Specifies the internal format(channels) of the texture, values:GL_ALPHA, GL_LUMINANCE, GL_LUMINANCE_ALPHA, GL_RGB, GL_RGBA.
				 glimtype,//Must match internalformat.
				 GL_UNSIGNED_BYTE,//Specifies the data type(depth) of the each texel data. values: GL_UNSIGNED_BYTE(8UCX), GL_UNSIGNED_SHORT(16UCX), GL_FLOAT(32UCX)
				 frame,
				 osg::Image::NO_DELETE,//NO_DELETE,USE_NEW_DELETE,USE_MALLOC_FREE
				 1);

	osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
	if(!texture.get()) {
		texture = new osg::Texture2D;
		texture->setResizeNonPowerOfTwoHint(true);
		texture->setWrap(osg::Texture::WRAP_S, osg::Texture::REPEAT);
		texture->setWrap(osg::Texture::WRAP_T, osg::Texture::REPEAT);
	}
	texture->setImage(im);

	osg::ref_ptr<osg::Drawable> quad = osg::createTexturedQuadGeometry(
										   osg::Vec3(-scrDim->x() * prm->width * 0.5f, +scrDim->y() * prm->height * 0.5f, 0.0f), //second term made negetive to make pic upside-down
										   osg::Vec3(scrDim->x() * prm->width, 0.0f, 0.0f),
										   osg::Vec3(0.0f, -scrDim->y() * prm->height, 0.0f));	//second term made negetive to make pic upside-down

	osg::ref_ptr<osg::Geode> geode = new osg::Geode;
	geode->addDrawable(quad.get());

	osg::ref_ptr<osg::StateSet> _state;
	_state = geode->getOrCreateStateSet();
	_state->setMode(GL_BLEND, osg::StateAttribute::ON);
	_state->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);   //sorts objects by distance, so multiplexing occurs
	//_state->setRenderBinDetails( 11, "RenderBin");
	//_state->setRenderBinDetails(1, "DepthSortedBin");
	osg::BlendFunc* bf = new osg::BlendFunc(osg::BlendFunc::SRC_ALPHA, osg::BlendFunc::ONE_MINUS_SRC_ALPHA); //default
	//osg::BlendFunc* bf = new osg::BlendFunc(osg::BlendFunc::SRC_ALPHA, osg::BlendFunc::DST_ALPHA);
	//osg::BlendFunc* bf = new osg::BlendFunc(osg::BlendFunc::SRC_COLOR, osg::BlendFunc::ONE_MINUS_SRC_COLOR);
	_state->setAttributeAndModes(bf);

#ifdef OSG_GL_FIXED_FUNCTION_AVAILABLE
	_state->setTextureAttributeAndModes(0, texture.get());
	_state->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
#else
	_state->setTextureAttribute(0, texture.get());
	osg::Uniform* baseTextureSampler = new osg::Uniform("baseTexture", 0);
	_state->addUniform(baseTextureSampler);
	//imc = new DynamicImageCallback();
	if(imc != NULL)
		baseTextureSampler->setUpdateCallback(imc);

	if(has_transparency == true) {
		osg::Uniform* transparencyUniform = new osg::Uniform("transparency", 1.0f);
		_state->addUniform(transparencyUniform);
		ac = new AnimateCallback();
		transparencyUniform->setUpdateCallback(ac);
	}

	osg::Program * prog = new osg::Program;
	prog->addShader(new osg::Shader(osg::Shader::VERTEX, imageVertexShader));
	/*if(has_transparency == false)
		prog->addShader ( new osg::Shader(osg::Shader::FRAGMENT, imageFragmentShaderGamma ) );
		//prog->addShader ( new osg::Shader(osg::Shader::FRAGMENT, imageFragmentShader ) );
	else
		prog->addShader ( new osg::Shader(osg::Shader::FRAGMENT, imageFragmentShader2 ) );*/
	prog->addShader(new osg::Shader(osg::Shader::FRAGMENT, imageFragmentShaderMapped));

	_state->setAttribute(prog);
#endif

	removeChild(0, 1); //added to remove memory leak when recreating an object
	addChild(geode.release());   //release on node does not remove object data! just some data used in the creation process

	setPivotPoint(osg::Vec3(prm->width * prm->rotCenterX, prm->height * prm->rotCenterY, 0));
	setPosition(osg::Vec3d(scrDim->x()*prm->posX, scrDim->y()*prm->posY, prm->posZ));
	//setPosition(osg::Vec3d(0,0,0));

	hideposition = getPosition();
	hidden = false;

	return 0;
}

int ImageDrawable::setImageFromTexture(ImageDrawableParams* prm, osg::Vec2* scrDim, osg::Texture2D* texture)
{
	bool has_transparency = false;

	osg::ref_ptr<osg::Drawable> quad = osg::createTexturedQuadGeometry(
										   osg::Vec3(-scrDim->x() * prm->width * 0.5f, -scrDim->y() * prm->height * 0.5f, 0.0f),
										   osg::Vec3(scrDim->x() * prm->width, 0.0f, 0.0f),
										   osg::Vec3(0.0f, +scrDim->y() * prm->height, 0.0f));

	osg::ref_ptr<osg::Geode> geode = new osg::Geode;
	geode->addDrawable(quad.get());

	osg::ref_ptr<osg::StateSet> _state;
	_state = geode->getOrCreateStateSet();
	_state->setMode(GL_BLEND, osg::StateAttribute::ON);
	_state->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);   //sorts objects by distance, so multiplexing occurs
	//_state->setRenderBinDetails( 11, "RenderBin");
	//_state->setRenderBinDetails(1, "DepthSortedBin");

	if(has_transparency == true) {
		osg::BlendFunc* bf = new osg::BlendFunc(osg::BlendFunc::SRC_ALPHA, osg::BlendFunc::ONE_MINUS_SRC_ALPHA); //default
		//osg::BlendFunc* bf = new osg::BlendFunc(osg::BlendFunc::SRC_ALPHA, osg::BlendFunc::DST_ALPHA);
		//osg::BlendFunc* bf = new osg::BlendFunc(osg::BlendFunc::SRC_COLOR, osg::BlendFunc::ONE_MINUS_SRC_COLOR);
		_state->setAttributeAndModes(bf);
	}

	//#ifdef OSG_GL_FIXED_FUNCTION_AVAILABLE
	//_state->setTextureAttributeAndModes( 0, texture );
	//_state->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	//#else
	_state->setTextureAttribute(0, texture);
	osg::Uniform* baseTextureSampler = new osg::Uniform("baseTexture", 0);
	_state->addUniform(baseTextureSampler);

	if(has_transparency == true) {
		osg::Uniform* transparencyUniform = new osg::Uniform("transparency", 1.0f);
		_state->addUniform(transparencyUniform);
		ac = new AnimateCallback();
		transparencyUniform->setUpdateCallback(ac);
	}

	osg::Program * prog = new osg::Program;
	prog->addShader(new osg::Shader(osg::Shader::VERTEX, imageVertexShader));
	if(has_transparency == false)
		prog->addShader(new osg::Shader(osg::Shader::FRAGMENT, imageFragmentShader));
	else
		prog->addShader(new osg::Shader(osg::Shader::FRAGMENT, imageFragmentShader2));
	//prog->addShader ( new osg::Shader(osg::Shader::FRAGMENT, depthReverseFragmentShader ) );

	_state->setAttribute(prog);
	//#endif

	removeChild(0, 1); //added to remove memory leak when recreating an object
	addChild(geode.release());   //release on node does not remove object data! just some data used in the creation process
	setPivotPoint(osg::Vec3(prm->width * prm->rotCenterX, prm->height * prm->rotCenterY, 0));
	setPosition(osg::Vec3d(scrDim->x()*prm->posX, scrDim->y()*prm->posY, prm->posZ));

	hideposition = getPosition();
	hidden = false;

	return 0;
}

int ImageDrawable::setLensFromTexture1(ImageDrawableParams* prm, osg::Vec2* scrDim, osg::Texture2D* baseTexture, osg::Texture2D* depthTexture, osg::Texture2D* vignetTexture, int render_what)
{
    osg::ref_ptr<osg::Drawable> quad = osg::createTexturedQuadGeometry(
										   osg::Vec3(-scrDim->x() * prm->width * 0.5f, -scrDim->y() * prm->height * 0.5f, 0.0f),
										   osg::Vec3(scrDim->x() * prm->width, 0.0f, 0.0f),
										   osg::Vec3(0.0f, +scrDim->y() * prm->height, 0.0f));

	osg::ref_ptr<osg::Geode> geode = new osg::Geode;
	geode->addDrawable(quad.get());

	osg::ref_ptr<osg::StateSet> _state;
	_state = geode->getOrCreateStateSet();

	_state->setTextureAttribute((int)0, baseTexture);
	_state->addUniform(new osg::Uniform("baseTexture1", (int)0));

	_state->setTextureAttribute((int)1, depthTexture);
	_state->addUniform(new osg::Uniform("depthTexture1", (int)1));

	_state->setTextureAttribute((int)2, vignetTexture);
	_state->addUniform(new osg::Uniform("vignetTexture", (int)2));

	osg::Uniform* inputVectUniform = new osg::Uniform(osg::Uniform::FLOAT, "inputVect", UNIFORM_NUM); //vector of elements
	_state->addUniform(inputVectUniform);
	inVecCallback = new InputVectorCallback();
	inputVectUniform->setUpdateCallback(inVecCallback);
	inVecCallback->setElement(UNIFORM_RENDER_WHAT, render_what);
	inVecCallback->setElement(UNIFORM_DOUBLE_INPUT, 0);

	osg::Program * prog = new osg::Program;
	prog->addShader(new osg::Shader(osg::Shader::VERTEX, lensVertexShader));
	prog->addShader(new osg::Shader(osg::Shader::FRAGMENT, lensFragmentShader));     //lensFragmentShader

	_state->setAttribute(prog);

	removeChild(0, 1); //added to remove memory leak when recreating an object
	addChild(geode.release());   //release on node does not remove object data! just some data used in the creation process

	setPivotPoint(osg::Vec3(prm->width * prm->rotCenterX, prm->height * prm->rotCenterY, 0));
	setPosition(osg::Vec3d(scrDim->x()*prm->posX, scrDim->y()*prm->posY, prm->posZ));

	hideposition = getPosition();
	hidden = false;

	return 0;
}

int ImageDrawable::setLensFromTexture2(ImageDrawableParams* prm, osg::Vec2* scrDim,
									   osg::Texture2D* baseTexture1, osg::Texture2D* baseTexture2,
									   osg::Texture2D* depthTexture1, osg::Texture2D* depthTexture2,
									   osg::Texture2D* vignetTexture, int render_what)
{
	osg::ref_ptr<osg::Drawable> quad = osg::createTexturedQuadGeometry(
										   osg::Vec3(-scrDim->x() * prm->width * 0.5f, -scrDim->y() * prm->height * 0.5f, 0.0f),
										   osg::Vec3(scrDim->x() * prm->width, 0.0f, 0.0f),
										   osg::Vec3(0.0f, +scrDim->y() * prm->height, 0.0f));

	osg::ref_ptr<osg::Geode> geode = new osg::Geode;
	geode->addDrawable(quad.get());

	osg::ref_ptr<osg::StateSet> _state;
	_state = geode->getOrCreateStateSet();

	int counter = 0;

	_state->setTextureAttribute((unsigned int)counter, baseTexture1);
	_state->addUniform(new osg::Uniform("baseTexture1", counter++));

	_state->setTextureAttribute((unsigned int)counter, baseTexture2);
	_state->addUniform(new osg::Uniform("baseTexture2", counter++));

	_state->setTextureAttribute((unsigned int)counter, depthTexture1);
	_state->addUniform(new osg::Uniform("depthTexture1", counter++));

	_state->setTextureAttribute((unsigned int)counter, depthTexture2);
	_state->addUniform(new osg::Uniform("depthTexture2", counter++));

	_state->setTextureAttribute((unsigned int)counter, vignetTexture);
	_state->addUniform(new osg::Uniform("vignetTexture", counter++));

	osg::Uniform* inputVectUniform = new osg::Uniform(osg::Uniform::FLOAT, "inputVect", UNIFORM_NUM); //vector of elements
	_state->addUniform(inputVectUniform);
	inVecCallback = new InputVectorCallback();
	inputVectUniform->setUpdateCallback(inVecCallback);
	inVecCallback->setElement(UNIFORM_RENDER_WHAT, render_what);
	inVecCallback->setElement(UNIFORM_DOUBLE_INPUT, 1);

	osg::Program * prog = new osg::Program;
	//prog->setName("lensShader");
	prog->addShader(new osg::Shader(osg::Shader::VERTEX, lensVertexShader));
	prog->addShader(new osg::Shader(osg::Shader::FRAGMENT, lensFragmentShader));     //lensFragmentShader
	_state->setAttribute(prog);

	removeChild(0, 1); //added to remove memory leak when recreating an object
	addChild(geode.release());   //release on node does not remove object data! just some data used in the creation process

	setPivotPoint(osg::Vec3(prm->width * prm->rotCenterX, prm->height * prm->rotCenterY, 0));
	setPosition(osg::Vec3d(scrDim->x()*prm->posX, scrDim->y()*prm->posY, prm->posZ));

	hideposition = getPosition();
	hidden = false;

	return 0;
}

void ImageDrawable::setTransparency(float _transparency)
{
	if(ac != NULL)
		ac->setTransparency(_transparency);
}

#define HighValue 100000

//setNodeMask is very slow, takes 3 seconds, even after dirtyBound();
void ImageDrawable::hide()
{
	//getChild(0)->setNodeMask(0x0); //hide
	//getChild(0)->dirtyBound();
	if(hidden == false) {
		hidden = true;
		hideposition = getPosition();
		setPosition(osg::Vec3d(HighValue, HighValue, 0));
	}
}

void ImageDrawable::show()
{
	//getChild(0)->setNodeMask(0xFFFFFFFF); //display
	//getChild(0)->dirtyBound();
	if(hidden == true) {
		hidden = false;
		setPosition(hideposition);
	}
}

#ifdef __cplusplus
}
#endif

