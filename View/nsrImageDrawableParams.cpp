#ifndef __ANDROID__
#include "/usr/include/time.h"
#endif

#include "nsrImageDrawableParams.h"
#include "Core/nsrCore.h"

#include <stdio.h> //for FILE, fopen, ...
#include "pugixml/pugixml.hpp"

using namespace pugi;

#undef TAG
#define TAG "Cpp:OsgImageDrawableParams:"

#ifdef __cplusplus
extern "C" {
#endif
	
int ImageDrawableParamsReader::addParameters(const char* filepath)
{
	pugi::xml_document doc;
	pugi::xml_parse_result result;
	result = doc.load_file(filepath);
	if(result) {
		LOGI(TAG, " xmlfile:%s loaded successfully!(%s)\n", filepath, result.description());
	} else {
		LOGE(TAG, " xmlfile:%s loaded with error: %s\n", filepath, result.description());
		return -1;
	}

	int srcWidth, srcHeight, dstWidth, dstHeight;
	int i = allprms.size() - 1;
	pugi::xml_node nsrgraphics = doc.child("nsrgraphics");
	for(pugi::xml_node elements = nsrgraphics.child("elements"); elements; elements = elements.next_sibling("elements")) {
		//srcsize
		pugi::xml_node srcsize = elements.child("srcsize");
		if(srcsize.attribute("width"))
			srcWidth = srcsize.attribute("width").as_int();
		else srcWidth = 1.;

		if(srcsize.attribute("height"))
			srcHeight = srcsize.attribute("height").as_int();
		else srcHeight = 1.;

		//destsize
		pugi::xml_node destsize = elements.child("destsize");
		if(destsize.attribute("width"))
			dstWidth = destsize.attribute("width").as_int();
		else dstWidth = 1.;

		if(destsize.attribute("height"))
			dstHeight = destsize.attribute("height").as_int();
		else dstHeight = 1.;

		//resource
		pugi::xml_node resource = elements.child("resource");
		//if(!resource || strlen(resource.child_value()) == 0) { //No file
		//else if(!resource || strcmp(resource.child_value(), "Num") == 0) { //Number
		//} else {//element has FileName

		pugi::xml_attribute atrb;
		for(pugi::xml_node node = elements.child("node"); node; node = node.next_sibling("node")) {
			allprms.push_back(new ImageDrawableParams());
			i++;

			//source///////////////////////////////////
			if(resource) {
				strcpy(allprms[i]->resource_name, resource.child_value());

				if(0 == endsWith(allprms[i]->resource_name, ".png")
						|| 0 == endsWith(allprms[i]->resource_name, ".bmp")
						|| 0 == endsWith(allprms[i]->resource_name, ".gif")
						|| 0 == endsWith(allprms[i]->resource_name, ".jpg")
						|| 0 == endsWith(allprms[i]->resource_name, ".jpeg"))
					allprms[i]->resource_type = ImageDrawableParams::RESOURCE_IMAGE;

				if(0 == endsWith(allprms[i]->resource_name, ".ttf"))
					allprms[i]->resource_type = ImageDrawableParams::RESOURCE_TEXT;
			}

			atrb = node.attribute("name");
			if(atrb) strcpy(allprms[i]->name, atrb.value());
			else continue;

			atrb = node.attribute("left");
			if(atrb) allprms[i]->left = atrb.as_float() / srcWidth;
			else allprms[i]->left = 0;

			atrb = node.attribute("up");
			if(atrb) allprms[i]->up = atrb.as_float() / srcHeight;
			else allprms[i]->up = 0;
			allprms[i]->up = 1. - allprms[i]->up; //change origin from top to down

			atrb = node.attribute("right");
			if(atrb) allprms[i]->right = atrb.as_float() / srcWidth;
			else allprms[i]->right = 1.;

			atrb = node.attribute("down");
			if(atrb) allprms[i]->down = atrb.as_float() / srcHeight;
			else allprms[i]->down = 1.;
			allprms[i]->down = 1. - allprms[i]->down; //change origin from top to down

			atrb = node.attribute("rot_center_x");
			if(atrb) allprms[i]->rotCenterX = atrb.as_float();
			else allprms[i]->rotCenterX = 0.;

			atrb = node.attribute("rot_center_y");
			if(atrb) allprms[i]->rotCenterY = atrb.as_float();
			else allprms[i]->rotCenterY = 0.;

			//destination//////////////////////////////
			atrb = node.attribute("rel");
			if(atrb) allprms[i]->rel = atrb.as_bool();
			else allprms[i]->rel = false;

			atrb = node.attribute("pos_x");
			if(atrb) allprms[i]->posX = atrb.as_float() / dstWidth;
			else allprms[i]->posX = (allprms[i]->left + allprms[i]->right) / 2.; //*srcWidth/dstWidth;

			atrb = node.attribute("pos_y");
			if(atrb) {
				allprms[i]->posY = atrb.as_float() / dstHeight;
				if(allprms[i]->rel == false)
					allprms[i]->posY = 1. - allprms[i]->posY; //change origin from top to down
				else {
					allprms[i]->posY = - allprms[i]->posY;//just reverse to allow relative data
				}
			} else
				allprms[i]->posY = (allprms[i]->up + allprms[i]->down) / 2.; //*srcHeight/dstHeight;

			atrb = node.attribute("pos_z");
			if(atrb) {
				allprms[i]->posZ = atrb.as_float();
				allprms[i]->hasPosZ = true;
			} else {
				allprms[i]->posZ = 0;
				allprms[i]->hasPosZ = false;
			}

			//
			allprms[i]->width = 0;
			allprms[i]->height = 0;
			atrb = node.attribute("width");
			if(atrb) {
				allprms[i]->width = atrb.as_float() / dstWidth;
				allprms[i]->hasWidth = true;
			} else {
				allprms[i]->width = (allprms[i]->right - allprms[i]->left) * srcWidth / dstWidth;
				allprms[i]->hasWidth = false;
			}
			atrb = node.attribute("height");
			if(atrb) {
				allprms[i]->height = atrb.as_float() / dstHeight;
				allprms[i]->hasHeight = true;
			} else {
				allprms[i]->height = -(allprms[i]->down - allprms[i]->up) * srcHeight / dstHeight;
				allprms[i]->hasHeight = false;
			}

			atrb = node.attribute("digit_width");
			if(atrb) allprms[i]->width = atrb.as_float() / dstWidth;

			atrb = node.attribute("digit_height");
			if(atrb) allprms[i]->height = atrb.as_float() / dstHeight;

			atrb = node.attribute("color");
			if(atrb) {allprms[i]->color = (uint32_t)strtoul(atrb.as_string(), NULL, 0);} //LOGE(TAG, ":::%s, %0X", atrb.as_string(), (uint32_t)strtol(atrb.as_string(), NULL, 0));}
			else allprms[i]->color = 0xFFFFFFFF;

			allprms[i]->dstAR = (float)dstWidth / (float)dstHeight;

			//printParam(allprms[i]);
			//}
		}
	}

	LOGI(TAG, " Drawable parameters file read successfully!\n");

	return 0;
}

/*void ImageDrawableParamsReader::correctParametersForAspectRatio(float aspect_ratio)
{
	int i;
	for(i=0; i<allprms.size(); i++) {
		LOGI(TAG, "%s\n", allprms[i]->name);
		if(allprms[i]->dstAR >0)
			allprms[i]->width *= aspect_ratio/allprms[i]->dstAR;
		if(allprms[i]->width > 1) allprms[i]->width = 1;
	}
}

void ImageDrawableParamsReader::setCorrectAR(float _aspect_ratio)
{
	aspect_ratio = _aspect_ratio;
}*/

/*void ImageDrawableParamsReader::setAddOffset(float _x_offset, float _y_offset)
{
	x_offset = _x_offset;
	y_offset = _y_offset;
}*/

/*void ImageDrawableParamsReader::setAddScale(float _x_scale, float _y_scale)
{
	x_scale = _x_scale;
	y_scale = _y_scale;
}*/

int ImageDrawableParamsReader::findParamByName(const char* name, ImageDrawableParams* _prm)
{
	int i;

	//LOGI(TAG, " searching for:%s\n",name);

	for(i = 0; i < allprms.size(); i++) {
		if(strcmp(name, allprms[i]->name) == 0) {

			if(_prm == NULL)
				_prm = new ImageDrawableParams();
			memcpy(_prm, allprms[i], sizeof(ImageDrawableParams));

			//_prm->width *= _x_scale;
			//_prm->height *= _y_scale;
			//_prm->posX *= _x_scale;
			//_prm->posY *= _y_scale;
			//_prm->posX += x_offset;
			//_prm->posY += y_offset;

			return 0;
			//return allprms[i];
		}
	}

	LOGW(TAG, " node not found by name: (%s)!\n", name);
	_prm->clear();
	return -1; //param not found
}

#ifdef __cplusplus
}
#endif

