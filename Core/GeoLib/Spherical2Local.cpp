#include "./nsrGeoLib.h"
#include <math.h>

#undef TAG
#define TAG "C:Spherical2Local:"

#ifdef __cplusplus
extern "C" {
#endif

//Convert NED bent frame(m) (not NED-parallel frame)
//to Lat(geocentric(deg)),lon(geocentric(deg)) & alt(m).
//bent NED frame(unlike NED-parallel frame) is accurate enough even in long multi 1000KMs
void Spherical2Local(double Lat, double lon, double alt,
					 double Lat0, double lon0, double alt0, //inputs
					 double &dx, double &dy, double &dz) //output
{
	double _REarth, _pi;

	_REarth = 6378137;
	_pi = 3.1415926535;

	dx = (Lat - Lat0) * (_pi / 180) * _REarth;
	dy = (lon - lon0) * (_pi / 180) * _REarth * cos(Lat0 / 180 * _pi);
	dz = -(alt - alt0); //z is negetive
}

#ifdef __cplusplus
}
#endif