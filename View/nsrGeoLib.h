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
double getEarthRadius();

void setCCEFCenter(double _ecef_x, double _ecef_y, double _ecef_z);
osg::Vec3d ECEF2CCEF(osg::Vec3d ecef);
osg::Vec3d CCEF2ECEF(osg::Vec3d ecef);
void ECEF2CCEF(double &ecef_x, double &ecef_y, double &ecef_z);
void CCEF2ECEF(double &ecef_x, double &ecef_y, double &ecef_z);

osg::Vec3d ECEF2LLA(osg::Vec3d ecef, int on_earth = 0);
void LLA2ECEF(double lat, double lon, double alt, double &ecef_x, double &ecef_y, double &ecef_z);

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

int setCamera_LLA_QU(osg::Camera *cam, const osg::Vec3d lla_cam, const osg::Quat cam_quat);

///////////////////////////////

//convert between 2 NED-bent frames(not NED-parallel frame)
//bent NED frame(unlike NED-parallel frame) is accurate enough even in long multi 1000KMs
void Local2Local(double dx1, double dy1, double dz1,
				 double Lat1, double lon1, double alt1,
				 double Lat2, double lon2, double alt2, //inputs
				 double &dx2, double &dy2, double &dz2); //output

//Convert Lat(geocentric(deg)),lon(geocentric(deg)) & alt(m)
//to NED-bent frame(not NED-parallel frame)
//bent NED frame(unlike NED-parallel frame) is accurate enough even in long multi 1000KMs
void Local2Spherical(double dx, double dy, double dz,
					 double Lat0, double lon0, double alt0, //inputs
					 double &Lat, double &lon, double &alt); //output

//Convert NED bent frame(m) (not NED-parallel frame)
//to Lat(geocentric(deg)),lon(geocentric(deg)) & alt(m).
//bent NED frame(unlike NED-parallel frame) is accurate enough even in long multi 1000KMs
void Spherical2Local(double Lat, double lon, double alt,
					 double Lat0, double lon0, double alt0, //inputs
					 double &dx, double &dy, double &dz); //output

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
