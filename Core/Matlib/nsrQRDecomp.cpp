/**
 * QR decomposition:
 * A = QR
 * Q'Q = I, Q is orthogonal
 * R is upper triangular
 *
 * QR decomposition is a fast way for solving linear system of equations:
 * AX=B
 * QRX=B
 * RX=Q'B
 * RX=B2 is easy to solve by back substitution
 *
 *
 */

#include "./nsrMathLib.h"
#include "./nsrQRDecomp.h"
//#include "nsrPlatform.h"
#include <assert.h>

#undef TAG
#define TAG "Cpp:QRDecomp:"

#ifdef __cplusplus
extern "C" {
#endif

#define EPS 1e-9

double column_length(Matrice &m, int column);
void matrix_column_copy(Matrice &mdst, int col2, Matrice &msrc, int col1);
void matrix_column_subtract(Matrice &m1, int col1, Matrice &m2, int col2);
void matrix_column_multiply(Matrice &m, int c, Tfloat k);

void givensrotation(Tfloat a, Tfloat b, Tfloat &c, Tfloat &s);

//Gram-Schmit///////////////////////////////////////////////////////////////////////////////////////////////////////
//verified
//less stable
//A is mxn m>=n
//O(mn2) complexity
void QRdecompGS(Matrice &A, Matrice &Q, Matrice &R)
{
	Tfloat r;
	int i, j, k;

	Q.reinit(A.rows(), A.rows());
	R.reinit(A.rows(), A.cols());

	// Using the Gram-Schmidt process
	// Temporary vector T and S used in calculations
	Matrice T(A.rows(), 1); ////////////////////////////////////////////////////////////////////////////////////////
	Matrice S(A.rows(), 1); ////////////////////////////////////////////////////////////////////////////////////////

	for(i = 0; i < A.cols(); i++) {
		//printf("i:%i\n", i); fflush(stdout);
		//Qi = Ui
		matrix_column_copy(Q, i, A, i);
		//printf("a\n"); fflush(stdout);

		for(j = 0; j < i; j++) {
			//r[j,i] = Qj^T * Ui
			matrix_column_copy(T, 0, Q, j);
			matrix_column_copy(S, 0, A, i);
			r = 0;
			for(k = 0; k < A.rows(); k++)
				r += T.elem(k, 0) * S.elem(k, 0);
			R.elem(j, i) = r;
			matrix_column_multiply(T, 0, r);
			matrix_column_subtract(Q, i, T, 0);
		}

		//r[i,i] = ||Qi||
		R.elem(i, i) = column_length(Q, i);
		//Qi = Qi/r[i,i]

		matrix_column_multiply(Q, i, 1. / R.elem(i, i));
	}
}

//HouseHolder method ////////////////////////////////////////////////////////////////////////////////////////
//verified
//A is mxn m>=n or m<n, both possible
//O(mn2) complexity
//complexer than GS method, a little more class functions calls and mallocs
//A:mxn, Q:mxm, R:mxn
void QRdecompHH(Matrice &A, Matrice &Q, Matrice &R)
{
	Tfloat nrm, sgn;
	int k, j, m, n;
	m = A.rows();
	n = A.cols();

	Q.reinit(A.rows(), A.rows());
	R.reinit(A.rows(), A.cols());

	Matrice v(m, 1); ////////////////////////////////////////////////////////////////////////////////////////
	Matrice u(n, 1); ////////////////////////////////////////////////////////////////////////////////////////

	R = A; //Start with R=A
	Q.fillEye(); //Q=eye(m); //Set Q as the identity matrix

	for(k = 0; k < n; k++) {
		for(j = 0; j <= k - 1; j++) v.elem(j, 0) = 0.;
		for(j = k; j <= m - 1; j++) v.elem(j, 0) = R.elem(j, k);

		sgn = v.elem(k, 0);
		sgn = sgn >= 0 ? (sgn > 0 ? 1 : 0) : -1;
		nrm = v.norm();
		v.elem(k, 0) += sgn * nrm;
		//Orthogonal transformation matrix that eliminates one element
		//below the diagonal of the matrix it is post-multiplying:

		if(!v.normalize()) continue;

		u = R.transpose() * v * 2.; // (mxn)' x (mx1),     %u = 2*R'*v; %pre calculating u, causes better numerical accuracy
		R = R - v * u.transpose(); //Product HR
		Q = Q - Q * v * v.transpose() * 2.; //Product QR
	}
}

//Specialized for filters
//A:mxn, Q:mxn, R:nxn
void QRdecompHH_JustR_Economy(Matrice &A, Matrice &Rout)
{
	Tfloat nrm, sgn;
	int k, j, m, n;
	m = A.rows();
	n = A.cols();

	Rout.reinit(A.rows(), A.cols());

	Matrice v(m, 1); ////////////////////////////////////////////////////////////////////////////////////////
	Matrice u(n, 1); ////////////////////////////////////////////////////////////////////////////////////////
	Matrice R(m, n); ////////////////////////////////////////////////////////////////////////////////////////

	R = A; //Start with R=A

	for(k = 0; k < n; k++) {
		for(j = 0; j <= k - 1; j++) v.elem(j, 0) = 0.;
		for(j = k; j <= m - 1; j++) v.elem(j, 0) = R.elem(j, k);

		sgn = v.elem(k, 0);
		sgn = sgn >= 0 ? (sgn > 0 ? 1 : 0) : -1;
		nrm = v.norm();
		v.elem(k, 0) += sgn * nrm;
		//Orthogonal transformation matrix that eliminates one element
		//below the diagonal of the matrix it is post-multiplying:

		if(!v.normalize()) continue;

		u = R.transpose() * v * 2.; // (mxn)' x (mx1),     %u = 2*R'*v; %pre calculating u, causes better numerical accuracy
		R = R - v * u.transpose(); //Product HR
	}

	Rout.fill3(0, n - 1, 0, n - 1,
			   R, 0, n - 1, 0, n - 1);
}

/* Returns the length of the vector column in m */
double column_length(Matrice &m, int column)
{
	int row, len;
	double length = 0;
	len = m.rows();
	for(row = 0; row < len; row++)
		length += m.elem(row, column) * m.elem(row, column);
	return sqrt(length);
}

/* Copies a matrix column from msrc at column col1 to mdst at column col2 */
void matrix_column_copy(Matrice &mdst, int col2, Matrice &msrc, int col1)
{
	int row, len;
	len = msrc.rows();
	for(row = 0; row < len; row++)
		mdst.elem(row, col2) = msrc.elem(row, col1);
}

/* Subtracts m2's column col2 from m1's column col1 */
void matrix_column_subtract(Matrice &m1, int col1, Matrice &m2, int col2)
{
	int row, len;
	len = m1.rows();
	for(row = 0; row < len; row++)
		m1.elem(row, col1) -= m2.elem(row, col2);
}

/* Multiplies the matrix column c in m by k */
void matrix_column_multiply(Matrice &m, int c, Tfloat k)
{
	int row, len;
	len = m.rows();
	for(row = 0; row < len; row++)
		m.elem(row, c) *= k;
}

//Givens Method////////////////////////////////////////////////////////////////
//verified
//O(mn2) complexity
//Givens method is little slower than house holder method(2/3)
//but performs faster with sparse matrices
//also needs more memory than House holder
//Parallelizable easier
//A:mxn, Q:mxm, R:mxn
//Ref: Nmerical Methods For Computational Science and Engineering, p.252

/*
verified matlab code:
% qrgivens.m
function [Q,R] = qrgivens(A)
  [m,n] = size(A);
  Q = eye(m);
  R = A;

  for j = 1:n
    for i = m:-1:(j+1)
      G = eye(m);
      [c,s] = givensrotation( R(i-1,j),R(i,j) );
      G([i-1, i],[i-1, i]) = [c -s; s c];
      R = G'*R;
      Q = Q*G;
    end
  end
end

% givensrotation.m
function [c,s] = givensrotation(a,b)
  if b == 0
    c = 1;
    s = 0;
  else
    if abs(b) > abs(a)
      r = a / b;
      s = 1 / sqrt(1 + r^2);
      c = s*r;
    else
      r = b / a;
      c = 1 / sqrt(1 + r^2);
      s = c*r;
    end
  end
end
*/

void QRdecompGiv(Matrice &A, Matrice &Q, Matrice &R)
{
	int i, j, k;
	int rows, cols;
	register Tfloat tmp1, tmp2, c, s;

	//Matrice G(2,2), Q_, R_;
	rows = A.rows();
	cols = A.cols();

	Q.reinit(rows, rows);
	Q.fillEye();

	R = A;

	for(i = 0; i < cols; i++) {
		for(j = rows - 1; j > i; j--) {
			givensrotation(R.elem(j - 1, i), R.elem(j, i), c, s);

			//G.fill2(c, s,
			//       -s, c);

			//problem in sign
			//R_ << R.subsr(j-1, j, 0, cols-1);
			//R_ = G*R_;
			for(k = 0; k < cols; k++) {
				tmp1 = c * R.elemc(j - 1, k) + s * R.elemc(j, k); //j-1 row
				tmp2 = -s * R.elemc(j - 1, k) + c * R.elemc(j, k); //j   row
				R.elem(j - 1, k) = tmp1; //j-1 row
				R.elem(j, k) = tmp2;  //j   row
			}

			//Q_ << Q.subsr(0, rows-1, j-1, j);
			//Q_ = Q_*G.transpose();
			for(k = 0; k < rows; k++) {
				tmp1 = c * Q.elemc(k, j - 1) + s * Q.elemc(k, j); //j-1 row
				tmp2 = -s * Q.elemc(k, j - 1) + c * Q.elemc(k, j); //j   row
				Q.elem(k, j - 1) = tmp1; //j-1 row
				Q.elem(k, j) = tmp2;  //j   row
			}
		}
	}
}

void QRdecompGiv_JustR_Economy(Matrice &A, Matrice &R)
{
	int i, j, k;
	int rows, cols;
	register Tfloat tmp1, tmp2, c, s;

	//Matrice G(2,2), Q_, R_;
	rows = A.rows();
	cols = A.cols();

	R = A;

	for(i = 0; i < cols; i++) {
		for(j = rows - 1; j > i; j--) {
			givensrotation(R.elem(j - 1, i), R.elem(j, i), c, s);

			//G.fill2(c, s,
			//       -s, c);

			//problem in sign
			//R_ << R.subsr(j-1, j, 0, cols-1);
			//R_ = G*R_;
			for(k = 0; k < cols; k++) {
				tmp1 = c * R.elemc(j - 1, k) + s * R.elemc(j, k); //j-1 row
				tmp2 = -s * R.elemc(j - 1, k) + c * R.elemc(j, k); //j   row
				R.elem(j - 1, k) = tmp1; //j-1 row
				R.elem(j, k) = tmp2;  //j   row
			}
		}
	}
}

// Form 2 × 2 Givens rotation matreix
//G << c, s;
//    −s, c;
void givensrotation(Tfloat a, Tfloat b, Tfloat &c, Tfloat &s)
{
	Tfloat t;
	if(equals(b, 0., EPS) == 1) { //element already zero
		c = 1;
		s = 0;
	} else {
		if(fabs(b) > fabs(a)) {
			t = a / b;
			s = 1. / sqrt(1. + t * t);
			c = s * t;
		} else {
			t = b / a;
			c = 1. / sqrt(1. + t * t);
			s = c * t;
		}
	}
}

//Solve//////////////////////////////////////////////////////////////////////////////
//verified
void solveQR(Matrice &A, Matrice &B, Matrice &x)
{
	register Tfloat tmp;
	Matrice Q, R, B2;

	//QRdecompHH(A, Q, R);
	QRdecompGiv(A, Q, R);

	//RX=Q'B=B2
	//B2 = Q.transpose()*B;
	B2 = (B.transpose() * Q).transpose(); //more efficient

	int n = B.rows();
	x.reinit(n, 1);
	x = 0.;

	//Solve Upper triangular equation system with back substitution
	int j, k;
	for(k = n - 1; k >= 0; k--) {
		tmp = 0;
		for(j = k + 1; j < n; j++)
			tmp += R.elemc(k, j) * x.elemc(j, 0);
		x.elem(k, 0) = (B2.elemc(k, 0) - tmp) / R.elemc(k, k);
	}
}

//NullSpace//////////////////////////////////////////////////////////////////////
//compute null(A) or right(ordinary) null space of A
//A*null(A) = 0
//Just possible if A.cols() > A.rows()
//A:mxn, null(A):nx(n-m), 0:mx(n-m)
//verified
void null(Matrice &A, Matrice &nul)
{
	Matrice AT, Q, R;

	assert(A.cols() > A.rows());

	AT = A.transpose();

	QRdecompGiv(AT, Q, R);
	//QRdecompHH(AT, Q, R);

	nul.reinit(A.cols(), A.cols() - A.rows());
	nul = Q.subs(0, AT.rows() - 1, AT.cols(), AT.rows() - 1);

	//printf("null2:%i, %i\n", nul.rows(), nul.cols());
	//nul.print("nul");
}

//compute null(A')' or transpose of left null space of A
//null(A')'*A = 0
//Just possible if A.cols() < A.rows()
//A:mxn, null'(A'):(m-n)xm, 0:(m-n)xn
//verified
//Let A be an m-by-n matrix with rank n. QR decomposition finds orthonormal m-by-m matrix Q and upper triangular m-by-n matrix R such that A = QR.
//If we define Q = [Q1 Q2], where Q1 is m-by-n and Q2 is m-by-(m-n), then the columns of Q2 form the null space of A^T.
//ref: https://stackoverflow.com/questions/2181418/computing-the-null-space-of-a-matrix-as-fast-as-possible
void nullTT(Matrice &A, Matrice &nul)
{
	Matrice Q, R, nulT;

	assert(A.cols() < A.rows());

	QRdecompGiv(A, Q, R);
	//QRdecompHH(A, Q, R);

	//A.print("A"); fflush(stdout);
	//Q.print("Q"); fflush(stdout);
	//printf("%i %i, %i, %i\n", 0,A.rows()-1,A.cols(),A.rows()-1);

	nul.reinit(A.rows() - A.cols(), A.rows());
	nulT << Q.subsr(0, A.rows() - 1, A.cols(), A.rows() - 1);
	nul = nulT.transpose();

	//nul.print("nul");
}

//NOT WORKS, leaves a single zero in the result //NOT WORKS
//Using Givens Rotations
//Calculate A'*Hx and store it in Ho
//A is a unitary matrix with columns forming the left null space of Hf
//C(A)=N(Hf')
//O(mn)
//Ref: Null-Space-based Marginalization: Analysis and Algorithm, Yang, Yu., Maley, J., Huang, Gu.
//matlab:
//A = null(Hf');
//Ho = A'*Hx;
//notice: Hf:2nx3 -> size(null(Hf') ) = (2n, 2n-3)
// when n=1 -> Hf:2x3 -> null(Hf') = empty, usage impossible
// But using repeating point is possible
void nullSpaceProjection(Matrice &Hf_, Matrice &Hx_, Matrice &Ho)
{
	int i, j, Hfrows, Hfcols, Hxrows, Hxcols, k;
	Tfloat aii, aji, den, c, s;
	register Tfloat tmp1, tmp2;

	Hfrows = Hf_.rows(); Hfcols = Hf_.cols();
	Hxrows = Hx_.rows(); Hxcols = Hx_.cols();

	//assert(Hfcols == 3);  //as in the ref
	assert(Hfcols < Hfrows);

	Matrice Hf = Hf_;  ///////////////////////////////////////////////////
	Ho = Hx_;

	for(i = 0; i < Hfcols; i++) {
		for(j = 0; j < Hfrows; j++) {

			printf("%i, %i:\n", j + 1, i + 1);
			Hf.print("Hf1");

			aii = Hf(i, i);
			aji = Hf(j, i);
			if(equals(aii, 0., EPS) == 1) { //element already zero
				c = 0.;
				s = 1.;
			} else {
				den = sqrt(aii * aii + aji * aji);
				c = aii / den;
				s = -aji / den;
			}

			//Hf(i&j, 1:3) = [c, -s; s, c]*Hf(i&j, :)
			for(k = 0; k < Hfcols; k++) {
				tmp1 = c * Hf.elemc(i, k) - s * Hf.elemc(j, k); //i row
				tmp2 = s * Hf.elemc(i, k) + c * Hf.elemc(j, k); //j row
				Hf.elem(i, k) = tmp1; //i row
				Hf.elem(j, k) = tmp2; //j row
			}

			//Ho(i&j,  : ) = [c, -s; s, c]*Ho(i&j, :)
			for(k = 0; k < Hxcols; k++) {
				tmp1 = c * Ho.elemc(i, k) - s * Ho.elemc(j, k); //i row
				tmp2 = s * Ho.elemc(i, k) + c * Ho.elemc(j, k); //j row
				Ho.elem(i, k) = tmp1; //i row
				Ho.elem(j, k) = tmp2; //j row
			}

			Hf.print("Hf2");
		}
	}
}

////////////////////////////////////////////////////////////////////
//ref: shelly(2014)
//m >= n, measurement vector bigger than states
//we want to reduce m to n
// inputs: H :mxn, innov :mx1, rms: 1x1
//outputs: H2:nxn, innov2:nx1,  Rq: nxn(instead of mxm)
void kalmanQR(Matrice &H, Matrice &innov, double _rms, //inputs
			  Matrice &TH, Matrice &innov2, Matrice &Rq)
{
	int i;
	Matrice Q, R;

	//Q(mxm), R(mxn)
	//Q1(mxn), R1(nxn)
	//Q2(mx(m-n)), R2 = 0((m-n)xn)
	int m = H.rows();
	int n = H.cols();

	assert(m >= n);

	//Rq
	double rms2 = _rms * _rms;
	Rq.reinit(n, n);
	Rq.fill(0.);
	for(i = 0; i < n; i++)
		Rq.elem(i, i) = rms2;

	/*if(n == m) { //Better not to interfere, may cause numerical errors
		innov2 = innov;
		TH = H;
	    return;
	}*/

	QRdecompGiv(H, Q, R);
	//Q.print("Q");  fflush(stdout);
	//R.print("R");  fflush(stdout);

	//innov2
	Matrice Q1;
	Q1 << Q.subsr(0, m - 1, 0, n - 1);
	innov2 = Q1.transpose() * innov;

	//TH
	TH.reinit(n, n);
	TH = R.subs(0, n - 1, 0, n - 1);
}

void test_qr_decomp()
{
	//QR decompositions
#if 0
	Matrice A = zeros(4, 3);
	A.fillRand(1, 10);
	A.fillRand(1, 10);
	A.print("A\n");

	//A.inverse().print("Ainv:");

	Matrice B = zeros(9, 1);
	B.fillRand(1, 10);
	//B.print("B\n");

	Matrice Q, R;
	Q.reinit(A.rows(), A.rows());
	R.reinit(A.rows(), A.cols());
	//A:mxn, Q:mxm, R:mxn

	/*QRdecompGS(A, Q, R);
	Q.print("Q");
	R.print("R");
	printf("I-Q'Q error:%f\n", (eye(Q.rows(), Q.rows())-Q.transpose()*Q).norm());
	printf("A-QR error:%f\n", (A-Q*R).norm());
	*/
	/*QRdecompHH(A, Q, R);
	Q.print("Q");
	R.print("R");
	printf("I-Q'Q error:%f\n", (eye(Q.rows(), Q.rows())-Q.transpose()*Q).norm());
	printf("A-QR error:%f\n", (A-Q*R).norm());
	*/

	QRdecompGiv(A, Q, R);
	Q.print("Q");
	R.print("R");
	printf("I-Q'Q error:%f\n", (eye(Q.rows(), Q.rows()) - Q.transpose()*Q).norm());
	printf("A-QR error:%f\n", (A - Q * R).norm());

	//QRdecompGiv_JustR_Economy(A, R);
	//R.print("R");

	/*Matrice x;
	solveQR(A, B, x);
	x.print("x");
	(A*x-B).print("Ax-B");
	*/
#endif

	//implicit left null space
#if 0
	Matrice Hf, Hx, Ho;
	Hf.reinit(4, 3);
	Hf.fillRand(1., 10.);

	nullSpaceProjection(Hf, Hf, Ho);
	Ho.print("Ho");
#endif

	//right null space
#if 1
	Matrice Hf;
	Hf.reinit(20, 30);
	Hf.fillRand(1., 10.);
	Hf.print("A");

	Matrice nul;
	null(Hf, nul);
	nul.print("nul");
	(Hf * nul).print("res");
#endif

	//left null space
#if 0
	Matrice Hf;
	Hf.reinit(30, 20);
	Hf.fillRand(1., 10.);
	Hf.print("A");

	Matrice nul;
	nullTT(Hf, nul);
	nul.print("nul");
	(nul * Hf).print("res");
#endif

}

#ifdef __cplusplus
}
#endif
