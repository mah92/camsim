/**
 * Usage:
 * int MapDrawable2::addSeenMapRegion(double latmean, double lonmean, double max_alt, double distance, int manual_zoom) //adds seen map
 * ->
 * int MapDrawable2::addMapRegion(double latmin, double latmax, double lonmin, double lonmax, int zoom)
 * ->
 * int MapDrawable2::addMapTile(int xTile, int yTile, int zoom, int dem_resolution) //-1 for no dem
 * ->
 * osg::Geometry* createTileGeometryInCPU( const std::string &texfile, int xTile, int yTile, int zoom, int dem_resolution = -1); //-1 for no dem
or

 * //image mapped to terrain, center_alt from terrain
 * int MapDrawable2::addImageToMap(const char* str, double center_lat, double center_lon, double center_alt, double metric_width, int dem_resolution) //-1 for no dem
 * ->
 * //image mapped to terrain, center_alt from terrain
 * osg::Geometry* createImageGeometryInCPU( const std::string &texfile, double center_lat, double center_lon, double center_alt, double metric_width, int dem_resolution = -1); //-1 for no dem);
 */

#ifndef __ANDROID__
#include "/usr/include/time.h"
#endif

#include "nsrMapDrawable2.h"
#include "Core/nsrCore.h"

#include "nsrOSGShaders.h"
#include "nsrImageDrawableParams.h"
#include "nsrSimParamReader.h"

#include <osg/BlendFunc>
#include <osgDB/ReadFile>
#include <osgText/Text>
#include <osg/ImageStream>
#include <osg/Geometry>

#if OSG_VERSION_MINOR <=2
#include <osg/ArrayDispatchers>
#endif

#include "View/nsrGeoLib.h"
#undef TAG
#define TAG "Cpp:OsgMapDrawable2:"

