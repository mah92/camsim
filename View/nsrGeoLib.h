#ifndef __NSRGEOLIB__
#define __NSRGEOLIB__

#include <osg/PositionAttitudeTransform>
#include <osg/Camera>
#include <osgDB/ReadFile>
#include <tiffio.h>

/*#ifdef __cplusplus
extern "C" {
#endif*/

#define M2FT 3.2808
#define MPS2KNOT 1.94384
#define MPS2FTPS M2FT
#define KM2NM 0.539957

class Pose
{
public:
	double lat;
	double lon;
	double alt; //wgs84 or priority
	int position_set;
	double yaw; //in deg
	double pitch;
	double roll;
	int attitude_set;
	Pose() : lat(0), lon(0), alt(0), position_set(0), yaw(0), pitch(0), roll(0), attitude_set(0) {}

	Pose(double _lat, double _lon, double _alt, double _roll, double _pitch, double _yaw) :
		lat(_lat), lon(_lon), alt(_alt), position_set(1), yaw(_yaw), pitch(_pitch), roll(_roll), attitude_set(1) {}
};

//////////////////////////////////////////

void lla2tile(double lat, double lon, int zoom,  double &x, double &y);
void tile2lla_rad(double x, double y, int zoom, double &lat, double &lon);
void tile2lla(double x, double y, int zoom, double &lat, double &lon);

void image2lla(double center_lat, double center_lon, double center_alt, double metric_width, double metric_height, double col_percent, double row_percent, double &lat, double &lon);

void setEarthScale(double scale);

osg::Vec3d ECEF2LLA(osg::Vec3d ecef, int on_earth = 0);
osg::Vec3d LLA2ECEF(osg::Vec3d lla);
void LLA2ECEF_rad(double lat_rad, double lon_rad, double alt, double &ecef_x, double &ecef_y, double &ecef_z);

osg::Vec3d ECEF2NED(osg::Vec3d ecef, double lat0, double lon0);
osg::Vec3d NED2ECEF(osg::Vec3d ned, double lat0, double lon0);

osg::Vec3d LLA2NED(osg::Vec3d lla, osg::Vec3d lla0);
osg::Vec3d NED2LLA(osg::Vec3d dr, osg::Vec3d lla0);

double X2LAT();
double Y2LON(double lat0);

void dLLA2dLocal(double lat0, double lon0, double lat, double lon, double &dx, double &dy);

double Geodetic2GeocentricLatRad(double lat);
double Geodetic2GeocentricLatDeg(double lat);
double Geocentric2GeodeticLatRad(double lat);
double Geocentric2GeodeticLatDeg(double lat);

double nsrGeoidGetHeight(double lat, double lon);

osg::Vec3d calcVfromLLA(const osg::Vec3d pre_lla, const osg::Vec3d lla, double dt);
osg::Vec3d calcVfromXYZ(const osg::Vec3d pre_xyz, const osg::Vec3d xyz, double dt);
osg::Vec3d calcAfromLLA(const osg::Vec3d lla1, const osg::Vec3d lla2, const osg::Vec3d lla3, double dt);
osg::Vec3d calcAfromXYZ(const osg::Vec3d xyz1, const osg::Vec3d xyz2, const osg::Vec3d xyz3, double dt);
osg::Vec3d calcWfromQu(osg::Quat pre_qu, osg::Quat qu, double dt);//resulting W is wrt. first axes, in second axes

///////////////////////////////
class DEM
{
public:
	uint16_t *raster;
	int lat;
	int lon;
	uint32 dem_width; //don't change data type, filled by pointer
	uint32 dem_height;

	DEM() : raster(NULL), lat(0), lon(0), dem_width(0), dem_height(0) {}
	~DEM();
};

void demInit();
int demLoad(DEM* pDem, int lat, int lon);
int demGetHeight(double lat, double lon, float &height);
void demClose();

////////////////////////////////

/*#ifdef __cplusplus
}
#endif*/

#endif
