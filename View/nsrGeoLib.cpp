
#ifndef __ANDROID__

#include"/usr/include/time.h"
#endif

#include "nsrGeoLib.h"
#include "Core/nsrCore.h"
#include "ThirdParty/geoid.h"

#undef TAG
#define TAG "Cpp:GeoLib:"

/*#ifdef __cplusplus
extern "C" {
#endif*/

//////////////////////////////////////////////////////////////
//- GlobalMercator (based on EPSG:900913 = EPSG:3785)
//       for Google Maps, Yahoo Maps, Microsoft Maps compatible tile
//       Google earth is different(GlobalGeodetic (based on EPSG:4326))
//ref: https://wiki.openstreetmap.org/wiki/Slippy_map_tilename
//ref: http://www.maptiler.org/google-maps-coordinates-tile-bounds-projection/s
//google uses spherical mercator so all latitudes are in spherical system(geocentric)

/*    The lat/lon coordinates are using WGS84 datum, yeh?
      Yes, all lat/lon we are mentioning should use WGS84 Geodetic Datum.
      Well, the web clients like Google Maps are projecting those coordinates by
      Spherical Mercator, so in fact lat/lon coordinates on sphere are treated as if
      the were on the WGS84 ellipsoid.

      From MSDN documentation:
      To simplify the calculations, we use the spherical form of projection, not
      the ellipsoidal form. Since the projection is used only for map display,
      and not for displaying numeric coordinates, we don't need the extra precision
      of an ellipsoidal projection. The spherical projection causes approximately
      0.33 percent(1/300) scale distortion in the Y direction, which is not visually noticable.
*/
#undef pi
#define pi 3.1415926535
//input: x: from left to right, 90deg at greenwitch
//input: y: from top to down, 0 at north pole, max int: two_pow_zoom-1, max float: two_pow_zoom
//output: geodetic lat & lon in rad, origin at top-left of image
void tile2lla_rad(double x, double y, int zoom, double &lat, double &lon)
{
	double n, two_pow_zoom;
	two_pow_zoom = 0x1 << zoom;
	lon = x / two_pow_zoom * 2.*pi - pi; //linear

	n = pi - 2 * pi * y / two_pow_zoom; //n: from pi to -pi
	lat = atan(0.5 * (exp(n) - exp(-n))); // atan(sinh(n))
}

//output: geodetic lat(deg) & lon(deg), origin at top-left of image
void tile2lla(double x, double y, int zoom, double &lat, double &lon)
{
	tile2lla_rad(x, y, zoom, lat, lon);
	lat *= 180 / pi;
	lon *= 180 / pi;
}

//input: geodetic lat(deg) & lon(deg)
void lla2tile(double lat, double lon, int zoom, double &x, double &y)
{
	assert(lat <= 85.05112878);

	double n, m, exp_n, two_pow_zoom;
	two_pow_zoom = 0x1 << zoom;

	x = (lon / 180 + 1) / 2 * two_pow_zoom;
	m = 2 * tan(lat * pi / 180); //==exp(n)-exp(-n)
	exp_n = m / 2 + sqrt(m * m + 4) / 2;
	n = log(exp_n);
	y = (pi - n) / (2.*pi) * two_pow_zoom;
}

/*
int long2tilex(double lat, double lon, int z)
{
	return (lon + 180.0) / 360.0 * (1 << z); //long
    double latrad = lat * M_PI/180.0;
	return (1.0 - asinh(tan(latrad)) / M_PI) / 2.0 * (1 << z);  //lat
}*/

///////////////////////////////////////////////////////////////
//input geocentric lat(deg), lon(deg), alt(m, from spherical earth)
///TODO: Also geodetic but up to 1/300 scale factor error when used alt is from ellipsoid
//row percent in row increase direction(up to down)
//col percent in col increase direction(left to right)
void image2lla(double center_lat, double center_lon, double center_alt, double metric_width, double metric_height, double col_percent, double row_percent, double &lat, double &lon)
{
	osg::Vec3d lla = NED2LLA(osg::Vec3d(- metric_height * (row_percent - 0.5), metric_width * (col_percent - 0.5), 0),
							 osg::Vec3d(center_lat, center_lon, center_alt));
	lat = lla.x();
	lon = lla.y();
}

