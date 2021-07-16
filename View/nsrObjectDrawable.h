#ifndef __nsrObjectDrawable__
#define __nsrObjectDrawable__

#include <osg/PositionAttitudeTransform>
#include "./nsrFilters.h"

#ifdef __cplusplus
extern "C" {
#endif

class ObjectDrawable: public osg::PositionAttitudeTransform
{
	bool hidden;
	osg::Vec3d hideposition;
    double lat, lon, alt_wgs84, alt_msl;
    double speed, inbetween_position_percent;
    bool onEarth;
    
    double last_t;
    std::vector<osg::Vec3d> llaPath; //alt from asl, alt not used if onEarth == true
    int current_point, next_point;
    
    SmoothPathFollower* smoother[6]; //6DOF smoothing of path
public:

	ObjectDrawable() : 
        PositionAttitudeTransform(), lat(0), lon(0), alt_wgs84(0), alt_msl(0),
        current_point(-1), next_point(-1), inbetween_position_percent(0.), last_t(-1) 
    {
        for(int i=0; i<6; i++) 
            smoother[i] = new SmoothPathFollower(); //NULL
    }
    
    ~ObjectDrawable()
    {
        for(int i=0; i<6; i++) 
            delete smoother[i];
    }
	
    int add3DFile(const char* file_addr, float scale = 1.);
    
    //geodetical WGS84 lattitude,
    //ref defined in enum AltRef {ALTREF_UNKNOWN=-1, ALTREF_WGS84=0, ALTREF_ASL, ALTREF_AGL, ALTREF_AHL};
	void setPositionLatLonAlt(double _lat, double _lon, double _alt_offset, int alt_ref); 

    //in rad
	void setRPY(double roll, double pitch, double yaw);

    //speed in m/s
    void setPath(char* _pathFile, bool _onEarth, double _phase, double _speed);
    void updatePath(double t);
    void updatePathByParameters();

	void hide();
	void show();
};

extern std::vector<osg::ref_ptr<ObjectDrawable> > allObjectDrawables;

#ifdef __cplusplus
}
#endif

#endif
