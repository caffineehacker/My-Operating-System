#pragma

typedef struct 
{
	double real;
	double i;
} _Complex;

#define complex _Complex
#define _Complex_I 0 /* TODO: Define as I(maginary) */

#ifndef _Imaginary_I
#define I _Complex_I
#else
#define I _Imaginary_I
#endif

extern complex cacos(complex z);
/*extern float complex cacosf(float complex z);
extern long double complex cacosl(long double complex z);
*/

extern complex casin(complex z);
/*extern float complex casinf(float complex z);
long double complex casinl(long double complex z);*/

extern complex catan(complex z);
/*float complex catanf(float complex z);
long double complex catanl(long double complex z);*/

extern complex ccos(complex z);
/*float complex ccosf(float complex z);
long double complex ccosl(long double complex z);*/

extern complex csin(complex z);
/*float complex csinf(float complex z);
long double complex csinl(long double complex z);*/

extern complex ctan(complex z);
/*float complex ctanf(float complex z);
long double complex ctanl(long double complex z);*/

extern complex cacosh(complex z);
/*float complex cacoshf(float complex z);
long double complex cacoshl(long double complex z);*/

extern complex casinh(complex z);
/*float complex casinhf(float complex z);
long double complex casinhl(long double complex z);*/

extern complex catanh(complex z);
/*float complex catanhf(float complex z);
long double complex catanhl(long double complex z);*/

extern complex ccosh(complex z);
/*float complex ccoshf(float complex z);
long double complex ccoshl(long double complex z);*/

extern complex csinh(complex z);
/*float complex csinhf(float complex z);
long double complex csinhl(long double complex z);*/

extern complex ctanh(complex z);
/*float complex ctanhf(float complex z);
long double complex ctanhl(long double complex z);*/

extern complex cexp(complex z);

extern complex clog(complex z);

extern double cabs(complex z);

extern complex cpow(complex x, complex y);

extern complex csqrt(complex z);

extern double carg(complex z);

extern double cimag(complex z);

extern complex conj(complex z);

extern complex cproj(complex z);

extern double creal(complex z);
