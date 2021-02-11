
/* ALLAH
 * //Why quaternions///////////////////////////////
 * Not euler angles, they can get locks
 * Not rotation matrices, they are hard to keep normalized
 * Not axis-angle, hard to convert to rotation matrices
 * But quaternions, easy to convert to rotation matrices(SO(3)), axis-angle & small rotations(so(3))
 *
 * //Quaternion definition://////////////////////
 * Titterton & simulink: [cos(mu/2) l*sin(mu/2) m*sin(mu/2) n*sin(mu/2)]
 * here, osg & clementon msckf impl.:[l*sin(mu/2) m*sin(mu/2) n*sin(mu/2) cos(mu/2)]
 *
 * //Quat2vector mult. definition:///////////////
 * quaternion imaginary math:
 * a vector is a quaternion with zero scalar number(0 + r1*i + r2*j + r3*k)
 * CB2I <-> q ** r := q * r * q.conj()
 * CI2B <-> q ** r := q.conj() * r * q
 *
 * 1) QUAT_IS_CI2B
 * - matlab quaternion rotation(aerospace blockset)  //verified
 * - matlab quaternion to DCM(aerospace blockset)  //verified
 * - recommended
 *
 * 2) QUAT_IS_CB2I
 * - titterton //verified
 * - osg //verified
 *
 * //Quat2Quat mult. definition://///////////////
 * 1) QUAT_IS_CI2B
 * - osg //verified
 * - recommended as complies with CI2B rotation
 *
 * 2) QUAT_IS_CB2I
 * Quaternion imaginary math(et,e1,e2,e3) = (et+e1*i+e2*j+e3*k)
 * It is reverse of CI2B(p.q instead of q.p )
 * - titterton //verified
 * - wikipedia(hamilton product) //verified
 * - matlab aerospace blockset(et,e1,e2,e3) //verified
 * - clementon msckf(e1,e2,e3,et) matlab code //verified
 *
 *   qt = et*pt -e1*p1 -e2*p2 -e3*p3;
 *   q1 = e1*pt  et*p1 -e3*p2  e2*p3;
 *   q2 = e2*pt  e3*p1  et*p2 -e1*p3;
 *   q3 = e3*pt -e2*p1  e1*p2  et*p3;
 * so:
 *   q1 = +et*p1 -e3*p2 +e2*p3 +e1*pt;
 *   q2 = +e3*p1 +et*p2 -e1*p3 +e2*pt;
 *   q3 = -e2*p1 +e1*p2 +et*p3 +e3*pt;
 *   qt = -e1*p1 -e2*p2 -e3*p3 +et*pt;
 *
 * //OSG Bug/////////////////////////////////////
 * open scene graph defines quaternion as CB2I in quat2vec mult.
 * but as CI2B in quat to quat mult!!!
 * so (q3*q2*q1)*B != q3*(q2*(q1*B))
 *
 * //Titterton Bug/////////////////////////////////////
 * Qu2Eu & Eu2Qu formulas are wrong within a few degrees
 */

#define QUAT_IS_CI2B
//#define QUAT_IS_CB2I

#ifndef __NSR_QUAT_H__
#define __NSR_QUAT_H__

#include "./nsrMathLib.h"
#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif

namespace nsr
{

	enum Axis {X_AXIS, Y_AXIS, Z_AXIS};

	/** A quaternion class. It can be used to represent an orientation in 3D space.*/
	class Quat
	{
	public:

		Tfloat e1, e2, e3, et; //main storage

		void init()
		{

		}

		inline Quat()
		{
			init();
			e1 = 0.0;
			e2 = 0.0;
			e3 = 0.0;
			et = 1.0;
		}

		inline Quat(Tfloat x, Tfloat y, Tfloat z, Tfloat w)
		{
			init();
			e1 = x; e2 = y; e3 = z;
			et = w;
		}

		//By 321 euler angles
		inline Quat(Tfloat phi, Tfloat theta, Tfloat psi)
		{
			init();
			this->setEu(phi, theta, psi);
		}

		//By axis-angle
		inline Quat(Tfloat angle, Axis axis)
		{
			init();
			this->setAxisAngle(angle, axis);
		}

