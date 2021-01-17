#include <math.h>
#include "Matlib/nsrMathLib.h"

#ifdef __cplusplus
extern "C" {
#endif

static Matrice CI2B1(3, 3), CI2B2(3, 3), CI2B_e(3, 3);

double calcEuError(double phi1, double theta1, double psi1,
				   double phi2, double theta2, double psi2)
{
	double SP, ST, SS, CP, CT, CS;
	SP = sin(phi1 * M_PI / 180);
	ST = sin(theta1 * M_PI / 180);
	SS = sin(psi1 * M_PI / 180);
	CP = cos(phi1 * M_PI / 180);
	CT = cos(theta1 * M_PI / 180);
	CS = cos(psi1 * M_PI / 180);

	CI2B1.fill2(+CT * CS,         +CT * SS,   -ST,
				SP * ST * CS - CP * SS, SP * ST * SS + CP * CS, SP * CT,
				CP * ST * CS + SP * SS, CP * ST * SS - SP * CS, CP * CT); //Verified

	SP = sin(phi2 * M_PI / 180);
	ST = sin(theta2 * M_PI / 180);
	SS = sin(psi2 * M_PI / 180);
	CP = cos(phi2 * M_PI / 180);
	CT = cos(theta2 * M_PI / 180);
	CS = cos(psi2 * M_PI / 180);

	CI2B2.fill2(+CT * CS,         +CT * SS, -ST,
				SP * ST * CS - CP * SS, SP * ST * SS + CP * CS, SP * CT,
				CP * ST * CS + SP * SS, CP * ST * SS - SP * CS, CP * CT); //Verified

	CI2B_e = CI2B1 * CI2B2.t();

	return 180. / M_PI * acos((trace(CI2B_e) - 1.) / 2.);
}

#ifdef __cplusplus
}
#endif
