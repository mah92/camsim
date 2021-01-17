#ifndef __SHARING_TEXTURES__
#define __SHARING_TEXTURES__

#ifdef __cplusplus
extern "C" {
#endif

class ReadAndShareImageCallback : public osgDB::ReadFileCallback
{
public:
	virtual osgDB::ReaderWriter::ReadResult readImage(const std::string &filename, const osgDB::Options* options);

protected:
	osg::Image* getImageByName(const std::string &filename);

	typedef std::map<std::string, osg::ref_ptr<osg::Image> > ImageMap;
	ImageMap _imageMap;
};

void share_textures_init();
void share_textures(osg::Node *node);

#ifdef __cplusplus
}
#endif

#endif
