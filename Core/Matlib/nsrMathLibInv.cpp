/*
 * Math functions
 * ver 1
 * see https://github.com/kristianpaul/osgps/blob/master/CMatrix3.cpp to:
 * 1) inspire why eigen uses high memory
 * 2) see mat inverse code
 */

#include "./nsrMathLib.h"
#include "nsrCore.h"
#include <string.h> //memset

#undef TAG
#define TAG "Cpp:MathLibInv:"

static bool debug_mode = true;

#define error(tag, ...) if(debug_mode) {printf(__VA_ARGS__);  fflush(stdout);}

Matrice Matrice::operator / (Matrice M)
{
	Matrice a(rows(), cols());/////////////////////////////////////////////////////////////////////////////

	a = (*this) * M.inverseKristianLUGeneral();
	return a;
}

//Tmp is the same size as A
//using 3n2 memory for nxn matrix(input, output, Tmp)
//O(n3)
int Matrice::equalInv(const Matrice &A, Matrice &Tmp)
{
	//matrix inverse
	if(rows() != cols()) {
		error(TAG, " Matrix not square!");
		return -1;
	}

	reinit(A.rows(), A.cols());
	Tmp.reinit(A.rows(), A.cols());

	if(rows() == 1) {
		if(A.elemc(0, 0) == 0)
			return -1;
		elem(0, 0) = 1. / A.elemc(0, 0);
		return 0;
	} else {//************** Gauss Jordan method for inverse matrix *******************
		//https://ganeshtiwaridotcomdotnp.blogspot.in/2009/12/c-c-code-gauss-jordan-method-for.html
		//is of order O(n^3) like matrix multiplication
		//can be used up to thousands of equations
		//but LU decomposition is better
		Tfloat ratio, d;
		int i, j, k;
#define SWAP(a,b) {d=a;a=b;b=d;}

		Tmp.equal(A);

		//fill Identity
		for(i = 0; i < rows(); i++) {
			for(j = 0; j < rows(); j++) {
				if(i == j)
					elem(i, j) = 1.0;
				else
					elem(i, j) = 0.0;
			}
		}

		//************* partial pivoting **************
		//for the case A(i,i) == 0, swap rows
		for(i = rows() - 1; i > 0; i--) {
			if(Tmp.elem(i - 1, 0) < Tmp.elem(i, 0)) {
				for(j = 0; j < rows(); j++)
					SWAP(Tmp.elem(i, j), Tmp.elem(i - 1, j))
					for(j = 0; j < rows(); j++)
						SWAP(elem(i, j), elem(i - 1, j))
					}
		}

		/*LOGN3("", "The pivoted matrix is: \n");

		for(i = 0; i < rows(); i++) {
			for(j = 0; j < rows(); j++) {
				LOGN3("", "%.2f", Tmp.elem(i,j));
				LOGN3("", "\t");
			}
			for(j = 0; j < rows(); j++) {
				LOGN3("", "%.2f", elem(i,j));
				LOGN3("", "\t");
			}
			LOGN3("", "\n");
		}*/

		//********* reducing to diagonal  matrix ***********
		for(i = 0; i < rows(); i++)
			for(j = 0; j < rows(); j++) //rows()*2 is wrong in reference
				if(i != j) {
					if(Tmp.elem(i, i) == 0)
						return -1;
					ratio = Tmp.elem(j, i) / Tmp.elem(i, i);
					for(k = 0; k < rows(); k++)
						Tmp.elem(j, k) -= ratio * Tmp.elem(i, k);
					for(k = 0; k < rows(); k++)
						elem(j, k) -= ratio * elem(i, k);
				}

		//************* reducing to unit matrix *************
		for(i = 0; i < rows(); i++) {
			d = Tmp.elem(i, i);
			if(d == 0)
				return -1;
			for(j = 0; j < rows(); j++)
				Tmp.elem(i, j) /= d;
			for(j = 0; j < rows(); j++)
				elem(i, j) /= d;
		}

		return 0;
	}
}

//************************************************************

// see pp 38. in Numerical Recipes
Tfloat Matrice::determinant()
{
	int i, d;
	Tfloat determinant;

	if(rows() != cols()) error(TAG, "matrix must be square for determinant()");

	Matrice indx(rows(), cols()); // create the "index vector" ////////////////////////////////////////////////////////////////////////////////////////

	// perform the decomposition once:
	Matrice decomp;
	lu_decompose(indx, d, decomp); ////////////////////////////////////////////////////////////////////////////////////////
	determinant = d;
	for(i = 0; i < cols() ; i++)
		determinant *= decomp.elem(i, i);
	return determinant;
}

