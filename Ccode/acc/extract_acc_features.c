#include <math.h>
#include <stdlib.h>
#include "extract_acc_features.h"
#include <float.h>

#include <time.h>
#include <glib.h>
#include <glib-object.h>
#include <glib/gstdio.h>
#include <gio/gio.h>
#include <stdlib.h>
#include <event2.h>
#include <syslog.h>
#define AUTO_LAG 30
#define NBR_FEATURES 9


static double calc_mean(double* sample,int sample_size)
{
	int i;
	double sum = 0;
	for(i = 0; i < sample_size;i++)
	{
		sum += sample[i];	
	}	
	return sum / sample_size;
}

static double calc_variance(double* sample,int sample_size, double sample_mean)
{
	int i;
	double sum = 0;
	for(i = 0; i < sample_size; i++)
	{	
		sum += pow(sample[i]-sample_mean,2);
		
	}
	return sum / sample_size;
}

static double calc_min(double* sample, int sample_size)
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

static double calc_max(double* sample, int sample_size)
{
	int i;
	double max = -DBL_MAX;
	for(i = 0; i < sample_size; i++)
	{
		if(abs(sample[i])>max)
		{
			max = abs(sample[i]);
		}	
	}
	return max;
}

static double calc_sum(double* sample, int sample_size)
{
	int i;
	double sum = 0;
	for(i = 0; i < sample_size; i++)
	{
		sum += sample[i];	
	}
	return sum;
}

/**
 * calc_acf:
 *
 * Calculates auto-correlation for a sample. The amount of lag is specified by a macro.
 * 
 * @samples: signal to be auto-correlated. 
 * @sample_size: the length of the sample.
 * @mean: the mean of the sample.
 * @variance: the variance of the sample.
 *
 * Returns: the auto-correlation of the sample. Stored as described below.
 * 	[X(0) ... X(lag)]
 *           
 */

void calc_acf(double* samples, int sample_size, double mean, double variance,double* acf)
{

	int i,j;
	double sum = 0;
	//allocate return array
	//double* acf = malloc(sizeof(double)*(AUTO_LAG+1));
	
	//for every lag including 0[0..lag]
	for(i=0; i <= AUTO_LAG; i++)
	{	
		sum = 0;
		for(j=0;j<=sample_size-i;j++)
		{
			sum +=(samples[j]-mean)*(samples[j+i]-mean);	
		}
		acf[i]= sum / ((sample_size-1)*variance);			
	}	
	//return acf;
}


/**
 * calc_xcf:
 *
 * Calculates cross-correlation for samplesX and samplesY.
 * 
 * @samplesX: First signal to be cross-correlated.
 * @samplesY: Second signal to be cross-correlated.
 * @sample_size: the length of the samples.
 * @stdX: the standard deviation of signal X.
 * @stdY: the standard deviation of signal Y.
 *
 * Returns: the cross-correlation stored according to the description below
 *   [X(k)*Y(k+sample_size-1) .. X(k)*Y(k+0)  X(k+1)*Y(k) .. X(k+sample_size-1)*Y(k)]
 *  
 *     note that X(k)*Y(k+0) == X(k+0)*Y(k)
 *          
 */
void calc_xcf(double* samplesX,double* samplesY, int sample_size, double stdX, double stdY,double* xcf)
{

	
	int j,lag;
	double sum1,sum2;
	//allocate return array
	//double* xcf = malloc(sizeof(double)*(2*sample_size-1));
	
	//for every lag including 0[0..lag] calc xcf 

	//optimize? Make use of cache better?
	for(lag=0; lag < sample_size; lag++)
	{	
		
		sum1 = 0;
		sum2 = 0;
		for(j=0;j<sample_size-lag;j++)
		{
			sum1 +=(samplesX[j])*(samplesY[j+lag]);	// lag signal Y with specified lag and calculate correlation with X
			sum2 +=(samplesY[j])*(samplesX[j+lag]);	// lag signal X with specified lag and calculate correlation with Y
		}
		xcf[sample_size+lag-1]= sum1 ;
		xcf[sample_size-lag-1]= sum2 ;				
	}
	//return xcf;

}

int get_nbr_features(void)
{
	return NBR_FEATURES;
}

