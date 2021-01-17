#ifndef __NSR_GEO_LIB_H__
#define __NSR_GEO_LIB_H__

#ifdef __cplusplus
extern "C" {
#endif

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

#ifdef __cplusplus
}
#endif

#endif