//Means if and only if (for all x), x'.Mat.x > 0
bool Matrice::isPositiveDefinite()
{
	int i, h;
	Tfloat err;
	assert(rows() == cols());

	//not symetric
	if(((*this) - (*this).transpose()).norm2() > 1e-6) {
		error(TAG, "Fail in positive definiteness test1\n");
		return false;
	}

	//negative diagonal element
	for(i = 0; i < rows(); i++) {
		if(elemc(i, i) < 0) {
			error(TAG, "Fail in positive definiteness test2, elem %i in (%ix%i)\n", i + 1, rows(), cols());
			return false;
		}
	}

	//positive sub determinants(sufficient cause)
	Matrice submat;
	h = p->matrix_indices_start_by_one ? 1 : 0;
	for(i = 0; i < rows(); i++) {
		submat << (*this).subsr(0 + h, i + h, 0 + h, i + h);
		err = submat.determinant();
		if(err < -1e-15) { //less than numerical round off error
			error(TAG, "Fail in positive definiteness test3, err:%.15f, elem %i in (%ix%i)\n", err, i + 1, rows(), cols());
			return false;
		}
	}

	return true;

}

//LU decomposition
//using (2n2 i/o)+2n2+3n memory for nxn matrix
//O(n2.4), experiment with random input:O(n3.+)!!!
//1000 LU corei7(6770HQ)-> 42, del->71
//500 LU corei7(6770HQ)->4.69, del->8.3
//250 LU corei7(6770HQ)->0.580, del->0.944
//see: https://github.com/kristianpaul/osgps/blob/master/CMatrix3.cpp
//verified
//always gives result even if det is null, some rows-columns will become very big
Matrice Matrice::inverseKristianLUGeneral()
{
	int col, d;

	if(rows() != cols()) error(TAG, "matrix must be square for inverse()");

	Matrice Y("I", cols()); // create an identity matrix ////////////////////////////////////////////////////////////////////////////////////////
	Matrice indx(cols(), cols()); // create the "index vector" ////////////////////////////////////////////////////////////////////////////////////////
	Matrice B(cols(), cols()); // see Press & Flannery ////////////////////////////////////////////////////////////////////////////////////////

	// perform the decomposition once:
	///TODO: remove transpose, needs nxn more memory
	Matrice decomp;
	Matrice this_transpose = (*this).transpose(); ///////////////////////////////////////////////////////////////////////////////////////
	this_transpose.lu_decompose(indx, d, decomp); ////////////////////////////////////////////////////////////////////////////////////////

	for(col = 0; col < cols(); col++) {
		B.copy_column(Y, col, 0);
		decomp.lu_back_subst(indx, B);
		Y.copy_column(B, 0, col);
	}

	return Y;
}

/************************************************************
The private support functions for determinant & inverse.
 ************************************************************/

// copy the from_col of mm to the to_col of "this"
void Matrice::copy_column(Matrice &mm, int from_col, int to_col)
{
	int row;
	if(rows() != mm.rows())  error(TAG, "number of rows must be equal for copy_column()");
	for(row = 0; row < rows(); row++)
		elem(row, to_col) = mm.elem(row, from_col);
}

void Matrice::switch_columns(int col1, int col2)
{
	int row;
	Tfloat temp;

	for(row = 0; row < rows(); row++) {// temporarily store col 1:
		temp = elem(row, col1);
		elem(row, col1) = elem(row, col2); // move col2 to col1
		elem(row, col2) = temp; // move temp to col2
	}
}

// scale a matrix (used in L-U decomposition)
Matrice Matrice::scale()
{
	int row, col;
	Tfloat temp;
	if(rows() <= 0 || cols() <= 0) error(TAG, "bad matrix size for scale()");
	if(rows() != cols())     error(TAG, "matrix must be square for scale()");
	Matrice scale_vector(rows(), cols()); ////////////////////////////////////////////////////////////////////////////////////////
	for(col = 0; col < cols(); col++) {
		Tfloat maximum = 0;
		for(row = 0; row < rows(); row++)
			if((temp = (Tfloat)fabs(elem(row, col))) > maximum)
				maximum = temp;  // find max column magnitude in this row
		if(maximum == 0) error(TAG, "singular matrix in scale()");
		scale_vector.elem(col, 0) = 1 / maximum; // save the scaling
	}

	return scale_vector;
}

