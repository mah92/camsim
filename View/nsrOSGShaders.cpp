#include "./nsrOSGShaders.h"

//Shaders//////////////////////////////////////////////////

//Helps:
//1)double is not known in glsl, use highp float
//2)an input var can not be changed in function for temporary use
//3)an int can't be multiplied/divided by float(even an int number like 2)
//4)a vertexShader is just evaluated in 4 corners of an image(in createTexturedQuadGeometry), use createGridGeometry instead
//ref: https://github.com/xarray/osgRecipes/blob/master/cookbook/chapter3/ch03_09/vertex_displacement.cpp
//5)you can not use float keyword before defining its precision, but you can you multiple definitions of precision in one shader

//shader test site: http://shdr.bkcore.com/
//shader test & idea site: www.shadertoy.com

//Help: for loading shaders from external files:
//osg::Shader * vshader = new osg::Shader(osg::Shader::VERTEX);
//osg::Shader * fshader = new osg::Shader(osg::Shader::FRAGMENT);
//vshader->loadShaderSourceFromFile( "/mnt/sdcard/AngryBirds/shaders/brick.vert");
//fshader->loadShaderSourceFromFile( "/mnt/sdcard/AngryBirds/shaders/brick.frag");
//prog->addShader ( vshader );
//prog->addShader ( fshader );

/*Android uses OpenGL ES, and GLSL versions are different between OpenGL ES and full OpenGL.

Valid versions:
	version 100: ES 2.0.
	version 110: OpenGL 2.0.
	version 120: OpenGL 2.1.
	version 130: OpenGL 3.0.
	version 140: OpenGL 3.1.
	version 150: OpenGL 3.2.
	version 300 es: ES 3.0.
	version 310 es: ES 3.1.
	version 330: OpenGL 3.3.
	version 400: OpenGL 4.0.
	version 410: OpenGL 4.1.
	version 420: OpenGL 4.2.
	version 430: OpenGL 4.3.
	version 440: OpenGL 4.4.
	version 450: OpenGL 4.5.

Comment on warnings:
	some comments inside string causes warning!!!
	not using tex.a in 4th channel causes warning, but works!!!
	precision highp(in fragment shader) not supported on some systems,  causes opengl warning
	not defining precision causes opengl warning
	no space after texture2D will cause warning
	defining unfound varying causes warning
	"float a = 1.;\n" causes warning!!!
	"uniform mat4 osg_ModelViewMatrix;\n" causes warning!!!
	///////////////////////////
	//So I disabled warnings in android!!!
	///////////////////////////
*/

//Help1: compatibility with gles2.0
//////__VERSION__ >= 130,
/// use in, out instead of varying, attribute
/// use any "out vec4" as gl_FragColor in fragment shader
/// default float precision in fragment shader is needed in es2 es3 gl3 opengl contexts

//some OpenGL contexts need #version, otherwise they are not compiled
//my ASUS laptop just supports 100(gles2), 110(gl2.0), 120(gl2.1), 130(gl3.0)
//array varying needs 100 or >=120
//#define OSG_GLES2_AVAILABLE
#ifdef OSG_GLES2_AVAILABLE
#define VERSION_COMMON \
	"#version 100\n"
#else
#define VERSION_COMMON \
	"#version 130\n" \
	"#define highp\n" \
	"#define mediump\n" \
	"#define lowp\n"
#endif

#ifdef ANDROID
#define VERTEX_COMMON \
	"#if __VERSION__ >= 130\n" \
	"   #define attribute in\n" \
	"   #define varying out\n" \
	"#endif\n" \
	"attribute vec4 osg_Vertex;\n" /*shows proposed pixel position(xyz) in image*/ \
	"attribute vec4 osg_Color;\n" \
	"attribute vec4 osg_MultiTexCoord0;\n" /*shows normal pixel coordinates in tile*/ \
	"uniform mat4 osg_ModelViewProjectionMatrix;\n" /*shows rotation from parent nodes*/ \
	"uniform mat4 osg_ModelViewMatrix;\n"
#else //desktop linux
#define VERTEX_COMMON \
	"#if __VERSION__ >= 130\n" \
	"   #define attribute in\n" \
	"   #define varying out\n" \
	"#endif\n" \
	"#define osg_Vertex gl_Vertex\n" \
	"#define osg_Color gl_Color\n" \
	"#define osg_MultiTexCoord0 gl_MultiTexCoord0\n" \
	"#define osg_ModelViewProjectionMatrix gl_ModelViewProjectionMatrix\n" \
	"#define osg_ModelViewMatrix gl_ModelViewMatrix\n" //definition causes warning, but works!!!

#endif

//"#define FTRANSFORM() gl_ModelViewProjectionMatrix * gl_Vertex;\n" /*works in desktop for 110, 120, 130, not 100, android???*/
//"#define FTRANSFORM() osg_ModelViewProjectionMatrix * osg_Vertex;\n" /*not works in desktop at all, works in android!!!*/
//"#define FTRANSFORM() ftransform();\n"*/ /*works in desktop for 110, 120, 130, not 100, android???*/
//no solution found for gles1.0(100) in desktop

#define FRAGMENT_COMMON \
	"#if __VERSION__ >= 130\n" \
	"   #define varying in\n" \
	"   #define texture2D texture\n" \
	"   out vec4 mgl_FragColor;\n" \
	"#else\n" \
	"   #define mgl_FragColor gl_FragColor\n" \
	"#endif\n"
/*is wrong, no needed
"#if __VERSION__ == 120\n" \
//"	#define texture2D texture2DRect\n" \
//"	#define sampler2D sampler2DRect\n" \
"#endif\n" \*/

//ints & floats "minimum" ranges:
//ints:
//	lowp [-255, +255]
//	mediump [-1023, +1023]
//	highp [-65535, +65535] -implementation is optional(but implemented in Galaxy S2)!!!! (#if GL_FRAGMENT_PRECISION_HIGH == 1)
//floats:
//	lowp - 8 bit, floating point range: -2 to 2, integer range: -2^8 to 2^8
//	mediump - 10 bit, floating point range: -2^14 to 2^14, integer range: -2^10 to 2^10
//	highp - 16-bit, floating point range: -2^62 to 2^62, integer range: -2^16 to 2^16 -implementation is optional!!!!

