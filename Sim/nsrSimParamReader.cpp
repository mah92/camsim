#ifndef __ANDROID__
#include "/usr/include/time.h"
#endif

#include "./nsrSimParamReader.h"

#include "Core/nsrCore.h"
#include "pugixml/pugixml.hpp"
//#include "View/nsrGeoLib.h"

#include "nsrObjectDrawable.h"

#include <math.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>

#include <nsrUtility.h>

#undef TAG
#define TAG "Cpp:SimParamReader:"

int execution_turn = -1;
char execution_desc[100];

int param_seed = 12;

int param_render_what = RENDER_COLOR_IMAGE;
int param_show_what = SHOW_PATTERN;
int param_do_what = DO_SAVE_BMP;
bool param_rosbag_compression = false;

char settings_dempath[MAX_PATH_LENGTH];
char settings_mappath[MAX_PATH_LENGTH];

double param_map_center_lat = 35.9548;
double param_map_center_lon = 52.1100;
double param_map_max_alt = 200;
double param_map_max_dist = 20000;
int param_map_zoom = -1; //auto

double param_pattern_metric_width = 1.;
char param_pattern_file[MAX_PATH_LENGTH] = "";

char param_path_file[MAX_PATH_LENGTH];
int param_position_format = POSITION_FMT_LLA;
int param_tracker_type = TRACKER_TYPE_2xSQRT_INOUT_FILTER;
double param_control_freq = 50.;
double param_max_acc = 0; //disabled
double param_max_ang_acc = 0; //disabled

double param_end_time = 1e10;
double param_speed_factor = 1.;
double param_world_scale = 1.;
double param_alt_offset = 0.;

double param_camera_fps = 30;
double param_camera_phase_percent = 0.;

static double param_resolution_scale = 1.0;
int param_width = 640; //should match settings.render2stream_width
int param_height = 480; //should match settings.render2stream_height
double param_fx = 100.;
double param_fy = 100.;
double param_ox = 640 / 2.;
double param_oy = 480 / 2.;
double param_f_err = 0.;
double param_ox_err = 0.;
double param_oy_err = 0.;

int param_max_lens_iterations = 20;
double param_max_pix_error = 0.1;
double param_k1 = 0.07;
double param_k2 = 0.01;
double param_t1 = 0;
double param_t2 = 0;
double param_k3 = 0;
double param_k4 = 0;
double param_k5 = 0;
double param_k6 = 0;
double param_k1_err = 0.;
double param_k2_err = 0.;
double param_t1_err = 0.;
double param_t2_err = 0.;
double param_k3_err = 0.;

char param_vignet_file[MAX_PATH_LENGTH] = "";
double param_vignet_thresh1 = 0.;
double param_vignet_thresh2 = 1.;

double param_td = 0.;
double param_tr = 0.033;
double param_te = 0.033;
int param_motion_blur_extra_samples = 5;
double param_ti = 1;
double param_td_err = 0.;
double param_tr_err = 0.;

double param_extra_margin = 1.5;
double param_extra_zoom = 1.0;

double param_day_light = 0.7;
double param_noise_amp_dynamic = 0.1;
double param_noise_amp_static1 = 0.1;
double param_noise_amp_static2 = 0.1;
double param_max_fog_distance = 1000.;

double param_cam_in_ac_roll = 0.;
double param_cam_in_ac_pitch = 0.;
double param_cam_in_ac_yaw = 90.;
double param_cam_in_ac_err_x = 0.;
double param_cam_in_ac_err_y = 0.;
double param_cam_in_ac_err_z = 0.;

