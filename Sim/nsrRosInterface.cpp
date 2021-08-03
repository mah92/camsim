#ifndef __ANDROID__
#include "/usr/include/time.h"
#endif

#include "Sim/nsrRosInterface.h"
#include "Core/Matlib/nsrQuat.h"

#include "nsrCore.h"

#ifndef ROS_FOUND

#ifdef __cplusplus
extern "C" {
#endif

void rosOpen(bool compress) {}
void rosClose() {}

void registerRosGroundTruth(double t, double lat, double lon, double alt, double e1, double e2, double e3, double et) {}
void registerRosGroundTruth2(double t, double lat, double lon, double alt, double roll, double pitch, double yaw) {}

void registerRosAcc(double t, double x, double y, double z, double rms_x, double rms_y, double rms_z) {}
void registerRosGyro(double t, double x, double y, double z, double rms_x, double rms_y, double rms_z) {}
void registerRosMag(double t, double x, double y, double z, double rms_x, double rms_y, double rms_z) {}
void registerRosMagRef(double t, double x, double y, double z, double rms_x, double rms_y, double rms_z) {}
void registerRosGPS(double t, double lat, double lon, double alt, double rms_x, double rms_y, double rms_z) {}
void registerRosGPSVel(double t, double x, double y, double z, double rms_x, double rms_y, double rms_z) {}
void registerRosPrsAlt(double t, double alt, double rms_x) {}

void registerRosImage(double t, const unsigned char* data, int width, int height, int step, int channels) {}
void registerRosCamInfo(double t) {}

#ifdef __cplusplus
}
#endif

#else

#include <ros/ros.h>
#include <rosbag/bag.h>
//#include <rosbag/view.h>

//#include <opencv2/highgui/highgui.hpp>
//#include <cv_bridge/cv_bridge.h> //for converting ros image messages to opencv cv::Mat

#include <std_msgs/Int32.h>
#include <std_msgs/String.h>

#include <sensor_msgs/Image.h>
#include <sensor_msgs/fill_image.h>

#include <sensor_msgs/CameraInfo.h>
#include <sensor_msgs/NavSatFix.h>
#include <sensor_msgs/Imu.h>
#include <sensor_msgs/MagneticField.h>

#include <geometry_msgs/PointStamped.h>
#include <geometry_msgs/Vector3Stamped.h>
#include <geometry_msgs/QuaternionStamped.h>
#include <geometry_msgs/Vector3Stamped.h>
#include <geometry_msgs/TransformStamped.h>
#include <geometry_msgs/TwistStamped.h>

#include "Sim/nsrSimParamReader.h"
//#include "nsrQuat.h"

#undef TAG
#define TAG "Cpp:RosInterface:"