//GEODETIC/////////////////////////////////////////////////////////////

// WGS84 ellipsoid constants
static double EarthRadius = 6378137; // radius(a)
static double e = 8.1819190842622e-2;  // eccentricity
static double a2 = EarthRadius * EarthRadius;
static double e2 = e * e;

static double _scale = 1.;
void setEarthScale(double scale)
{
	_scale = scale;
	EarthRadius = scale * 6378137;
	a2 = EarthRadius * EarthRadius;
}

/*
 *  ECEF - Earth Centered Earth Fixed(m)
 *  LLA - Geodetic Lat(deg, WGS84), Lon(deg), Alt(WGS84)
 *  ported from https://gist.github.com/klucar/1536194
 */
osg::Vec3d ECEF2LLA(osg::Vec3d ecef, int on_earth)
{
	double x = ecef.x();
	double y = ecef.y();
	double z = ecef.z();

	double b = sqrt(a2 * (1 - e2));
	double bsq = pow(b, 2);
	double ep = sqrt((a2 - bsq) / bsq);
	double p = sqrt(pow(x, 2) + pow(y, 2));
	double th = atan2(EarthRadius * z, b * p);

	double lon = atan2(y, x);
	double lat = atan2((z + pow(ep, 2) * b * pow(sin(th), 3)), (p - e2 * EarthRadius * pow(cos(th), 3)));
	double N = EarthRadius / (sqrt(1 - e2 * pow(sin(lat), 2)));
	double alt = p / cos(lat) - N;

	if(on_earth == 1)
		alt = 0;
	// mod lat to 0-2pi
	lon = normalize_angle(lon);

	// correction for altitude near poles left out.
	return osg::Vec3d(lat * 180 / pi, lon * 180 / pi, alt);
}

/*
 *  ECEF - Earth Centered Earth Fixed(m)
 *  LLA - Geodetic Lat(rad, WGS84) Lon(rad) Alt(WGS84)
 *  ported from https://gist.github.com/klucar/1536194
 */
void LLA2ECEF_rad(double lat_rad, double lon_rad, double alt, double &ecef_x, double &ecef_y, double &ecef_z)
{
	double sinlat = sin(lat_rad);
	double sin2lat = sinlat * sinlat;
	double coslat = cos(lat_rad);
	double sinlon = sin(lon_rad);
	double coslon = cos(lon_rad);

	// intermediate calculation
	// (prime vertical radius of curvature)
	double N = EarthRadius / sqrt(1. - e2 * sin2lat);

	//ecef_x = lon_rad*100.;
	//ecef_y = -lat_rad*100.;
	//ecef_z = 0.;
	ecef_x = (N + alt) * coslat * coslon;
	ecef_y = (N + alt) * coslat * sinlon;
	ecef_z = ((1. - (e2)) * N + alt) * sinlat;
}

/*
 *  ECEF - Earth Centered Earth Fixed(m)
 *  LLA - Geodetic Lat(deg, WGS84) Lon(deg) Alt(WGS84)
 */
osg::Vec3d LLA2ECEF(osg::Vec3d lla)
{
	double ecef_x, ecef_y, ecef_z;
	double lat = lla.x() * pi / 180;
	double lon = lla.y() * pi / 180;
	double alt = lla.z();

	LLA2ECEF_rad(lat, lon, alt, ecef_x, ecef_y, ecef_z);

	return osg::Vec3d(ecef_x, ecef_y, ecef_z);
}

/*
 *  LLA - Geodetic Lat(deg) Lon(deg) Alt(m, WGS84)
 *  DR - Diff vector in first Lat-lon NED frame
 */
osg::Vec3d LLA2NED(osg::Vec3d lla, osg::Vec3d lla0)
{
	osg::Vec3d R0, R, dR;
	R0 = LLA2ECEF(lla0);
	R  = LLA2ECEF(lla);
	dR = R - R0;
	double lat = lla0.x();
	double lon = lla0.y();
	lat = Geodetic2GeocentricLatDeg(lat);
	return ECEF2NED(dR, lat, lon);
}

/*
 * DR - Diff vector in Local lla0-Parallel NED frame(m)
 * LLA - Geodetic Lat(deg), Lon(deg), Alt(m, from WGS84 ellipsoid)
 * Experimental
 */
