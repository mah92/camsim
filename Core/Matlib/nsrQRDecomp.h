#ifndef __NSRQRDECOMP_H__
#define __NSRQRDECOMP_H__

#ifdef __cplusplus
extern "C" {
#endif

void QRdecompGS(Matrice &A, Matrice &Q, Matrice &R);
void QRdecompHH(Matrice &A, Matrice &Q, Matrice &R);
void QRdecompHH_JustR_Economy(Matrice &A, Matrice &Rout);
void QRdecompGiv(Matrice &A, Matrice &Q, Matrice &R);
void QRdecompGiv_JustR_Economy(Matrice &A, Matrice &Rout);

/////////////////////////////////

void solveQR(Matrice &A, Matrice &B, Matrice &x);

/////////////////////////////////

//compute null(A) or right(ordinary) null space of A
//A*null(A) = 0
//Just possible if A.cols() > A.rows()
void null(Matrice &A, Matrice &nul);

//compute null(A')' or transpose of left null space of A
//null(A')'*A = 0
//Just possible if A.cols() < A.rows()
void nullTT(Matrice &A, Matrice &nul);

//void nullSpaceProjection(Matrice& Hf_, Matrice& Hx, Matrice& Ho); //NOT WORKS

//////////////////////////////////
//sensors with different rms in channels (like GPS in Vertical and horizontal channels) should be fed separately
//Use TH as new H, innov2 as new innov and Rq as R matrix
void kalmanQR(Matrice &H, Matrice &innov, double rms, //inputs
			  Matrice &TH, Matrice &innov2, Matrice &Rq);

void test_qr_decomp();

#ifdef __cplusplus
}
#endif

#endif /* __NSRQRDECOMP_H__ */

