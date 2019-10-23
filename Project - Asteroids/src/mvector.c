#include "mvector.h"
#include <stdio.h>

mvector mvector_create(mpoint P, mpoint Q) {

	mvector PQ;
	PQ.P = P;
	PQ.x = Q.x - P.x;
	PQ.y = Q.y - P.y;
	
	return PQ;
}

double mvector_magnitude(mvector *v1) {
	
	double mag = pow( v1->x, 2 ) + pow( v1->y, 2 );
	return sqrt(mag);
}

void mvector_add(mvector *v1, mvector *v2) {
	
	v1->x += v2->x;
	v1->y += v2->y;
}

void mvector_subtract(mvector *v1, mvector *v2) {

	v1->x -= v2->x;
	v1->y -= v2->y;
}

void mvector_multiplication(mvector *v1, double scalar) {

	v1->x *= scalar;
	v1->y *= scalar;
}

void mvector_division(mvector *v1, double scalar) {

	v1->x /= scalar;
	v1->y /= scalar;
}

double mvector_dot_product(mvector *v1, mvector *v2) {		
	
	return ( v1->x * v2->x + v1->y * v2->y );
}

mvector mvector_get_versor (mvector *v1){
	
	mvector versor = *v1;
	double magn = mvector_magnitude (v1);
	mvector_division (&versor, magn);
	return versor;
}

void mvector_limit (mvector *v1, double limit){
	
	double magn = mvector_magnitude(v1);
	
	if (magn > limit)
		mvector_multiplication(v1, limit / magn);
}

double mvector_angle(mvector *v1) {

	double magnitude = mvector_magnitude(v1);
	double radians = acos(v1->x / magnitude);
	double degrees = (radians / mpi) * 180;
	if (v1->y < 0.0)
		degrees *= -1;

	return degrees;
}

void mvector_rotate(mvector *v1, double degrees) {

	double radians = degrees * mpi / 180;
	double angle_cos = cos(radians);
	double angle_sin = sin(radians);
	double x = v1->x * angle_cos - v1->y * angle_sin;
	double y = v1->x * angle_sin + v1->y * angle_cos;
	v1->x = x;
	v1->y = y;
}

void mvector_print(mvector *v1) {
	
	printf("( %f , %f )\n", v1->x, v1->y);
}




