#include <stdlib.h> 
#include <stdio.h>
#include <stdint.h>
#include <math.h>

/* Test of Kahan Summation Algo */


double kahan(double input, double tosum, double times)
{
	double c = 0.0;
	double sum = input;
	double y;
	double t;
	int count;
	for(count=0; count<times; count++)
	{
		y = tosum-c;
		t = sum+y;
		c = (t-sum)-y;
		sum =  t;
	}
	
	return(sum);
}

double ssa(double input, double tosum, double times)
{
	
	
	//return(ssaSum);
}

double lsa(double input, double tosum, double times)
{
	

	//return(lsaSum);
}

double kahanSum()
{
	// Preforms Kahan Summation
	double s;
	long int i;
	s = 0.0;
	for (i = 0; i < 1e9; i++) 
	{
		s += M_PI;
	}

	printf ("(float)pi = %.16g\n", M_PI);
	printf ("1e9 additions of pi = %.16g, error = %.6g\n", s, 1e9*M_PI-s);
	printf ("1e9 Kahan additions of pi = %.16g, error = %.6g\n", kahan(0.0,M_PI, 1e9), kahan(0.0,M_PI, 1e9)-1e9*M_PI);
	
	return 0;
	
}

double ssaSum()
{
	
	
	return 0;
}

double lsaSum()
{
	
	
	return 0;	
}

int main() 
{	
	kahanSum();
	
}