#ifdef __cplusplus
extern "C" {
#endif

////////////////////////////////////////////
//Notes:
//1)Reference frame:
//screen reference frame starts from bottom-left
//but image reference is center of image as defined in this function
//2)image types:
//png reading cause SIGSEGV )-:
//tiff returns null(Image not found at(*.tif)!) )-:
//bmp shown but no transparency(even in 32-bit version saved by photoshop) (-:
//jpg shown but has no transparency at all (-:
//jpeg-2000 (jp2) not compiled )-:
//gif shown successfully and has transparency :-)(-:
//4)image names -can- have spaces

//createTexturedQuadGeometry just evaluates vertex shader at 4 corners, use createGridGeometry to evaluate displacement at more points
//see: osg/Geometry
//re setting a single osg::Image and osg::Texture2D as uniform in gpu will not have overhead(??if appropriate sharing textures functions called??)

//with this function, coordinates are calculated in cpu
///TODO: add WGS84->EGM96 correction
osg::Geometry* createTileGeometryInCPU(const std::string &texfile, int xTile, int yTile, int zoom, int dem_resolution = -1)  //-1 for no dem
{
	unsigned int i, j;
	unsigned int columns, rows;
	double lat, lon;
	float alt;
	double ecef_x, ecef_y, ecef_z;
	float row_percent, col_percent;

	bool use_dem = dem_resolution > 0 ? true : false;

	osg::ref_ptr<osg::Image> im = NULL;
	if(texfile != "") {
		im = osgDB::readImageFile(texfile);

		if(im == NULL) {
			LOGW(TAG, " Image not found at (%s)!\n", texfile.c_str());
			//return -1;
		}
	} else
		im = NULL;

	osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
	texture->setImage(im);

	//use LINEAR_MIPMAP_LINEAR to prevent aliasing
	//http://www.cuboslocos.com/tutorials/OSG-BasicTexturing
	//https://github.com/xarray/osgRecipes/blob/master/effectcompositor/SkyBox.cpp
	//https://www.opengl.org/discussion_boards/showthread.php/182411-Flickering-textures
	texture->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR_MIPMAP_LINEAR);
	texture->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);
	texture->setWrap(osg::Texture2D::WRAP_S, osg::Texture::CLAMP_TO_EDGE); //otherwise, black lines are seen in opengl(not es)
	texture->setWrap(osg::Texture2D::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
	//////////////////////////////

	if(zoom <= 10)
		use_dem = false;

	int two_pow_zoom = 1 << zoom;
	if(use_dem) {
		columns = rows = nsrMax(dem_resolution, 2);//nsrMax(32/two_pow_zoom, 2); //with dem
	} else {
		columns = rows = nsrMin(2 * two_pow_zoom, 10.); //no dem
	}

	osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array(columns * rows);
	osg::ref_ptr<osg::Vec2Array> texcoords = new osg::Vec2Array(columns * rows);
	for(i = 0; i < rows; i++) {
		for(j = 0; j < columns; j++) {
			row_percent = (float)i / (float)(rows - 1);
			col_percent = (float)j / (float)(columns - 1);
			tile2lla(xTile + col_percent, yTile + row_percent, zoom, lat, lon);//lat is geodetic

			if(!use_dem)
				alt = 0;
			else
				demGetHeight(lat, lon, alt);

			//alt+=47.5.; // + N(undulation) to convert EGM96 msl to WGS-84///????
			alt += nsrGeoidGetHeight(lat, lon);
			//printf("((%f\n", nsrGeoidGetHeight(lat_rad*180./M_PI, lon_rad*180./M_PI));

			//LLA - Geodetic Lat(rad, WGS84) Lon(rad) Alt(WGS84)
			LLA2ECEF(lat, lon, alt, ecef_x, ecef_y, ecef_z);
            ECEF2CCEF(ecef_x, ecef_y, ecef_z);

			(*vertices)[i * columns + j].set(ecef_x, ecef_y, ecef_z);
			//vertices[0]*=0.00003;
			(*texcoords)[i * columns + j].set((float)col_percent, (float)(1. - row_percent));
		}
	}

	osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
	geom->setUseDisplayList(false);
	geom->setUseVertexBufferObjects(true);
	geom->setVertexArray(vertices.get());
	geom->setTexCoordArray(0, texcoords.get());
	for(i = 0; i < rows - 1; i++) {
		osg::ref_ptr<osg::DrawElementsUInt> de = new osg::DrawElementsUInt(GL_QUAD_STRIP, columns * 2);
		for(j = 0; j < columns; j++) {
			(*de)[j * 2 + 0] = i * columns + j;
			(*de)[j * 2 + 1] = (i + 1) * columns + j;
		}
		geom->addPrimitiveSet(de.get());
	}

	//////////////////////////////////
	//alhamdolellah: (0.5day)using setTextureAttributeAndModes with !OSG_GL_FIXED_FUNCTION_AVAILABLE causes gpu error

	/*#if (OPENSCENEGRAPH_MAJOR_VERSION < 3 || (OPENSCENEGRAPH_MAJOR_VERSION==3 && OPENSCENEGRAPH_MINOR_VERSION < 4))
	    geom->getOrCreateStateSet()->setTextureAttribute(0, texture.get());
	#else
		geom->getStateSet()->setTextureAttribute(0, texture.get());
	#endif*/
	geom->getOrCreateStateSet()->setTextureAttribute(0, texture.get());

	//geom->getOrCreateStateSet()->addUniform( new osg::Uniform("baseTexture", 0) );

	////////////////////////
	/*
	 * Should be added for every camera respectively
	osg::ref_ptr<osg::Program> program = new osg::Program;
	if(!render_z) {
		program->addShader( new osg::Shader(osg::Shader::VERTEX, imageVertexShader) );
		program->addShader( new osg::Shader(osg::Shader::FRAGMENT, imageFragmentShader) );
	} else {
		program->addShader( new osg::Shader(osg::Shader::VERTEX, depthVertexShader) );
		program->addShader( new osg::Shader(osg::Shader::FRAGMENT, depthFragmentShader) );
	}

	geom->getOrCreateStateSet()->setAttributeAndModes( program.get() );
	*/
	return geom.release();
}

