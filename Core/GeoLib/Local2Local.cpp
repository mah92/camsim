#include "./nsrGeoLib2.h"
#include <math.h>

#undef TAG
#define TAG "C:Local2Local:"

#ifdef __cplusplus
extern "C" {
#endif

//convert between 2 NED-bent frames(not NED-parallel frame)
//bent NED frame(unlike NED-parallel frame) is accurate enough even in long multi 1000KMs
void Local2Local(double dx1, double dy1, double dz1,
				 double Lat1, double lon1, double alt1,
				 double Lat2, double lon2, double alt2, //inputs
				 double &dx2, double &dy2, double &dz2) //output
{
	double Lat, lon, alt;
	//[Lat,lon,alt]=Local2Spherical(Local, Spherical1);
	Local2Spherical(dx1, dy1, dz1,
					Lat1, lon1, alt1,
					Lat, lon, alt);
	//[dx,dy,dz]=Spherical2Local([Lat,lon,alt, Spherical2);
	Spherical2Local(Lat, lon, alt,
					Lat2, lon2, alt2,
					dx2, dy2, dz2);
}

#ifdef __cplusplus
}
#endif
