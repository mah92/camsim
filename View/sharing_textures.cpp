/* -*-c++-*- Copyright (C) 2010 Wang Rui <wangray84 at gmail dot com>
 * OpenSceneGraph Beginners Guide
 * Using a reading callback to share textures with the same filename
*/

#ifndef __ANDROID__
#include"/usr/include/time.h"
#endif

#include <osg/Texture2D>
#include <osg/Geometry>
#include <osg/Geode>
#include <osg/Group>
#include <osgDB/ReadFile>

#include "sharing_textures.h"

osg::Image* ReadAndShareImageCallback::getImageByName(const std::string &filename)
{
	ImageMap::iterator itr = _imageMap.find(filename);
	if(itr != _imageMap.end()) return itr->second.get();
	return NULL;
}

osgDB::ReaderWriter::ReadResult ReadAndShareImageCallback::readImage(const std::string &filename, const osgDB::Options* options)
{
	osg::Image* image = getImageByName(filename);
	if(!image) {
		osgDB::ReaderWriter::ReadResult rr =
			osgDB::Registry::instance()->readImageImplementation(filename, options);
		if(rr.success()) _imageMap[filename] = rr.getImage();
		return rr;
	}
	return image;
}

void share_textures_init()
{
	osgDB::Registry::instance()->setReadFileCallback(new ReadAndShareImageCallback);
	osgDB::Registry::instance()->getOrCreateSharedStateManager();
}

void share_textures(osg::Node *node)
{
	osgDB::SharedStateManager* ssm =
		osgDB::Registry::instance()->getSharedStateManager();
	if(ssm) ssm->share(node);
}