//with this function, coordinates are calculated in cpu
//geocentric lat(deg) & lon(deg)
///TODO: add WGS84->EGM96 correction
//image mapped to terrain, center_alt from terrain
//geocentric lat(deg), lon(deg), alt(m, from spherical earth)
//Geodetic Lat(rad, WGS84) Lon(rad) Alt(WGS84)
//problem!!!
osg::Geometry* createImageGeometryInCPU(const std::string &texfile, double center_lat, double center_lon, double center_alt, double metric_width, int dem_resolution = -1)  //-1 for no dem
{
	unsigned int i, j;
	unsigned int columns, rows;
	double lat, lon;
	float alt;
	double ecef_x, ecef_y, ecef_z;
	float row_percent, col_percent;
	double metric_height;

	bool use_dem = dem_resolution > 0 ? true : false;

	osg::ref_ptr<osg::Image> im = NULL;
	if(texfile != "") {
		im = osgDB::readImageFile(texfile);

		if(im == NULL) {
			LOGW(TAG, " Image not found at (%s)!\n", texfile.c_str());
			//return -1;
		}
	} else
		im = NULL;
	metric_height = metric_width * im->t() / im->s();
	//LOGI(TAG, "\n\n metric_height:%f, %i, %i \n\n", metric_height, im->s(),im->t()); //width, height

	osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
	texture->setImage(im);

	//use LINEAR_MIPMAP_LINEAR to prevent aliasing
	//http://www.cuboslocos.com/tutorials/OSG-BasicTexturing
	//https://github.com/xarray/osgRecipes/blob/master/effectcompositor/SkyBox.cpp
	//https://www.opengl.org/discussion_boards/showthread.php/182411-Flickering-textures
	texture->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR_MIPMAP_LINEAR);
	texture->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);
	texture->setWrap(osg::Texture2D::WRAP_S, osg::Texture::CLAMP_TO_EDGE); //otherwise, black lines are seen in opengl(not es)
	texture->setWrap(osg::Texture2D::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
	//////////////////////////////

	if(use_dem) {
		columns = rows = nsrMax(dem_resolution, 2);//nsrMax(32/two_pow_zoom, 2); //with dem
	} else {
		columns = rows = 2; //no dem
	}

	osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array(columns * rows);
	osg::ref_ptr<osg::Vec2Array> texcoords = new osg::Vec2Array(columns * rows);
	for(i = 0; i < rows; i++) {
		for(j = 0; j < columns; j++) {
			row_percent = (float)i / (float)(rows - 1);
			col_percent = (float)j / (float)(columns - 1);
			image2lla(center_lat, center_lon, center_alt, metric_width, metric_height, col_percent, row_percent, lat, lon); //geocentric

			if(!use_dem)
				alt = 0;
			else
				demGetHeight(lat, lon, alt);
			alt += center_alt;

			alt += nsrGeoidGetHeight(lat, lon);
			//printf("((%f\n", nsrGeoidGetHeight(lat, lon));

            //LLA - Geodetic Lat(rad, WGS84) Lon(rad) Alt(WGS84)
			LLA2ECEF(lat, lon, alt, ecef_x, ecef_y, ecef_z);
            ECEF2CCEF(ecef_x, ecef_y, ecef_z);
            
			(*vertices)[i * columns + j].set(ecef_x, ecef_y, ecef_z);
			//vertices[0]*=0.00003;
			(*texcoords)[i * columns + j].set((float)col_percent, (float)(1. - row_percent));
		}
	}

	osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
	geom->setUseDisplayList(false);
	geom->setUseVertexBufferObjects(true);
	geom->setVertexArray(vertices.get());
	geom->setTexCoordArray(0, texcoords.get());
	for(i = 0; i < rows - 1; i++) {
		osg::ref_ptr<osg::DrawElementsUInt> de = new osg::DrawElementsUInt(GL_QUAD_STRIP, columns * 2);
		for(j = 0; j < columns; j++) {
			(*de)[j * 2 + 0] = i * columns + j;
			(*de)[j * 2 + 1] = (i + 1) * columns + j;
		}
		geom->addPrimitiveSet(de.get());
	}
	//geom->setInitialBound( osg::BoundingBox(-1.0f,-1.0f,-100.0f, 1.0f, 1.0f, 100.0f) ); //used to inform zbuffer priority(cookbook, p.123)

	//////////////////////////////////
	//alhamdolellah: (0.5day)using setTextureAttributeAndModes with !OSG_GL_FIXED_FUNCTION_AVAILABLE causes gpu error
	geom->getOrCreateStateSet()->setTextureAttribute(0, texture.get());
	//geom->getOrCreateStateSet()->addUniform( new osg::Uniform("baseTexture", 0) );

	////////////////////////
	/*
	 * Should be added for every camera respectively
	osg::ref_ptr<osg::Program> program = new osg::Program;
	if(!render_z) {
		program->addShader( new osg::Shader(osg::Shader::VERTEX, imageVertexShader) );
		program->addShader( new osg::Shader(osg::Shader::FRAGMENT, imageFragmentShader) );
	} else {
		program->addShader( new osg::Shader(osg::Shader::VERTEX, depthVertexShader) );
		program->addShader( new osg::Shader(osg::Shader::FRAGMENT, depthFragmentShader) );
	}

	geom->getOrCreateStateSet()->setAttributeAndModes( program.get() );
	*/
	return geom.release();
}

