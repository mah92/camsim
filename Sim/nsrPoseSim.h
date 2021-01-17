#ifndef _NSRPOSESSIM_H_
#define _NSRPOSESSIM_H_

#include <osg/PositionAttitudeTransform>

#ifdef __cplusplus
extern "C" {
#endif

void nsrPoseMakerInit();

double nsrPoseMakerGetStartTime();

void nsrPoseMakerLoop(double time_barrier);

void nsrPoseMakerExtract(double time_s, int do_log, osg::Vec3d *_lla, osg::Vec3d *_v_ac, osg::Vec3d *_a_ac, osg::Vec3d *_acc_ac, osg::Quat *_acInNedQu, osg::Vec3d *_w_ac, osg::Vec3d *_v_cam, osg::Quat *_camInNedQu, osg::Vec3d *_w_cam); //cam/ac, in cam

void nsrPoseMakerClose();

#ifdef __cplusplus
}
#endif

#endif /* _NSRPOSESSIM_H_ */