//defaults in vertex shader:
//	precision highp float;
//	precision highp int;
//defaults in fragment shader:
//	precision mediump int;

#ifdef OSG_GLES2_AVAILABLE
//in vertex, defaults are already highp
#define FRAGMENT_MEDIUMP \
	"precision mediump float;\n"
#define FRAGMENT_HIGHP  \
	"#if GL_FRAGMENT_PRECISION_HIGH == 1\n" \
	"	precision highp float;\n" \
	"#else\n" \
	"	precision mediump float;\n" \
	"#endif\n"
#else
//precision qualifiers not supported in opengl
#define FRAGMENT_MEDIUMP ""
#define FRAGMENT_HIGHP ""
#endif

//simplified white shader//////////////////////////////////
//verified in laptop 110, 120, 130
char whiteVertexShader[] =
	VERSION_COMMON
	VERTEX_COMMON
	"const vec4 mColor = vec4(0.8, 0.8, 0.8, 1.0);\n"
	"varying vec4 vertexColor;\n"
	"void main() {\n"
	"    gl_Position = osg_ModelViewProjectionMatrix * osg_Vertex;\n"
	"    vertexColor = mColor;\n"
	"}\n";

char whiteFragmentShader[] =
	VERSION_COMMON
	FRAGMENT_COMMON
	FRAGMENT_MEDIUMP
	"varying vec4 vertexColor;\n"
	"void main() {\n"
	"  mgl_FragColor = vertexColor;\n"
	"}\n";

//image shader/////////////////////////////////////////////
//verified in laptop 110, 120, 130
char imageVertexShader[] =
	VERSION_COMMON
	VERTEX_COMMON
	"varying vec4 texCoord;\n"
	"\n"
	"void main(void) {\n"
	"   gl_Position = osg_ModelViewProjectionMatrix * osg_Vertex;\n"
	"   texCoord = osg_MultiTexCoord0;\n"
	"}\n";

char imageFragmentShader[] =
	VERSION_COMMON
	FRAGMENT_COMMON
	FRAGMENT_MEDIUMP
	"uniform sampler2D baseTexture;\n"
	"varying vec4 texCoord;\n"
	"void main() {\n"
	"	vec4 tex = texture2D (baseTexture, texCoord.xy);\n" //no space after texture2D will cause warning
	"	mgl_FragColor = vec4(tex.r, tex.g, tex.b, tex.a);\n"
	"}\n";

char imageFragmentShaderGamma[] =
	VERSION_COMMON
	FRAGMENT_COMMON
	FRAGMENT_MEDIUMP
	"uniform sampler2D baseTexture;\n"
	"varying vec4 texCoord;\n"
	"void main() {\n"
	"	vec4 tex = texture2D (baseTexture, texCoord.xy);\n" //no space after texture2D will cause warning
	//"	vec3 newColor = pow(tex.rgb, vec3(2.2));\n" //bigger than 1 gamma_factor causesimage darkening but just in the middle darkness
	"	vec3 newColor = tex.rgb*0.8;\n" //bigger than 1 gamma_factor causesimage darkening but just in the middle darkness
	"	mgl_FragColor = vec4(newColor.r, newColor.g, newColor.b, tex.a);\n"
	"}\n";

char imageFragmentShader2[] =
	VERSION_COMMON
	FRAGMENT_COMMON
	FRAGMENT_MEDIUMP
	"uniform sampler2D baseTexture;\n"
	"uniform float transparency;\n"
	"varying vec4 texCoord;\n"
	"void main() {\n"
	"	vec4 tex = texture2D (baseTexture, texCoord.xy);\n" //no space after texture2D will cause warning
	"	mgl_FragColor = vec4(tex.r, tex.g, tex.b, tex.a*transparency);\n"
	"}\n";

//texture2D input(texCoord) and output are normalized:[0,1], origin at x:left, y:up
//Due to unknown error, just including uniform sampler1D causes shader to disappear
char imageFragmentShaderMapped[] =
	VERSION_COMMON
	FRAGMENT_COMMON
	FRAGMENT_MEDIUMP
	"uniform sampler2D baseTexture;\n"
	"uniform float transparency;\n"
	"uniform float total_brightness;\n"
	"varying vec4 texCoord;\n"
	"void main() {\n"
	"	vec4 tex = texture2D (baseTexture, texCoord.xy);\n" //no space after texture2D will cause warning
	"	float grayLevel = (tex.r + tex.g + tex.b)*0.333;\n"
	"	float newval = texture2D (baseTexture, vec2(grayLevel, 0.)).r;\n" //no space after texture2D will cause warning
	"	//if(texCoord.y<0.1 || texCoord.x<0.1 || texCoord.x>0.9 || texCoord.y>0.9)\n"
	"		//mgl_FragColor = tex*0.9-0.025;\n" //best
	"		mgl_FragColor = vec4(tex.r*0.78-0.13, tex.g*0.7-0.1, tex.b*0.8-0.1, tex.a);\n" //test
	"		//mgl_FragColor = vec4(tex.r*0.9-0.025, tex.g*0.9-0.025, tex.b*0.9-0.025, tex.a);\n" //better
	"		//mgl_FragColor = vec4(tex.r*0.6, tex.g*0.6, tex.b*0.6, tex.a);\n"
	"	//else {\n"
	"		//mgl_FragColor = vec4(0.5, 0.5, 0.5, tex.a);\n"
	"		//mgl_FragColor = vec4(newval*tex.r, newval*tex.g, newval*tex.b, tex.a);\n"
	"		//mgl_FragColor = vec4(newval, newval, newval, tex.a);\n"
	"		//mgl_FragColor = vec4(newval.r, newval.g, newval.b, tex.a);\n"
	"	//}\n"
	"}\n";

//////////////////////////////////////////////////////////

char depthVertexShader[] =
	VERSION_COMMON
	VERTEX_COMMON
	"varying vec4 texCoord;\n"
	"varying vec4 cs_position;\n"
	"\n"
	"void main(void) {\n"
	"   gl_Position = osg_ModelViewProjectionMatrix * osg_Vertex;\n"
	"   texCoord = osg_MultiTexCoord0;\n"
	"   cs_position = osg_ModelViewMatrix * osg_Vertex;\n" //in camera coordinates
	"}\n";

