#ifndef __ANDROID__
#include "/usr/include/time.h"
#endif

#include "./nsrSimParamReader.h"

#include "Core/nsrCore.h"
#include "pugixml/pugixml.hpp"
//#include "View/nsrGeoLib.h"

#include <math.h>
#include <nsrUtility.h>

#undef TAG
#define TAG "Cpp:SimParamReader:"

int param_seed = 12;

int param_render_what = RENDER_IMAGE;
int param_show_what = SHOW_PATTERN;
int param_do_what = DO_SAVE_BMP;

double param_map_center_lat = 35.9548;
double param_map_center_lon = 52.1100;
double param_map_max_alt = 200;
double param_map_max_dist = 20000;
int param_map_zoom = -1; //auto

double param_pattern_metric_width = 1.;
char param_pattern_file[MAX_PATH_LENGTH] = "";

char param_path_file[MAX_PATH_LENGTH];
int param_position_format = POSITION_FMT_LLA;
int param_tracker_type = TRACKER_TYPE_3xSQRT;
double param_control_freq = 50.;
double param_max_acc = 0; //disabled
double param_max_ang_acc = 0; //disabled

double param_speed_factor = 1.;
double param_world_scale = 1.;
double param_alt_offset = 0.;

double param_camera_fps = 30;
double param_camera_phase_percent = 0.;

