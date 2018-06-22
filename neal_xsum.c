/*
Exact Summation of floating point numbers via small super accumulators

written by armaan kohli

based on the work of Radford M. Neal

*/

#include <limits.h>
#include <stdio.h>
#include <float.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// constants
static const uint64_t mantissa_mask       = 0x000FFFFFFFFFFFFFllu;
static const uint64_t mantissa_impliedBit = 0x0010000000000000llu;
static const uint64_t expo_mask           = 0x7FF0000000000000llu;
static const uint64_t expo_norm           = 0x3FE0000000000000llu;
static const uint64_t sign_mask           = 0x8000000000000000llu;
static const int expo_NaN		  = 0x07FF;
static const int expo_Bias 	          = 1023;
static const int numbits_acc 		  = 64;
static const int numbits_mantissa 	  = 52;
static const int numbits_low_mantissa     = 32;
static const int numbits_expo 		  = 11;
static const int numbits_low_expo	  = 5;
static const int numbits_high_expo	  = 6;
static const uint64_t low_mantissa_mask	  = 0x00000000FFFFFFFFL;
static const uint64_t low_expo_mask 	  = 0x1F;
static const uint64_t high_expo_mask 	  = 0x3F;

typedef enum {false, true} bool; // huh, didn't know that C doesn't have a default boolean data type

// function that preforms kahan summation
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

// utility function to get double in terms of mantissa, signed bit and exponent 
double expnot(double number, int *exp) 
{
	union 
	{
		double d;
		uint64_t u;
	} x = { number };
	uint64_t mantissa = x.u & mantissa_mask;
	int expo = (x.u & expo_mask) >> 52;

	if (expo == expo_NaN) 
	{  // Behavior for Infinity and NaN is unspecified.
		*exp = 0;
	return number;
	}
	if (expo > 0) 
	{
		mantissa |= mantissa_impliedBit;  // This line is illustrative, not needed.
		expo -= expo_Bias;
	}
	else if (mantissa == 0) 
	{
		*exp = 0;
		return number;  // Do not return 0.0 as that does not preserve -0.0
	}
	else 
	{
	// de-normal or sub-normal numbers
		expo = 1 - expo_Bias;  // Bias different when biased exponent is 0
		while (mantissa < mantissa_impliedBit) 
		{
			mantissa <<= 1;
			expo--;
		}
	}
	*exp = expo + 1;
	mantissa &= ~mantissa_impliedBit;
	x.u = (x.u & sign_mask) | expo_norm | mantissa;
	return x.d;
}

// conversion of a long to a binary representation 
char* long_to_binary(unsigned long n)
{
        static char c[65];
        c[0] = '\0';

        unsigned long val;
        for (val = 1UL << (sizeof(unsigned long)*8-1); val > 0; val >>= 1)
        {   
            strcat(c, ((n & val) == val) ? "1" : "0");
        }
        return c;
}

long ssachunks[67] = {};

// function to add a double to a small accumulator
double add_double(long value)
{
	long binvalue = (long) long_to_binary(value); // need to convert to binary
	long mantissa = binvalue & mantissa_mask;
	int expnt = (binvalue >> numbits_mantissa) & expo_mask;
	
	if(expnt != 0 && expnt !=2047)
	{
		//normalized value
		mantissa |= (1L << numbits_mantissa); 
	}
	else if(expnt == 0)
	{
		//denomalized or zero
		if(mantissa == 0)
		{
			 return 0; //zero
		}
		expnt = 1;
	}
	else
	{
		//Inf or NaN - exit program 
		printf("Inf or NaN");
		exit;
	}
	
	int low_expnt = expnt & low_mantissa_mask;
	int high_expnt = expnt >> numbits_low_expo;
	long low_mantissa = (mantissa << low_expnt) & low_mantissa_mask;
	long high_mantissa = mantissa >> (numbits_low_mantissa - low_expnt);
	
	if(binvalue < 0)
	{
		low_mantissa = -low_mantissa;
		high_mantissa = -high_mantissa;
	}
	
	// detect overflow and propogate carry bit to higher order mantissa
	// overflow happens when two numbers hvae the same sign and sum has opposite sign
	long sum = ssachunks[high_expnt] + low_mantissa;
	bool overflow_test = (ssachunks[high_expnt] ^ low_mantissa) >= 0 && (low_mantissa ^ sum) < 0;
	int temp_expnt = high_expnt;
	ssachunks[temp_expnt++] = sum;
	while(overflow_test && temp_expnt < sizeof ssachunks)
	{
		overflow_test = ssachunks[temp_expnt] >> numbits_low_mantissa == 0xFFFFFFFFL;
		ssachunks[temp_expnt] += ssachunks[temp_expnt -1] < 0 ? (1L << 32) : -(1L << 32);
		temp_expnt++;
	}
	
	high_expnt++;
	sum = ssachunks[high_expnt] + high_mantissa;
	overflow_test= (ssachunks[high_expnt] ^ low_mantissa) >= 0 && (low_mantissa ^ sum) < 0;
	temp_expnt = high_expnt;
	while(overflow_test && temp_expnt < sizeof ssachunks)
	{
		overflow_test = ssachunks[high_expnt] >> numbits_low_mantissa == 0xFFFFFFFFL;
		ssachunks[temp_expnt] += ssachunks[temp_expnt -1] < 0 ? (1L << 32) : -(1L << 32);
		temp_expnt++;
	}
	
	return sum;
}

void test()
{
	double result = 1023;
	for(int i = 0; i<100; i++)
	{
		double result = add_double(result);
	}
	printf("Result = %s\n", long_to_binary(result));
}

int main()
{
	
	test();
	
	return(0);
}