		inline Quat(const Matrice &v)
		{
			init();
			this->set(v);
		}

		/* ----------------------------------
		   Methods to access data members
		---------------------------------- */

		inline void set(Tfloat x, Tfloat y, Tfloat z, Tfloat w)
		{
			e1 = x; e2 = y; e3 = z;
			et = w;
		}

		inline void normalize()
		{
			Tfloat normSize = sqrt(et * et + e1 * e1 + e2 * e2 + e3 * e3);
			et = et / normSize;
			e1 = e1 / normSize; e2 = e2 / normSize; e3 = e3 / normSize;
		}

		//Eulers in 321 order
		inline void setEu(Tfloat phi, Tfloat theta, Tfloat psi)
		{
			double SP2, ST2, SS2, CP2, CT2, CS2, NormSize;
			//Matlab(Simulink has et,e1,e2,e3 & psi,theta,phai order(not phai,theta,psi))
			SP2 = sin(phi / 2);
			ST2 = sin(theta / 2);
			SS2 = sin(psi / 2);
			CP2 = cos(phi / 2);
			CT2 = cos(theta / 2);
			CS2 = cos(psi / 2);

			et = CS2 * CT2 * CP2 + SS2 * ST2 * SP2;
			e1 = CS2 * CT2 * SP2 - SS2 * ST2 * CP2;
			e2 = CS2 * ST2 * CP2 + SS2 * CT2 * SP2;
			e3 = SS2 * CT2 * CP2 - CS2 * ST2 * SP2;

			//Correct for numerical errors
			normalize();
		}

		inline void setAxisAngle(Tfloat angle, int axis)
		{
			double l, m, n,
				   S, C;

			S = sin(angle / 2.);
			C = cos(angle / 2.);

			if(axis == X_AXIS) {
				l = 1; m = 0; n = 0;
			}
			if(axis == Y_AXIS) {
				l = 0; m = 1; n = 0;
			}
			if(axis == Z_AXIS) {
				l = 0; m = 0; n = 1;
			}

			set(l * S, m * S, n * S, C);
		}

		//Axis angle ~= so(3) (real so(3) is 3x3 cross mat of axis angle)
		//R = exp(so(3))
		//input 3x1 Matrice
		inline void setAxisAngle(/*const*/ Matrice &rod)
		{
			double l, m, n,
				   amp, S, C;

			assert(rod.rows() == 3 && rod.cols() == 1);

			l = rod.x(); m = rod.y(); n = rod.z();

			amp = sqrt(l * l + m * m + n * n);
			if(amp < 1e-10) {
				set(0., 0., 0., 1.);
				return;
			}

			l /= amp; m /= amp; n /= amp;

			S = sin(amp / 2.);
			C = cos(amp / 2.);

			set(l * S, m * S, n * S, C);
		}

		inline void setAxisAngle(double l, double m, double n)
		{
			double amp, S, C;

			amp = sqrt(l * l + m * m + n * n);
			if(amp < 1e-10) {
				set(0., 0., 0., 1.);
				return;
			}

			l /= amp; m /= amp; n /= amp;

			S = sin(amp / 2.);
			C = cos(amp / 2.);

			set(l * S, m * S, n * S, C);
		}

		inline int set(const Matrice &v)
		{
			assert(v.rows() == 4 && v.cols() == 1);

			set(v.x(), v.y(), v.z(), v.w());
			return 0;
		}

