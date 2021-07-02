//ALLAH
#ifndef __NSR_ROS_INTERFACE_H__
#define __NSR_ROS_INTERFACE_H__

#ifdef __cplusplus
extern "C" {
#endif

void rosOpen();
void rosClose();

//Quaternions are in JPL convention
void registerRosGroundTruth(double t, double lat, double lon, double alt, double e1, double e2, double e3, double et); 

void registerRosAcc(double t, double x, double y, double z, double rms_x, double rms_y, double rms_z);
void registerRosGyro(double t, double x, double y, double z, double rms_x, double rms_y, double rms_z);
void registerRosMag(double t, double x, double y, double z, double rms_x, double rms_y, double rms_z);
void registerRosMagRef(double t, double x, double y, double z, double rms_x, double rms_y, double rms_z);
void registerRosGPS(double t, double lat, double lon, double alt, double rms_x, double rms_y, double rms_z);
void registerRosGPSVel(double t, double x, double y, double z, double rms_x, double rms_y, double rms_z);
void registerRosPrsAlt(double t, double alt, double rms_x);

void registerRosImage(double t, const unsigned char* data, int width, int height, int step, int channels);
void registerRosCamInfo(double t);

#ifdef __cplusplus
}
#endif

#endif