osg::Vec3d NED2LLA(osg::Vec3d dr, osg::Vec3d lla0)
{
	osg::Vec3d R0, R, dR;
	double lat0 = Geodetic2GeocentricLatDeg(lla0.x());
	dR = NED2ECEF(dr, lat0, lla0.y());
	R0 = LLA2ECEF(lla0);
	R = dR + R0;
	return ECEF2LLA(R);
}

//GEOCENTRIC////////////////////////////////////////
//assuming spherical earth
double X2LAT()
{
	return (1 / EarthRadius * 180. / M_PI);
}

double Y2LON(double lat0)
{
	return (1 / (EarthRadius * cos(lat0 * M_PI / 180.)) * 180. / M_PI);
}

/*
 * Converts a vector from ECEF coord to NED coord
 *  ECEF - Earth Centered Earth Fixed(m)
 *  LLA - Geocentric Lat(deg), Lon(deg)
 */
osg::Vec3d ECEF2NED(osg::Vec3d ecef, double lat0, double lon0)
{
	lat0 *= pi / 180;
	lon0 *= pi / 180;

	/*osg::Matrix3d M_ecef2ned=[-sin(lat0)*cos(lon0) -sin(lat0)*sin(lon0) cos(lat0);
			   -sin(lon0)            cos(lon0)                0;
			   -cos(lat0)*cos(lon0) -cos(lat0)*sin(lon0) -sin(lat0)];
	return M_ecef2ned*ecef;*/
	return osg::Vec3d(-sin(lat0) * cos(lon0) * ecef.x() - sin(lat0) * sin(lon0) * ecef.y() + cos(lat0) * ecef.z(),
					  -sin(lon0)          * ecef.x() + cos(lon0)          * ecef.y() 			+ 0,
					  -cos(lat0) * cos(lon0) * ecef.x() - cos(lat0) * sin(lon0) * ecef.y() - sin(lat0) * ecef.z());
}

/*
 * Converts a vector from NED coord to ECEF coord
 *  LLA - Geocentric Lat(deg), Lon(deg)
 *  return: ECEF - Earth Centered Earth Fixed(m)
 */
osg::Vec3d NED2ECEF(osg::Vec3d ned, double lat0, double lon0)
{
	lat0 *= pi / 180;
	lon0 *= pi / 180;

	return osg::Vec3d(-sin(lat0) * cos(lon0) * ned.x() - sin(lon0)           * ned.y() - cos(lat0) * cos(lon0) * ned.z(),
					  -sin(lat0) * sin(lon0) * ned.x() + cos(lon0)           * ned.y() - cos(lat0) * sin(lon0) * ned.z(),
					  +cos(lat0)           * ned.x() + 0.                  *ned.y() - sin(lat0)           * ned.z());
}

/*
 * DR - Diff vector in Local Spheroid-Parallel NED frame(m)
 * LLA - Geocentric Lat(deg), Lon(deg), Alt(m, from spheroid)
 * TODO: up to 1/300 scale/alt error
 */
/*osg::Vec3d NED2LLA2(osg::Vec3d dr, osg::Vec3d lla0)
{
	double lat0 = lla0.x(), lon0 = lla0.y(), alt0 = lla0.z();
	double lat, lon, alt;
	lat=dr.x()/(EarthRadius+alt0)*(180/pi)+lat0;
	lon=dr.y()/(EarthRadius+alt0)*(180/pi)/cos(lat0*(pi/180))+lon0;
	alt=-dr.z()+alt0; //z(in dr) is negetive

	osg::Vec3d lla(lat, lon, alt);
	return lla;
}*/

/*
 * LLA - Geocentric Lat(deg), Lon(deg)
 * DR - Local Spheroid-Parallel NED frame(m)
 */
void dLLA2dLocal(double lat0, double lon0, double lat, double lon, double &dx, double &dy)
{
	dx = (lat - lat0) * (EarthRadius * (pi / 180.));
	dy = (lon - lon0) * cos(lat0 * (pi / 180.)) * (EarthRadius * (pi / 180.));
	//dz=-(alt-alt0); //z is negetive
}

