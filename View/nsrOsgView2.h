#ifndef _NSROSGVIEW2_H_
#define _NSROSGVIEW2_H_

#include "nsrOsgCamSimScene.h"

#ifdef __cplusplus
extern "C" {
#endif

extern osg::ref_ptr<CamSimScene> camSimScene1;

void setCameraToViewScene();

#ifdef __cplusplus
}
#endif

#endif /* _NSROSGVIEW2_H_ */
