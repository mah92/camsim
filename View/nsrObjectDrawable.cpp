#ifndef __ANDROID__
#include "/usr/include/time.h"
#endif

#include "nsrObjectDrawable.h"
#include "nsrGeoLib.h"

#include "Core/nsrCore.h"

#include "pugixml/pugixml.hpp"

//#include <osg/Light>
//#include <osg/LightSource>
#include <osgDB/ReadFile>
#include <osg/Texture2D>
#include <osg/TexGen>

#undef TAG
#define TAG "Cpp:OsgObjectDrawable:"

#ifdef __cplusplus
extern "C" {
#endif

std::vector<osg::ref_ptr<ObjectDrawable> > allObjectDrawables;
    

int parseCoordLine(const char* linebuf, int linelen, double* vh_lat, double* vh_lon, double* vh_alt)
{
	int foundlen;
	int foundFrom, foundTo;
	char tmpstr[20];

#define GET_INT_IN_LINE(var) \
	{ \
		foundlen = findIntInLine(linebuf, foundTo+1, linelen-1, &foundFrom, &foundTo); \
		if(foundlen <= 0) return -1; \
		memcpy(tmpstr, &linebuf[foundFrom], foundTo-foundFrom+1); \
		tmpstr[foundTo-foundFrom+1] = '\0'; \
		var = atoi(tmpstr); \
	}
#define GET_FLOAT_IN_LINE(var) \
	{ \
		foundlen = findFloatInLine(linebuf, foundTo+1, linelen-1, &foundFrom, &foundTo); \
		if(foundlen <= 0) return -1; \
		memcpy(tmpstr, &linebuf[foundFrom], foundTo-foundFrom+1); \
		tmpstr[foundTo-foundFrom+1] = '\0'; \
		var = atof(tmpstr); \
	}

	foundTo = -1;

    GET_FLOAT_IN_LINE(*vh_lon);
    GET_FLOAT_IN_LINE(*vh_lat);
	GET_FLOAT_IN_LINE(*vh_alt);

	return 0;
}

//Gazebo requires that mesh files be formatted as STL, Collada(.dae) or Alias Wavefront(.OBJ), with Collada and OBJ being the preferred formats
//Collada and OBJ file formats allow you to attach materials to the meshes. Use this mechanism to improve the visual appearance of your meshes.
//STL, however, cannot store information related to appearance.
//So we can used obj or dae files as the main 3d file format in openscenegraph
//Collada(.dae) requires Collada DOM library. See Collada plugin details
//In tests, all Collada files where loaded black
    
//For editing models, blender is a good tool in linux
    
//For downloading: 3dexport.com
// downloadfree3d.com
//free3d.com
    
int ObjectDrawable::add3DFile(const char* file_addr, float scale)
{
    
    // Create a sphere
    /*osg::ref_ptr<osg::ShapeDrawable> sd = new osg::ShapeDrawable(new osg::Sphere(osg::Vec3(0.0 , 0.0 , 0.0) , osg::WGS_84_RADIUS_POLAR)) ;
 
    osg::ref_ptr<osg::Geode> geode = new osg::Geode() ;
    geode->addDrawable(sd.get()) ;
 
         // Set texture
    std::string filename("Images/1.jpg") ;
    geode->getOrCreateStateSet()->setTextureAttributeAndModes(0 , new osg::Texture2D(osgDB::readImageFile(filename))) ;
 
         // Create a coordinate system node
    osg::ref_ptr<osg::CoordinateSystemNode> csn = new osg::CoordinateSystemNode;
         // Set the ellipsoid model, the default coordinate system is WGS-84
    csn->setEllipsoidModel(new osg::EllipsoidModel) ;
	csn->addChild(geode.get());   //release on node does not remove object data! just some data used in the creation process

    addChild(csn);
    return 0;*/
 
    //osgDB::Options* opt = new osgDB::Options;
    //opt->setOptionString("DIFFUSE=0");
    osg::ref_ptr<osg::Node> loadedModel = osgDB::readNodeFile(file_addr /*, opt*/); //importing object

    /*osg::ref_ptr<osg::Image> image = osgDB::readImageFile(tex_addr); //loading texture from file
    
    if(image.get()) {
        osg::ref_ptr<osg::StateSet> ss = this->getOrCreateStateSet();
        
        osg::ref_ptr<osg::Texture2D> tex(new osg::Texture2D());  //creating texture for model from image
        tex->setImage(image);
        ss->setTextureAttributeAndModes(0, tex);
    
        osg::ref_ptr<osg::TexGen> texGen(new osg::TexGen());
        texGen->setPlane(osg::TexGen::S, osg::Plane(0.075, 0.0, 0.0, 0.5));
        texGen->setPlane(osg::TexGen::T, osg::Plane(0.0, 0.035, 0.0, 0.3));
        ss->setTextureAttributeAndModes(0, texGen);
    }*/
    
    removeChild(0, 1); //added to remove memory leak when recreating an object
	addChild(loadedModel.release());   //release on node does not remove object data! just some data used in the creation process
    
    this->setScale(osg::Vec3d(scale, scale, scale));

    return 0;
}
    
void ObjectDrawable::setPositionLatLonAlt(double _lat, double _lon, double _alt_offset, int alt_ref)
{
    double ecef_x, ecef_y, ecef_z;
    
    lat = _lat;
    lon = _lon;
    
    float _alt;
    
    if(alt_ref == ALTREF_AGL) {
        demGetHeight(_lat, _lon, _alt);
        _alt += _alt_offset;
        alt_msl = _alt; //alt from EGM96
        
        _alt += nsrGeoidGetHeight(_lat, _lon); //Convert alt from EGM96 to WGS84
        alt_wgs84 = _alt; //alt from WGS84
        //printf("((%f, %f, %f\n\n\n\n", agl, nsrGeoidGetHeight(lat, lon), alt);
    }
    
    if(alt_ref == ALTREF_ASL) {
        _alt = _alt_offset;
        alt_msl = _alt; //alt from EGM96
        
        _alt += nsrGeoidGetHeight(_lat, _lon); //Convert alt from EGM96 to WGS84
        alt_wgs84 = _alt; //alt from WGS84
        //printf("((%f, %f, %f\n\n\n\n", agl, nsrGeoidGetHeight(lat, lon), alt);
    }
    
    if(alt_ref == ALTREF_WGS84) {
        _alt = _alt_offset;        
        alt_wgs84 = _alt; //alt from WGS84
        
        _alt -= nsrGeoidGetHeight(_lat, _lon); //Convert alt from WGS84 to EGM96
        alt_msl = _alt; //alt from EGM96
        //printf("((%f, %f, %f\n\n\n\n", agl, nsrGeoidGetHeight(lat, lon), alt);
    }
    
    LLA2ECEF(_lat, _lon, alt_wgs84, ecef_x, ecef_y, ecef_z); //geodetic lat, WGS84 alt
    //printf("ecef:%f, %f, %f\n", ecef_x, ecef_y, ecef_z);
    ECEF2CCEF(ecef_x, ecef_y, ecef_z);
    //printf("ccef:%f, %f, %f\n", ecef_x, ecef_y, ecef_z);
    setPosition(osg::Vec3d(ecef_x, ecef_y, ecef_z));
}

void ObjectDrawable::setRPY(double roll, double pitch, double yaw)
{
    osg::Quat q_cam, q_lon, q_lat, q_mid, q_yaw, q_pitch, q_roll, q_last, q_last2, q_all;

	//We are now in ecef
	q_lon = osg::Quat(osg::inDegrees(lon), osg::Z_AXIS);
	q_lat = osg::Quat(osg::inDegrees(-lat), osg::Y_AXIS); //latitude is reverse rotation along Y_AXIS of earth

	//we are now in spherical coordinates

	q_mid = osg::Quat(osg::inDegrees(-90.), osg::Y_AXIS); //later, we need a rotation along y to get to NED

	//we are now in NED coordinates

	//AC attitude/////////////
	q_yaw = osg::Quat(yaw, osg::Z_AXIS);
	q_pitch = osg::Quat(pitch, osg::Y_AXIS);
	q_roll = osg::Quat(roll, osg::X_AXIS);

	q_all = q_roll * q_pitch * q_yaw * q_mid * q_lat * q_lon; //right applied first
	
	setAttitude(q_all);
}


void ObjectDrawable::setPath(char* _pathFile, bool _onEarth, double _phase, double _speed)
{
    speed = _speed;
    onEarth = _onEarth;
    if(_phase < 0.) _phase = RAND(0., 1.);
    printf("::%f\n", _phase);
    //Read kml file///////////////////////////////////////////////////
    char node_name[MAX_PARAM_LENGTH];
	pugi::xml_parse_result result;

	pugi::xml_document calibDoc;
	result = calibDoc.load_file((std::string(_pathFile)).c_str());
	if(!result) {
		LOGE(TAG, " %s loaded with error: %s, using defaults...\n", _pathFile, result.description());
		exit(1);
		return;
	}
	LOGI(TAG, " %s loaded successfully!(%s)\n", _pathFile, result.description()); fflush(stdout);

    const char* str;
    
    //Note: kml usually gives altitude as abosulte meaning above sea level(egm96)
    
    //line/shape:
    // Search for the first matching entry
    std::string searchStr = "kml/Document/Placemark/LineString/coordinates";
    pugi::xpath_node xpathNode = calibDoc.select_single_node(searchStr.c_str());
    if (xpathNode) {
        pugi::xml_node selectedNode = xpathNode.node();
        str = selectedNode.text().as_string();
    }
    
    //polygon(a normal line/shape with end=start):
    std::string searchStr2 = "kml/Document/Placemark/Polygon/outerBoundaryIs/LinearRing/coordinates";
    pugi::xpath_node xpathNode2 = calibDoc.select_single_node(searchStr2.c_str());
    if (xpathNode2) {
        pugi::xml_node selectedNode = xpathNode2.node();
        str = selectedNode.text().as_string();
    }
    
    //printf("%s\n", str);
    
    //parse coordinates str in format: lon1,lat1,alt1 lon2,lat2,alt2 lon3,lat3,alt3 ...
    
    int i;
    int searching_for_coord_end = 0, start_char = 0;
    double _lat, _lon, _alt;
    int total_str_len = strlen(str);
    llaPath.clear();
    for(i=0;i<total_str_len; i++) {
        if(str[i]==' ' || str[i] == '\t' || str[i] == '\r' || str[i] == '\n') {
            if(searching_for_coord_end) {
                if(0 == parseCoordLine(&(str[start_char]), i - start_char, &_lat, &_lon, &_alt)) {
                    //printf("%f, %f, %f\n", _lat, _lon, _alt);
                    llaPath.push_back(osg::Vec3d(_lat, _lon, _alt));
                }
                searching_for_coord_end = 0;
            }else {
                //printf(":");    
                continue;
            }
        }
        
        if(searching_for_coord_end == 0) {
            searching_for_coord_end = 1;
            start_char = i;
        }
        //printf("%c", str[i]);    
    }
    //printf("end\n\n\n");
    
    //Set initial start point//////////////////////////////////////////////////////////////////
    //calc horizontal total distance
    double local_distance = 0;
    double total_distance = 0;
    for(i=0;i<llaPath.size()-1;i++){
        local_distance = sqrt(pow2((llaPath[i+1].x() - llaPath[i].x())/X2LAT()) + pow2((llaPath[i+1].y() - llaPath[i].y())/Y2LON(llaPath[i].x())));
        total_distance += local_distance;
    }
    
    printf("total_distance:%f\n", total_distance);
    
    //traverse the path to find initial position
    assert(_phase>=0. && _phase <=1.);
    double target_initial_distance = total_distance*_phase;
    double passed_distance = 0;
    for(i=0;i<llaPath.size()-1;i++){
        local_distance = sqrt(pow2((llaPath[i+1].x() - llaPath[i].x())/X2LAT()) + pow2((llaPath[i+1].y() - llaPath[i].y())/Y2LON(llaPath[i].x())));
        passed_distance += local_distance;
        if(passed_distance >= target_initial_distance) { //we moved more than necessary
            current_point = i;
            next_point = i+1;
            if(local_distance < 1e-6)
                inbetween_position_percent = 0.;
            else {
                inbetween_position_percent = (target_initial_distance-(passed_distance-local_distance)) / local_distance;
            }
            
            printf("phase:%f, cur:%i,  next:%i, percent:%f\n", _phase, current_point, next_point, inbetween_position_percent);
            break;
        }
    }
    
    updatePathByParameters(); //might not be enoupg if called before setCCEFCenter()
}

void ObjectDrawable::updatePathByParameters()
{
    double _lat, _lon, _alt;
    _lat = llaPath[current_point].x() + inbetween_position_percent*(llaPath[next_point].x()-llaPath[current_point].x());
    _lon = llaPath[current_point].y() + inbetween_position_percent*(llaPath[next_point].y()-llaPath[current_point].y());
    _alt = llaPath[current_point].z() + inbetween_position_percent*(llaPath[next_point].z()-llaPath[current_point].z());
    //printf("initial lla:%f, %f, %f, %i\n\n\n\n", _lat, _lon, _alt, onEarth?1:0);    
            
    if(onEarth) {
        setPositionLatLonAlt(_lat, _lon, 1., ALTREF_AGL); 
    } else
        setPositionLatLonAlt(_lat, _lon, _alt, ALTREF_ASL); 
    
    double _roll, _pitch, _yaw;
    _roll = 0*M_PI/180;
    _pitch = 0*M_PI/180;
    
    _yaw = atan2((llaPath[next_point].y() - llaPath[current_point].y())/Y2LON(llaPath[current_point].x()),
                 (llaPath[next_point].x() - llaPath[current_point].x())/X2LAT());
    
    if(speed < 0)
        _yaw +=180*M_PI/180;
    setRPY(_roll, _pitch, _yaw);
    
}

void ObjectDrawable::updatePath(double t)
{
    //setPositionLatLonAlt(lat+0.000003, lon, 1., ALTREF_AGL);  //1 meter offset to resolve for numerical errors
    
    if(last_t < 0.) {
        last_t = t;
        updatePathByParameters(); //might not be enough if called before setCCEFCenter()
        return;
    } 
       
    double local_distance = sqrt(pow2((llaPath[next_point].x() - llaPath[current_point].x())/X2LAT()) + pow2((llaPath[next_point].y() - llaPath[current_point].y())/Y2LON(llaPath[current_point].x())));       
    if(local_distance> 1e-6)
        inbetween_position_percent+= speed/local_distance*(t-last_t);
       
    //printf("me:%f, %i, %i, %f\n", inbetween_position_percent, current_point, next_point, local_distance);
    //move to next point if necessary
    if((inbetween_position_percent >= 1. || local_distance < 1e-6) && speed >= 0) {
        next_point++;
        current_point++;
        if(next_point == llaPath.size()) next_point = 0;
        if(current_point == llaPath.size()) current_point = 0;
        inbetween_position_percent = 0;
    }
    
    if((inbetween_position_percent <= 0. || local_distance < 1e-6) && speed < 0) {
        next_point--;
        current_point--;
        if(next_point == -1) next_point = llaPath.size()-1;
        if(current_point == -1) current_point = llaPath.size()-1;
        inbetween_position_percent = 1.;
    }
    
    last_t = t;
    updatePathByParameters();
}

    
#define HighValue 100000

//setNodeMask is very slow, takes 3 seconds, even after dirtyBound();
void ObjectDrawable::hide()
{
	//getChild(0)->setNodeMask(0x0); //hide
	//getChild(0)->dirtyBound();
	if(hidden == false) {
		hidden = true;
		hideposition = getPosition();
		setPosition(osg::Vec3d(HighValue, HighValue, 0));
	}
}

void ObjectDrawable::show()
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
