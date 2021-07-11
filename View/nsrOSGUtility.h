#ifndef __nsrOSGImage__
#define __nsrOSGImage__

#include <osg/PositionAttitudeTransform>
#include <osgText/Text>
#include <osg/PolygonMode>
#include "View/nsrImageDrawableParams.h"

osg::Camera* createCamera(bool draw_on_others, bool transparent);

osg::Geode* createScreenQuad(float width, float height, float scale = 1.0f);

#define EYE_DISTANCE 1000000 //in pix
int setCameraToViewForm(osg::Camera* cam, ImageDrawableParams **prms, osg::Vec2* scrDim);

#endif