/*
 Returns the L-U decomposition of a matrix. indx is an output
 vector which records the row permutation effected by the
 partial pivoting, d is output as +-1 depending on whether the
 number of row interchanges was even or odd, respectively.
 This routine is used in combination with lu_back_subst to
 solve linear equations or invert a matrix.
 */

void Matrice::lu_decompose(Matrice &indx, int &d, Matrice &lu_decomp)
{
	int row, col, k, col_max; // counters
	Tfloat dum; // from the book -- I don't know significance
	Tfloat sum;
	Tfloat maximum;

	if(rows() != cols()) error(TAG, "Matrix must be square to L-U decompose!\n");
	d = 1; // parity check

	//Matrice lu_decomp(rows(), cols()); ////////////////////////////////////////////////////////////////////////////////////////
	// make a direct copy of the original matrix:
	//deepcopy(*this, lu_decomp);
	lu_decomp.equal(*this);

	Matrice scale_vector = lu_decomp.scale(); // scale the matrix, nx1 vector ////////////////////////////////////////////////////////////////////////////////////////

	// The loop over columns of Crout's method:
	for(row = 0; row < rows(); row++) {
		if(row > 0) { // eqn 2.3.12 except for row=col:
			for(col = 0; col <= row - 1; col++) {
				sum = lu_decomp.elem(row, col);
				if(col > 0) {
					for(k = 0; k <= col - 1; k++)
						sum -= lu_decomp.elem(row, k) * lu_decomp.elem(k, col);
					lu_decomp.elem(row, col) = sum;
				}
			}
		}

		// Initialize for the search for the largest pivot element:
		maximum = 0;
		// i=j of eq 2.3.12 & i=j+1..N of 2.3.13:
		for(col = row; col <= cols() - 1; col++) {
			sum = lu_decomp.elem(row, col);
			if(row > 0) {
				for(k = 0; k <= row - 1; k++)
					sum -=  lu_decomp.elem(k, col) * lu_decomp.elem(row, k);
				lu_decomp.elem(row, col) = sum;
			}
			// figure of merit for pivot:
			dum = scale_vector.elem(col, 0) * fabs(sum);
			if(dum >= maximum) {  // is it better than the best so far?
				col_max = col;
				maximum = dum;
			}
		}

		// Do we need to interchange rows?
		if(row != col_max) {
			lu_decomp.switch_columns(col_max, row); // Yes, do so...
			d *= -1;  // ... and change the parity of d
			// also interchange the scale factor:
			dum = scale_vector.elem(col_max, 0);
			scale_vector.elem(col_max, 0) = scale_vector.elem(row, 0);
			scale_vector.elem(row, 0) = dum;
		}
		indx.elem(row, 0) = col_max;

		// Now, finally, divide by the pivot element:
		if(row != rows() - 1) {
			if(lu_decomp.elem(row, row) == 0)
				lu_decomp.elem(row, row) = 1e-20;
			// If the pivot element is zero the matrix is
			// singular (at least to the precision of the
			// algorithm).  For some applications on singular
			// matrices, it is desirable to substitute tiny for zero
			dum = 1 / lu_decomp.elem(row, row);
			for(col = row + 1; col <= cols() - 1; col++)
				lu_decomp.elem(row, col) *= dum;
		}
	}

	if(lu_decomp.elem(rows() - 1, cols() - 1) == 0)
		lu_decomp.elem(rows() - 1, cols() - 1) = 1e-20;
	return;
}

/*
 Solves the set of N linear equations A*X = B.  Here "this"
 is the LU-decomposition of the matrix A, determined by the
 routine lu_decompose(). Indx is input as the permutation
 vector returned  by lu_decompose().  B is input as the
 right-hand side vector B,  and returns with the solution
 vector X.  This routine takes into  account the possibility
 that B will begin with many zero elements,  so it is efficient
 for use in matrix inversion.   See pp 36-37 in
 Press & Flannery.
 */
void Matrice::lu_back_subst(const Matrice &indx, Matrice &b)
{
	int row, col, ll;
	int ii = 0;
	Tfloat sum;

	if(rows() != cols())
		error(TAG, "non-square lu_decomp matrix in lu_back_subst()");
	if(rows() != b.rows())
		error(TAG, "wrong size B vector passed to lu_back_subst()");
	if(rows() != indx.rows())
		error(TAG, "wrong size indx vector passed to lu_back_subst()");

	for(col = 0; col < cols(); col++) {
		ll = (int)indx.elemc(col, 0);
		sum = b.elem(ll, 0);
		b.elem(ll, 0) = b.elem(col, 0);
		if(ii >= 0)
			for(row = ii; row <= col - 1; row++)
				sum -= elem(row, col) * b.elem(row, 0);
		else if(sum != 0)
			ii = col;
		b.elem(col, 0) = sum;
	}

	for(col = cols() - 1; col >= 0; col--) {
		sum = b.elem(col, 0);
		if(col < cols() - 1)
			for(row = col + 1; row <= rows() - 1; row++)
				sum -= elem(row, col) * b.elem(row, 0);
		// store a component of the soln vector X:
		b.elem(col, 0) = sum / elem(col, col);
	}
}
//************************************************************