//(rad)
//source: https://en.wikipedia.org/wiki/Latitude
//These latittudes have about 40Km/0.2 deg/11.5 arcmin difference at a geodetic latitude of approximately 45
//They are equal in equator and poles
double Geodetic2GeocentricLatRad(double lat)
{
	return atan(tan(lat) * (1 - e2));
}

//in deg
double Geodetic2GeocentricLatDeg(double lat)
{
	return atan(tan(lat * pi / 180.) * (1 - e2)) * 180. / pi;
}

double Geocentric2GeodeticLatRad(double lat)
{
	return atan(tan(lat) / (1 - e2));
}

//in deg
double Geocentric2GeodeticLatDeg(double lat)
{
	return atan(tan(lat * pi / 180.) / (1 - e2)) * 180. / pi;
}

//////////////////////////////////////////////////////////
//validated by https://geographiclib.sourceforge.io/cgi-bin/GeoidEval
struct geoid_ctx * _geoid_ctx = NULL;

///TODO: ?? lat(deg), lon(deg)
///TODO: validate with https://geographiclib.sourceforge.io/cgi-bin/GeoidEval
double nsrGeoidGetHeight(double lat, double lon)
{
	if(_geoid_ctx == NULL) {
		char geoid_pgm_filename[200];
		//sprintf(geoid_pgm_filename, "%s/%s", globals.datapath, "egm96-5min.pgm");
		sprintf(geoid_pgm_filename, "%s/%s", globals.datapath, "egm96-15min.pgm");
		_geoid_ctx = geoid_init(geoid_pgm_filename);
	}

	return geoid_height_linear(_geoid_ctx, lat, lon);
}

//////////////////////////////////////////////////////////
//assuming spherical earth
osg::Vec3d calcVfromLLA(const osg::Vec3d pre_lla, const osg::Vec3d lla, double dt)
{
	double dx, dy;
	osg::Vec3d v;

	dLLA2dLocal(pre_lla.x(), pre_lla.y(), lla.x(), lla.y(), dx, dy);
	v = osg::Vec3d(dx / dt, dy / dt, -(lla.z() - pre_lla.z()) / dt);

	return v;
}

osg::Vec3d calcAfromLLA(const osg::Vec3d lla1, const osg::Vec3d lla2, const osg::Vec3d lla3, double dt)
{
	osg::Vec3d a;

	//deprecated, low accuracy///
	/*
	osg::Vec3d v1, v2;
	v1 = calcVfromLLA(lla1, lla2, dt);
	v2 = calcVfromLLA(lla2, lla3, dt);
	a = calcAfromV(v1, v2, dt);*/

	//double dx1, dx2, dy1, dy2, dz1, dz2;
	//dLLA2dLocal(lla1.x(), lla1.y(), lla2.x(), lla2.y(), dx1, dy1);
	//dLLA2dLocal(lla2.x(), lla2.y(), lla3.x(), lla3.y(), dx2, dy2);

	double dt2inv = 1. / dt / dt;
	a.x() = (lla3.x() - 2 * lla2.x() + lla1.x()) * (EarthRadius * (pi / 180.)) * dt2inv;
	a.y() = (lla3.y() - 2 * lla2.y() + lla1.y()) * cos(lla2.x() * (pi / 180.)) * (EarthRadius * (pi / 180.)) * dt2inv;
	a.z() = -(lla3.z() - 2 * lla2.z() + lla1.z()) * dt2inv;

	return a;
}

/*osg::Vec3d calcAfromXYZ(const osg::Vec3d xyz1, const osg::Vec3d xyz2, const osg::Vec3d xyz3, double dt)
{
	osg::Vec3d v1, v2, a;
	v1 = calcVfromXYZ(xyz1, xyz2, dt);
	v2 = calcVfromXYZ(xyz2, xyz3, dt);

	a = calcAfromV(v1, v2, dt);
	return a;
}*/

