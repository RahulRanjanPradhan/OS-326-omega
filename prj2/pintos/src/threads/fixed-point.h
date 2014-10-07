#ifndef THREADS_FIXED_POINT_H
#define THREADS_FIXED_POINT_H

#include <stddef.h>
#include <stdint.h>

/* 	Do Fixed-point Real Arithmetic 
		p.q  17.14 fixed-point number representation
		1 sign bit
    x and y are fixed-point numbers , n is an integer. */


typedef int fixed_point;
#define F (1<<14) //16384



fixed_point int_to_fp(int n);
int 				fp_to_int_rtz(fixed_point x);
int 				fp_to_int_rtn(fixed_point x);
fixed_point fp_add_fp(fixed_point x, fixed_point y);
fixed_point fp_minus_fp(fixed_point x, fixed_point y);
fixed_point fp_add_int(fixed_point x, int n);
fixed_point fp_minus_int(fixed_point x, int n);
fixed_point fp_time_fp(fixed_point x, fixed_point y);
fixed_point fp_time_int(fixed_point x, int n);
fixed_point fp_divideby_fp(fixed_point x, fixed_point y);
fixed_point fp_divideby_int(fixed_point x, int n);

 
/* Convert n to fixed point. */
fixed_point int_to_fp(int n)
{
	return n * F;
};

/* Convert x to integer (rounding toward zero). */
int fp_to_int_rtz(fixed_point x)
{
	return x / F;
};

/* Convert x to integer (rounding to nearest). */
int fp_to_int_rtn(fixed_point x)
{
	if(x >= 0)
		return ((x) + (F/2))/F;
	else
		return ((x) - (F/2))/F;
};

/* Add x and y. */
fixed_point fp_add_fp(fixed_point x, fixed_point y)
{
	return x + y;
};

/* Subtract y from x.*/
fixed_point fp_minus_fp(fixed_point x, fixed_point y)
{
	return x - y;
};

/* Add x and n.*/
fixed_point fp_add_int(fixed_point x, int n)
{
	return ((x) + (n)*(F));
};

/* Subtract n from x.*/
fixed_point fp_minus_int(fixed_point x, int n)
{
	return ((x) - (n*F));
};

/* Multiply x by y.*/
fixed_point fp_time_fp(fixed_point x, fixed_point y)
{
	return ((int64_t)x) * y / F;
};

/* Multiply x by n.*/
fixed_point fp_time_int(fixed_point x, int n)
{
	return x * n;
};

/* Divide x by y.*/
fixed_point fp_divideby_fp(fixed_point x, fixed_point y)
{
	return ((int64_t)x) * F / y;
};

/* Divide x by n.*/
fixed_point fp_divideby_int(fixed_point x, int n)
{
	return x / n;
};

#endif /* threads/fixed-point.h */