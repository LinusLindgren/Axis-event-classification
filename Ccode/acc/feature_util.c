#include <math.h>
#include <stdlib.h>
#include <float.h>
#include "feature_util.h"

#include <time.h>
#include <glib.h>
#include <glib-object.h>
#include <glib/gstdio.h>
#include <gio/gio.h>
#include <stdlib.h>
#include <event2.h>
#include <syslog.h>


double calc_skewness(double* sample, int sample_size,double mean, double variance)
{

	double variance_skewness = variance*(sample_size-1) / sample_size;
	int i;	
	// (1/n) * sum 1->n with (x-mean)^3
	double nominator = 0;
	for(i = 0; i < sample_size; i++)
	{ 
		nominator+= (sample[i] - mean) * (sample[i] - mean) * (sample[i] - mean); 
	}
	nominator /= sample_size;
	double denominator = pow(variance_skewness,1.5);
	//double denominator_test = variance * sqrt(variance);
	double res = nominator / denominator;
	return res;
	
}

double calc_kurtosis(double* sample, int sample_size, double mean,double variance)
{
	double variance_kurtosis = variance*(sample_size-1) / sample_size;
	int i;	
	// (1/n) * sum 1->n with (x-mean)^4
	double nominator = 0;
	for(i = 0; i < sample_size; i++)
	{
		nominator+= (sample[i] - mean) * (sample[i] - mean) * (sample[i] - mean) * (sample[i] - mean); 
	}
	nominator /= sample_size;
	double denominator = pow(variance_kurtosis,2);
	return nominator / denominator;
}

double calc_mean(double* sample,int sample_size)
{
	int i;
	double sum = 0;
	for(i = 0; i < sample_size;i++)
	{
		sum += sample[i];	
	}	
	return sum / sample_size;
}

double calc_variance(double* sample,int sample_size, double sample_mean)
{
	int i;
	double sum = 0;
	for(i = 0; i < sample_size; i++)
	{	
		sum += (sample[i]-sample_mean) * (sample[i]-sample_mean);
		
	}
	//matlab normalizes with n-1
	return sum / (sample_size-1);
}

double calc_min(double* sample, int sample_size)
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

double calc_max(double* sample, int* index_of_max_value, int sample_size)
{
	int i;
	double max = -DBL_MAX;
	
	for(i = 0; i < sample_size; i++)
	{
		if(sample[i] > max)
		{
			max = sample[i];
			// +1 due to corresponding matlab index
			*index_of_max_value = i+1;
		}	
	}
	return max;
}

double calc_sum(double* sample, int sample_size)
{
	int i;
	double sum = 0;
	for(i = 0; i < sample_size; i++)
	{
		sum += sample[i];	
	}
	return sum;
}