//resulting W is wrt. first axes, in second axes
osg::Vec3d calcWfromQu(osg::Quat pre_qu, osg::Quat qu, double dt)
{
	double len, angle = 0.;
	osg::Quat dQu;
	osg::Vec3d w, unitvect;

	qu = qu / qu.length();
	pre_qu = pre_qu / pre_qu.length();

	//LOGI(TAG, "((%f, %f, %f, %f)", qu.x(), qu.y(), qu.z(), qu.w());
	//ref: https://math.stackexchange.com/questions/2282938/converting-from-quaternion-to-angular-velocity-then-back-to-quaternion
	//w ~= Im(2*little quaternion)/dt = 2*(qu*pre_qu.conj())/dt;
	//osg::Quat dQu = qu*pre_qu.conj();
	//return osg::Vec3d(2*dQu.x()/dt, 2*dQu.y()/dt, 2*dQu.z()/dt);

	//ref: http://answers.unity3d.com/questions/49082/rotation-quaternion-to-angular-velocity.html
	//w = 2*ln(qu*pre_qu.conj())/dt = unitvec*angle/dt
	dQu = qu * pre_qu.conj();
	//LOGI(TAG, "((%f, %f, %f, %f)", dQu.x(), dQu.y(), dQu.z(), dQu.w());

	unitvect = osg::Vec3d(dQu.x(), dQu.y(), dQu.z());
	len = unitvect.length();
	if(equals(len, 0., 1e-12) == 0) { //nonzero
		unitvect /= len;
		//Error cutting
		dQu.w() = dQu.w() > 1. ? 1. : dQu.w();
		dQu.w() = dQu.w() < -1. ? -1. : dQu.w();
		angle = 2 * acos(dQu.w());
		w = unitvect * (angle / dt);
	} else {
		w = osg::Vec3d(0., 0., 0.);
	}

	return w;
}

//////////////////////////////////////////////////////////////////////////////
#define MAX_DEM_IN_MEM 4
DEM dem[MAX_DEM_IN_MEM];

DEM::~DEM()
{
	if(raster != NULL)
		_TIFFfree(raster);
}

int demLoad(DEM* pDem, int lat, int lon)
{
	char str[MAX_PATH_LENGTH];
	double time_s = myTime();
	//i.e:n27_e051_1arc_v3.tif //dem name always points to lower-left of region
	//sprintf(str, "%s/%c%02i_%c%03i_1arc_v3.tif", settings.dempath, lat>=0?'n':'s', (int)(lat>=0?lat:1-lat), lon>=0?'e':'w', (int)(lon>=0?lon:1-lon) ); //for float input
	//sprintf(str, "/storage/usb3host/dem/IRAN/%c%02i_%c%03i_1arc_v3.tif", lat>=0?'n':'s', (int)(lat>=0?lat:1-lat), lon>=0?'e':'w', (int)(lon>=0?lon:1-lon) ); //for float input
	//sprintf(str, "/mnt/sdcard/PitechGCS/dem/tehran/%c%02i_%c%03i_1arc_v3.tif", lat>=0?'n':'s', (int)(lat>=0?lat:1-lat), lon>=0?'e':'w', (int)(lon>=0?lon:1-lon) ); //for float input
	//sprintf(str, "/mnt/sdcard/PitechGCS/dem/tehran/%c%02i_%c%03i_1arc_v3.tif", lat>=0?'n':'s', lat>=0?lat:-lat, lon>=0?'e':'w', lon>=0?lon:-lon ); //for int input
	sprintf(str, "%s/%c%02i_%c%03i_1arc_v3.tif", settings.dempath, lat >= 0 ? 'n' : 's', lat >= 0 ? lat : -lat, lon >= 0 ? 'e' : 'w', lon >= 0 ? lon : -lon); //for int input

	pDem->lat = nsrFloor(lat);
	pDem->lon = nsrFloor(lon);

	//osg::ref_ptr<osg::Image> im;
	//im = osgDB::readImageFile(std::string(str));
	//if(im == NULL){
	//	LOGW(TAG, " Image not found at (%s)!\n", str);
	//	//return -1;
	//}

	//LOGI(TAG, " Search image at (%i, %i)!\n", lat, lon);

	//TIFFErrorHandler needed for linux
	TIFFErrorHandler oldhandler;
	oldhandler = TIFFSetWarningHandler(NULL);

	TIFF* tif = TIFFOpen(str, "r");
	if(tif == NULL) {
		//LOGI(TAG, " NULL tiff image at (%s)!\n", str);
		return -1;
	}

	TIFFSetWarningHandler(oldhandler);

	uint32 width, height;
	uint16 BitsPerSample;           // normally 8 for grayscale image, in dem 16
	uint16 SamplesPerPixel;         // normally 1 for grayscale image

	TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &pDem->dem_width);
	TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &pDem->dem_height);
	TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &BitsPerSample);
	TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &SamplesPerPixel);
	//LOGI(TAG, "%i, %i, %i, %i", (int)pDem->dem_width, (int)pDem->dem_height, BitsPerSample, SamplesPerPixel);
	if(SamplesPerPixel > 1) {
		LOGI(TAG, " Multisample pixel!"); //every pixel can have multiple values, stored beside each other
		return -2;
	}

	if(pDem->raster == NULL) //just malloc if not malloc before
		pDem->raster = (uint16_t *) _TIFFmalloc(pDem->dem_width * pDem->dem_height * (BitsPerSample / 8));
	unsigned char *scanline = NULL;//(unsigned char *)_TIFFmalloc(dem_width*sizeof(uint16_t));
	int row;
	for(row = 0; row < pDem->dem_height; row++) {
		scanline = (unsigned char *)pDem->raster + row * pDem->dem_width * sizeof(uint16_t);
		TIFFReadScanline(tif, scanline, row, 0);
	}

	//origin of the raster is at the lower left corner.
	//getHeight(35., 51.);
	//getHeight(35.99999, 51.);
	//LOGW(TAG, " Search time(%f)!\n", time_s-myTime()); //(0.4s->sdcard, 0.2s->tablet) + 26MB Ram for loading each image

	if(tif != NULL)
		TIFFClose(tif);
	return 0;
}

