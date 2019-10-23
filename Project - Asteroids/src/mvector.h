/** @author Nuno Miguel Marques */

#pragma once
#include <math.h>

/** @defgroup mvector mvector
* @{
*	Two-dimensional Euclidean vector operations for C
*/

/*	PI 		*/
#define mpi 3.14159265358979323846264338327950288	
/*	PI / 2  */
#define mpi2 1.57079632679489661923132169163975144	

/** @brief Two-dimensional cartesian point */
typedef struct {
	double x;
	double y;
} mpoint;
/** @brief Two-dimensional vector */
typedef struct {
	/** @brief P point in PQ */
	mpoint P; 
	double x;
	double y;
}mvector;


/**
 * @brief Creates a vector using two points
 * 
 * V = PQ = ( Qx - Px , Qy - Py)
 * @param P Cartesian point P
 * @param Q Cartesian point Q
 * @returns The vector created using point P and Q
 */
mvector mvector_create(mpoint P, mpoint Q);
/**
 * @brief Calculates the magnitude of a vector
 * 
 * ||v1|| = sqrt(x^2+y^2)
 * @param v1 Two-dimensional vector
 * @returns The magnitude of the vector as a real number
 */
double mvector_magnitude(mvector *v1);
/**
 * @brief Adds a vector v2 to vector v1
 * 
 * v1 = ( v1x + v2x, v1y + v2y)
 * @param v1 Two-dimensional vector
 * @param v2 Two-dimensional vector
 */
void mvector_add(mvector *v1, mvector *v2);
/**
 * @brief Subtracts a vector v2 from vector v1
 * 
 * v1 = ( v1x - v2x, v1y - v2y)
 * @param v1 Two-dimensional vector
 * @param v2 Two-dimensional vector
 */
void mvector_subtract(mvector *v1, mvector *v2);
/**
 * @brief Vector scalar multiplication
 * 
 * v1 = ( v1.x * scalar, v1.y * scalar)
 * @param v1 Two-dimensional vector
 * @param scalar Real number
 */
void mvector_multiplication(mvector *v1, double scalar);
/**
 * @brief Vector scalar division
 * 
 * v1 = ( v1.x / scalar, v1.y / scalar)
 * @param v1 Two-dimensional vector
 * @param scalar Real number
 */
void mvector_division(mvector *v1, double scalar);
/**
 * @brief Vector dot product. Multiplies two vectors and returns a real number
 * 
 * v1 . v2 = (v1.x * v2.x) + (v1.y * v2.y)
 * @param v1 Two-dimensional vector
 * @param v2 Two-dimensional vector
 */
double mvector_dot_product(mvector *v1, mvector *v2);
/**
 * @brief Returns the versor of v1, a vector with the same direction of v1 but with magnitude 1
 * 
 * Normalized v1 = v1/(magnitude of v1)
 * @param v1 Two-dimensional vector
 * @returns Returns the versor of v1
 */
mvector mvector_get_versor (mvector *v1);
/**
 * @brief Reduces vector magnitude to a limit
 * 
 * If limit is bigger than the magnitude of v1 then
 *
 * v1 = v1 * (limit/(magnitude of v1);
 * @param v1 Two-dimensional vector
 * @param limit v1's magnitude will be limited by this value
 */
void mvector_limit (mvector *v1, double limit);
/**
 * @brief Returns the angle of a vector in degrees
 * 
 * angle = acos(x / r)
 * angle is negative when y < 0
 *
 * @param v1 Two-dimensional vector
 * @returns Returns the angle of a vector in degrees as a real number
 */
double mvector_angle(mvector *v1);
/**
 * @brief Rotates vector using the rotation matrix
 * 
 * Vector Rotation Matrix
 *
 * { cos(angle) , -sin(angle) }
 *
 * { sin(angle) , cos(angle)  }
 *
 *
 * rotated v1.x = v1.x * cos(angle) - v1.y * sin(angle)
 *
 * rotated v1.y = v1.x * sin(angle) + v1.y * cos(angle)
 *
 * @param v1 Two-dimensional vector
 * @param degrees v1 rotation in degrees
 */
void mvector_rotate(mvector *v1, double degrees);
/**
 * @brief Print vector in ( x , y ) format
 *
 * @param v1 Two-dimensional vector
 */
void mvector_print(mvector *v1);
