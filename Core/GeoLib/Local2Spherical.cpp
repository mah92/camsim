#include "./nsrGeoLib.h"
#include <math.h>

#undef TAG
#define TAG "C:Local2Spherical:"

#ifdef __cplusplus
extern "C" {
#endif

//Convert Lat(geocentric(deg)),lon(geocentric(deg)) & alt(m)
//to NED-bent frame(not NED-parallel frame)
//bent NED frame(unlike NED-parallel frame) is accurate enough even in long multi 1000KMs
void Local2Spherical(double dx, double dy, double dz,
					 double Lat0, double lon0, double alt0, //inputs
					 double &Lat, double &lon, double &alt) //output
{
	double _REarth, _pi;

	_REarth = 6378137;
	_pi = 3.1415926535;

	Lat = dx / _REarth * (180 / _pi) + Lat0;
	lon = dy / _REarth * (180 / _pi) / cos(Lat0 * (_pi / 180)) + lon0;
	alt = -dz + alt0; //z is negetive
}

#ifdef __cplusplus
}
#endif