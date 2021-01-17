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
int setCamera_LLA_QU(osg::Camera *cam, const osg::Vec3d lla_cam, const osg::Quat cam_quat, const osg::Vec3d ecef_center);

#endif
