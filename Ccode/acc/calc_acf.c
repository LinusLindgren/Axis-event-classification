#include <math.h>
#include <stdlib.h>
#include "calc_acf.h"
#include <float.h>

#define lag 30
#define NBR_FEATURES 10

double mean(double* sample,int sample_size)
{
	int i;
	double sum = 0;
	//calc mean
	for(i = 0; i < sample_size;i++)
	{
		sum += samples[i];	
	}	
	return sum / sample_size;
}

double calc_variance(double* sample,int sample_size, double mean)
{
	int i;
	double sum = 0;
	for(i = 0; i < sample_size; i++)
	{
		sum += pow(samples[i]-mean,2);	
	}
	return sum / sample_size;
}

double min(double* sample, int sample_size)
{
	int i;
	double min = DBL_MAX;
	for(i = 0; i < sample_size; i++)
	{
		if(sample[i]<min)
		{
			min = sample[i];
		}	
	}
	return min;
}

double sum(double* sample, int sample_size)
{
	int i;
	double sum = 0;
	for(i = 0; i < sample_size; i++)
	{
		sum += sample[i];	
	}
	return sum;
}

double* calc_acf(double* samples, int sample_size,int lag, double mean, double variance)
{

	int i,j;
	double sum = 0;
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



double* calc_xcf(double* samplesX,double* samplesY, int sample_size, double meanX, double meanY, double varianceX, double varianceY)
{

	int i,j,lag;
	//allocate return array
	double* xcf = malloc(sizeof(double)*(2*sample_size+1));
	
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


	//handle complex denomiator	
	if(varianceX*varianceY)
	{
		
	}	


	return xcf;

}





double* extract_features(double* sampleX,double* sampleY, double* sampleZ,int sample_size)
{

	double* features = malloc(sizeof(double)*NBR_FEATURES);
	double meanX = mean(sampleX,sample_size);
	double meanY = mean(sampleY,sample_size);
	double meanZ = mean(sampleZ,sample_size);

	double varianceX = calc_variance(sampleX,sample_size,meanX);
	double varianceY = calc_variance(sampleY,sample_size,meanY);
	double varianceZ = calc_variance(sampleZ,sample_size,meanZ);

	double* acf_x = calc_acf(sampleX,sample_size,lag,meanX,varianceX);	
	double* acf_y = calc_acf(sampleY,sample_size,lag,meanY,varianceY);	
	double* acf_z = calc_acf(sampleZ,sample_size,lag,meanZ,varianceZ);

	double acf_x_min = min(acf_x,lag+1);
	double acf_y_min = min(acf_y,lag+1);
	double acf_z_min = min(acf_z,lag+1);
	double acf_x_sum = sum(acf_x,lag+1);
	double acf_y_sum = sum(acf_y,lag+1);
	double acf_z_sum = sum(acf_z,lag+1);

	double* xcf_xy = calc_xcf(sampleX,sampleY,sample_size,meanX,meanY.varianceX,varianceY);	
	double* xcf_xz = calc_xcf(sampleX,sampleZ,sample_size,meanX,meanZ.varianceX,varianceZ);	
	double* xcf_yz = calc_xcf(sampleY,sampleZ,sample_size,meanY,meanZ.varianceY,varianceZ);	


	//extract features

	free(acf_x);
	free(acf_y);
	free(acf_z);
	free(xcf_xy);
	free(xcf_xz);
	free(xcf_yz);
	return features;
}