#ifdef __cplusplus
extern "C" {
#endif

void copyToSavePath(const char* filename)
{
	int len;
	char source_addr[MAX_PATH_LENGTH];
	char dest_addr[MAX_PATH_LENGTH];
	uint8_t buf[200];

	if(strlen(filename)==0) return;
	
	sprintf(source_addr, "%s/%s", globals.datapath, filename);
	sprintf(dest_addr, "%s/%s", globals.savepath, filename);

	LOGI(TAG, "Copying:%s->%s\n", source_addr, dest_addr);
	FILE *src, *dst;
	src = fopen(source_addr, "r");
	if(src==NULL) { LOGE(TAG, "Source does not exist!(%s)\n", source_addr); return;}
	dst = fopen(dest_addr, "w");
	if(dst==NULL) { LOGE(TAG, "Destination file could not be created!(%s)\n", dest_addr); return;}
	
	while((len = fread(buf, 1, sizeof(buf), src)) > 0) {
		fwrite(buf, 1, len, dst);
	}

	fclose(src);
	fclose(dst);
}

//calls itself recursively
void nsrReadSimParams(const char* filename)
{
	char node_name[MAX_PARAM_LENGTH];
	pugi::xml_parse_result result;
    pugi::xml_attribute atr;

	pugi::xml_document calibDoc;
	result = calibDoc.load_file((std::string(globals.datapath) + "/" + filename).c_str());
	if(!result) {
		LOGE(TAG, " %s loaded with error: %s, using defaults...\n", filename, result.description());
		exit(1);
		return;
	}
	LOGI(TAG, " %s loaded successfully!(%s)\n", filename, result.description()); fflush(stdout);

	//copyToSavePath(filename);

	pugi::xml_node simParams = calibDoc.child("simParams");
	pugi::xml_node node;
	for(node = simParams.first_child(); node; node = node.next_sibling()) {
		if(node.empty())
			continue;

		strcpy(node_name, node.name());
	
		if(strcmp(node_name, "description") == 0) {
			if((atr = node.attribute("executeTurn"))) execution_turn = atr.as_int();
			if((atr = node.attribute("description"))) strcpy(execution_desc, atr.as_string());
		}
		
		if(strcmp(node_name, "mainParams") == 0) {
			if((atr = node.attribute("renderWhat"))) {
				char strparam_render_what[20];
				strcpy(strparam_render_what, atr.as_string());
                if(strncmp("LUM", strparam_render_what, 3) == 0)
					param_render_what = RENDER_LUMINANCE;
				if(strncmp("COL", strparam_render_what, 3) == 0 || strncmp("IM", strparam_render_what, 2) == 0)
					param_render_what = RENDER_COLOR_IMAGE;
				if(strncmp("DEP", strparam_render_what, 3) == 0)
					param_render_what = RENDER_DEPTH;

			}
			if((atr = node.attribute("showWhat"))) {
				char strparam_show_what[20];
				strcpy(strparam_show_what, atr.as_string());
				if(strcmp("PATTERN", strparam_show_what) == 0)
					param_show_what = SHOW_PATTERN;
				if(strcmp("MAP", strparam_show_what) == 0)
					param_show_what = SHOW_MAP;
			}
			if((atr = node.attribute("doWhat"))) {
				char strparam_do_what[20];
				strcpy(strparam_do_what, atr.as_string());
                
				if(strcmp("NOTHING", strparam_do_what) == 0)
					param_do_what = DO_NOTHING;
                if(strcmp("SAVE_BMP", strparam_do_what) == 0)
					param_do_what = DO_SAVE_BMP;
				if(strcmp("IMAGE_PROC", strparam_do_what) == 0)
					param_do_what = DO_IMAGE_PROC;
                if(strcmp("SAVE_ROS_BAG", strparam_do_what) == 0)
                    param_do_what = DO_SAVE_ROS_BAG;
			}
			
			if((atr = node.attribute("rosBagCompression"))) param_rosbag_compression = atr.as_bool();

			LOGI(TAG, "mainParams: %i, %i, %i, %i\n", param_render_what, param_show_what, param_do_what, param_rosbag_compression?1:0);
		}
		
        //demParams////////////////////////////////////////////
        if(strcmp(node_name, "demParams") == 0) {
            if((atr = node.attribute("addr"))) {
                strcpy(settings_dempath, atr.as_string());
                LOGI(TAG, " dempath: %s\n", settings_dempath);    
            }
        }

        //mapParams////////////////////////////////////////////
		if(strcmp(node_name, "mapParams") == 0) {
         
            if((atr = node.attribute("addr"))) {
                strcpy(settings_mappath, atr.as_string());
                
                struct stat info;
                if(stat(settings_mappath, &info) != 0) {
                    sleep(5); //wait maybe sdcard loads
                    if(stat(settings_mappath, &info) != 0) {
                        sleep(5); //wait maybe sdcard loads
                    }
                }
                
                LOGI(TAG, " mappath: %s\n", settings_mappath);
            }
            
			if((atr = node.attribute("centerLat"))) param_map_center_lat = atr.as_double();
			if((atr = node.attribute("centerLon"))) param_map_center_lon = atr.as_double();
			if((atr = node.attribute("maxAlt"))) param_map_max_alt = atr.as_double();
			if((atr = node.attribute("maxDist"))) param_map_max_dist = atr.as_double();
			if((atr = node.attribute("zoomLevel"))) param_map_zoom = atr.as_int();

			LOGI(TAG, "mapParams: %f, %f, %f, %f, %i\n", param_map_center_lat, param_map_center_lon, param_map_max_alt, param_map_max_dist, param_map_zoom);
		}

		if(strcmp(node_name, "patternParams") == 0) {
			if((atr = node.attribute("metricWidth"))) param_pattern_metric_width = atr.as_double();
			if((atr = node.attribute("patternFile"))) {
				strcpy(param_pattern_file, globals.datapath);
				strcat(param_pattern_file, "/");
				strcat(param_pattern_file, atr.as_string());
                //copyToSavePath(atr.as_string());
			}

			LOGI(TAG, "patternParams: %f, %s\n", param_pattern_metric_width, param_pattern_file);
		}

		if(strcmp(node_name, "pathParams") == 0) {
			if((atr = node.attribute("pathFile"))) {
				strcpy(param_path_file, globals.datapath);
				strcat(param_path_file, "/");
				strcat(param_path_file, atr.as_string());
                //copyToSavePath(atr.as_string());
			}
			if((atr = node.attribute("csvPositionFmt"))) {
				char strparam_position_format[10];
				strcpy(strparam_position_format, atr.as_string());
				if(strcmp("LLA", strparam_position_format) == 0)
					param_position_format = POSITION_FMT_LLA;
				if(strcmp("XYA", strparam_position_format) == 0)
					param_position_format = POSITION_FMT_XYA;
				if(strcmp("XYZ", strparam_position_format) == 0)
					param_position_format = POSITION_FMT_XYZ;
			}

			if((atr = node.attribute("trackerType"))) param_tracker_type = atr.as_int();
			if((atr = node.attribute("controlFreq"))) param_control_freq = atr.as_double();
			if((atr = node.attribute("accMax"))) param_max_acc = atr.as_double();
			if((atr = node.attribute("angAccMax"))) param_max_ang_acc = atr.as_double();
			LOGI(TAG, "pathParams: %s, %i, %i, %f, %f, %f\n", param_path_file, param_tracker_type, param_position_format, param_control_freq, param_max_acc, param_max_ang_acc);
		}

		if(strcmp(node_name, "pathParamsExtra") == 0) {
            if((atr = node.attribute("endTime"))) param_end_time = atr.as_double();
			if((atr = node.attribute("speedFactor"))) param_speed_factor = atr.as_double();
			if((atr = node.attribute("worldScale"))) param_world_scale = atr.as_double();
			if((atr = node.attribute("altOffset"))) param_alt_offset = atr.as_double();
			LOGI(TAG, "pathParamsExtra: %f, %f, %f, %f\n", param_end_time, param_speed_factor, param_world_scale, param_alt_offset);
		}

		if(strcmp(node_name, "sensorParams") == 0) {
			if((atr = node.attribute("cameraFps"))) param_camera_fps = atr.as_double();
			if((atr = node.attribute("phasePercent"))) param_camera_phase_percent = atr.as_double();
			LOGI(TAG, "sensorParams: %f, %f\n", param_camera_fps, param_camera_phase_percent);
		}

		if(strcmp(node_name, "idealParams") == 0) {
			double param_fov_x, param_fov_y;
			if((atr = node.attribute("resolutionScale"))) param_resolution_scale = atr.as_double();
			if((atr = node.attribute("width"))) param_width = atr.as_int();
			if((atr = node.attribute("height"))) param_height = atr.as_int();

			if((atr = node.attribute("oxOffset"))) {
				param_ox = atr.as_double();
				param_ox += param_width / 2.;
			}
			if((atr = node.attribute("oyOffset"))) {
				param_oy = atr.as_double();
				param_oy += param_height / 2.;
			}

			if((atr = node.attribute("ox"))) {
				param_ox = atr.as_double();
			}
			if((atr = node.attribute("oy"))) {
				param_oy = atr.as_double();
			}

			if((atr = node.attribute("fovX"))) {
				param_fov_x = atr.as_double();
				param_fx = (param_width / 2.) / tan(param_fov_x / 2.*M_PI / 180.);
			}
			if((atr = node.attribute("fovY"))) {
				param_fov_y = atr.as_double();
				param_fy = (param_height / 2.) / tan(param_fov_y / 2.*M_PI / 180.);
			}

			if(node.attribute("fovX") && !node.attribute("fovY"))
				param_fy = param_fx;

			if(!node.attribute("fovX") && node.attribute("fovY"))
				param_fx = param_fy;

			if((atr = node.attribute("f" ))) param_fx = param_fy = atr.as_double();
			if((atr = node.attribute("fx"))) param_fx = atr.as_double();
			if((atr = node.attribute("fy"))) param_fy = atr.as_double();

			if((atr = node.attribute("fErr" ))) param_f_err = atr.as_double();
			if((atr = node.attribute("oxErr"))) param_ox_err = atr.as_double();
			if((atr = node.attribute("oyErr"))) param_oy_err = atr.as_double();

			param_width *= param_resolution_scale;
			param_height *= param_resolution_scale;
			param_ox *= param_resolution_scale;
			param_oy *= param_resolution_scale;
			param_fx *= param_resolution_scale;
			param_fy *= param_resolution_scale;
			param_f_err *= param_resolution_scale;
			param_ox_err *= param_resolution_scale;
			param_oy_err *= param_resolution_scale;

			LOGI(TAG, "idealParams: res:%ix%i, ox:%f, oy:%f, fx:%f, fy:%f\n", param_width, param_height, param_ox, param_oy, param_fx, param_fy);
		}

		if(strcmp(node_name, "staticParams") == 0) {
			if((atr = node.attribute("maxLensIterations"))) param_max_lens_iterations = atr.as_int();
			if((atr = node.attribute("maxPixError"))) param_max_pix_error = atr.as_double();
			if((atr = node.attribute("k1"))) param_k1 = atr.as_double();
			if((atr = node.attribute("k2"))) param_k2 = atr.as_double();
			if((atr = node.attribute("t1"))) param_t1 = atr.as_double();
			if((atr = node.attribute("t2"))) param_t2 = atr.as_double();
			if((atr = node.attribute("k3"))) param_k3 = atr.as_double();
			if((atr = node.attribute("k4"))) param_k4 = atr.as_double();
			if((atr = node.attribute("k5"))) param_k5 = atr.as_double();
			if((atr = node.attribute("k6"))) param_k6 = atr.as_double();
			LOGI(TAG, "staticParams: %i, %f, %f, %f, %f, %f, %f, %f, %f, %f\n", param_max_lens_iterations, param_max_pix_error, param_k1, param_k2, param_t1, param_t2,
				 param_k3, param_k4, param_k5, param_k6);

			if((atr = node.attribute("k1Err"))) param_k1_err = atr.as_double();
			if((atr = node.attribute("k2Err"))) param_k2_err = atr.as_double();
			if((atr = node.attribute("t1Err"))) param_t1_err = atr.as_double();
			if((atr = node.attribute("t2Err"))) param_t2_err = atr.as_double();
			if((atr = node.attribute("k3Err"))) param_k3_err = atr.as_double();
		}

		if(strcmp(node_name, "vignetParams") == 0) {
			if((atr = node.attribute("vignetFile"))) {
				strcpy(param_vignet_file, globals.datapath);
				strcat(param_vignet_file, "/");
				strcat(param_vignet_file, atr.as_string());
                //copyToSavePath(atr.as_string());
			}
			if((atr = node.attribute("vignetThresh1"))) param_vignet_thresh1 = atr.as_double();
			if((atr = node.attribute("vignetThresh2"))) param_vignet_thresh2 = atr.as_double();
			LOGI(TAG, "vignetParams: %s, %f, %f\n", strlen(param_vignet_file) > 0 ? param_vignet_file : "NO VIGNET", param_vignet_thresh1, param_vignet_thresh2);
		}

		if(strcmp(node_name, "dynamicParams") == 0) {
			if((atr = node.attribute("td"))) param_td = atr.as_double();
			if((atr = node.attribute("tr"))) param_tr = atr.as_double();
			if((atr = node.attribute("te"))) param_te = atr.as_double();
			if((atr = node.attribute("motionBlurExtraSamples"))) param_motion_blur_extra_samples = atr.as_int();
			if((atr = node.attribute("ti"))) param_ti = atr.as_double();

			//The real timing contains these errors
			if((atr = node.attribute("tdErr"))) param_td_err = atr.as_double();
			if((atr = node.attribute("trErr"))) param_tr_err = atr.as_double();

			LOGI(TAG, "dynamicParams: %f, %f, %f, %i, %f, errTd:%f, errTr:%f\n", param_td, param_tr, param_te, param_motion_blur_extra_samples, param_ti,
				 param_td_err, param_tr_err);
		}

		if(strcmp(node_name, "marginParams") == 0) {
			if((atr = node.attribute("extraMargin"))) param_extra_margin = atr.as_double();
			if((atr = node.attribute("extraZoom"))) param_extra_zoom = atr.as_double();
			LOGI(TAG, "marginParams: %f, %f,\n", param_extra_margin, param_extra_zoom);
		}

		if(strcmp(node_name, "signalParams") == 0) {
			if((atr = node.attribute("dayLight"       ))) param_day_light = atr.as_double();
			if((atr = node.attribute("noiseAmpDynamic"))) param_noise_amp_dynamic = atr.as_double();
            if((atr = node.attribute("noiseAmpStatic1"))) param_noise_amp_static1 = atr.as_double();
            if((atr = node.attribute("noiseAmpStatic2"))) param_noise_amp_static2 = atr.as_double();
            if((atr = node.attribute("fogVisibilityDistance"))) param_max_fog_distance = atr.as_double();
			if((atr = node.attribute("seed"))) atr.as_int();
			LOGI(TAG, "signalParams: %f, %f, %f, %f, %f, %i\n", param_day_light, param_noise_amp_dynamic, param_noise_amp_static1, param_noise_amp_static2, param_max_fog_distance, param_seed);
		}

		if(strcmp(node_name, "camInAcEu") == 0) {
			//Nominal installation angles
			if((atr = node.attribute("roll" ))) param_cam_in_ac_roll = atr.as_double();
			if((atr = node.attribute("pitch"))) param_cam_in_ac_pitch = atr.as_double();
			if((atr = node.attribute("yaw"  ))) param_cam_in_ac_yaw = atr.as_double();

			//The real rotation contains these errors
			if((atr = node.attribute("xErr"))) param_cam_in_ac_err_x = atr.as_double();
			if((atr = node.attribute("yErr"))) param_cam_in_ac_err_y = atr.as_double();
			if((atr = node.attribute("zErr"))) param_cam_in_ac_err_z = atr.as_double();

			double param_cam_in_ac_err = 0;
			if((atr = node.attribute("err"))) param_cam_in_ac_err = atr.as_double();
			if(!node.attribute("xErr")) param_cam_in_ac_err_x = ((rand() % 2) * 2 - 1) * param_cam_in_ac_err;
			if(!node.attribute("yErr")) param_cam_in_ac_err_y = ((rand() % 2) * 2 - 1) * param_cam_in_ac_err;
			if(!node.attribute("zErr")) param_cam_in_ac_err_z = ((rand() % 2) * 2 - 1) * param_cam_in_ac_err;

			LOGI(TAG, "camInAcEu: %f, %f, %f, errX:%f, errY:%f, errZ:%f\n", param_cam_in_ac_roll, param_cam_in_ac_pitch, param_cam_in_ac_yaw,
				 param_cam_in_ac_err_x, param_cam_in_ac_err_y, param_cam_in_ac_err_z);
		}
		
        if(strcmp(node_name, "vehicle") == 0) {
            static bool path_inited = false;
            static char pathFile[MAX_PATH_LENGTH] = "";
            static bool roadDirReverse = false;
            
            char objFile[MAX_PATH_LENGTH];
            double phase, speed;
            int numVehicles;
            bool onEarth;            
            
            strcpy(objFile, "");
            
            if(path_inited == false)
                strcpy(pathFile, "");
            
			if((atr = node.attribute("pathFile"))) {
                strcpy(pathFile, globals.datapath);
				strcat(pathFile, "/");
                strcat(pathFile, atr.as_string());
                path_inited = true;
            }
            
            if((atr = node.attribute("roadDirReverse"))) roadDirReverse = atr.as_bool();

            onEarth = true;
			if((atr = node.attribute("onEarth"))) onEarth = atr.as_bool();
                
            phase = -1;
            if((atr = node.attribute("startPhase"))) phase = atr.as_double();
            
            numVehicles = 1;
            if((atr = node.attribute("numVehicles"))) numVehicles = atr.as_int();                
            
            speed = 60.;
			if((atr = node.attribute("speed"))) speed = atr.as_double();
            if(roadDirReverse) speed = -speed;

            if((atr = node.attribute("objFile"))) {
                strcpy(objFile, globals.datapath);
				strcat(objFile, "/");
                strcat(objFile, atr.as_string());
            }
			LOGI(TAG, "vehicle: %s, %i, %f, %i, %f, %s\n", pathFile, onEarth?1:0, phase, numVehicles, speed, objFile);
            
            if(strlen(pathFile) > 0 && strlen(objFile) > 0) {
                int k;
                for(k=0;k<numVehicles;k++) {
                    allObjectDrawables.push_back(new ObjectDrawable());
                    allObjectDrawables.back()->add3DFile(objFile);
                    allObjectDrawables.back()->setPath(pathFile, onEarth, phase, speed/3.6); //convert speed from KM/h to m/s
                }
            }
		}

		//overrides
		if(strcmp(node_name, "includeParams") == 0) {
			char include_file[MAX_PATH_LENGTH];
            
			if((atr = node.attribute("includeFile")))
                strcpy(include_file, atr.as_string());

			LOGI(TAG, "includeParams1: %s\n", include_file); fflush(stdout);

			nsrReadSimParams(include_file);
			LOGI(TAG, "includeParams2: %s\n", include_file); fflush(stdout);
		}
	}

	//param_cam_in_ac_err_x = 10;

	/*if(execution_turn >= 0) {
		//param_noise_amp=execution_turn*0.01; strcpy(execution_desc, "param_noise_amp=execution_turn*0.01");
		//param_speed_factor=5+execution_turn*5; strcpy(execution_desc, "param_speed_factor=5+execution_turn*5");
		LOGI(TAG, " !!!!!!!!!!!!!!!!!!!Breaking news!!!!!!!!!!!!!!!!!!!!\n Override command: %s\n", execution_desc);
	}*/

	LOGI(TAG, " file: %s reading completed!\n", filename); fflush(stdout);
}

void nsrSaveSimParams(const char* filename)
{
	char dest_addr[MAX_PATH_LENGTH];
	uint8_t buf[200];

	if(strlen(filename)==0) return;
	
	sprintf(dest_addr, "%s/%s", globals.savepath, filename);

	pugi::xml_document doc;
	pugi::xml_node parent_node = doc.append_child("simParams");
	pugi::xml_node param;
	
	///////////////////////////////////////////////
	if(strlen(execution_desc)>0 || execution_turn>=0) {
		param = parent_node.append_child("description");
	
		param.append_attribute("executeTurn") = execution_turn;
		param.append_attribute("description") = execution_desc;
	}
	
	///////////////////////////////////////////////
	param = parent_node.append_child("idealParams");
	
	param.append_attribute("width") = param_width;
	param.append_attribute("height") = param_height;
	
	if(EQUALS(param_fx, param_fy, 1e-6)) {
		param.append_attribute("f") = param_fx;	
	} else {
		param.append_attribute("fx") = param_fx;
		param.append_attribute("fy") = param_fy;
	}
	
	double fov_x, fov_y;
	fov_x = atan((param_width  / 2.) / param_fx) * 2. * 180./M_PI;
	fov_y = atan((param_height / 2.) / param_fy) * 2. * 180./M_PI;
			
	param.append_attribute("fovX") = fov_x;
	param.append_attribute("fovY") = fov_y;
	
	double ox_offset, oy_offset;
	ox_offset = param_ox - param_width / 2.;
	oy_offset = param_oy - param_height / 2.;
	
	if(fabs(ox_offset) > 1e-6) param.append_attribute("oxOffset") = ox_offset;
	if(fabs(oy_offset) > 1e-6) param.append_attribute("oyOffset") = oy_offset;

	if(fabs(param_f_err) + fabs(param_ox_err) + fabs(param_oy_err) > 1e-6) {
		param = parent_node.append_child("idealParams");
		
		if(fabs(param_f_err)  > 1e-6) param.append_attribute("fErr") = param_f_err;
		if(fabs(param_ox_err) > 1e-6) param.append_attribute("oxErr") = param_ox_err;
		if(fabs(param_oy_err) > 1e-6) param.append_attribute("oyErr") = param_oy_err;
	}
	
	///////////////////////////////////////
	param = parent_node.append_child("staticParams");

	param.append_attribute("k1") = param_k1;
	param.append_attribute("k2") = param_k2;
	param.append_attribute("t1") = param_t1;
	param.append_attribute("t2") = param_t2;
	param.append_attribute("k3") = param_k3;
	
	if(fabs(param_k1_err) + fabs(param_k2_err) + fabs(param_t1_err) + fabs(param_t2_err) + fabs(param_k3_err) > 1e-6) {
		param = parent_node.append_child("staticParams");
		
		if(fabs(param_k1_err) > 1e-6) param.append_attribute("k1Err") = param_k1_err;
		if(fabs(param_k2_err) > 1e-6) param.append_attribute("k2Err") = param_k2_err;
		if(fabs(param_t1_err) > 1e-6) param.append_attribute("t1Err") = param_t1_err;
		if(fabs(param_t2_err) > 1e-6) param.append_attribute("t2Err") = param_t2_err;
		if(fabs(param_k3_err) > 1e-6) param.append_attribute("k3Err") = param_k3_err;
	}

	///////////////////////////////////////

	//remove path from file name
	int slash_position = -1;
	int path_len = strlen(param_vignet_file);
	for(int i=0;i<path_len; i++)
		if(param_vignet_file[i]=='/') slash_position = i;
	
	if(path_len > 0 && slash_position<path_len-1) { //path is not empty and slash is not the last char
		param = parent_node.append_child("vignetParams");
		
		param.append_attribute("vignetFile") = (&param_vignet_file[slash_position+1]);
		param.append_attribute("vignetThresh1") = param_vignet_thresh1;
		param.append_attribute("vignetThresh2") = param_vignet_thresh2;
	}

	///////////////////////////////////////
	param = parent_node.append_child("dynamicParams");
	
	param.append_attribute("td") = param_td;
	param.append_attribute("tr") = param_tr;
	param.append_attribute("te") = param_te;
	param.append_attribute("ti") = param_ti;
	
	if(fabs(param_td_err) + fabs(param_tr_err) > 1e-6) {
		param = parent_node.append_child("dynamicParams");
		
		param.append_attribute("tdErr") = param_td_err;
		param.append_attribute("trErr") = param_tr_err;	
	}

	///////////////////////////////////////
	param = parent_node.append_child("camInAcEu");
	
	param.append_attribute("roll") = param_cam_in_ac_roll;
	param.append_attribute("pitch") = param_cam_in_ac_pitch;
	param.append_attribute("yaw") = param_cam_in_ac_yaw;

	if(fabs(param_cam_in_ac_err_x) + fabs(param_cam_in_ac_err_y) + fabs(param_cam_in_ac_err_z) > 1e-6) {
		param = parent_node.append_child("camInAcEu");
		
		if(fabs(param_cam_in_ac_err_x) > 1e-6) param.append_attribute("xErr") = param_cam_in_ac_err_x;
		if(fabs(param_cam_in_ac_err_y) > 1e-6) param.append_attribute("yErr") = param_cam_in_ac_err_y;
		if(fabs(param_cam_in_ac_err_z) > 1e-6) param.append_attribute("zErr") = param_cam_in_ac_err_z;
	}

    //doc.print(std::cout);
	doc.save_file(dest_addr);
}

#ifdef __cplusplus
}
#endif
