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
	//calc mean
	for(i = 0; i < sample_size;i++)
	{
		sum += sample[i];	
	}	
	return sum / sample_size;
}

/*
static double calc_norm2 (double* sample, int sample_size)
{
	int i;
	double sum = 0;
	for(i = 0; i < sample_size; i++)
	{
		//syslog (LOG_INFO, "sum %f current value %f", sum, sample[i]);
		sum += pow(sample[i],2);
	}
	return sqrt(sum);
}
*/
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

double* calc_acf(double* samples, int sample_size, double mean, double variance)
{

	int i,j;
	double sum = 0;
	//allocate return array
	double* acf = malloc(sizeof(double)*(AUTO_LAG+1));
	
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
	return acf;
}



double* calc_xcf(double* samplesX,double* samplesY, int sample_size, double stdX, double stdY)
{

	int j,lag;
	double sum;
	//allocate return array
	double* xcf = malloc(sizeof(double)*(2*sample_size-1));
	
	//for every lag including 0[0..lag] calc xcf 
	for(lag=0; lag < sample_size; lag++)
	{	
		sum = 0;
		for(j=0;j<sample_size-lag;j++)
		{
			sum +=(samplesX[j])*(samplesY[j+lag]);	
		}
		xcf[sample_size+lag-1]= sum ;
		/// (sample_size * stdX * stdY);

		sum = 0;
		for(j=0;j<sample_size-lag;j++)
		{
			sum +=(samplesY[j])*(samplesX[j+lag]);	
		}
		
		xcf[sample_size-lag-1]= sum ;
		/// (sample_size * stdX * stdY);				
	}
	//syslog (LOG_INFO, "denominator %f", (sample_size * stdX * stdY));
	//double norm =calc_norm2(xcf,sample_size*2-1);
	//syslog (LOG_INFO, "norm %f", norm);
	//for(i = 0; i < sample_size*2-1;i++)
	//{
	//	xcf[i]=xcf[i]/norm;
	//}
	return xcf;

}




// Extracts the accelerometer features from a sample. The number of features are stated in NBR_FEATURES.
double* extract_features(double* sampleX,double* sampleY, double* sampleZ,int sample_size)
{

	
 
	
	double* features = malloc(sizeof(double)*NBR_FEATURES);
	double meanX = calc_mean(sampleX,sample_size);
	double meanY = calc_mean(sampleY,sample_size);
	double meanZ = calc_mean(sampleZ,sample_size);

	double varianceX = calc_variance(sampleX,sample_size,meanX);
	double varianceY = calc_variance(sampleY,sample_size,meanY);
	double varianceZ = calc_variance(sampleZ,sample_size,meanZ);

	double* acf_x = calc_acf(sampleX,sample_size,meanX,varianceX);	
	double* acf_y = calc_acf(sampleY,sample_size,meanY,varianceY);	
	double* acf_z = calc_acf(sampleZ,sample_size,meanZ,varianceZ);



	double* xcf_xy = calc_xcf(sampleX,sampleY,sample_size,sqrt(varianceX),sqrt(varianceY));	
	double* xcf_xz = calc_xcf(sampleX,sampleZ,sample_size,sqrt(varianceX),sqrt(varianceZ));	
	double* xcf_yz = calc_xcf(sampleY,sampleZ,sample_size,sqrt(varianceY),sqrt(varianceZ));	


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
	
	double norm = sqrt(pow(xcf_xy_max,2) + pow(xcf_xz_max,2) + pow(xcf_yz_max,2));
	xcf_xy_max = xcf_xy_max / norm;
	xcf_xz_max = xcf_xz_max / norm;
	xcf_yz_max = xcf_yz_max / norm;
	features[0] = acf_x_sum;
	features[1] = acf_y_sum;
	features[2] = acf_z_sum;
	features[3] = acf_x_min;
	features[4] = acf_y_min;
	features[5] = acf_z_min;
	features[6] = xcf_xy_max;
	features[7] = xcf_xz_max;
	features[8] = xcf_yz_max;

	//temp prints
	
	int i;
	char buffer[32]; // The filename buffer.
    	// Put "file" then k then ".txt" in to filename.
    	snprintf(buffer, sizeof(char) * 32, "xcf_xy_of_sample%i", (int)time(NULL));
	FILE* filename = g_fopen (buffer,"w");
	for(i=0;i<sample_size*2-1; i++){
		g_fprintf(filename,"%f\n",xcf_xy[i]);
	}
	fclose(filename);

	snprintf(buffer, sizeof(char) * 32, "xcf_xzof_sample%i", (int)time(NULL));
	filename = g_fopen (buffer,"w");
	for(i=0;i<sample_size*2-1; i++){
		g_fprintf(filename,"%f\n",xcf_xz[i]);
	}
	fclose(filename);

	snprintf(buffer, sizeof(char) * 32, "xcf_yz_of_sample%i", (int)time(NULL));
	filename = g_fopen (buffer,"w");
	for(i=0;i<sample_size*2-1; i++){
		g_fprintf(filename,"%f\n",xcf_yz[i]);
	}
	fclose(filename);
	
	// end temp
	free(acf_x);
	free(acf_y);
	free(acf_z);
	free(xcf_xy);
	free(xcf_xz);
	free(xcf_yz);
	return features;
}



