#ifndef __NSRTRIAD_H__
#define __NSRTRIAD_H__

#include "nsrMathLib.h"
#include "nsrQuat.h"

#ifdef __cplusplus
extern "C" {
#endif

//Triad//////////////////////////////////////
//ref: https://www.aero.iitb.ac.in/satelliteWiki/index.php/Triad_Algorithm
//input s sensor and m sensor data,
//s is assumed more accurate
//gives CI2B
static Matrice getRotationBy2Vectors(Matrice &si, Matrice &sb, Matrice &mi, Matrice &mb)
{

	Matrice t1b = sb;
	t1b.normalize();

	Matrice t1i = si;
	t1i.normalize();

	Matrice t2b = cross(sb, mb);
	t2b.normalize();

	Matrice t2i = cross(si, mi);
	t2i.normalize();

	Matrice t3b = cross(t2b, t1b);
	Matrice t3i = cross(t2i, t1i);

	//[t1b t2b t3b] = R x [t1i t2i t3i]
	Matrice Rb(3, 3), Ri(3, 3);
	Rb(0, 0) = t1b[0]; Rb(0, 1) = t2b[0]; Rb(0, 2) = t3b[0];
	Rb(1, 0) = t1b[1]; Rb(1, 1) = t2b[1]; Rb(1, 2) = t3b[1];
	Rb(2, 0) = t1b[2]; Rb(2, 1) = t2b[2]; Rb(2, 2) = t3b[2];

	Ri(0, 0) = t1i[0]; Ri(0, 1) = t2i[0]; Ri(0, 2) = t3i[0];
	Ri(1, 0) = t1i[1]; Ri(1, 1) = t2i[1]; Ri(1, 2) = t3i[1];
	Ri(2, 0) = t1i[2]; Ri(2, 1) = t2i[2]; Ri(2, 2) = t3i[2];

	return Rb * Ri.t();
}

static int testTriad()
{
	double time_s;

	Matrice g_i, mm_i, g_b, mm_b;
	Matrice CI2B;

	time_s = myTime();

	//LOGI("","In The Name Of Allah \n\n");

	nsr::Quat coordX(nsr::inDegrees(10), nsr::inDegrees(20), nsr::inDegrees(30));

	g_i = colmat(0, 0, 9.81);
	mm_i = colmat(100, 15, 100);

	coordX.getRotMat(CI2B);
	CI2B.print("CI2B\n");

	g_b = CI2B * g_i;
	g_b.print("g_b:\n");

	mm_b = CI2B * mm_i;
	mm_b.print("mm_b:\n");

	Matrice R = getRotationBy2Vectors(g_i, g_b, mm_i, mm_b);
	R.print("R:\n");

	return 0;
}

#ifdef __cplusplus
}
#endif

#endif