		//Rot Mat input
		//equals to SO(3)
		inline int setRotMat(const Matrice &v)
		{
			assert(v.rows() == 3 && v.cols() == 3);

			double x, y, z, w,
				   Rtxx, Rtxy, Rtxz,
				   Rtyx, Rtyy, Rtyz,
				   Rtzx, Rtzy, Rtzz;

			Rtxx = v.elemc(0, 0); Rtxy = v.elemc(1, 0); Rtxz = v.elemc(2, 0);
			Rtyx = v.elemc(0, 1); Rtyy = v.elemc(1, 1); Rtyz = v.elemc(2, 1);
			Rtzx = v.elemc(0, 2); Rtzy = v.elemc(1, 2); Rtzz = v.elemc(2, 2);

			//w = sqrt( trace( R ) + 1 ) / 2;
			w = sqrt(Rtxx + Rtyy + Rtzz + 1) / 2;

			// check if w is real. Otherwise, zero it.
			//if( imag( w ) > 0 ) {
			//	 w = 0;
			//}

			x = sqrt(1 + Rtxx - Rtyy - Rtzz) / 2;
			y = sqrt(1 + Rtyy - Rtxx - Rtzz) / 2;
			z = sqrt(1 + Rtzz - Rtyy - Rtxx) / 2;

			if(w >= x && w >= y && w >= z) {  //w is max
				x = (Rtzy - Rtyz) / (4 * w);
				y = (Rtxz - Rtzx) / (4 * w);
				z = (Rtyx - Rtxy) / (4 * w);
			} else if(x >= w && x >= y && x >= z) {  //x is max
				w = (Rtzy - Rtyz) / (4 * x);
				y = (Rtxy + Rtyx) / (4 * x);
				z = (Rtzx + Rtxz) / (4 * x);
			} else if(y >= w && y >= x && y >= z) {  //y is max
				w = (Rtxz - Rtzx) / (4 * y);
				x = (Rtxy + Rtyx) / (4 * y);
				z = (Rtyz + Rtzy) / (4 * y);
			} else {
				//if( z >= w && z >= x && z >= y ) {//z is max
				w = (Rtyx - Rtxy) / (4 * z);
				x = (Rtzx + Rtxz) / (4 * z);
				y = (Rtyz + Rtzy) / (4 * z);
			}

			set(x, y, z, w);

			return 0;
		}

		//equals to SO(3)
		void getRotMat(Matrice &CI2B) const
		{
			double et_2, e1_2, e2_2, e3_2;
			CI2B.reinit(3, 3);

			e1_2 = e1 * e1; e2_2 = e2 * e2; e3_2 = e3 * e3;
			et_2 = et * et;

			CI2B.fill2(
				+e1_2 - e2_2 - e3_2 + et_2, +2 * e1 * e2 + 2 * e3 * et, +2 * e1 * e3 - 2 * e2 * et, //Verified
				+2 * e1 * e2 - 2 * e3 * et, -e1_2 + e2_2 - e3_2 + et_2, +2 * e2 * e3 + 2 * e1 * et,
				+2 * e1 * e3 + 2 * e2 * et, +2 * e2 * e3 - 2 * e1 * et, -e1_2 - e2_2 + e3_2 + et_2);
		}

		//Eulers in 321 order
		//Take care with Matlab's formulation(Simulink has psi,theta,phai order)
		void getEu(Tfloat &phi, Tfloat &theta, Tfloat &psi) const
		{
			//Don't Use atan az it may miss sign
			//Don't use Tittertons formulation as it causes errors
			phi = atan2(2 * (e2 * e3 + et * e1), et * et - e1 * e1 - e2 * e2 + e3 * e3);
			theta = asin(-2 * (e1 * e3 - et * e2));
			psi = atan2(2 * (e1 * e2 + et * e3), et * et + e1 * e1 - e2 * e2 - e3 * e3);
		}

		//Always >=0
		Tfloat getAngle() const
		{
			//simplified from acos((trace(R)-1)/2)
			return acos(-e1 * e1 - e2 * e2 - e3 * e3 + et * et);
		}

		//Axis angle ~= so(3) (real so(3) is 3x3 cross mat of axis angle)
		//so(3) == log(R)
		Tfloat getAxisAngle(Matrice &v) const
		{
			Tfloat th, d;
			v.reinit(3, 1);
			th = getAngle();

			if(fabs(th) > 1e-6)
				d = 2.*et * th / sin(th);
			else
				d = 2.*et;

			v.fill2(d * e1, d * e2, d * e3);
            return th;
		}

		//Operations/////////////////////////
		/// Multiply by scalar
		inline const Quat operator * (Tfloat rhs) const
		{
			return Quat(e1 * rhs, e2 * rhs, e3 * rhs, et * rhs);
		}

