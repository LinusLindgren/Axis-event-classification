#include <math.h>
#include <stdlib.h>
#include "calc_acf.h"
double* calc_acf(double* samples, int sample_size,int lag)
{

	int i,j;
	double sum = 0;
	//calc mean
	for(i = 0; i < sample_size;i++)
	{
		sum += samples[i];	
	}	
	double mean = sum / sample_size;
	
	//calc sample variance
	sum = 0;
	for(i = 0; i < sample_size; i++)
	{
		sum += pow(samples[i]-mean,2);	
	}
	double variance = sum / sample_size;
	

	//allocate return array
	double* acf = malloc(sizeof(double)*(lag+1));
	
	//for every lag including 0[0..lag]
	for(i=0; i <= lag; i++)
	{	
		sum = 0;
		for(j=0;j<=sample_size-i;j++)
		{
			sum +=(samples[j]-mean)*(samples[j+i]-mean);	
		}
		acf[i]= sum / ((sample_size-1)*variance);			
	}	
	return acf;
}