double param_resolution_scale = 1.0;
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
	sprintf(source_addr, "%s/%s", globals.datapath, filename);
	sprintf(dest_addr, "%s/%s", settings.savepath, filename);

	FILE *src, *dst;
	src = fopen(source_addr, "r");
	dst = fopen(dest_addr, "w");

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

	pugi::xml_document calibDoc;
	result = calibDoc.load_file((std::string(globals.datapath) + "/" + filename).c_str());
	if(!result) {
		LOGE(TAG, " %s loaded with error: %s, using defaults...\n", filename, result.description());
		exit(1);
		return;
	}
	LOGI(TAG, " %s loaded successfully!(%s)\n", filename, result.description()); fflush(stdout);

	copyToSavePath(filename);

	pugi::xml_node simParams = calibDoc.child("simParams");
	pugi::xml_node node;
	for(node = simParams.first_child(); node; node = node.next_sibling()) {
		if(node.empty())
			continue;

		strcpy(node_name, node.name());

		if(strcmp(node_name, "mainParams") == 0) {
			if(node.attribute("renderWhat")) {
				char strparam_render_what[20];
				strcpy(strparam_render_what, node.attribute("renderWhat").as_string());
				if(strcmp("IMAGE", strparam_render_what) == 0)
					param_render_what = RENDER_IMAGE;
				if(strcmp("DEPTH", strparam_render_what) == 0)
					param_render_what = RENDER_DEPTH;
			}
			if(node.attribute("showWhat")) {
				char strparam_show_what[20];
				strcpy(strparam_show_what, node.attribute("showWhat").as_string());
				if(strcmp("PATTERN", strparam_show_what) == 0)
					param_show_what = SHOW_PATTERN;
				if(strcmp("MAP", strparam_show_what) == 0)
					param_show_what = SHOW_MAP;
			}
			if(node.attribute("doWhat")) {
				char strparam_do_what[20];
				strcpy(strparam_do_what, node.attribute("doWhat").as_string());
				if(strcmp("SAVE_BMP", strparam_do_what) == 0)
					param_do_what = DO_SAVE_BMP;
				if(strcmp("IMAGE_PROC", strparam_do_what) == 0)
					param_do_what = DO_IMAGE_PROC;
                if(strcmp("SAVE_ROS_BAG", strparam_do_what) == 0)
                    param_do_what = DO_SAVE_ROS_BAG;
			}

			LOGI(TAG, "mainParams: %i, %i, %i\n", param_render_what, param_show_what, param_do_what);
		}

		if(strcmp(node_name, "mapParams") == 0) {
			if(node.attribute("centerLat")) param_map_center_lat = node.attribute("centerLat").as_double();
			if(node.attribute("centerLon")) param_map_center_lon = node.attribute("centerLon").as_double();
			if(node.attribute("maxAlt")) param_map_max_alt = node.attribute("maxAlt").as_double();
			if(node.attribute("maxDist")) param_map_max_dist = node.attribute("maxDist").as_double();
			if(node.attribute("zoomLevel")) param_map_zoom = node.attribute("zoomLevel").as_int();

			LOGI(TAG, "mapParams: %f, %f, %f, %f, %i\n", param_map_center_lat, param_map_center_lon, param_map_max_alt, param_map_max_dist, param_map_zoom);
		}

		if(strcmp(node_name, "patternParams") == 0) {
			if(node.attribute("metricWidth")) param_pattern_metric_width = node.attribute("metricWidth").as_double();
			if(node.attribute("patternFile")) {
				strcpy(param_pattern_file, globals.datapath);
				strcat(param_pattern_file, "/");
				strcat(param_pattern_file, node.attribute("patternFile").as_string());
			}

			LOGI(TAG, "patternParams: %f, %s\n", param_pattern_metric_width, param_pattern_file);
		}

		if(strcmp(node_name, "pathParams") == 0) {
			if(node.attribute("pathFile")) {
				strcpy(param_path_file, globals.datapath);
				strcat(param_path_file, "/");
				strcat(param_path_file, node.attribute("pathFile").as_string());
			}
			if(node.attribute("csvPositionFmt")) {
				char strparam_position_format[10];
				strcpy(strparam_position_format, node.attribute("csvPositionFmt").as_string());
				if(strcmp("LLA", strparam_position_format) == 0)
					param_position_format = POSITION_FMT_LLA;
				if(strcmp("XYA", strparam_position_format) == 0)
					param_position_format = POSITION_FMT_XYA;
				if(strcmp("XYZ", strparam_position_format) == 0)
					param_position_format = POSITION_FMT_XYZ;
			}

			if(node.attribute("trackerType")) param_tracker_type = node.attribute("trackerType").as_int();
			if(node.attribute("controlFreq")) param_control_freq = node.attribute("controlFreq").as_double();
			if(node.attribute("accMax")) param_max_acc = node.attribute("accMax").as_double();
			if(node.attribute("angAccMax")) param_max_ang_acc = node.attribute("angAccMax").as_double();
			LOGI(TAG, "pathParams: %s, %i, %i, %f, %f, %f\n", param_path_file, param_tracker_type, param_position_format, param_control_freq, param_max_acc, param_max_ang_acc);
		}

		if(strcmp(node_name, "pathParamsExtra") == 0) {
			if(node.attribute("speedFactor")) param_speed_factor = node.attribute("speedFactor").as_double();
			if(node.attribute("worldScale")) param_world_scale = node.attribute("worldScale").as_double();
			if(node.attribute("altOffset")) param_alt_offset = node.attribute("altOffset").as_double();
			LOGI(TAG, "pathParamsExtra: %f, %f, %f\n", param_speed_factor, param_world_scale, param_alt_offset);
		}

		if(strcmp(node_name, "sensorParams") == 0) {
			if(node.attribute("cameraFps")) param_camera_fps = node.attribute("cameraFps").as_double();
			if(node.attribute("phasePercent")) param_camera_phase_percent = node.attribute("phasePercent").as_double();
			LOGI(TAG, "sensorParams: %f, %f\n", param_camera_fps, param_camera_phase_percent);
		}

		if(strcmp(node_name, "idealParams") == 0) {
			double param_fov_x, param_fov_y;
			if(node.attribute("resolutionScale")) param_resolution_scale = node.attribute("resolutionScale").as_double();
			if(node.attribute("width")) param_width = node.attribute("width").as_int();
			if(node.attribute("height")) param_height = node.attribute("height").as_int();

			if(node.attribute("oxOffset")) {
				param_ox = node.attribute("oxOffset").as_double();
				param_ox += param_width / 2.;
			}
			if(node.attribute("oyOffset")) {
				param_oy = node.attribute("oyOffset").as_double();
				param_oy += param_height / 2.;
			}

			if(node.attribute("ox")) {
				param_ox = node.attribute("ox").as_double();
			}
			if(node.attribute("oy")) {
				param_oy = node.attribute("oy").as_double();
			}

			if(node.attribute("fovX")) {
				param_fov_x = node.attribute("fovX").as_double();
				param_fx = (param_width / 2.) / tan(param_fov_x / 2.*M_PI / 180.);
			}
			if(node.attribute("fovY")) {
				param_fov_y = node.attribute("fovY").as_double();
				param_fy = (param_height / 2.) / tan(param_fov_y / 2.*M_PI / 180.);
			}

			if(node.attribute("fovX") && !node.attribute("fovY"))
				param_fy = param_fx;

			if(!node.attribute("fovX") && node.attribute("fovY"))
				param_fx = param_fy;

			if(node.attribute("f"))
				param_fx = param_fy = node.attribute("f").as_double();

			if(node.attribute("fx"))
				param_fx = node.attribute("fx").as_double();

			if(node.attribute("fy"))
				param_fy = node.attribute("fy").as_double();

			if(node.attribute("fErr"))
				param_f_err = node.attribute("fErr").as_double();
			if(node.attribute("oxErr"))
				param_ox_err = node.attribute("oxErr").as_double();
			if(node.attribute("oyErr"))
				param_oy_err = node.attribute("oyErr").as_double();

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
			if(node.attribute("maxLensIterations")) param_max_lens_iterations = node.attribute("maxLensIterations").as_int();
			if(node.attribute("maxPixError")) param_max_pix_error = node.attribute("maxPixError").as_double();
			if(node.attribute("k1")) param_k1 = node.attribute("k1").as_double();
			if(node.attribute("k2")) param_k2 = node.attribute("k2").as_double();
			if(node.attribute("t1")) param_t1 = node.attribute("t1").as_double();
			if(node.attribute("t2")) param_t2 = node.attribute("t2").as_double();
			if(node.attribute("k3")) param_k3 = node.attribute("k3").as_double();
			if(node.attribute("k4")) param_k4 = node.attribute("k4").as_double();
			if(node.attribute("k5")) param_k5 = node.attribute("k5").as_double();
			if(node.attribute("k6")) param_k6 = node.attribute("k6").as_double();
			LOGI(TAG, "staticParams: %i, %f, %f, %f, %f, %f, %f, %f, %f, %f\n", param_max_lens_iterations, param_max_pix_error, param_k1, param_k2, param_t1, param_t2,
				 param_k3, param_k4, param_k5, param_k6);

			if(node.attribute("k1Err")) param_k1_err = node.attribute("k1Err").as_double();
			if(node.attribute("k2Err")) param_k2_err = node.attribute("k2Err").as_double();
			if(node.attribute("t1Err")) param_t1_err = node.attribute("t1Err").as_double();
			if(node.attribute("t2Err")) param_t2_err = node.attribute("t2Err").as_double();
			if(node.attribute("k3Err")) param_k3_err = node.attribute("k3Err").as_double();
		}

		if(strcmp(node_name, "vignetParams") == 0) {
			if(node.attribute("vignetFile")) {
				strcpy(param_vignet_file, globals.datapath);
				strcat(param_vignet_file, "/");
				strcat(param_vignet_file, node.attribute("vignetFile").as_string());
			}
			if(node.attribute("vignetThresh1")) param_vignet_thresh1 = node.attribute("vignetThresh1").as_double();
			if(node.attribute("vignetThresh2")) param_vignet_thresh2 = node.attribute("vignetThresh2").as_double();
			LOGI(TAG, "vignetParams: %s, %f, %f\n", strlen(param_vignet_file) > 0 ? param_vignet_file : "NO VIGNET", param_vignet_thresh1, param_vignet_thresh2);
		}

		if(strcmp(node_name, "dynamicParams") == 0) {
			if(node.attribute("td")) param_td = node.attribute("td").as_double();
			if(node.attribute("tr")) param_tr = node.attribute("tr").as_double();
			if(node.attribute("te")) param_te = node.attribute("te").as_double();
			if(node.attribute("motionBlurExtraSamples")) param_motion_blur_extra_samples = node.attribute("motionBlurExtraSamples").as_int();
			if(node.attribute("ti")) param_ti = node.attribute("ti").as_double();

			//The real timing contains these errors
			if(node.attribute("tdErr")) param_td_err = node.attribute("tdErr").as_double();
			if(node.attribute("trErr")) param_tr_err = node.attribute("trErr").as_double();

			LOGI(TAG, "dynamicParams: %f, %f, %f, %i, %f, errTd:%f, errTr:%f\n", param_td, param_tr, param_te, param_motion_blur_extra_samples, param_ti,
				 param_td_err, param_tr_err);
		}

		if(strcmp(node_name, "marginParams") == 0) {
			if(node.attribute("extraMargin")) param_extra_margin = node.attribute("extraMargin").as_double();
			if(node.attribute("extraZoom")) param_extra_zoom = node.attribute("extraZoom").as_double();
			LOGI(TAG, "marginParams: %f, %f,\n", param_extra_margin, param_extra_zoom);
		}

		if(strcmp(node_name, "signalParams") == 0) {
			if(node.attribute("dayLight")) param_day_light = node.attribute("dayLight").as_double();
			if(node.attribute("noiseAmpDynamic")) param_noise_amp_dynamic = node.attribute("noiseAmpDynamic").as_double();
            if(node.attribute("noiseAmpStatic1")) param_noise_amp_static1 = node.attribute("noiseAmpStatic1").as_double();
            if(node.attribute("noiseAmpStatic2")) param_noise_amp_static2 = node.attribute("noiseAmpStatic2").as_double();
            if(node.attribute("fogVisibilityDistance")) param_max_fog_distance = node.attribute("fogVisibilityDistance").as_double();
			if(node.attribute("seed")) param_seed = node.attribute("seed").as_int();
			LOGI(TAG, "signalParams: %f, %f, %f, %f, %f, %i\n", param_day_light, param_noise_amp_dynamic, param_noise_amp_static1, param_noise_amp_static2, param_max_fog_distance, param_seed);
		}

		if(strcmp(node_name, "camInAcEu") == 0) {
			//Nominal installation angles
			if(node.attribute("roll")) param_cam_in_ac_roll = node.attribute("roll").as_double();
			if(node.attribute("pitch")) param_cam_in_ac_pitch = node.attribute("pitch").as_double();
			if(node.attribute("yaw")) param_cam_in_ac_yaw = node.attribute("yaw").as_double();

			//The real rotation contains these errors
			if(node.attribute("xErr")) param_cam_in_ac_err_x = node.attribute("xErr").as_double();
			if(node.attribute("yErr")) param_cam_in_ac_err_y = node.attribute("yErr").as_double();
			if(node.attribute("zErr")) param_cam_in_ac_err_z = node.attribute("zErr").as_double();

			double param_cam_in_ac_err = 0;
			if(node.attribute("err")) param_cam_in_ac_err = node.attribute("err").as_double();
			if(!node.attribute("xErr")) param_cam_in_ac_err_x = ((rand() % 2) * 2 - 1) * param_cam_in_ac_err;
			if(!node.attribute("yErr")) param_cam_in_ac_err_y = ((rand() % 2) * 2 - 1) * param_cam_in_ac_err;
			if(!node.attribute("zErr")) param_cam_in_ac_err_z = ((rand() % 2) * 2 - 1) * param_cam_in_ac_err;

			LOGI(TAG, "camInAcEu: %f, %f, %f, errX:%f, errY:%f, errZ:%f\n", param_cam_in_ac_roll, param_cam_in_ac_pitch, param_cam_in_ac_yaw,
				 param_cam_in_ac_err_x, param_cam_in_ac_err_y, param_cam_in_ac_err_z);
		}

		//overrides
		if(strcmp(node_name, "includeParams") == 0) {
			char include_file[MAX_PATH_LENGTH];
			if(node.attribute("includeFile")) {
				strcpy(include_file, node.attribute("includeFile").as_string());
			}

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

#ifdef __cplusplus
}
#endif