int MapDrawable2::addMapTile(int xTile, int yTile, int zoom, int dem_resolution) //-1 for no dem
//int MapDrawable2::addMapTile(const std::string &texfile, float _width, float _height)
{
	char str[100];
	//sprintf(str, "%s/%i/%i/%i.jpg", globals.datapath, zoom, xTile, yTile);
	sprintf(str, "%s/%i/%i/%i.jpg", settings_mappath, zoom, xTile, yTile);
	std::string texfile(str);

	osg::ref_ptr<osg::Geode> geode = new osg::Geode;
	geode->addDrawable(createTileGeometryInCPU(texfile, xTile, yTile, zoom, dem_resolution));
	geode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

	//removeChild(0, 1); //added to remove memory leak when recreating an object
	addChild(geode.release());   //release on node does not remove object data! just some data used in the creation process
	//setPivotPoint(_pivot);
	//setPosition(_pos);

	return 0;
}

//image mapped to terrain, center_alt from terrain
//???
int MapDrawable2::addImageToMap(const char* str, double center_lat, double center_lon, double center_alt, double metric_width, int dem_resolution) //-1 for no dem
{
	std::string texfile(str);

	osg::ref_ptr<osg::Geode> geode = new osg::Geode;
	geode->addDrawable(createImageGeometryInCPU(texfile, center_lat, center_lon, center_alt, metric_width, dem_resolution));
	geode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

	//removeChild(0, 1); //added to remove memory leak when recreating an object
	addChild(geode.release());   //release on node does not remove object data! just some data used in the creation process
	//setPivotPoint(_pivot);
	//setPosition(_pos);

	return 0;
}

//geocentric lat(deg) & lon(deg)
int MapDrawable2::addMapRegion(double latmin, double latmax, double lonmin, double lonmax, int zoom)
{
	int i, j, tiles, tile_dem_res;
	double xmin, ymin, xmax, ymax;
	int xTileMin, xTileMax, yTileMin, yTileMax;
	lla2tile(latmax, lonmin, zoom,  xmin, ymin);
	lla2tile(latmin, lonmax, zoom,  xmax, ymax);
	xTileMin = (int)xmin;
	yTileMin = (int)ymin;
	xTileMax = (int)xmax + 1;
	yTileMax = (int)ymax + 1;

	tiles = (yTileMax - yTileMin) * (xTileMax - xTileMin);
	tile_dem_res = sqrt((float)MAX_TILES / tiles) * TILES_DEM_RES_FACTOR;
	for(i = xTileMin; i < xTileMax; i++)
		for(j = yTileMin; j < yTileMax; j++)
			addMapTile(i, j, zoom, tile_dem_res);

	LOGW(TAG, " Loaded %i tiles using %iMB memory///\n", tiles, tiles*(256*256*3)/1024/1024);
	LOGW(TAG, " //zoom:%i///\n", zoom);
	LOGW(TAG, " ///tile dem res: %i///\n", tile_dem_res);

	return tiles;
}

#define EARTH_RADIUS (getEarthRadius()) // 6378137.

//geocentric lat(deg) & lon(deg)
int MapDrawable2::addSeenMapRegion(double latmean, double lonmean, double max_alt, double distance, int manual_zoom) //adds seen map
{
	int zoom, tiles_max_zoom;
	double seen_distance, earth_seen_fov, seen_arc_of_earth;
	double latmin, latmax, lonmin, lonmax;
	double xmin, xmax, ymin, ymax;
	int xTileMin, xTileMax, yTileMin, yTileMax;

	//direct distance to horizon(max see-able in all conditions) assuming spherical earth
	seen_distance = sqrt(max_alt * (max_alt + 2 * EARTH_RADIUS));
	seen_distance = nsrMin(distance, seen_distance);
	earth_seen_fov = 2 * atan(EARTH_RADIUS / seen_distance);
	seen_arc_of_earth = 2 * (M_PI / 2. - earth_seen_fov / 2.);
	seen_arc_of_earth *= 180 / M_PI;
	LOGE(TAG, " //seen_distance:%f///\n", seen_distance);
	LOGE(TAG, " //earth_seen_fov:%f///\n", earth_seen_fov * 180. / M_PI);
	LOGE(TAG, " //seen_arc_of_earth:%f///\n", seen_arc_of_earth);

	latmin = latmean - seen_arc_of_earth / 2.;
	latmax = latmean + seen_arc_of_earth / 2.;
	lonmin = lonmean - seen_arc_of_earth / 2.*cos(latmean * M_PI / 180.);
	lonmax = lonmean + seen_arc_of_earth / 2.*cos(latmean * M_PI / 180.);

	lla2tile(latmax, lonmin, MAX_ZOOM,  xmin, ymin);
	lla2tile(latmin, lonmax, MAX_ZOOM,  xmax, ymax);
	xTileMin = (int)xmin;
	yTileMin = (int)ymin;
	xTileMax = (int)xmax + 1;
	yTileMax = (int)ymax + 1;
	tiles_max_zoom = (yTileMax - yTileMin) * (xTileMax - xTileMin);

	//tiles(zoom) = pow(4, (zoom-MAX_ZOOM)) * tiles_max_zoom = MAX_TILES
	if(manual_zoom < 0) {
		zoom = log((double)MAX_TILES / tiles_max_zoom) / log(4) + MAX_ZOOM;
		zoom = nsrMin(zoom, MAX_ZOOM);
	} else
		zoom = manual_zoom;

	return addMapRegion(latmin, latmax, lonmin, lonmax, zoom);
}