char depthFragmentShader[] =
	VERSION_COMMON
	FRAGMENT_COMMON
	FRAGMENT_HIGHP
	"uniform sampler2D baseTexture;\n"
	"varying vec4 texCoord;\n" //varyings are interpolated(in osg::Texture2D::LINEAR) between vertices
	"varying vec4 cs_position;\n"
	"void main() {\n"
	"	float originalZ = -cs_position.z;\n"	//in vertex shader: "	cs_position = osg_ModelViewMatrix * osg_Vertex;\n" //in camera coordinates
	//direct distance, contour solution
	//use (1., 1., 1.) as infinity space
	// max = 255.*255. = 65 km, resolution = 1/255m = 4mm
	"	mgl_FragColor = vec4(floor(floor(originalZ)/255.)/255., fract(floor(originalZ)/255.), fract(originalZ), 1.);\n"  //verified in osg::Texture2D::NEAREST
	//recovery: color.r*(255.*255.) + color.g*255. + color.b //verified, (colors within [0,1])
	"}\n";

//lens shader/////////////////////////////////////////////
char lensVertexShader[] =
	VERSION_COMMON
	VERTEX_COMMON
	"varying vec4 distortedTexCoord;\n"
	"\n"
	"void main(void) {\n"
	"   gl_Position = osg_ModelViewProjectionMatrix * osg_Vertex;\n"
	"   distortedTexCoord = osg_MultiTexCoord0;\n"
	"}\n";

