#ifndef __nsrMapDrawable2__
#define __nsrMapDrawable2__

#include "nsrImageDrawableParams.h"
#include <osg/PositionAttitudeTransform>
//#include <osg/ImageStream>
#include <osg/Texture2D>
#include <osgGA/CameraManipulator>
#include <vector>

#include "Core/nsrCore.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_ZOOM 15
#define MAX_TILES 64.
#define TILES_DEM_RES_FACTOR 100 //if tiles = MAX_TILES, every tile is TILES_DEM_RES_FACTOR^2 vertices

class Viewpoint
{
private:
	std::string name;
	double lon, lat, alt;
	double heading, pitch, range;

public:
	//Constructs a blank (invalid) viewpoint.
	Viewpoint() : lon(0), lat(0), alt(0), heading(0), pitch(0), range(0) {}

	Viewpoint(const char* _name,
			  double _lon, //geopoint longitude(deg)
			  double _lat, //geopoint lattitude(deg)
			  double _alt, //geopoint altitude(m)
			  double _heading, //camera heading(deg)
			  double _pitch, //camera pitch(deg)
			  double _range //range to geopoint(m)
			 ) : name(_name), lon(_lon), lat(_lat), alt(_alt), heading(_heading), pitch(_pitch), range(_range)
	{}

	~Viewpoint() { name.clear(); }
	double getRange() {return range;}
	double getHeading() {return heading;}
};

/*class EarthManipulator : public osgGA::CameraManipulator
{
public:
	EarthManipulator();

	void zoom(double dummy, double dzoom);
	void pan(double dx, double dy);
	void rotate(double dtheta, double dummy); //dazimuth, dpitch

	Viewpoint getViewpoint();
	void setViewpoint(const Viewpoint &vp, double duration_seconds = 0.0);
};*/

class MapDrawable2: public osg::PositionAttitudeTransform
{

public:
	bool hidden;
	osg::Vec3d hideposition;
	bool use_dem;

	MapDrawable2(bool _use_dem = false) : PositionAttitudeTransform(), use_dem(_use_dem) {}

	int addMapTile(int xTile, int yTile, int zoom, int dem_resolution = -1); //-1 for no dem
	int addMapRegion(double latmin, double latmax, double lonmin, double lonmax, int zoom);
	int addSeenMapRegion(double latmean, double lonmean, double max_alt, double distance = 10000., int manual_zoom = -1); //adds seen map
	int addImageToMap(const char* str, double center_lat, double center_lon, double center_alt, double metric_width, int dem_resolution = -1); //-1 for no dem
	int removeMapTiles();

	//Gui like functions
	void hide();
	void show();

protected:
	virtual ~MapDrawable2() {}

};

#ifdef __cplusplus
}
#endif

#endif