#ifdef __cplusplus
extern "C" {
#endif

//b2: check if ground truth lat is geodetic
//f2:add save_to_rosbag/publish to ros to Parameters.xml
//b2: is gps alt from wgs84 or MSL?-> not used
//f3-> also add sensor_msgs/FluidPressure for pressure
//b2-> we stored lat/lon/alt in geometry_msgs/TransformStamped which is not best practice

static rosbag::Bag bag;
static bool ros_open = 0;
static uint32_t seq = 0;

static ros::Time epoch0;
static int acc_registered = 0;

void rosOpen(bool compress)
{
	std::string str = std::string(globals.savepath) + "/test.bag";
	
	//Remove the bag if it already exists
	std::remove(str.c_str());

	FILE *file;
	if ((file = fopen(str.c_str(), "r"))) {
		fclose(file);
		LOGE(TAG, "Unable to remove previous rosbag, remove it manually!\n");
		exit(100);
	}
	
    ros::Time::init(); //for standalone applications 	
    epoch0 = ros::Time::now();
    
    if(compress)
        bag.setCompression(rosbag::compression::BZ2); //lowers fps by 30%, but compresses images about 6times
    bag.open(str, rosbag::bagmode::Write);
    ros_open = true;
}

void rosClose()
{
    if(ros_open == false) return;
    
    ros_open = false;
    bag.close();
    LOGW(TAG, " Ros is Closed!\n");
}

void registerRosGroundTruth(double t, double lat, double lon, double alt, double e1, double e2, double e3, double et)
{
    if(ros_open == false) return;
    
    ros::Time epoch = epoch0;
    epoch.sec += floor(t);
    epoch.nsec += (t - floor(t))*1e9;
    
    //printf("t:%f\n", epoch.sec + epoch.nsec*1e-9);
    
    // /vicon/firefly_sbx/firefly_sbx : geometry_msgs/TransformStamped
    /*
    std_msgs/Header header
    geometry_msgs/Vector3 magnetic_field
    float64[9] magnetic_field_covariance
    */
        
    geometry_msgs::TransformStamped tfs;
    tfs.header.stamp = epoch;
    tfs.header.seq = seq++;
    
    tfs.header.frame_id = "NED";
    tfs.child_frame_id = "AircraftBody";
    
    tfs.transform.translation.x = lat;
    tfs.transform.translation.y = lon;
    tfs.transform.translation.z = alt;
    
    //nsr::Quat q; //JPL convention
    //q.setEu(roll, pitch, yaw);
 
    //JPL convention, as used in ASL-FSR2015 rosbags ground truth
    tfs.transform.rotation.x = e1;
    tfs.transform.rotation.y = e2;
    tfs.transform.rotation.z = e3;
    tfs.transform.rotation.w = et;
    
    bag.write("/vicon/firefly_sbx/firefly_sbx", epoch, tfs);
}

void registerRosGroundTruth2(double t, double lat, double lon, double alt, double roll, double pitch, double yaw)
{
    if(ros_open == false) return;
    
    ros::Time epoch = epoch0;
    epoch.sec += floor(t);
    epoch.nsec += (t - floor(t))*1e9;
    
    // /vicon/firefly_sbx/firefly_sbx : geometry_msgs/TransformStamped
    /*
    std_msgs/Header header
    geometry_msgs/Vector3 magnetic_field
    float64[9] magnetic_field_covariance
    */
        
    geometry_msgs::TransformStamped tfs;
    tfs.header.stamp = epoch;
    tfs.header.seq = seq++;
    
    tfs.header.frame_id = "NED";
    tfs.child_frame_id = "AircraftBody";
    
    tfs.transform.translation.x = lat;
    tfs.transform.translation.y = lon;
    tfs.transform.translation.z = alt;
	
	nsr::Quat qu;
	qu.setEu(roll, pitch, yaw);
    
    //nsr::Quat q; //JPL convention
    //q.setEu(roll, pitch, yaw);
 
    //JPL convention, as used in ASL-FSR2015 rosbags ground truth
    tfs.transform.rotation.x = qu.e1;
    tfs.transform.rotation.y = qu.e2;
    tfs.transform.rotation.z = qu.e3;
    tfs.transform.rotation.w = qu.et;
    
    bag.write("/vicon/firefly_sbx/firefly_sbx", epoch, tfs);
}

static sensor_msgs::Imu imu; //catch accelerometer to publish acc-gyro together, this is needed for some algorithms like ROVIO

void registerRosAcc(double t, double x, double y, double z, double rms_x, double rms_y, double rms_z) 
{
    if(ros_open == false) return;
    
    //ros::Time epoch = epoch0;
    //epoch.sec += floor(t);
    //epoch.nsec += (t - floor(t))*1e9;

    /*std_msgs/Header header
    geometry_msgs/Quaternion orientation
    float64[9] orientation_covariance
    geometry_msgs/Vector3 angular_velocity
    float64[9] angular_velocity_covariance
    geometry_msgs/Vector3 linear_acceleration
    float64[9] linear_acceleration_covariance
    */
        
    //imu.header.stamp = epoch;
    //imu.header.seq = seq++;
    
    imu.linear_acceleration.x = x;
    imu.linear_acceleration.y = y;
    imu.linear_acceleration.z = z;
    imu.linear_acceleration_covariance[0] = rms_x*rms_x;
    imu.linear_acceleration_covariance[4] = rms_y*rms_y;
    imu.linear_acceleration_covariance[8] = rms_z*rms_z;
    
    acc_registered++;
    
    //bag.write("/px4/raw/imu", epoch, imu);
}

void registerRosGyro(double t, double x, double y, double z, double rms_x, double rms_y, double rms_z) 
{
    if(ros_open == false) return;
    
    ros::Time epoch = epoch0;
    epoch.sec += floor(t);
    epoch.nsec += (t - floor(t))*1e9;
    
    /*std_msgs/Header header
    geometry_msgs/Quaternion orientation
    float64[9] orientation_covariance
    geometry_msgs/Vector3 angular_velocity
    float64[9] angular_velocity_covariance
    geometry_msgs/Vector3 linear_acceleration
    float64[9] linear_acceleration_covariance
    */
    
    //sensor_msgs::Imu imu;
    imu.header.stamp = epoch;
    imu.header.seq = seq++;
    
    imu.orientation_covariance[0]=-1; //Orientation(and it's covariance) is Unknown
    imu.angular_velocity.x = x;
    imu.angular_velocity.y = y;
    imu.angular_velocity.z = z;
    imu.angular_velocity_covariance[0] = rms_x*rms_x;
    imu.angular_velocity_covariance[4] = rms_y*rms_y;
    imu.angular_velocity_covariance[8] = rms_z*rms_z;
    
    acc_registered--;
    assert(acc_registered == 0); //make sure acc & gyro frequency are the same

	bag.write("/px4/raw/imu", epoch, imu);
}

void registerRosMag(double t, double x, double y, double z, double rms_x, double rms_y, double rms_z)
{
    if(ros_open == false) return;
    
    ros::Time epoch = epoch0;
    epoch.sec += floor(t);
    epoch.nsec += (t - floor(t))*1e9;

    /*
    std_msgs/Header header
    geometry_msgs/Vector3 magnetic_field
    float64[9] magnetic_field_covariance
    */
        
    sensor_msgs::MagneticField mag;
    mag.header.stamp = epoch;
    mag.header.seq = seq++;
    
    mag.magnetic_field.x = x;
    mag.magnetic_field.y = y;
    mag.magnetic_field.z = z;
    mag.magnetic_field_covariance[0] = rms_x*rms_x;
    mag.magnetic_field_covariance[4] = rms_y*rms_y;
    mag.magnetic_field_covariance[8] = rms_z*rms_z;
    
    bag.write("/px4/raw/mag", epoch, mag);
}

void registerRosMagRef(double t, double x, double y, double z, double rms_x, double rms_y, double rms_z)
{
    if(ros_open == false) return;
    
    ros::Time epoch = epoch0;
    epoch.sec += floor(t);
    epoch.nsec += (t - floor(t))*1e9;
    
    /*
    std_msgs/Header header
    geometry_msgs/Vector3 magnetic_field
    float64[9] magnetic_field_covariance
    */
    
    sensor_msgs::MagneticField magRef;
    
    magRef.header.stamp = epoch;
    magRef.header.seq = seq++;
    
    magRef.magnetic_field.x = x;
    magRef.magnetic_field.y = y;
    magRef.magnetic_field.z = z;    
    magRef.magnetic_field_covariance[0] = rms_x*rms_x;
    magRef.magnetic_field_covariance[4] = rms_y*rms_y;
    magRef.magnetic_field_covariance[8] = rms_z*rms_z;
    
    bag.write("/px4/raw/magref", epoch, magRef);
}

void registerRosGPS(double t, double lat, double lon, double alt, double rms_x, double rms_y, double rms_z)
{
    if(ros_open == false) return;
    
    ros::Time epoch = epoch0;
    epoch.sec += floor(t);
    epoch.nsec += (t - floor(t))*1e9;
    
    /*
    sensor_msgs/NavSatFix:
    uint8 COVARIANCE_TYPE_UNKNOWN=0
    uint8 COVARIANCE_TYPE_APPROXIMATED=1
    uint8 COVARIANCE_TYPE_DIAGONAL_KNOWN=2
    uint8 COVARIANCE_TYPE_KNOWN=3
    std_msgs/Header header
    sensor_msgs/NavSatStatus status
    float64 latitude
    float64 longitude
    float64 altitude
    float64[9] position_covariance
    uint8 position_covariance_type
    */
    
    sensor_msgs::NavSatFix satFix;
    satFix.header.stamp = epoch;
    satFix.header.seq = seq++;
    
    satFix.latitude = lat; //degrees
    satFix.longitude = lon; //degrees
    satFix.altitude = alt; //above the WGS 84 ellipsoid
    
    satFix.position_covariance[0] = rms_x*rms_x;
    satFix.position_covariance[4] = rms_y*rms_y;
    satFix.position_covariance[8] = rms_z*rms_z;
    satFix.position_covariance_type = sensor_msgs::NavSatFix::COVARIANCE_TYPE_DIAGONAL_KNOWN;
 
    /*
    sensor_msgs/NavSatStatus:
    int8 STATUS_NO_FIX=-1
    int8 STATUS_FIX=0
    int8 STATUS_SBAS_FIX=1
    int8 STATUS_GBAS_FIX=2
    uint16 SERVICE_GPS=1
    uint16 SERVICE_GLONASS=2
    uint16 SERVICE_COMPASS=4
    uint16 SERVICE_GALILEO=8
    int8 status
    uint16 service
    */
        
    satFix.status.status = sensor_msgs::NavSatStatus::STATUS_FIX;
    satFix.status.service = sensor_msgs::NavSatStatus::SERVICE_GPS | sensor_msgs::NavSatStatus::SERVICE_GLONASS;
        
    bag.write("/px4/raw/gps", epoch, satFix);
}

void registerRosGPSVel(double t, double x, double y, double z, double rms_x, double rms_y, double rms_z)
{
    if(ros_open == false) return;
    
    ros::Time epoch = epoch0;
    epoch.sec += floor(t);
    epoch.nsec += (t - floor(t))*1e9;

    // /kitti/oxts/gps/vel : geometry_msgs/TwistStamped

    /*
    std_msgs/Header header
    geometry_msgs/Twist twist
    */
    /* Twist:
    geometry_msgs/Vector3 linear
    geometry_msgs/Vector3 angular
    */
    
    geometry_msgs::TwistStamped tws;
    tws.header.stamp = epoch;
    tws.header.seq = seq++;
    
    tws.twist.linear.x = x;
    tws.twist.linear.y = y;
    tws.twist.linear.z = z;

    bag.write("/px4/raw/velocity", epoch, tws);
}

void registerRosPrsAlt(double t, double alt, double rms_x)
{
    if(ros_open == false) return;
    
    ros::Time epoch = epoch0;
    epoch.sec += floor(t);
    epoch.nsec += (t - floor(t))*1e9;

    //geometry_msgs/PointStamped, z: alt(m), y: cov (defined here)
    
    /*
    std_msgs/Header header
    geometry_msgs/Point point
    */
    
    geometry_msgs::PointStamped prsAlt;
    
    prsAlt.header.stamp = epoch;
    prsAlt.header.seq = seq++;
    
    prsAlt.point.x = 0;
    prsAlt.point.y = rms_x*rms_x;
    prsAlt.point.z = alt;
    
    bag.write("/pressure_height", epoch, prsAlt);
}

void registerRosImage(double t, const unsigned char* data, int width, int height, int step, int channels)
{
    if(ros_open == false) return;
    
    ros::Time epoch = epoch0;
    epoch.sec += floor(t);
    epoch.nsec += (t - floor(t))*1e9;
    
    /*
    std_msgs/Header header
    uint32 height
    uint32 width
    string encoding
    uint8 is_bigendian
    uint32 step
    uint8[] data
    */
    //sensor_msgs::Image<uint8_t*> img(data);
    //sensor_msgs::Image<uint8_t> img(data);
    //sensor_msgs::Image img(data);
    sensor_msgs::Image img;
    
    img.header.stamp = epoch;
    img.header.seq = seq++;
    
    img.height = height;
    img.width = width;
    img.step = step;
    //img.data.Ptr = (uint8_t*)data;
    img.is_bigendian = false;
    
    size_t st0 = (step * height);
    img.data.resize(st0);
    memcpy(&img.data[0], (void*)data, st0);
    //img.data[0] = (void*)data;

    /*sensor_msgs::fillImage( img,
                            sensor_msgs::image_encodings::MONO8,
                            height, // height
                            width, // width
                            step, // stepSize
                            data);*/
    
    if(channels == 1) {
        img.encoding = "mono8"; //"rgb8", "rgba8", "bgr8", "bgra8"    
        bag.write("/cam0/image", epoch, img);
    }
    
    if(channels == 3) { //packed
        img.encoding = "rgb8"; //"rgb8", "rgba8", "bgr8", "bgra8"    
        bag.write("/cam0/image_color", epoch, img);
    }
    
    if(channels == 4) { //packed
        img.encoding = "rgba8"; //"rgb8", "rgba8", "bgr8", "bgra8"    
        bag.write("/cam0/image_color", epoch, img);
    }
    
    /*    
    #   image_raw - raw data from the camera driver, possibly Bayer encoded
    #   image            - monochrome, distorted
    #   image_color      - color, distorted
    #   image_rect       - monochrome, rectified
    #   image_rect_color - color, rectified
    */
}

//CamInfo need to pass param_fx not param_fx+err
void registerRosCamInfo(double t)
{
    if(ros_open == false) return;
    
    ros::Time epoch = epoch0;
    epoch.sec += floor(t);
    epoch.nsec += (t - floor(t))*1e9;
    
    /*
    std_msgs/Header header
    uint32 height
    uint32 width
    string distortion_model
    float64[] D
    float64[9] K
    float64[9] R //stereo only
    float64[12] P
    uint32 binning_x //not used by default
    uint32 binning_y //not used by default
    sensor_msgs/RegionOfInterest roi //not used by default
    */

    sensor_msgs::CameraInfo info;
    
    info.header.stamp = epoch;
    info.header.seq = seq++;
    
    
    info.height = param_height;
    info.width = param_width;

    //     [fx  0 cx]
    // K = [ 0 fy cy]
    //     [ 0  0  1]

    info.K[0] = param_fx;
    info.K[4] = param_fy;
    info.K[2] = param_ox;
    info.K[5] = param_oy;
    info.K[8] = 1.;

    info.distortion_model = "plumb_bob";
    info.D.resize(5);
    info.D[0] = param_k1;
    info.D[1] = param_k2;
    info.D[2] = param_t1;
    info.D[3] = param_t2;
    info.D[4] = param_k3;

    //monocular
    info.R[0] = 1.;
    info.R[4] = 1.;
    info.R[8] = 1.;

    //monocular
    //     [fx'  0  cx' 0]
    // P = [ 0  fy' cy' 0]
    //     [ 0   0   1  0]
    info.P[0] = info.K[0]; //fx
    info.P[5] = info.K[4]; //fy
    info.P[2] = info.K[2]; //ox
    info.P[6] = info.K[5]; //oy
    info.P[10] = info.K[8]; //1.
    
    bag.write("/cam0/camera_info", epoch, info);
}

#ifdef __cplusplus
}
#endif

#endif