char lensFragmentShader[] =
	VERSION_COMMON
	FRAGMENT_COMMON
	FRAGMENT_HIGHP
	"uniform sampler2D baseTexture1;\n"
	"uniform sampler2D baseTexture2;\n"
	"uniform sampler2D depthTexture1;\n"
	"uniform sampler2D depthTexture2;\n"
	"uniform sampler2D vignetTexture;\n"
	"varying vec4 distortedTexCoord;\n"

	"uniform float inputVect[39];\n"

	"float \n"
	"	render_depth, seed, time_s, wx, wy, wz, vx, vy, vz,\n"
	"		k1, k2, t1, t2, k3, k4, k5, k6,\n"
	"			vignet_thresh1, vignet_thresh2, \n"
	"				td, tr, te, ti,\n"
	"				width, height,\n"
	"					fx, fy, ox, oy, extra_margin, extra_zoom,\n"
	"						noise_amplitude_dyn, noise_amplitude_st1, noise_amplitude_st2, day_light;\n"
	"int extra_sampling_points, double_input;\n"

	//direct sampling = 4, interpolated = 2, + xx + xx + xx +, equals 10 points
	"#define DIRECT_SAMPLING_POINTS 2\n" //for motion blur
	"#define MAX_LENS_ITERATIONS (int(inputVect[9]))\n" //points interploated between two sampling points, more efficient than just using DIRECT_SAMPLING_POINTS
	"#define MAX_PIX_ERR inputVect[10]\n"

	"#define aa 8121.	\n"
	"#define cc 0.845213304	\n"
	"#define rand(_seed) fract(aa*_seed+cc)\n" //inputs and outputs[0.,1.)

	"#define pi 3.141592653\n"

	//ref: https://thebookofshaders.com/edit.php?log=161119150756
	//http://byteblacksmith.com/improvements-to-the-canonical-one-liner-glsl-rand-for-opengl-es-2-0/
	"float random (in vec2 st) {\n"
	"	//return fract(sin(dot(st.xy,vec2(12.9898,78.233))) * 43758.5453);\n"
	"	return fract(sin(mod(dot(st.xy,vec2(12.9898,78.233)),3.14)) * 43758.5453);\n"
	"}\n"

	/* normal random variate generator */
	/* The polar form of the Box-Muller transformation*/
	/* mean 0., standard deviation 1. */
	/* Improvements can be done by getting 2 random number instead of one*/
	"float randomN(in vec2 st)\n"
	"{\n"
	"	float x1, x2, w;\n"
	"	do {\n"
	"		x1 = 2.*random(st) - 1.;\n"
	"		st.x += 0.1;\n"
	"		x2 = 2.*random(st) - 1.;\n"
	"		st.y += 0.1;\n"
	"		w = x1*x1 + x2*x2;\n"
	"	} while (w >= 1.);\n"

	"	if(w < 0.00001)  w = 0.00001;\n" //prevent w==0 which causes division by zero
	"	w = sqrt( -(2.*log(w)) / w );\n"
	"	return x1*w; // or x2*w\n"
	"}\n"

	///Inverse Lens///////////////////////////////////////////////////////
	//Distortion based on five parameters correction model used in opencv
	//Here we need to distort but because we use a fragment shader, inverse formula is needed
	//which is the distortion correction(undistortion) model
	//There is no undistortion formula so we use an iterative approach to calc the inverse

	//note: Radial distortion can be written as additive, so that radial and tangential distortion turn can be changed
	//mistake notes: in shelley r:=r2 so dr:= 1+k1*r+k2*r2+k3*r3
	//mistake notes: in OriellyOpenCV, distortion formula is wrong, see other opencv docs
	//mistake notes: in shelley2014(p.33) && opencv doxygen document, distortion formula is used as camera model,
	//mistake notes: opencv tutorial(not doxygen) is !!!WRONG!!! expressing an undistortion model,
	//see:http://docs.opencv.org/2.4/doc/tutorials/calib3d/camera_calibration/camera_calibration.html
	//NOTICE: this function is different to standard function in that it also removes ox, oy offsets
	"float inverse_lens(in vec2 distortedTexCoord, out vec2 texCoord_noLens) {\n"
	"	int i;\n"
	"	float x, y, u_distorted, v_distorted, u, v, u2, v2, uv, r2, r4, r6, _dr, pix_x, pix_y, error;\n"
	"	vec2 dt, distortion, uv_nolens, pix, res;\n"

	//convert to pixels
	"	x = distortedTexCoord.x*width;\n"
	"	y = (1.-distortedTexCoord.y)*height;\n" //y in a picture is defined from up to down

	"	u_distorted = (x - ox)/fx;\n"
	"	v_distorted = (y - oy)/fy;\n"

	//First assumption: dt, dr does not change much after distortion
	//By iterating we reach real coordinates
	"	u = u_distorted;\n" //first assumption
	"	v = v_distorted;\n"

	"	for(i = 0; i < MAX_LENS_ITERATIONS; i++) {\n"
	"		u2 = u*u; v2 = v*v; uv = u*v;\n"
	"		r2 = u2 + v2; r4 = r2*r2; r6 = r4*r2;\n"
	"		dt = vec2(2.*t1*uv + t2*(r2+2.*u2),"
	"				t1*(r2+2.*v2) + 2.*t2*uv);\n"
	"		_dr = 1. + k1*r2 + k2*r4 + k3*r6;\n" //3 param radial distortion
	//"		_dr = (1. + k1*r2 + k2*r4 + k3*r6)/(1. + k4*r2 + k5*r4 + k6*r6);\n" //6 param radial distortion

	//Increases fps
	"       //res = vec2(u_distorted, v_distorted) - (_dr* vec2(u, v) + dt);\n"
	"       //if(abs(res.x*width*1.42) < MAX_PIX_ERR && abs(res.y*height*1.42) < MAX_PIX_ERR)\n"
	"       //     break;\n"

	//Consider convergance to out of ideal(bigger) frame as divergance
	//Increases fps
	"       pix_x = u/extra_margin*fx+ox; pix_y = v/extra_margin*fy+oy;\n"
	"       if(pix_x <0. || pix_x > width || pix_y < 0. || pix_y > height)\n"
	"            return 1000.;\n" //high value as error

	"		distortion = (_dr*vec2(u, v) + dt) - vec2(u, v);\n"
	"		u = u_distorted - distortion.x;\n"
	"		v = v_distorted - distortion.y;\n"
	"	}\n"

	"	uv_nolens = vec2(u, v);\n"
	"	uv_nolens *= extra_zoom;\n"

	//convert to ideal rendered coordinates, in ideal coordinates, ox, oy offsets are not used
	"	pix = vec2(fx*uv_nolens.x + width/2., fy*uv_nolens.y + height/2.);\n" //converting to pixels in out texture
	"	texCoord_noLens = vec2(pix.x/width, 1.-pix.y/height);\n" //texCoord is defined from up to down

	//final error calculation
	"	u2 = u*u; v2 = v*v; uv = u*v;\n"
	"	r2 = u2 + v2; r4 = r2*r2; r6 = r4*r2;\n"
	"   res = vec2(u_distorted, v_distorted) - (_dr* vec2(u, v) + dt);\n"
	"   res.x*=width; res.y*=height;\n"
	"   error = length(res);\n"
	"   return error; //error in pixels \n"
	"}\n"

	//standard gauss-newton solver, 8 param(and 5 param),
	//simplified form of kalibr inverse radialTangential model(in kalibr, no simplification used for symetric J)
	//NOTICE: this function is different to standard function in that it also removes ox, oy offsets
	"float inverse_lens2(in vec2 distortedTexCoord, out vec2 texCoord_noLens) {\n"
	"  int i;\n"
	"  float x, y, u_distorted, v_distorted, u_proposed, v_proposed, u, v, u2, v2, uv, r2, r4, r6, _dr;\n"
	"  float pix_x, pix_y, pre_error, error = 1000.; //a large number\n"
	"  float dr_num, dr_den_inv, Ddr_num, Ddr_den, s0, den;\n"
	"  vec2 dt, distortion, uv_nolens, pix, J1, J2, res, update;\n"

	//convert to pixels
	"  x = distortedTexCoord.x*width;\n"
	"  y = (1.-distortedTexCoord.y)*height;\n" //y in a picture is defined from up to down

	"  u_distorted = (x - ox)/fx;\n"
	"  v_distorted = (y - oy)/fy;\n"

	//First assumption: dt, dr does not change much after distortion
	"  u = u_distorted;\n" //first assumption
	"  v = v_distorted;\n"

	"  for(i = 0; i < MAX_LENS_ITERATIONS; i++) {\n"
	//calc J/////////////////////////////////////////////////
	//assuming 6 param radial distortion

	//J = -Dv/Dparams = [Df/Du; Df/Dv]
	//[J1; = [-Dv1/Dparams;
	// J2]    -Dv2/Dparams]
	//J = [J1x, J1y; = [Dfu/Du, Dfu/Dv;
	//     J2x, J2y]    Dfv/Du, Dfv/Dv]

	"      u2 = u*u; v2 = v*v; uv = u*v;\n"
	"      r2 = u2 + v2; r4 = r2*r2; r6 = r4*r2;\n"

	"      dr_num = 1. + k1*r2 + k2*r4 + k3*r6;\n"
	"      //dr_den_inv = 1./(1. + k4*r2 + k5*r4 + k6*r6);\n"
	"      _dr = dr_num;//*dr_den_inv;\n"

	"      dt = vec2(2.*t1*uv + t2*(r2+2.*u2),\n"
	"			t1*(r2+2.*v2) + 2.*t2*uv);\n"

	"      Ddr_num = k1+2*k2*r2+3*k3*r4;\n" //D(dr_num)/D(r2)
	"      //Ddr_den = k4+2*k5*r2+3*k6*r4;\n" //D(dr_den)/D(r2)
	"      //s0 = (Ddr_num - Ddr_den * _dr) * dr_den_inv;\n" //D(dr)/D(r2)
	"      s0 = Ddr_num;\n" //D(dr)/D(r2)

	"      J1.x = (6.*t2*u + 2.*t1*v) + (_dr + 2.*u2*s0);\n" //J1x = Dfu/Du
	"      J1.y = (2.*t1*u + 2.*t2*v) + (2.*uv*s0);\n"       //J1y = Dfu/Dv
	"      J2.x = J1.y;\n"                                   //J2x = Dfv/Du
	"      J2.y = (2.*t2*u + 6.*t1*v) + (_dr + 2.*v2*s0);\n" //J2y = Dfv/Dv

	//calc update////////////////////////////////////////////
	//update = (J'J)^-1 * J'v = (J1'J1 + J2'J2)^-1 * (J1'v1 + J2'v2)
	//J=J' so:
	//update = (J'J)^-1*J'v = (JJ)^-1*J*(J*J^-1)*v = (JJ)^-1*(JJ)*J^-1*v = J^-1*v
	//update = [  J2y/(J1x*J2y - J2x*J1y), -J1y/(J1x*J2y - J2x*J1y);
	//           -J2x/(J1x*J2y - J2x*J1y),  J1x/(J1x*J2y - J2x*J1y)]
	//          * [res.x; res.y];
	"      den = J1.x*J2.y - J2.x*J1.y;\n"
	"      res = vec2(u_distorted, v_distorted) - (_dr*vec2(u, v) + dt);\n"
	"      update.x = +J2.y/den*res.x - J1.y/den * res.y;\n"
	"      update.y = -J2.x/den*res.x + J1.x/den * res.y;\n"

	"      u_proposed = u + update.x;\n"
	"      v_proposed = v + update.y;\n"

	//calc error///////////////////////////////
	//we want to find (u,v) from (u_distorted, v_distorted)
	//residual = v = [u_dist, v_dist]' - f(u,v)
	//v1 = u_dist - fu(u,v)
	//v2 = v_dist - fv(u,v)

	"      u2 = u_proposed*u_proposed; v2 = v_proposed*v_proposed; uv = u_proposed*v_proposed;\n"
	"      r2 = u2 + v2; r4 = r2*r2; r6 = r4*r2;\n"

	"      dr_num = 1. + k1*r2 + k2*r4 + k3*r6;\n"
	"      //dr_den_inv = 1./(1. + k4*r2 + k5*r4 + k6*r6);\n"
	"      _dr = dr_num;//*dr_den_inv;\n"
	"      dt = vec2(2.*t1*uv + t2*(r2+2.*u2),\n"
	"			t1*(r2+2.*v2) + 2.*t2*uv);\n"

	"      res = vec2(u_distorted, v_distorted) - (_dr* vec2(u_proposed, v_proposed) + dt);\n"
	"      pre_error = error;\n"
	"      res.x*=width; res.y*=height;\n"
	"      error = length(res);\n"

	"      //total_iter_count++;\n"
	"      if(error > 500.) break;\n" //diverge
	"      if(error > pre_error) {error = pre_error; break;}\n" //starting to diverge, don't update

	"      u=u_proposed;\n"
	"      v=v_proposed;\n"

	"      if(error < MAX_PIX_ERR) break;\n" //converged
	"	}\n"

	"	uv_nolens = vec2(u, v);\n"
	"	uv_nolens = uv_nolens*extra_zoom;\n"

	//convert to ideal rendered coordinates, in ideal coordinates, ox, oy offsets are not used
	"	pix = vec2(fx*uv_nolens.x + width/2., fy*uv_nolens.y + height/2.);\n" //converting to pixels in out texture
	"	texCoord_noLens = vec2(pix.x/width, 1.-pix.y/height);\n" //texCoord is defined from up to down
	"   return error; //error in pixels \n"
	"}\n"

	///Inverse Rolling Shutter////////////////////////////////////////////////////////////
	//differentiating camera model h(shelley-eq5.4)) with respect to X, Y, Z
	//dh/dt = dh/dX*dX/dt + dh/dY*dY/dt + dh/dZ*dZ/dt
	//calculating fixed points speed in cam coordinates
	//eq.2-102, Greenwood, principles of Dynamics, 2nd edition, p.50
	//Vel := vec3(dX/dt, dY/dt, dZ/dt) = velocity with respect to cam axis, in cam axis
	//Vel = speed of camera (wrt inertia in cam axes) + ...
	// ... + speed of point (wrt. inertia in in cam axes) + ...
	// ... + angular speed of camera (in cam axes) x ...
	// ... x position of feature(in cam axes)
	//Vel = VCamWrtInertiaInCam + VpointWrtInertiaInCam + WCamWrtInertiaInCam x RpointInCam
	//			= (CNED2Cam * VCamWrtInertiaInNed) + (CNED2Cam * VpointWrtInertiaInNed) + (CNED2Cam * WCamWrtInertiaInNed) x RpointInCam
	//Rpoint = vec3(X, Y, Z) //point pose in cam
	//Vel = -VCam - cross(WCam, Rpoint) //velocity of point in cam axis
	//Vel2Z = -VCam2Z -cross(WCam, Rpoint2Z)
	//udot = Vel2Z.x - u*Vel2Z.z
	//vdot = Vel2Z.y - v*Vel2Z.z

	//When no lens distortion:
	//dh2du = vec2(fx, 0)
	//dh2dv = vec2(0, fy)

	//dh2dt = dh2du*udot + dh2dv*vdot = vec2(fx*udot, fy*vdot);\n"

	//h_with_rs(pixels) = h_no_rs(pixels) + dh2dt*deltaT; //dh2dt calculated at h_no_rs
	//or: h_no_rs(pixels) = h_with_rs(pixels) - dh2dt*deltaT; //dh2dt calculated at h_with_rs

	//Assuming no u, v change in RS length(so no udot, vdot change)
	"void calc_dh2dt(in vec2 h, in vec3 _VCam2Z, in vec3 _WCam, out vec2 _dh2dt) {"
	"	float u, v, udot, vdot;\n"
	"	vec3 Rpoint2Z, Vel2Z;\n"
	"	u = (h.x - width/2.)/fx;\n"
	"	v = (h.y - height/2.)/fy;\n"
	"	Rpoint2Z = vec3(u, v, 1.);\n" //point pose in cam / Z
	"	Vel2Z = -_VCam2Z -cross(_WCam, Rpoint2Z);\n"
	"	udot = Vel2Z.x - u*Vel2Z.z;\n"
	"	vdot = Vel2Z.y - v*Vel2Z.z;\n"
	//We are after lens, so we assume no lens distortion
	//Lens just affects deltaT(pixel syncronous lines occur after lens) which we computed before
	"	_dh2dt = vec2(fx*udot, fy*vdot);\n" //calculated at h_no_rs(1st approximation)
	"}\n"

	//frame_time_offset is the center frame time offset compensated outside for sensor inputs
    //Assuming no depth change in RS period
	"void inverse_rolling_shutter(in vec2 distortedTexCoord_, in vec2 texCoord_noLens, in float frame_time_offset, in float distance, out vec2 texCoord_noLensNoRS[DIRECT_SAMPLING_POINTS]) {\n"
	"	int i,j,row, RK2_STEPS;\n"
	"	float deltaT, deltaT2;\n"
	"	vec2 h_no_rs, h_with_rs, h1, h2, dh2dt0, dh2dt1, dh2dt2;\n"
	"	vec3 VCam2Z, WCam;\n"

	//positive deltaT means future
	//we assume upper rows are scanned first(past, older) so have negetive deltaT
	"	//row = distortedTexCoord_.x*width;\n"
	"	row = int((1.-distortedTexCoord_.y)*height);\n"

	"	deltaT = -frame_time_offset - td + tr*(0.5-distortedTexCoord_.y) + (row/2*2==row?-ti:0.);\n" // distortedTexCoord increases from down to up and left to right

	//for comparison
	"	//if(texCoord_noLens.x < 0.5) {deltaT = 0.;texCoord_noLens.x+=0.5;}\n"

	"	WCam = vec3(wx, wy, wz);\n" //W of cam/ned in cam axes

	"	VCam2Z = vec3(vx / distance, vy / distance, vz / distance);\n" //Vel of aircraft wrt. ned in camera coordinates

	//Corrected texCoord can be converted to u, v with a linear relationship
	"	h_with_rs = vec2(texCoord_noLens.x*width, (1.-texCoord_noLens.y)*height);\n"
	"	calc_dh2dt(h_with_rs, VCam2Z, WCam, dh2dt0);\n" //calculated at h_with_rs

	"	for(i = 0; i < DIRECT_SAMPLING_POINTS; i++) {\n"
	"		deltaT2 = deltaT;\n"
	"		if(DIRECT_SAMPLING_POINTS > 1) \n"
	"			deltaT2 += te*(-0.5 + float(i)/float(DIRECT_SAMPLING_POINTS-1));\n" //from te*(-0.5)(past) to te*(+0.5)(future)

	//calculating h_no_rs
	// RK2 pixel errors based on degree of predicted rotations
	// every2deg->0.02pix, 6-> 0.01, 12->0.11, 30->1.65
	//experimentally add a step at least every 2.5 degrees of rotation
	"		#define STEP_SIZE 2.5\n"
	"		RK2_STEPS = int(abs(deltaT2*length(WCam))*180./pi/STEP_SIZE) + 1;\n"
	//0.5  1  2  3 4 6 9 12 18 36
	//72  36 18 12 9 6 4 3   2  1
	"		//RK2_STEPS = 72;\n"
	"		deltaT2 /= float(RK2_STEPS);\n" //real delta

	"		for(j=0;j<RK2_STEPS;j++){\n"
	"			if(j==0) {\n"
	"				h1 = h_with_rs;\n"
	"				dh2dt1 = dh2dt0;\n"
	"			} else {\n"
	"				h1 = h2;\n"
	"				calc_dh2dt(h1, VCam2Z, WCam, dh2dt1);\n"
	"			}\n"

	//first assumption (RK1 solution)
	"			h2 = h1 - dh2dt1*deltaT2;\n"
	//recalculation of derivative at second point(1st approximation)
	"			calc_dh2dt(h2, VCam2Z, WCam, dh2dt2);\n"

	//RK2 solution
	"			h2 = h1 - (dh2dt1 + dh2dt2)*0.5*deltaT2;\n"
	"		}\n"

	"		h_no_rs = h2;\n"
	"		texCoord_noLensNoRS[i] = vec2(h_no_rs.x/width, 1.-h_no_rs.y/height);"
	"	}\n"
	"}\n"

	///Motion Blur////////////////////////////////////////////////////////
	//also zooms to center rectangle
	"void blur_distortion(in vec2 texCoord_corrected[DIRECT_SAMPLING_POINTS], in int frame_num, out vec4 texColor) \n"
	"{	\n"
	"	vec2 texCoord3_interpolated, texCoord_realRes;\n"
	"	vec4 color;\n"
	"	int i, j, count, do_break;\n"
	"	texColor = vec4(0., 0., 0., 0.);\n"
	"	count = 0;\n"
	"	do_break = 0;\n"
	"	for(i = 0; i < DIRECT_SAMPLING_POINTS; i++) {\n"
	"		for(j = 0; j < 1+extra_sampling_points; j++) {\n"
	"			count++;\n"
	"			if(i != DIRECT_SAMPLING_POINTS - 1)\n" //not last
	"				texCoord3_interpolated = (texCoord_corrected[i+1].xy*float(j) + texCoord_corrected[i].xy*float(extra_sampling_points + 1 - j))/float(extra_sampling_points + 1);\n"
	"			else \n" //i is the last direct sampling point
	"				texCoord3_interpolated = texCoord_corrected[i].xy;\n"

	//convert to real resolution
	"			texCoord_realRes = (texCoord3_interpolated - vec2(.5, .5)) / extra_margin + vec2(.5, .5);\n" //zoom to needed resolution
	"			if(texCoord_realRes.x <= 1. && texCoord_realRes.x >= 0. && texCoord_realRes.y <= 1. && texCoord_realRes.y >= 0.){\n"
	"				if(frame_num==0)\n"
	"					color = texture2D (baseTexture1, texCoord_realRes.xy);\n" //no space after texture2D will cause warning
	"				else\n"
	"					color = texture2D (baseTexture2, texCoord_realRes.xy);\n" //no space after texture2D will cause warning
	"				if(color.a < 0.5)\n" //SKY
	//"					texColor+=vec4(0.,0.75,1., 1.);\n" //Blue SKY
    //"					texColor+=vec4(0.875,0.875,1., 1.);\n" //White-Gray SKY
    "					texColor+=vec4(0.65,0.85,1., 1.);\n" //White-Gray-Blue SKY
	"				else\n"
	"					texColor+=color;\n"

	"			} else {\n" //out of range
	"				texColor = vec4(0., 0., 0., 0.);\n"
	"				do_break = 1; break;\n"
	"			}\n"
	"			if(i == DIRECT_SAMPLING_POINTS - 1) {do_break = 1; break;}\n" //don't extrapolate after last sampling point
	"		}\n"
	"		if(do_break == 1) break;\n"
	"	}\n"
	"	texColor /= float(count);\n" //sampling = 4, interpolated = 2, + xx + xx + xx +, equals 10 points
	"}\n"
        
    //gives dpeth in Z direction not distance
    "float get_depth(in vec2 distortedTexCoord_, in vec2 texCoord_noLens, in int frame_num) {\n"
	"	int row;\n"
	"	float Z;\n"
	"	vec2 texCoord_realRes;\n"
	"	vec4 depthTex;\n"

	"	row = int((1.-distortedTexCoord_.y)*height);\n"
	"	texCoord_realRes = (texCoord_noLens - vec2(.5, .5)) / extra_margin + vec2(.5, .5);\n" //zoom to needed resolution
	"	if(frame_num == 0)\n"
	"		depthTex = texture2D (depthTexture1, texCoord_realRes);\n" //no space after texture2D will cause warning
	"	else\n"
	"		depthTex = texture2D (depthTexture2, texCoord_realRes);\n" //no space after texture2D will cause warning

	"	Z = depthTex.r*(255.*255.) + depthTex.g*255. + depthTex.b\n;" //verified
    "   return Z;\n"
    "}\n"
    
    "vec4 applyFog( in vec4  rgb,\n"      // original color of the pixel
    "           in float distance,\n" // camera to point distance
    "           in vec3  rayOri,\n"   // camera position
    "           in vec3  rayDir ) {\n"  // camera to point vector
    
    "   float FogStart = 10.;\n"
    "   float FogEnd = 1000.;\n"
    "   //float fogAmount = 0.5; \n"
    "   float fogAmount = clamp((distance - FogStart) / (FogEnd - FogStart), 0., 1.);\n"
    "   //float fogAmount = c*exp(-rayOri.y*b)*(1.0-exp(-distance*rayDir.y*b))/rayDir.y;\n"
    "   vec4  fogColor  = vec4(0.5,0.6,0.7, 1.);\n"
    "   return mix( rgb, fogColor, fogAmount );\n"
    "}\n"

	//Main////////////////////////////////////////////////////////
	"void main() {\n"
	"	float n1,n2,n3,s1,s2, val, err;\n"
    "   float depth1, depth2, depth;"
	"	vec2 texCoord_noLens, texCoord3f1[DIRECT_SAMPLING_POINTS], texCoord3f2[DIRECT_SAMPLING_POINTS], texCoord_realRes, st;\n"
	"	vec2 mean_texCoord, texCoord_realResMean, texCoord_realResMin, texCoord_realResMax;\n"
	"	vec4 texColor, vignetTex, depthTex;\n"

	///Getting uniform inputs
	"	render_depth = inputVect[0];\n"
	"	seed = inputVect[1];\n"
    "	time_s = inputVect[2];\n"
	"	wx = inputVect[3]; wy = inputVect[4]; wz = inputVect[5];\n" //W of cam/ned in cam axes
	"	vx = inputVect[6]; vy = inputVect[7]; vz = inputVect[8];\n" //Vel of aircraft wrt. ned in camera coordinates
	"	k1 = inputVect[11]; k2 = inputVect[12]; t1 = inputVect[13]; t2 = inputVect[14]; k3 = inputVect[15]; k4 = inputVect[16]; k5 = inputVect[17]; k6 = inputVect[18];\n"
	"	vignet_thresh1 = inputVect[19]; vignet_thresh2 = inputVect[20];\n"
	"	td = inputVect[21];\n" //center row delay of camera
	"	tr = inputVect[22];\n" //up2down delay of camera, sign verified
	"	te = inputVect[23];\n" //in-row delay, works when DIRECT_SAMPLING_POINTS > 1
	"	extra_sampling_points = int(inputVect[24]);\n" //points interpolated between two sampling points, more efficient than just using DIRECT_SAMPLING_POINTS
	"	ti = inputVect[25];\n"
	"	width = inputVect[26]; height = inputVect[27];\n" //for normalizing calibration params
	"	fx = inputVect[28]; fy = inputVect[29];\n" //width, height, causes changing t1 = inputVect[10] if used by #define!!!!
	"	ox = inputVect[30]; oy = inputVect[31];\n"
	"	extra_margin = inputVect[32];\n" //extra margin used in ideally rendered image
	"	extra_zoom = inputVect[33];\n" //used so that image get smaller
	"	day_light = inputVect[34]; noise_amplitude_dyn = inputVect[35]; noise_amplitude_st1 = inputVect[36]; noise_amplitude_st2 = inputVect[37];\n"
	"	double_input = int(inputVect[38]);\n"

	///Photometric distortion
	"	vignetTex = texture2D (vignetTexture, distortedTexCoord.xy);\n" //no space after texture2D will cause warning
	"	vignetTex = vec4(vignetTex.x>vignet_thresh2?1.:vignetTex.x, vignetTex.y>vignet_thresh2?1.:vignetTex.y, vignetTex.z>vignet_thresh2?1.:vignetTex.z, 1.);\n"
	"	vignetTex = vec4(vignetTex.x<vignet_thresh1?0.:vignetTex.x, vignetTex.y<vignet_thresh1?0.:vignetTex.y, vignetTex.z<vignet_thresh1?0.:vignetTex.z, 1.);\n"

	"   n1=0.;n2=0.;n3=0.;\n"
	"	if(render_depth < 0.5) {\n" //if rendering color

    "		s1 = rand(fract(seed*0.99));\n"

            //Static noise type1 (pixel wise, gray)
    "		if(noise_amplitude_st1 > 1e-3) {\n"
    "			st = distortedTexCoord.xy;\n"
    "       	st.x+=s1; val = noise_amplitude_st1*randomN(st);\n"
    "			n1 += val; n2 += val; n3 += val;\n" //gray   
    "       }"

            //Static noise type2 (row wise, gray)
    "		if(noise_amplitude_st2 > 1e-3) {\n"
    "			st = vec2(0., distortedTexCoord.y);\n"
    "       	st.y+=s1; val = noise_amplitude_st2*0.707*randomN(st);\n"
    "			n1 += val; n2 += val; n3 += val;\n" //gray   
    "       }\n"

            //Static noise type2 (col wise, gray)
    "		if(noise_amplitude_st2 > 1e-3) {\n"
    "			st = vec2(distortedTexCoord.x, 0.);\n"
    "			st.x+=s1; val = noise_amplitude_st2*0.707*randomN(st);\n"
	"			n1 += val; n2 += val; n3 += val;\n" //gray
    "       }\n"

    "		if(noise_amplitude_dyn > 1e-3) {\n"    
    "			s1 = rand(fract(seed+time_s));\n"
    "			st = distortedTexCoord.xy;\n"
    
                //Uniform noise
	"			//st.x+=s1; n1 += noise_amplitude_dyn*(random(st) - 0.5)*2.;\n" //red
	"			//st.y+=s1; n2 += noise_amplitude_dyn*(random(st) - 0.5)*2.;\n" //green
	"			//st.x-=s1; n3 += noise_amplitude_dyn*(random(st) - 0.5)*2.;\n" //blue

                //Gaussian noise
	"			st.x+=s1; n1 += noise_amplitude_dyn*randomN(st);\n" //red
	"			st.y+=s1; n2 += noise_amplitude_dyn*randomN(st);\n" //green
	"			st.x-=s1; n3 += noise_amplitude_dyn*randomN(st);\n" //blue
    "		}\n"
	"   }\n"

    
    
	//vignet invisible, no need for further processing
	"   if(vignetTex.x < 0.001 && vignetTex.y < 0.001 && vignetTex.z < 0.001) {\n"
	"       mgl_FragColor = vec4(n1, n2, n3, 1.);\n"
	"       return;\n"
	"   }\n"

	///Geometric distortion
	//In fragment shader, movement is done from final pixel coordinates to ideal input pixel coordinates
	//Here we reach from distorted to undistorted coordinates
	"	err = inverse_lens(distortedTexCoord.xy, texCoord_noLens);\n" //classic
	"	//err = inverse_lens2(distortedTexCoord.xy, texCoord_noLens);\n" //gauss-newton
	"	//texCoord_noLens = distortedTexCoord.xy;\n"
	//Pure red as Divergance Warning
	"   if(err > MAX_PIX_ERR) {\n"
	"       mgl_FragColor = vec4(1., 0., 0., 1.);\n"
	"       return;\n"
	"   }\n"

	///Temporal distortion
	"	//texCoord3f1[0] = texCoord_noLens;\n"
	"	if(double_input == 0) {\n"
    "       depth1 = get_depth(distortedTexCoord.xy, texCoord_noLens, 0);\n"
	"		inverse_rolling_shutter(distortedTexCoord.xy, texCoord_noLens, -td-0.5*ti          , depth1, texCoord3f1);\n" //because delay is compensated outside
    "       depth = depth1;\n"
	"	} else {\n"
    "       depth1 = get_depth(distortedTexCoord.xy, texCoord_noLens, 0);\n"
	"		inverse_rolling_shutter(distortedTexCoord.xy, texCoord_noLens, -td-0.5*tr-ti-0.5*te, depth1, texCoord3f1);\n" //because delay is compensated outside
    "       depth2 = get_depth(distortedTexCoord.xy, texCoord_noLens, 1);\n"
	"		inverse_rolling_shutter(distortedTexCoord.xy, texCoord_noLens, -td+0.5*tr-0.+0.5*te, depth2, texCoord3f2);\n" //because delay is compensated outside
    "       depth = (depth1 + depth2)/2.;\n"
	"	}\n"

	"	//if(texCoord_noLens.x > 0.5) {\n" //render depth, not color
	"	if(render_depth > 0.5) {\n" //render depth, not color
	"		mean_texCoord = (texCoord3f1[0] + texCoord3f1[DIRECT_SAMPLING_POINTS-1])/2.;\n"
	"		texCoord_realResMean = (mean_texCoord - vec2(.5, .5)) / extra_margin + vec2(.5, .5);\n"
	"		texCoord_realResMin = (texCoord3f1[0] - vec2(.5, .5)) / extra_margin + vec2(.5, .5);\n"
	"		texCoord_realResMax = (texCoord3f1[DIRECT_SAMPLING_POINTS-1] - vec2(.5, .5)) / extra_margin + vec2(.5, .5);\n"
	"		if(texCoord_realResMin.x > 0. && texCoord_realResMin.x < 1. && texCoord_realResMin.y > 0. && texCoord_realResMin.y < 1. &&  \n"
	"			texCoord_realResMax.x > 0. && texCoord_realResMax.x < 1. && texCoord_realResMax.y > 0. && texCoord_realResMax.y < 1.) {  \n"
	"			depthTex = texture2D (depthTexture1, texCoord_realResMean);\n" //no space after texture2D will cause warning\n"
	"		} else {\n"
	"			depthTex = vec4(1., 0., 0., 1.);\n"
	"		}\n"
	"		mgl_FragColor = depthTex;\n"
	"	} else {\n" //render color

	///Temporal distortion, averaging part
	"		//if(texCoord_noLens.x < 0.5) \n"
	"			blur_distortion(texCoord3f1, 0, texColor);\n"
	"		//else \n"
	"		//	blur_distortion(texCoord3f2, 1, texColor);\n"

    //fog, assuming no change of depth during frame
	"       float u = (texCoord_noLens.x*width - ox)/fx;\n"
	"       float v = ((1.-texCoord_noLens.y)*height - oy)/fy;\n" //y in a picture is defined from up to down
    "       float distance = depth*sqrt(1. + u*u + v*v);\n"
    "       vec3 rayOri = vec3(0., 0., 0.);\n"
    "       vec3 rayDir = vec3(0., 0., 0.);\n"
    "       texColor = applyFog(texColor, distance, rayOri, rayDir);\n"
    
	"		mgl_FragColor = vec4(vignetTex.x*day_light*texColor.r + n1,"
	"								vignetTex.y*day_light*texColor.g + n2,"
	"									vignetTex.z*day_light*texColor.b + n3, 1.);\n"

	"		//if(texCoord_noLens.x < 0.47)\n"
	"		//	mgl_FragColor = texture2D (baseTexture1, texCoord_noLens.xy);\n" //no space after texture2D will cause warning
	"		//else if(texCoord_noLens.x < 0.5)\n"
	"		//	mgl_FragColor = texture2D (depthTexture1, texCoord_noLens.xy);\n" //no space after texture2D will cause warning
	"		//else if(texCoord_noLens.x < 0.53)\n"
	"		//	mgl_FragColor = texture2D (baseTexture2, texCoord_noLens.xy);\n" //no space after texture2D will cause warning
	"		//else if(texCoord_noLens.x < 0.57)\n"
	"		//	mgl_FragColor = texture2D (depthTexture2, texCoord_noLens.xy);\n" //no space after texture2D will cause warning
	"	}\n"
	"}\n";