//verified but Y is released before return!!!
//************** Gauss Jordan method for inverse matrix *******************
//using (2n2 i/o) + 2n2 memory for nxn matrix
//O(n3) (like matrix multiplication), experiment: better than O(n3)
// !DEFINITION_2D
//1000 GJ corei7(6770HQ)->25.8
//500 GJ corei7(6770HQ)->3.26
//250 GJ corei7(6770HQ)->0.41
//can be used in PC up to thousands of equations
//https://ganeshtiwaridotcomdotnp.blogspot.in/2009/12/c-c-code-gauss-jordan-method-for.html
Matrice Matrice::inverseGaussJordanGeneral()
{
	if(rows() != cols()) error(TAG, "matrix must be square for inverse()");
	Matrice Y("I", rows()); // create an identity matrix ////////////////////////////////////////////////////////////////////////////////////////

	Matrice Tmp(rows(), rows()); /////////////////////////////////////////////////////////////////////////////////////////

	int i, j, k;
	Tfloat ratio, d;

	Tmp = (*this);

	//************* partial pivoting **************
	//for the case elem(i,i) == 0, swap rows
	for(i = rows() - 1; i > 0; i--) {
		if(Tmp.elem(i - 1, 0) < Tmp.elem(i, 0)) {
			for(j = 0; j < rows(); j++)
				SWAP(Tmp.elem(i, j), Tmp.elem(i - 1, j))
				for(j = 0; j < rows(); j++)
					SWAP(Y.elem(i, j), Y.elem(i - 1, j))
				}
	}

	//********* reducing to diagonal matrix ***********

	for(i = 0; i < rows(); i++)
		for(j = 0; j < rows(); j++) //Y.rows()*2 is wrong in reference
			if(i != j) {
				if(Tmp.elem(i, i) == 0)
					return Matrice(); //rows:0,cols:0 matrix
				ratio = Tmp.elem(j, i) / Tmp.elem(i, i);
				for(k = 0; k < rows(); k++)
					Tmp.elem(j, k) -= ratio * Tmp.elem(i, k);
				for(k = 0; k < rows(); k++)
					Y.elem(j, k) -= ratio * Y.elem(i, k);
			}

	//************* reducing to unit matrix *************
	for(i = 0; i < rows(); i++) {
		d = Tmp.elem(i, i);
		if(d == 0)
			return Matrice(); //rows:0,cols:0 matrix
		for(j = 0; j < rows(); j++)
			Tmp.elem(i, j) /= d;
		for(j = 0; j < rows(); j++)
			Y.elem(i, j) /= d;
	}

	return Y;
}

