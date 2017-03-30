#include <math.h>
#include <stdlib.h>
#include <float.h>
#include <stdio.h>
#define MATH_PI 3.14159265358979323846

void calc_periodogram(double* sample,int sample_size,int freq, double* periodogram)
{

	double real, imag, abs;
	int i,j;
	for(i = 0; i < sample_size/2+1; i++)
	{
		real = 0;
		imag = 0;
		//fft
		for(j= 0; j < sample_size;j++)
		{
			real += sample[j]* cos(-2*MATH_PI/sample_size * (j-1) * (i-1));
			imag += sample[j] * sin(-2*MATH_PI/sample_size * (j-1) * (i-1));
		}
		//psdx = (1/(Fs*N)) * abs(xdft).^2;
		abs = sqrt(real*real+imag*imag);
		periodogram[i] = abs * abs / freq / sample_size;
	}
	
	//psdx(2:end-1) = 2*psdx(2:end-1);
	for(i = 1 ; i < sample_size/2; i++)
	{
		periodogram[i] *= 2;
	}

	for(i = 0 ; i < sample_size/2+1; i++)
	{
		periodogram[i] = 10*log10(periodogram[i]);
	}

	
}


void main()
{
	int i;
	int sample_size = 512;
	//double sample[] ={123,45,756,12,567,213,765,34,23,321,3,12,546,23,76};
	double sample[sample_size];
	for(i = 0; i < sample_size ; i++)
	{
		sample[i] = rand();
	}
	
	double periodogram[sample_size/2+1];
	clock_t t1, t2;
 	t1 = clock(); 
	calc_periodogram(sample, sample_size, 200, periodogram);
	t2 = clock(); 
	double diff = ((float)(t2 - t1) / 1000000.0F ) * 1000;   
    	printf("psdx: %f \n",diff); 
	
	for(i = 0; i < sample_size/2+1; i++)
	{
		printf("periodogram[%d]: %f \n", i, periodogram[i]);
	}
}