/**
 * extract_features:
 *
 * Extracts the accelerometer features from a sample containing X,Y and Z dimensions. The number of features is specified in the macro NBR_FEATURES
 * 
 * @sampleX: The X dimension part of the signal.
 * @sampleY: The Y dimension part of the signal. 
 * @sampleZ: The Z dimension part of the signal.
 * @sample_size: the length of the sample.
 *
 * Returns: the features extracted from the signal. Stored as follows:
 * [sum of acf in each respective dimension, minimum of acf in each respective dimension, the max of the absolute value crosscorrelation between X & Y,
the max of the absolute value crosscorrelation between X & Z, the max of the absolute value crosscorrelation between Y & Z]         
 */


// Extracts the accelerometer features from a sample. The number of features are stated in NBR_FEATURES.
double* extract_features(double* sampleX,double* sampleY, double* sampleZ,int sample_size)
{

	clock_t t1, t2;
 	
	
	t1 = clock(); 
	double* features = malloc(sizeof(double)*NBR_FEATURES);
	double meanX = calc_mean(sampleX,sample_size);
	double meanY = calc_mean(sampleY,sample_size);
	double meanZ = calc_mean(sampleZ,sample_size);

	double varianceX = calc_variance(sampleX,sample_size,meanX);
	double varianceY = calc_variance(sampleY,sample_size,meanY);
	double varianceZ = calc_variance(sampleZ,sample_size,meanZ);
	t2 = clock(); 
	float diff = ((float)(t2 - t1) / 1000000.0F ) * 1000;   
    	syslog (LOG_INFO, "mean/variance: %f",diff); 
	//auto correlation for every dimension
	t1 = clock(); 
	double acf_x[AUTO_LAG+1];
	double acf_y[AUTO_LAG+1];
	double acf_z[AUTO_LAG+1];
	calc_acf(sampleX,sample_size,meanX,varianceX,acf_x);	
	calc_acf(sampleY,sample_size,meanY,varianceY,acf_y);	
	calc_acf(sampleZ,sample_size,meanZ,varianceZ,acf_z);
	t2 = clock(); 
	diff = ((float)(t2 - t1) / 1000000.0F ) * 1000;   
    	syslog (LOG_INFO, "acf: %f",diff); 
	
	t1 = clock(); 
	double xcf_xy[2*sample_size-1];
	double xcf_xz[2*sample_size-1];
	double xcf_yz[2*sample_size-1];
	//cross correlation for every dimension pair
	calc_xcf(sampleX,sampleY,sample_size,sqrt(varianceX),sqrt(varianceY),xcf_xy);	
	calc_xcf(sampleX,sampleZ,sample_size,sqrt(varianceX),sqrt(varianceZ),xcf_xz);	
	calc_xcf(sampleY,sampleZ,sample_size,sqrt(varianceY),sqrt(varianceZ),xcf_yz);	
	t2 = clock(); 
	diff = ((float)(t2 - t1) / 1000000.0F ) * 1000;   
    	syslog (LOG_INFO, "xcf: %f",diff); 

	//extract features

	//acf features sum and min for every dimension
	double acf_x_min = calc_min(acf_x,AUTO_LAG+1);
	double acf_y_min = calc_min(acf_y,AUTO_LAG+1);
	double acf_z_min = calc_min(acf_z,AUTO_LAG+1);
	double acf_x_sum = calc_sum(acf_x,AUTO_LAG+1);
	double acf_y_sum = calc_sum(acf_y,AUTO_LAG+1);
	double acf_z_sum = calc_sum(acf_z,AUTO_LAG+1);

	//xcf features max(abs(xcf) for every dimension pair
	double xcf_xy_max = calc_max(xcf_xy,sample_size*2-1);
	double xcf_xz_max = calc_max(xcf_xz,sample_size*2-1);
	double xcf_yz_max = calc_max(xcf_yz,sample_size*2-1);
	
	//normalize using the 2-norm
	double norm = sqrt(pow(xcf_xy_max,2) + pow(xcf_xz_max,2) + pow(xcf_yz_max,2));
	xcf_xy_max = xcf_xy_max / norm;
	xcf_xz_max = xcf_xz_max / norm;
	xcf_yz_max = xcf_yz_max / norm;

	//add the features in the predefined order
	features[0] = acf_x_sum;
	features[1] = acf_y_sum;
	features[2] = acf_z_sum;
	features[3] = acf_x_min;
	features[4] = acf_y_min;
	features[5] = acf_z_min;
	features[6] = xcf_xy_max;
	features[7] = xcf_xz_max;
	features[8] = xcf_yz_max;


	//free(acf_x);
	//free(acf_y);
	//free(acf_z);
	//free(xcf_xy);
	//free(xcf_xz);
	//free(xcf_yz);
	return features;
}