		/// Unary multiply by scalar
		inline Quat &operator *= (Tfloat rhs)
		{
			e1 *= rhs;
			e2 *= rhs;
			e3 *= rhs;
			et *= rhs;
			return *this;        // enable nesting
		}

		inline Quat operator+(const Quat &rhs) const
		{
			return Quat(+ e1 + rhs.e1,
						+ e2 * rhs.e2,
						+ e3 * rhs.e3,
						+ et * rhs.et);
		}

		/// Binary multiply
		inline Quat operator*(const Quat &rhs) const
		{
#ifdef QUAT_IS_CI2B
			//osg implementation
			return Quat(+ et * rhs.e1 + e3 * rhs.e2 - e2 * rhs.e3 + e1 * rhs.et,
						- e3 * rhs.e1 + et * rhs.e2 + e1 * rhs.e3 + e2 * rhs.et,
						+ e2 * rhs.e1 - e1 * rhs.e2 + et * rhs.e3 + e3 * rhs.et,
						- e1 * rhs.e1 - e2 * rhs.e2 - e3 * rhs.e3 + et * rhs.et);
#endif
#ifdef QUAT_IS_CB2I
			return Quat(+ et * rhs.e1 - e3 * rhs.e2 + e2 * rhs.e3 + e1 * rhs.et,
						+ e3 * rhs.e1 + et * rhs.e2 - e1 * rhs.e3 + e2 * rhs.et,
						- e2 * rhs.e1 + e1 * rhs.e2 + et * rhs.e3 + e3 * rhs.et,
						- e1 * rhs.e1 - e2 * rhs.e2 - e3 * rhs.e3 + et * rhs.et);
#endif

		}

		/// Unary multiply
		inline Quat &operator*=(const Quat &rhs)
		{
#ifdef QUAT_IS_CI2B
			//osg implementation
			Tfloat x = + et * rhs.e1 + e3 * rhs.e2 - e2 * rhs.e3 + e1 * rhs.et;
			Tfloat y = - e3 * rhs.e1 + et * rhs.e2 + e1 * rhs.e3 + e2 * rhs.et ;
			Tfloat z = + e2 * rhs.e1 - e1 * rhs.e2 + et * rhs.e3 + e3 * rhs.et;
#endif

#ifdef QUAT_IS_CB2I
			Tfloat x = + et * rhs.e1 - e3 * rhs.e2 + e2 * rhs.e3 + e1 * rhs.et;
			Tfloat y = + e3 * rhs.e1 + et * rhs.e2 - e1 * rhs.e3 + e2 * rhs.et;
			Tfloat z = - e2 * rhs.e1 + e1 * rhs.e2 + et * rhs.e3 + e3 * rhs.et;
#endif

			e1 = x; e2 = y; e3 = z;
			et = et * rhs.et - e1 * rhs.e1 - e2 * rhs.e2 - e3 * rhs.e3;

			return (*this);            // enable nesting
		}

		/// Binary divide
		inline const Quat operator/(const Quat &denom) const
		{
			return ((*this) * denom.inverse());
		}

		/// Unary divide
		inline Quat &operator/=(const Quat &denom)
		{
			(*this) = (*this) * denom.inverse();
			return (*this);            // enable nesting
		}

		/// Divide by scalar
		inline Quat operator / (Tfloat rhs) const
		{
			Tfloat div = 1.0 / rhs;
			return Quat(e1 * div, e2 * div, e3 * div, et * div);
		}

		/// Unary divide by scalar
		inline Quat &operator /= (Tfloat rhs)
		{
			Tfloat div = 1.0 / rhs;
			e1 *= div;
			e2 *= div;
			e3 *= div;
			et *= div;
			return *this;
		}

		/** Rotate a vector by this quaternion.*/

