/**
 * @file
 * این فایل اعداد ثابتی مثل مکان داده ها را در بر دارد. معادل این کلاس
 * در زبان جاوا هم وجود دارد که باید با هم مطابق باشند
 * <p>
 * فراموشی در معادل سازی این دو کد یکی از مهم ترین دلایل خطاهای عجیب است
 *
 * @see com.nasir.NsrIndex
 * @see nsrNumBuffer.c
 * @since 1390
 */

#include "./nsrIndex.h"
#include <string.h> //for malloc android
#include <stdlib.h> // malloc, free in linux

#include "./nsrStrUtility.h" //for MAX_PATH_LENGTH
#include "./nsrDataDef.h"

#undef TAG
#define TAG " C:Index:"

#ifdef __cplusplus
extern "C" {
#endif

DataIndex n;

int createBuf(int* elem, int width)
{
	static int next_elem = 1;
	*elem = next_elem;
	next_elem = *elem + width; //next_elem
	return 0;
}
	
void SetIndexes()
{
	int i;

	//Memorial: Just "ya Hosein" saved you as you forgot to set Total equal to what it is in java part!!!
	n.Z.Total = 36 + 14; //Size Of SensorsVector
	n.Z.Depth = 5000;    //SensorsBuffer saves SensDepth * SensorsVector
	//Depth == 2 needed for trapezoidal gyro, acc integration
	n.Z.Readers = 3; //first is for datalog, second for ordinary usage

	//Sensor vector
	createBuf(&n.Z.Acc, 3); 		   //3*Accelerometer(m/s2)(shows:F_contact/m = F_total/m - g)
	createBuf(&n.Z.Gyro, 3); 	   //3*Gyroscope(rad/s)
	createBuf(&n.Z.MMM, 4);         //(3+1)*Magnetometer:{x,y,z norm, total(nT)}
	createBuf(&n.Z.MMR, 4);         //(3+1)*Magnetic reference:{x,y,z norm in ref frame, total(nT)}
	createBuf(&n.Z.Eu, 3);          //3*ZYX Eulers(X:phai:roll, Y:theta:pitch, Z:psi:yaw) in rad
	createBuf(&n.Z.GPS_LLA, 4); //(4)*GPS_LLA:{Lat(deg),lon(deg), alt(m), alt_ref(ALT_REF enum)}
	createBuf(&n.Z.GPS_V, 3);     //(3)*GPS_V:{direction(deg,CW of North),speed(m/s),dalt(m/s)}
	createBuf(&n.Z.GPS_DOP, 3); //(3)*GPS_DOP:{HDOP(m),VDOP(m),PDOP(m)}
	createBuf(&n.Z.GPS_Other, 2); //(2)*GPS_Other:{epoch(ms since epoch),satinuse}
	createBuf(&n.Z.Prs, 1);         //1*Pressures(pa)
	createBuf(&n.Z.PrsAlt, 1);   //1*Pressures alt(m)
	createBuf(&n.Z.QuExt, 4);     //4*Orientation(in Quaternions): {et,e1,e2,e3}
	createBuf(&n.Z.RATE, 1);	   //1*RATE:Render Fps
	//total: 36
	
	//Filter
	createBuf(&n.Z.WExt, 3); //3*External W(Table)
	createBuf(&n.Z.VExt, 3); //3*External V(Table)
	createBuf(&n.Z.PrsAcc, 1); //1*Pressure
	createBuf(&n.Z.Time, 1); //1*Last measured Time
	createBuf(&n.Z.GROUND_TRUTH_LLA, 3); //(3)*GROUND_TRUTH_LLA:{Lat(deg),lon(deg), alt(m)}
	createBuf(&n.Z.GROUND_TRUTH_EU, 3); //3*ZYX Eulers(X:phai:roll, Y:theta:pitch, Z:psi:yaw) in rad
	//total: 14
}
	
#ifdef __cplusplus
}
#endif