int MapDrawable2::removeMapTiles()
{
	removeChild(0, getNumChildren());
    return 0;
}

#define HighValue 100000

//setNodeMask is very slow, takes 3 seconds, even after dirtyBound();
void MapDrawable2::hide()
{
	//getChild(0)->setNodeMask(0x0); //hide
	//getChild(0)->dirtyBound();
	if(hidden == false) {
		hidden = true;
		hideposition = getPosition();
		setPosition(osg::Vec3d(HighValue, HighValue, 0));
	}
}

void MapDrawable2::show()
{
	//getChild(0)->setNodeMask(0xFFFFFFFF); //display
	//getChild(0)->dirtyBound();
	if(hidden == true) {
		hidden = false;
		setPosition(hideposition);
	}
}

#ifdef __cplusplus
}
#endif

/*
osg::Geometry* createGridGeometry( const std::string &texfile, unsigned int columns, unsigned int rows )
{
	osg::ref_ptr<osg::Image> im;
	if(texfile != ""){
		im = osgDB::readImageFile(texfile);

		if(im == NULL){
			LOGW(TAG, " Image not found at (%s)!\n",texfile.c_str());
			//return -1;
		}
	} else
		im = NULL;

    osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array(columns * rows);
    osg::ref_ptr<osg::Vec2Array> texcoords = new osg::Vec2Array(columns * rows);
    for ( unsigned int i=0; i<rows; ++i )
    {
        for ( unsigned int j=0; j<columns; ++j )
        {
            (*vertices)[i*columns + j].set( (float)i, (float)j, 0.0f );
            (*texcoords)[i*columns + j].set( (float)i/(float)(rows-1), (float)j/(float)(columns-1) );
        }
    }

    osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
    geom->setUseDisplayList( false );
    geom->setUseVertexBufferObjects( true );
    geom->setVertexArray( vertices.get() );
    geom->setTexCoordArray( 0, texcoords.get() );
    for ( unsigned int i=0; i<rows-1; ++i )
    {
        osg::ref_ptr<osg::DrawElementsUInt> de = new osg::DrawElementsUInt(GL_QUAD_STRIP, columns*2);
        for ( unsigned int j=0; j<columns; ++j )
        {
            (*de)[j*2 + 0] = i*columns + j;
            (*de)[j*2 + 1] = (i+1)*columns + j;
        }
        geom->addPrimitiveSet( de.get() );
    }
    geom->setInitialBound( osg::BoundingBox(-1.0f,-1.0f,-100.0f, 1.0f, 1.0f, 100.0f) ); //used to inform zbuffer priority(cookbook, p.123)

    osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
    texture->setImage( im );
    texture->setFilter( osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR );
    texture->setFilter( osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR );
    geom->getOrCreateStateSet()->setTextureAttributeAndModes( 0, texture.get() );
    geom->getOrCreateStateSet()->addUniform( new osg::Uniform("baseTexture", 0) );
	osg::Uniform* xTileUniform = new osg::Uniform("xTile", 1.0f);
    geom->getOrCreateStateSet()->addUniform(xTileUniform);
    osg::Uniform* yTileUniform = new osg::Uniform("yTile", 0.0f);
    geom->getOrCreateStateSet()->addUniform(yTileUniform);
	osg::Uniform* zoomUniform = new osg::Uniform("two_over_two_pow_zoom", 1.0f);
    geom->getOrCreateStateSet()->addUniform(zoomUniform);

    osg::ref_ptr<osg::Program> program = new osg::Program;
    program->addShader( new osg::Shader(osg::Shader::VERTEX, imageDemVertexShader) );
    program->addShader( new osg::Shader(osg::Shader::FRAGMENT, imageDemFragmentShader) );
    geom->getOrCreateStateSet()->setAttributeAndModes( program.get() );
    return geom.release();
}
 */