		inline Matrice operator* (/*const*/ Matrice v)    /*const*/
		{
			Matrice res(3, 1);
			double et_2, e1_2, e2_2, e3_2;

			assert(v.rows() == 3 && v.cols() == 1);

			e1_2 = e1 * e1; e2_2 = e2 * e2; e3_2 = e3 * e3;
			et_2 = et * et;

#ifdef QUAT_IS_CI2B
			res[0] = (+e1_2 - e2_2 - e3_2 + et_2) * v[0] + (+2 * e1 * e2     +     2 * e3 * et) * v[1] + (+2 * e1 * e3     -     2 * e2 * et) * v[2];
			res[1] = (+2 * e1 * e2     -     2 * e3 * et) * v[0] + (-e1_2 + e2_2 - e3_2 + et_2) * v[1] + (+2 * e2 * e3     +     2 * e1 * et) * v[2];
			res[2] = (+2 * e1 * e3     +     2 * e2 * et) * v[0] + (+2 * e2 * e3     -     2 * e1 * et) * v[1] + (-e1_2 - e2_2 + e3_2 + et_2) * v[2];
#endif
#ifdef QUAT_IS_CB2I
			//osg implementation
			res[0] = (+e1_2 - e2_2 - e3_2 + et_2) * v[0] + (+2 * e1 * e2     -     2 * e3 * et) * v[1] + (+2 * e1 * e3     +     2 * e2 * et) * v[2];
			res[1] = (+2 * e1 * e2     +     2 * e3 * et) * v[0] + (-e1_2 + e2_2 - e3_2 + et_2) * v[1] + (+2 * e2 * e3     -     2 * e1 * et) * v[2];
			res[2] = (+2 * e1 * e3     -     2 * e2 * et) * v[0] + (+2 * e2 * e3     +     2 * e1 * et) * v[1] + (-e1_2 - e2_2 + e3_2 + et_2) * v[2];
#endif
			return res;
		}

		/// Conjugate
		inline Quat conj() const
		{
			return Quat(-e1, -e2, -e3, et);
		}

		Tfloat norm2() const
		{
			return e1 * e1 + e2 * e2 + e3 * e3 + et * et;
		}

		/// Multiplicative inverse method: q^(-1) = q^*/(q.q^*)
		inline const Quat inverse() const
		{
			return conj() / norm2();
		}

#if 0
		void slerp(Tfloat t, const Quat &from, const Quat &to)
		{
			const double epsilon = 0.00001;
			double omega, cosomega, sinomega, scale_from, scale_to ;

			nsr::Quat quatTo(to);
			// this is a dot product

			cosomega = from.asVec4() * to.asVec4();

			if(cosomega < 0.0) {
				cosomega = -cosomega;
				quatTo = -to;
			}

			if((1.0 - cosomega) > epsilon) {
				omega = acos(cosomega) ; // 0 <= omega <= Pi (see man acos)
				sinomega = sin(omega) ;  // this sinomega should always be +ve so
				// could try sinomega=sqrt(1-cosomega*cosomega) to avoid a sin()?
				scale_from = sin((1.0 - t) * omega) / sinomega ;
				scale_to = sin(t * omega) / sinomega ;
			} else {
				/* --------------------------------------------------
				The ends of the vectors are very close
				we can use simple linear interpolation - no need
				to worry about the "spherical" interpolation
				-------------------------------------------------- */
				scale_from = 1.0 - t ;
				scale_to = t ;
			}

			*this = (from * scale_from) + (quatTo * scale_to);

			// so that we get a Vec4
		}
#endif

		inline void printEu(const char* name = NULL)
		{
			double phi, theta, psi;
			getEu(phi, theta, psi);
			if(name == NULL) printf("euler: %f, %f, %f\n", phi * 180.0 / M_PI, theta * 180.0 / M_PI, psi * 180.0 / M_PI);
			else printf("%s euler: %f, %f, %f\n", name, phi * 180.0 / M_PI, theta * 180.0 / M_PI, psi * 180.0 / M_PI);
		}

		inline void printQu(const char* name = NULL)
		{
			if(name == NULL) printf("quat: %f, %f, %f, %f\n", e1, e2, e3, et);
			else printf("%s quat: %f, %f, %f, %f\n", name, e1, e2, e3, et);
		}

	};    // end of class prototype

	inline Tfloat inDegrees(Tfloat angle)
	{
		return angle * M_PI / 180.0;
	}

}    // end of namespace

#ifdef __cplusplus
}
#endif

#endif
