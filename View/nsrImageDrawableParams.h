#ifndef __nsrImageDrawableParams__
#define __nsrImageDrawableParams__

#include "Core/nsrStrUtility.h"

#include <vector>
#include <string.h> //for memset
#include <stdint.h> //uint32_t

#ifdef __cplusplus
extern "C" {
#endif

#define hex2osgVec4(color) \
	(osg::Vec4(((uint32_t)color >> 8*3)/255., ((uint32_t)color << 8 >> 8*3)/255., ((uint32_t)color << 8*2 >> 8*3)/255., ((uint32_t)color << 8*3 >> 8*3)/255.))

class ImageDrawableParams
{
public:
	char name[MAX_FILENAME_LENGTH];
	char resource_name[MAX_FILENAME_LENGTH];
	enum {RESOURCE_UNKNOWN, RESOURCE_IMAGE, RESOURCE_TEXT};
	int resource_type;
	//int imagefile_index;
	bool rel;
	float left, up, right, down, rotCenterX, rotCenterY, posX, posY, posZ, width, height, dstAR;
	bool hasWidth, hasHeight, hasPosZ;
	uint32_t color;

	ImageDrawableParams()
	{
		clear();
	}

	int clear()
	{
		memset(name, 0, 1); //setting strlen to zero, sizeof does not detect len in other function so not used
		memset(resource_name, 0, 1); //setting strlen to zero, sizeof does not detect len in other function so not used
		resource_type = RESOURCE_UNKNOWN;
		//imagefile_index = -1;
		left = -1;
		up = -1;
		right = -1;
		down = -1;

		rotCenterX = -1;
		rotCenterY = -1;

		rel = false;
		posX = -1;
		posY = -1;
		posZ = -1;
		hasPosZ = false;

		width = -1;
		hasWidth = false;
		height = -1;
		hasHeight = false;
		dstAR = -1;
		color = 0xFFFFFFFF;
        return 0;
	}

	bool isEmpty()
	{
		if(strlen(name) <= 0)
			return true;
		else
			return false;
	}
};

enum {P_NAME = 1, P_RESNAME = 2, P_RESTYPE = 4,
	  P_LEFT = 8, P_UP = 16, P_RIGHT = 32, P_DOWN = 64,
	  P_ROTCENTERX = 128, P_ROTCENTERY = 256,
	  P_REL = 512,
	  P_POSX = 1024, P_POSY = 2048, P_POSZ = 4096,
	  P_WIDTH = 8192, P_HEIGHT = 16384,
	  P_DSTAR = 32768, P_COLOR = 65536
	 };

class ImageDrawableParamsReader
{
	std::vector <ImageDrawableParams*> allprms;

	float aspect_ratio;
	float x_offset, y_offset;
	float x_scale, y_scale;

public:
	//int total_image_files;
	//std::vector < char* > imageFileNames;
	ImageDrawableParamsReader()
	{
		aspect_ratio = 1;;
		x_offset = 0; y_offset = 0;
		x_scale = 1; y_scale = 1;
	}

	ImageDrawableParamsReader(const char* filepath)
	{
		aspect_ratio = 1;;
		x_offset = 0; y_offset = 0;
		x_scale = 1; y_scale = 1;

		addParameters(filepath);
	}

	int clear()
	{
		int i;
		for(i = 0; i < allprms.size(); i++)
			delete allprms[i];
		allprms.clear();
        return 0;
	}

	~ImageDrawableParamsReader() {clear();}

	int addParameters(const char* filepath);
	int findParamByName(const char* name, ImageDrawableParams* allprms);
};

#ifdef __cplusplus
}
#endif

#endif
