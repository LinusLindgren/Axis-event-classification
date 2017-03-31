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
	int i;	
	// (1/n) * sum 1->n with (x-mean)^3
	double nominator = 0;
	// (  sqrt(  (1/n) * sum 1->n with (x-mean)^2  )  )^2
	//double denominator = 0;
	for(i = 0; i < sample_size; i++)
	{
		//nominator+= pow(sample[i] - mean,3); 
		nominator+= (sample[i] - mean) * (sample[i] - mean) * (sample[i] - mean); 
		//denominator+= pow(sample[i] - mean,2); 
	}
	nominator /= sample_size;
	//denominator /= sample_size;
	//denominator = sqrt(denominator);
	double denominator = pow(variance,1.5);
	return nominator / denominator;
	
}

double calc_kurtosis(double* sample, int sample_size, double mean,double variance)
{
	
	int i;	
	// (1/n) * sum 1->n with (x-mean)^4
	double nominator = 0;
	// ((1/n) * sum 1->n with (x-mean)^2)^2
	//double denominator = 0;
	for(i = 0; i < sample_size; i++)
	{
		//nominator+= pow(sample[i] - mean,4); 
		nominator+= (sample[i] - mean) * (sample[i] - mean) * (sample[i] - mean) * (sample[i] - mean);
		//denominator+= pow(sample[i] - mean,2); 
	}
	nominator /= sample_size;
	//denominator /= sample_size;
	double denominator = pow(variance,2);
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
	//double sum_temp = 0;
	for(i = 0; i < sample_size; i++)
	{	
		//sum += pow(sample[i]-sample_mean,2);
		sum += (sample[i]-sample_mean) * (sample[i]-sample_mean);
		
	}

	//syslog (LOG_INFO, "sum: %f sumtemp %f \n",sum, sum_temp);
	
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

double calc_max(double* sample, int sample_size)
{
	int i;
	double max = DBL_MIN;
	for(i = 0; i < sample_size; i++)
	{
		if(sample[i] > max)
		{
			max = abs(sample[i]);
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
