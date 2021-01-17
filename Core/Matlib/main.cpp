#include <iostream>
#include <stdio.h>
#include "nsrMathLib.h"
#include "nsrQRDecomp.h"
#include "nsrUtility.h"
#include "nsrCore.h"
#include <unistd.h>

using namespace std;

#ifdef __cplusplus
extern "C" {
#endif

int main()
{
	double time_s;
	time_s = myTime();

	LOGI("", "In The Name Of Allah \n\n");

	int D1 = 2, D2 = 2, D3 = 3, D4 = 3;
	//		    Tfloat C=3,r;
	//		    int j=0,k=0;
	//		    LOGI("","Rows of first A: %i \n\n ",D1);
	//    cin >> D1;
	//		    LOGI("","Cols of first A: %i \n\n ",D2);
	//    cin >> D2;
	//		    LOGI("","Rows of first B:%i \n\n ",D3);
	//    cin >> D3;
	//		    LOGI("","Cols of first B: %i \n\n ",D4);
	//    cin >> D4;
	//Matrice a(true);
	//a.initialize(D1,D2, NULL, NULL, true);

	Matrice C3(3, 3);
	C3.fillRand(1, 10);
	C3.print("C3");

	C3.subs(0, 0, 1, 2).print("res:\n");

	C3.print("C3");

	return 0;

	Matrice cc = colmat(3., 4., 5., 6.);
	cc.print("cc:");
	return 0;
	//while(1)
	{
		LOGI("", "\n Start:%i\n ", allocated_size);
		//, sizeof(Matheader)); == 64

		/*Matrice a, b, c;

		b = eye(100, 200);
		b.fillRand(1,10);

		c = eye(200, 300);
		c.fillRand(1,10);

		a= b*c;*/

		//LOGI("","\n B is:\n ");
		//a.print();
		//b.print();
		//c.print();

		Matrice b = eye(5, 5);
		b.fillRand(1, 10);
		//b = b.inverse();
		b.print("b:\n");

		Matrice d;
		//d << b.subs(0,b.rows()-1, 0,b.cols()-1);
		//d << b.diag();
		//d << b.col(0);
		d = b.subs(0, 3, 1, 4);
		d.print("d:\n");

		//LOGI("","\n B diag ++:\n ");
		//d = d * 2;

		//d=d+1;
		//b.print(\n B:\n );

		LOGI("", "End:%i\n\n ", allocated_size);
		usleep(1e2);
	}
	/*a = a.inverse();
	LOGI("","\n Inverse(%fs) is:\n ", myTime()-time_s);
	a.print();

	a = a.inverse();
	a.print();*/
	/*
		Matrice Q, R;

		Q.initialize(D1,D1);
		R.initialize(D1,D2);

		QRdecompHH(a, Q, R);

		LOGI("","Q:\n");
		Q.print();
		LOGI("","R:\n");
		R.print();

		LOGI("","Q*R:\n");
		(Q*R).print();

		Matrice Reco;
		Reco.initialize(D2,D2);
		QRdecompHH_JustR_Economy(a, Reco);
		LOGI("","Reco:\n");
		Reco.print();*/

	/*c=a*b;
	LOGI("","\n\n a*b is :\n\n\n");
	c.print();

	b.equaldiag(a);
	LOGI("","\n\n equaldiag is :\n\n\n");
	b.print();

	c=a.diag();
	LOGI("","\n\n a.diag() is :\n\n\n");
	c.print();*/

	/*
	    c.colequal(0,a);
	   LOGI("","\n\n colequal() is :\n\n\n");
	    c.print();

	        a.normalize();
	   LOGI("","\n\n normalize() is :\n\n\n");
	    a.print();

	    Tfloat sum=0;
	 sum=a.rms();
	       LOGI("","\n\n rms is :%f \n\n\n",sum);
	       sum=a.norm();
	         LOGI("","\n\n norm is :%f \n\n\n",sum);
	/*
	/*

			//   b.print();

			//    LOGI("","\n A+B is:\n ");
			//    c.print();

			//    c=a-b;
			//    LOGI("","\n A-B is:\n ");
			//    c.print();

			//    c=a*b;
			//    LOGI("",""\n A*B is:\n ");
			//    c.print();
			   LOGI("","\n\n  c is : \n\n");
			 		    c.print();
			//    c=a.transpose();
			//    LOGI("","\n Transpose of A is:\n ");
			//    c.print();
			    double t1 = myTime();
			 //  a.invers(I);

			    int u=0;
			    LOGI("","\n\n Before 'for' u is : %i\n\n\n",u);

			 //   for(u=0;u<100;u++)
			  //  {
			    	c=a.inverse();
			       LOGI("","\n\n in 'for' u is : %i\n\n\n",u);
			 //   }
			    LOGI("","\n\n After 'for' u is : %i\n\n\n",u);
			//    c=a*b;                    //When we do not define  2D_definition the assignment overloading don't work .
			//    c=a.dot(b);
			//    c=a.transpose();
			    double t2 = myTime();
			    LOGI("","\n\n error of c is : %i \n\n",c.p->error);
			    LOGI("","\n\n error of a is : %i \n\n",a.p->error);
			    LOGI("","\n\n Inverse of a is :\n\n");
			    c.print();
	//		 	    LOGI("","\n A+B is:\n ");
	//	     	    c.print();
			   LOGI("","\n\nTime taken: %.5fs\n\n\n",t2-t1);
	*/

	return 0;
}

#ifdef __cplusplus
}
#endif
