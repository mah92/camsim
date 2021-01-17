//osgDB

#ifndef __ANDROID__
#include"/usr/include/time.h"
#endif

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#ifdef __ANDROID__
//Static plugins Macro
USE_OSGPLUGIN(tiff)

USE_OSGPLUGIN(ive)
USE_OSGPLUGIN(osg)
USE_OSGPLUGIN(osg2)
USE_OSGPLUGIN(terrain)
USE_OSGPLUGIN(rgb)
USE_OSGPLUGIN(OpenFlight)
USE_OSGPLUGIN(dds)
USE_OSGPLUGIN(bmp)
USE_OSGPLUGIN(jpeg)
USE_OSGPLUGIN(png)
USE_OSGPLUGIN(gif)
//USE_OSGPLUGIN(shp)
USE_OSGPLUGIN(freetype) //for font
//Static DOTOSG
USE_DOTOSGWRAPPER_LIBRARY(osg)
USE_DOTOSGWRAPPER_LIBRARY(osgFX)
USE_DOTOSGWRAPPER_LIBRARY(osgParticle)
USE_DOTOSGWRAPPER_LIBRARY(osgTerrain)
USE_DOTOSGWRAPPER_LIBRARY(osgText)
USE_DOTOSGWRAPPER_LIBRARY(osgViewer)
USE_DOTOSGWRAPPER_LIBRARY(osgVolume)
//Static serializer
USE_SERIALIZER_WRAPPER_LIBRARY(osg)
USE_SERIALIZER_WRAPPER_LIBRARY(osgAnimation)
USE_SERIALIZER_WRAPPER_LIBRARY(osgFX)
USE_SERIALIZER_WRAPPER_LIBRARY(osgManipulator)
USE_SERIALIZER_WRAPPER_LIBRARY(osgParticle)
USE_SERIALIZER_WRAPPER_LIBRARY(osgTerrain)
USE_SERIALIZER_WRAPPER_LIBRARY(osgText)
USE_SERIALIZER_WRAPPER_LIBRARY(osgVolume)

#endif

#ifdef HAVE_OSGEARTH

#include <osgViewer/GraphicsWindow>
#include <osgDB/Registry>

//windowing system
#ifndef ANDROID
USE_GRAPICSWINDOW_IMPLEMENTATION(IOS)
#endif

//osg plugins////////////////////////
//USE_OSGPLUGIN(OpenFlight) //repeated
USE_OSGPLUGIN(obj)
USE_OSGPLUGIN(shp)
//USE_OSGPLUGIN(ive) //repeated

//depreceated osg format
//USE_OSGPLUGIN(osg) //repeated
//USE_DOTOSGWRAPPER_LIBRARY(osg) //repeated

//USE_OSGPLUGIN(osg2) //repeated
//USE_SERIALIZER_WRAPPER_LIBRARY(osg) //repeated
//USE_SERIALIZER_WRAPPER_LIBRARY(osgAnimation) //repeated

USE_OSGPLUGIN(rot)
USE_OSGPLUGIN(scale)
USE_OSGPLUGIN(trans)

//image files/////////////////////
//USE_OSGPLUGIN(tiff) //repeated

#ifndef ANDROID
USE_OSGPLUGIN(imageio)
#else
//USE_OSGPLUGIN(gif) //repeated
//USE_OSGPLUGIN(png) //repeated
//USE_OSGPLUGIN(jpeg) //repeated
#endif
//USE_OSGPLUGIN(zip) //didn't build on Android for some reason
USE_OSGPLUGIN(curl)
//USE_OSGPLUGIN(freetype) //repeated
USE_OSGPLUGIN(kml)

//osgearth plugins/////////////////
USE_OSGPLUGIN(osgearth_sky_simple)
USE_OSGPLUGIN(osgearth_sky_gl)
USE_OSGPLUGIN(osgearth_feature_wfs)
USE_OSGPLUGIN(osgearth_feature_tfs)
USE_OSGPLUGIN(osgearth_tms)
USE_OSGPLUGIN(osgearth_wms)
//USE_OSGPLUGIN(osgearth_label_overlay)
USE_OSGPLUGIN(osgearth_xyz)
USE_OSGPLUGIN(osgearth_label_annotation)
USE_OSGPLUGIN(osgearth_mask_feature)
USE_OSGPLUGIN(osgearth_model_feature_geom)
//USE_OSGPLUGIN(osgearth_agglit)  //causing link errors on android
USE_OSGPLUGIN(osgearth_feature_ogr)
USE_OSGPLUGIN(osgearth_model_feature_stencil)
USE_OSGPLUGIN(osgearth_vdatum_egm2008)
USE_OSGPLUGIN(osgearth_model_simple)
USE_OSGPLUGIN(osgearth_engine_mp)
USE_OSGPLUGIN(osgearth_engine_byo)
USE_OSGPLUGIN(osgearth_vdatum_egm96)
//USE_OSGPLUGIN(osgearth_ocean_surface)
USE_OSGPLUGIN(osgearth_debug)
//USE_OSGPLUGIN(osgearth_mbtiles)
USE_OSGPLUGIN(osgearth_vdatum_egm84)
USE_OSGPLUGIN(osgearth_tileservice)
USE_OSGPLUGIN(osgearth_yahoo)
USE_OSGPLUGIN(osgearth_arcgis_map_cache)
USE_OSGPLUGIN(osgearth_tilecache)
USE_OSGPLUGIN(osgearth_wcs)
USE_OSGPLUGIN(osgearth_gdal)
USE_OSGPLUGIN(earth)
USE_OSGPLUGIN(osgearth_cache_filesystem)
USE_OSGPLUGIN(osgearth_arcgis)
USE_OSGPLUGIN(osgearth_osg)
#endif