//rows:2317,2311,2311,2310,.....,798
//input geodetic lat(deg), lon(deg) (WGS84 horizontal datum)
//output alt(m) from EGM96(EGM96 vertical datum)
int demGetHeight(double lat, double lon, float &height)
{
	int row, col, w;
	int i, slot_index;
	float max_dist, dist;
	bool dem_slot_found = false;

	//find dem if previously loaded
	for(i = 0; i < MAX_DEM_IN_MEM; i++)
		if(dem[i].raster != NULL && nsrFloor(lat) == dem[i].lat && nsrFloor(lon) == dem[i].lon) {
			slot_index = i;
			dem_slot_found = true;
			break;
		}

	if(dem_slot_found == false) { //needing to load dem
		//find an empty slot
		for(i = 0; i < MAX_DEM_IN_MEM; i++)
			if(dem[i].raster == NULL) {
				slot_index = i;
				dem_slot_found = true;
				break;
			}

		//if no empty slot, find the most irrelevant
		if(dem_slot_found == false) {
			max_dist = 0;
			slot_index = 0;
			for(i = 0; i < MAX_DEM_IN_MEM; i++) {
				dist = abs(dem[i].lat + 0.5 - lat) + abs(dem[i].lon + 0.5 - lon); //Manhattan distance
				if(dist > max_dist) {
					max_dist = dist;
					slot_index = i;
				}
			}
			dem_slot_found = true;
		}

		//LOGI(TAG, " Search image at (%f, %f @%i)!\n", lat, lon, slot_index);

		//load dem
		if(0 != demLoad(&dem[slot_index], nsrFloor(lat), nsrFloor(lon))) {
			height = 0; //assume ocean
			LOGI(TAG, " Dem not found!(%i,%i)\n", nsrFloor(lat), nsrFloor(lon));
			return -1; //dem not found
		} else {
			LOGI(TAG, " Dem loaded!(%i,%i@%i)\n", nsrFloor(lat), nsrFloor(lon), slot_index);
		}
	}

	row = nsrRound((nsrFloor(lat) + 1 - lat) * 3600); //rows increase from top to bottom
	col = nsrRound((lon - nsrFloor(lon)) * 3600);
	w = dem[slot_index].dem_width;
	height = dem[slot_index].raster[row * w + col];	//matrix unit is in meters(uint16_t)

	if(height > 32768) height = height - 65536; //verified
	if(height < -1000 || height > 10000) {  // -32767:no data
		height = 0; //assume ocean
		return -2; //dem no data
	}

	height *= _scale;
	//LOGI(TAG, "height(m):%f \t", height); //just read first sample
	return 0;
}

void demClose()
{
}