// It would be preferable to use an include such as lapack.h.
// But lapack.h is not available from the octave or liblapack-dev packages...
#ifdef __cplusplus
extern "C" {
#endif

/**
 * DPOTRF computes the Cholesky factorization of a real symmetric
 * positive definite matrix A.
 *
 * The factorization has the form
 * A = U**T * U,  if UPLO = 'U', or
 * A = L  * L**T,  if UPLO = 'L',
 * where U is an upper triangular matrix and L is lower triangular.
 */

extern void dpotrf_(char* uplo, ptrdiff_t* n, double* A, ptrdiff_t* lda, ptrdiff_t* info);
/**
 * DPOTRI computes the inverse of a real symmetric positive definite
 * matrix A using the Cholesky factorization A = U**T*U or A = L*L**T
 * computed by DPOTRF.
 */
extern void dpotri_(char* uplo, ptrdiff_t* n, double* A, ptrdiff_t* lda, ptrdiff_t* info);

// LU decomoposition of a general matrix
extern void dgetrf_(int* M, int *N, double* A, int* lda, int* IPIV, int* INFO);

// generate inverse of a matrix given its LU decomposition
extern void dgetri_(int* N, double* A, int* lda, int* IPIV, double* WORK, int* lwork, int* INFO);

#ifdef __cplusplus
}
#endif
/*
//************** Lapack LU factorization for general matrices *******************
//using ? memory for nxn matrix
//O(n3) (like matrix multiplication), experiment: about O(n3), but 20x faster than our LU inv!!!
//1000 LU del-> 2.9
//500 LU del->0.32
//250 LU del->0.043
//verified
Matrice Matrice::inverseLapackLUGeneral()
{
	Matrice Y("I",rows()); // create an identity matrix ////////////////////////////////////////////////////////////////////////////////////////

	Y = *this;

	ptrdiff_t p0 = rows();
	int dim = rows();
	int *IPIV = new int[dim+1];
    int dim2 = dim*dim;
	double *WORK = new double[dim2];

    int info;
#ifndef DEFINITION_2D
    dgetrf_(&dim, &dim, Y.p->data, &dim, IPIV, &info);
    dgetri_(&dim, Y.p->data, &dim, IPIV, WORK, &dim2, &info);
	//dpotri_((char*) "U", &dim, Y.p->data, &dim, &info);

#else
    #warning( "Inverse Not Implemented!")
#endif

	LOGN3("", "info: %i\n", info);
	//Y.print();
	delete IPIV;
    delete WORK;

	return Y;
}

//LU factorization for positive matrices
Matrice Matrice::inverseLapackPositive()
{
	Matrice Y("I",rows()); // create an identity matrix ////////////////////////////////////////////////////////////////////////////////////////

	Y = *this;

	ptrdiff_t dim = rows();
    ptrdiff_t info;

#ifndef DEFINITION_2D
	dpotrf_((char*) "U", &dim, Y.p->data, &dim, &info);
	dpotri_((char*) "U", &dim, Y.p->data, &dim, &info);
#else
    #warning( "Inverse Not Implemented!")
#endif

	LOGN3("", "info: %i\n", info);
	Y.print();

	return Y;
}
*/

#if 0
#ifdef __cplusplus
extern "C" {
#endif

void QUIC(char mode, uint32_t &p, const double* S, double* Lambda0,
		  uint32_t &pathLen, const double* path, double &tol,
		  int32_t &msg, uint32_t &maxIter,
		  double* X, double* W, //input/output
		  //outputs
		  double* opt, //optimization residual
		  double* cputime, //time
		  uint32_t* iter, //itertaions done
		  double* dGap //duality gap
		 );

#ifdef __cplusplus
}
#endif

//QUIC method for sparse positive matrices
Matrice Matrice::inverseQuicPositive(float lambda)
{

	LOGN3("", "Y1\n");

	//notes:
	//Quic designed to convert covariance to precision = concentration = information matrix
	//input & ouput both seemed sparse???
	//1)for more than 20kx20k, or using multicore parallel cpus,
	//	use big quic, which cause near linear reduction in computation cost
	//2)for denser result matrices (density > 5%) algorithms like IPM, PSM converge faster(noted in QUIC article, p.33)
	//result matrice is sparsified by lambda

	// mode = {'D', 'P', 'T'} for 'default', 'path' or 'trace'.
	//param [in]: p: mat dimension
	//S: covariance
	//Lambda0: a matrix implementing landa for every element
	//	lamda tries to make inverse covariance sparser

	uint32_t dim = rows();
	Matrice Lambda0(rows(), rows()); /////////////////////////////////////////////////////////////////////////////////////////
	Matrice XMat("I", rows()); /////////////////////////////////////////////////////////////////////////////////////////
	Matrice WMat("I", rows()); /////////////////////////////////////////////////////////////////////////////////////////
	uint32_t pathLen = 1;
	double* path = NULL; //just used in path mode

	//Lambda0: regularization parameters seeks to:
	// make inverse mat smaller(more sparse) thats a systematic error
	// so the inverse of inverse bigger
	// final inverse mat elements error seems to be equal to lambda!!!
	Lambda0.fill(lambda);

	int32_t msg = 2; //verbosity level
	uint32_t maxIter = 100;
	double tol = 1e-6;
	//double* X = XMat.p->data;
	//double* W = WMat.p->data;
	double opt;
	double cputime;
	uint32_t iter;
	double dGap;

	QUIC('D', dim, p->data, Lambda0.p->data,
		 pathLen, path, tol,
		 msg, maxIter,
		 //input/outputs
		 XMat.p->data, WMat.p->data,
		 //outputs
		 &opt, &cputime,
		 &iter, &dGap);
	LOGN3("", "iters:%i\n", iter);
	XMat.print();
	WMat.print();

	memcpy(p->data, XMat.p->data, dim * dim * sizeof(double));

	return XMat;
}
#endif


