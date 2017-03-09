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

double* calc_xcf(double* samplesX,double* samplesY, int sample_size)
{

	int i,j,lag;
	double sumX = 0;
	double symY = 0;
	//calc mean
	for(i = 0; i < sample_size;i++)
	{
		sumX += samplesX[i];
		sumY += samplesY[i];	
	}	
	double meanX = sumX / sample_size;
	double meanY = sumY / sample_size;

	//calc sample variance, stupid to to this multiple time for each correlation, fix 		later
	

	//allocate return array
	double* acf = malloc(sizeof(double)*(2*sample_size+1));
	
	//for every lag including 0[0..lag]
	for(lag=0; lag <= sample_size; lag++)
	{	
		sum = 0;
		for(j=0;j<=sample_size-lag;j++)
		{
			sum +=(samplesX[j]-meanX)*(samplesY[j+lag]-meanY);	
		}
		acf[sample_size+lag]= sum / (sample_size);

		sum = 0;
		for(j=0;j<=sample_size+lag;j++)
		{
			sum +=(samplesY[j]-meanY)*(samplesX[j-lag]-meanX);	
		}
		acf[sample_size-lag]= sum / (sample_size);				
	}

	//calc variance
	sumX = 0;
	sumY = 0;	
	for(i = 0; i < sample_size; i++)
	{
		sumX += pow(samplesX[i]-meanX,2);
		sumY += pow(samplesY[i]-meanY,2);		
	}
	double varianceX = sumX / sample_size;
	double varianceY = sumY / sample_size;

	//handle complex denomiator	
	if(varianceX*varianceY)
	{
		
	}	


	return acf;



}

