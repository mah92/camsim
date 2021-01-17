#ifndef __ANDROID__

#include"/usr/include/time.h"
#endif

#include "Core/nsrCore.h"

#include "nsrOSGUtility.h"
#include "View/nsrOSGShaders.h"
#include "View/nsrGeoLib.h"

#include <osg/Texture2D>
#include <osg/Geometry>
#include <osg/Geode>
#include <osg/ShapeDrawable>
#include <osgDB/ReadFile>
#include <osgText/Font>
#include <osgViewer/Viewer>
#include <osgUtil/Tessellator>

//help:
//GL_LIGHTING, if set to off, 3d objects may become completely whie or black
//GL_DEPTH_TEST, if set to off, image will always be at top
//GL_BLEND, allows image to be blended with background
//TRANSPARENT_BIN, allows out of image to be transparented

#undef TAG
#define TAG "Cpp:OsgUtility:"

osg::ref_ptr<osg::StateSet> _state;

osg::Camera* createCamera(bool draw_on_others, bool transparent)
{
	osg::ref_ptr<osg::Camera> camera = new osg::Camera;
	camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
	if(draw_on_others) {
		camera->setClearMask(GL_DEPTH_BUFFER_BIT);  // only clear the depth buffer(not color) to always draw
		camera->setRenderOrder(osg::Camera::POST_RENDER);   //render after main camera view, to draw on all others
	} else {
		camera->setRenderOrder(osg::Camera::PRE_RENDER);
	}
	if(transparent) {
		camera->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);
		camera->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
	}
	camera->setAllowEventFocus(false);  // we don't want the camera to grab event focus from the viewers main camera(s).

	return camera.release();
}

osg::Geode* createScreenQuad(float width, float height, float scale)
{
	osg::Geometry* geom = osg::createTexturedQuadGeometry(
							  osg::Vec3(), osg::Vec3(width, 0.0f, 0.0f), osg::Vec3(0.0f, height, 0.0f),
							  0.0f, 0.0f, width * scale, height * scale);
	osg::ref_ptr<osg::Geode> quad = new osg::Geode;
	quad->addDrawable(geom);

	int values = osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED;
	quad->getOrCreateStateSet()->setAttribute(new osg::PolygonMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::FILL), values);
	quad->getOrCreateStateSet()->setMode(GL_LIGHTING, values);
	return quad.release();
}

int setCameraToViewForm(osg::Camera *cam, ImageDrawableParams **prms, osg::Vec2* scrDim)
{
	float frame_width, frame_height,
		  frame_posx, frame_posy;

	frame_width = prms[0]->width * scrDim->x();
	frame_height = prms[0]->height * scrDim->y();
	frame_posx = prms[0]->posX * scrDim->x();
	frame_posy = prms[0]->posY * scrDim->y();

#if 0
	//seeing(x:[-1,1], y:[-1,1]) in a rectangular screen causes rotating objects to be distorted
	cam->setProjectionMatrix(osg::Matrixd::identity());
	cam->setViewMatrix(osg::Matrixd::identity());
#endif

#if 0
	//nothing displayed in any depth other than zero!!!
	cam->setProjectionMatrix(osg::Matrix::ortho(
								 0, frame_width,//left,right,
								 0, frame_height, //bottom,top
								 -10, +10));//zNear,zFar
	cam->setViewMatrix(osg::Matrix::identity());
#endif

#if 0
	//test camera
	//for a centered cam, provide input as: -frame_width/2, frame_width/2, -frame_height/2, frame_height/2
	cam->setProjectionMatrix(osg::Matrix::ortho2D(
								 -frame_width / 2, frame_width / 2, //left,right,
								 -frame_height / 2, frame_height / 2)); //bottom,top
	cam->setViewMatrix(osg::Matrix::identity() * osg::Matrix::translate(osg::Vec3(-frame_posx, -frame_posy, 0)));
#endif

#if 1
	cam->setProjectionMatrix(osg::Matrix::perspective(
								 2 * atan(frame_height / 2. / EYE_DISTANCE) * 180. / M_PI, //double fovy(deg), calculated for height not width
								 (float)frame_width / (float)frame_height, //double aspectRatio,
								 0.8 * EYE_DISTANCE, //double zNear,
								 1.2 * EYE_DISTANCE)); //double zFar
	cam->setViewMatrix(
		osg::Matrix::lookAt(osg::Vec3d(frame_posx, frame_posy, EYE_DISTANCE), //const osg::Vec3d & 	eye,
							osg::Vec3d(frame_posx, frame_posy, 0), //const osg::Vec3d & 	center,
							osg::Vec3d(0, 1, 0))); //const osg::Vec3d & 	up
#endif

	return 0;
}

//input lattitude should be geodetic, so that rotations along body become exact
//input quaternion is of camera coordinates(looking at z(+), y=down, x=right) with respect to NED
int setCamera_LLA_QU(osg::Camera *cam, const osg::Vec3d lla_cam, const osg::Quat cam_quat, const osg::Vec3d ecef_center)
{
	osg::Quat q_mid, q_mid2, q_first, q_convert, q_cam0, q_first1, q_first2, q_lat, q_lon;
	osg::Matrixd final;
	osg::Vec3d pos_ecef = LLA2ECEF(lla_cam);

	pos_ecef.x() -= ecef_center.x();
	pos_ecef.y() -= ecef_center.y();
	pos_ecef.z() -= ecef_center.z();

	q_lon = osg::Quat(osg::inDegrees(lla_cam.y()), osg::Z_AXIS);
	q_lat = osg::Quat(osg::inDegrees(-lla_cam.x()), osg::Y_AXIS); //geodetic latitude is exact reverse rotation of body along Y_AXIS of earth

	//to bring osg camera z(-) toward earth(down)
	q_mid = osg::Quat(osg::inDegrees(90.), osg::Z_AXIS) * osg::Quat(osg::inDegrees(90.), osg::Y_AXIS);

	//osg camera looks at z(-), y=up, x=right( standard was looking at z(+), y=down, x=right)
	//q_convert.inverse()* quat * q_convert, converts quat in our standard to osg standard
	q_convert = osg::Quat(osg::inDegrees(-90.), osg::Z_AXIS) * osg::Quat(osg::inDegrees(180.), osg::X_AXIS);
	q_cam0 = osg::Quat(osg::inDegrees(-90.), osg::Z_AXIS); //turn camera into it's zero position, multiplied after cam_quat to do rotation in cam coords

	final = osg::Matrixd::rotate(q_convert.inverse() * (q_cam0 * cam_quat) * q_convert * q_mid * q_lat * q_lon) * osg::Matrixd::translate(pos_ecef); //verified

	//mat(col, row)!
	/*LOGI(TAG, "%f, %f, %f,  %f\n%f, %f, %f,  %f\n%f, %f, %f,  %f\n%f, %f, %f,  %f\n\n",
			final(0,0), final(1,0), final(2,0), final(3,0),
			final(0,1), final(1,1), final(2,1), final(3,1),
			final(0,2), final(1,2), final(2,2), final(3,2),
			final(0,3), final(1,3), final(2,3), final(3,3)
			);*/

	cam->setViewMatrix(osg::Matrixd::inverse(final));

	return 0;
}