/*Geometry* osg::createTexturedQuadGeometry(const Vec3& corner,const Vec3& widthVec,const Vec3& heightVec, float l, float b, float r, float t)
{
    Geometry* geom = new Geometry;

    Vec3Array* coords = new Vec3Array(4);
    (*coords)[0] = corner+heightVec;
    (*coords)[1] = corner;
    (*coords)[2] = corner+widthVec;
    (*coords)[3] = corner+widthVec+heightVec;
    geom->setVertexArray(coords);

    Vec2Array* tcoords = new Vec2Array(4);
    (*tcoords)[0].set(l,t);
    (*tcoords)[1].set(l,b);
    (*tcoords)[2].set(r,b);
    (*tcoords)[3].set(r,t);
    geom->setTexCoordArray(0,tcoords);

    osg::Vec4Array* colours = new osg::Vec4Array(1);
    (*colours)[0].set(1.0f,1.0f,1.0,1.0f);
    geom->setColorArray(colours, osg::Array::BIND_OVERALL);

    osg::Vec3Array* normals = new osg::Vec3Array(1);
    (*normals)[0] = widthVec^heightVec;
    (*normals)[0].normalize();
    geom->setNormalArray(normals, osg::Array::BIND_OVERALL);

#if defined(OSG_GLES1_AVAILABLE) || !defined(OSG_GLES2_AVAILABLE)
    DrawElementsUByte* elems = new DrawElementsUByte(PrimitiveSet::TRIANGLES);
    elems->push_back(0);
    elems->push_back(1);
    elems->push_back(2);

    elems->push_back(2);
    elems->push_back(3);
    elems->push_back(0);
    geom->addPrimitiveSet(elems);
#else
    geom->addPrimitiveSet(new DrawArrays(PrimitiveSet::QUADS,0,4));
#endif

    return geom;
}*/

#if 0
//////////////////////////////////////////////
//https://github.com/gwaldron/osgearth/blob/2.7/src/osgEarthUtil/EarthManipulator.cpp
//https://www.apt-browse.org/browse/ubuntu/trusty/universe/amd64/libosgearth-dev/2.4.0+dfsg-6/file/usr/include/osgEarthUtil/EarthManipulator
//http://trac.openscenegraph.org/documentation/OpenSceneGraphReferenceDocs/a00104.html#a3a419df526bc66ea39086701ad9f0d4c
EarthManipulator::EarthManipulator() : osgGA::CameraManipulator() {}
void EarthManipulator::zoom(double dummy, double dzoom)
{
	//double scale = 1.0f + dy;
	//setDistance( _distance * scale );
}

void EarthManipulator::pan(double dx, double dy)
{
	/*osg::Matrix m = getMatrix() * osg::Matrixd::inverse(_centerLocalToWorld);
	osg::Vec3d look = -getUpVector( m );
	osg::Vec3d up = getFrontVector( m );

	osg::Vec3d new_localUp = getUpVector( _centerLocalToWorld );
	osg::Vec3d localUp(0.0f,0.0f,1.0f);

	osg::Quat pan_rotation;
	pan_rotation.makeRotate( localUp, new_localUp );*/
}

void EarthManipulator::rotate(double dtheta, double dummy)
{

}

Viewpoint EarthManipulator::getViewpoint()
{
	Viewpoint vp;
	/*vp.lat = ;
	vp.lon = ;
	vp.alt = ;
	vp.heading = ;
	vp.pitch = ;
	vp.range = ;*/

	return vp;
}

void EarthManipulator::setViewpoint(const Viewpoint &vp, double duration_seconds)
{

}

//called automatically by viewer class
/*void EarthManipulator::updateCamera(osg::Camera &camera)
{

}

//called by callback  set in updateCamera, see: https://github.com/gwaldron/osgearth/blob/2.7/src/osgEarthUtil/EarthManipulator.cpp

void EarthManipulator::updateTether()
{

}*/
#endif
