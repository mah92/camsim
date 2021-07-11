#ifndef _NSROSGCAMSIMSCENE_H_
#define _NSROSGCAMSIMSCENE_H_

#include "Core/nsrCore.h"

#include "View/nsrImageDrawable.h"
#include "View/nsrMapDrawable2.h"
#include "View/nsrObjectDrawable.h"

#ifdef __cplusplus
extern "C" {
#endif

class CamSimScene : public osg::PositionAttitudeTransform
{
	osg::ref_ptr<osg::Camera> idealImageCam1, idealImageCam2;
	osg::ref_ptr<osg::Camera> idealDepthCam1, idealDepthCam2;

	osg::ref_ptr<osg::Texture2D> idealImageTexture1, idealImageTexture2;
	osg::ref_ptr<osg::Texture2D> idealDepthTexture1, idealDepthTexture2;
	osg::ref_ptr<osg::Texture2D> vignetTexture;
	osg::ref_ptr<ImageDrawable> distortedDrawable;

	osg::ref_ptr<MapDrawable2> idealMapDrawable;
    
public:
	ImageDrawableParamsReader* prmReader;
	CamSimScene();
	int InitOsgWindow(int x, int y, int _screen_width, int _screen_height);
	int Draw(double frame_timestamp_s);
	void Pause();
	void hideExtra();
	void showExtra();

protected:
	virtual ~CamSimScene();
};

#ifdef __cplusplus
}
#endif

#endif /* _NSROSGCAMSIMSCENE_H_ */
