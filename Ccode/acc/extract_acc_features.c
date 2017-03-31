#include <math.h>
#include <stdlib.h>
#include "extract_acc_features.h"
#include <float.h>
#include "kiss_fft.h"
#include "calc_periodogram_features.h"
#include "feature_util.h"

#include <time.h>
#include <glib.h>
#include <glib-object.h>
#include <glib/gstdio.h>
#include <gio/gio.h>
#include <stdlib.h>
#include <event2.h>
#include <syslog.h>

#define AUTO_LAG 30
#define NBR_FEATURES 59
#define MATH_PI 3.14159265358979323846

static double* calc_tilt(double* sample_A, double* sample_B, int size)
{
	double* tilt_values = malloc(size*sizeof(double));
	int i;
	for(i = 0; i < size; i++)
	{
		tilt_values[i] = atan2(sample_A[i],sample_B[i]);
	}	
	return tilt_values;
}	


static double calc_sum_changes(double* sample, int sample_size)
{
	int i;	
	double sum = 0;
	for(i = 1; i < sample_size; i++)
	{
		sum += abs(sample[i]-sample[i-1]);
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
	//double temp[sample_size+1];
	//for every lag including 0[0..lag]
	/*for(i=0; i < sample_size;i++)
	{
		temp[i] = samples[i]-mean;
	}
	*/
	for(i=0; i <= AUTO_LAG; i++)
	{	
		sum = 0;
		//FIX BUG LATER < and not <= fix and compare feature
		for(j=0;j<sample_size-i;j++)
		{
			sum +=(samples[j]-mean)*(samples[j+i]-mean);
			//sum +=temp[i]*temp[i+j];		
		}
		//syslog (LOG_INFO, "%f",(samples[sample_size-i]-mean)*(samples[sample_size-i+i]-mean));
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
//varför skickar vi med std? kolla
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
double* extract_features(double* sampleX,double* sampleY, double* sampleZ,int sample_size,svm_linear_model_data_type* svm_model, int sample_freq)
{
	int i;
	clock_t t1, t2;
 	float diff;

	//calc tilt values
	t1 = clock(); 
	double* tiltXY = calc_tilt(sampleX,sampleY,sample_size);
	double* tiltXZ = calc_tilt(sampleX,sampleY,sample_size);
	double* tiltYZ = calc_tilt(sampleX,sampleY,sample_size);

	t2 = clock(); 
	diff = ((float)(t2 - t1) / 1000000.0F ) * 1000;   
    	syslog (LOG_INFO, "calc tilt: %f",diff);
	
	//calc mean and variance
	t1 = clock(); 
	double* features = malloc(sizeof(double)*NBR_FEATURES);
	double meanX = calc_mean(sampleX,sample_size);
	double meanY = calc_mean(sampleY,sample_size);
	double meanZ = calc_mean(sampleZ,sample_size);

	double varianceX = calc_variance(sampleX,sample_size,meanX);
	double varianceY = calc_variance(sampleY,sample_size,meanY);
	double varianceZ = calc_variance(sampleZ,sample_size,meanZ);
	t2 = clock(); 
	diff = ((float)(t2 - t1) / 1000000.0F ) * 1000;   
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
	
	//calc cross correlation
	t1 = clock(); 
	double xcf_xz[2*sample_size-1];
	calc_xcf(sampleX,sampleZ,sample_size,sqrt(varianceX),sqrt(varianceZ),xcf_xz);	
	t2 = clock(); 
	diff = ((float)(t2 - t1) / 1000000.0F ) * 1000;   
    	syslog (LOG_INFO, "xcf: %f",diff); 

	//extract features

	//acf features sum and min for every dimension
	t1 = clock(); 
	double acf_x_min = calc_min(acf_x,AUTO_LAG+1);
	double acf_y_min = calc_min(acf_y,AUTO_LAG+1);
	double acf_z_min = calc_min(acf_z,AUTO_LAG+1);
	double acf_x_sum = calc_sum(acf_x,AUTO_LAG+1);
	double acf_y_sum = calc_sum(acf_y,AUTO_LAG+1);
	double acf_z_sum = calc_sum(acf_z,AUTO_LAG+1);
	t2 = clock(); 
	diff = ((float)(t2 - t1) / 1000000.0F ) * 1000;   
    	syslog (LOG_INFO, "acf min/sum: %f",diff); 


	//mean and variance for acor,used for skewness and kurtosis
	t1 = clock(); 
	double acor_meanX = calc_mean(acf_x,AUTO_LAG+1);
	double acor_meanY = calc_mean(acf_y,AUTO_LAG+1);
	double acor_meanZ = calc_mean(acf_z,AUTO_LAG+1);

	double acor_varianceX = calc_variance(acf_x,AUTO_LAG+1,acor_meanX);
	double acor_varianceY = calc_variance(acf_y,AUTO_LAG+1,acor_meanY);
	double acor_varianceZ = calc_variance(acf_z,AUTO_LAG+1,acor_meanZ);
	t2 = clock(); 
	diff = ((float)(t2 - t1) / 1000000.0F ) * 1000;   
    	syslog (LOG_INFO, "acf mean: %f",diff); 
	
	double xcf_xz_max = calc_max(xcf_xz,sample_size*2-1);





	t1 = clock(); 

	psdx_feature_data_type* psdx_raw[3];
	psdx_feature_data_type* psdx_tilt[3];


	psdx_raw[0] = extract_psdx_features(sampleX, sample_size,sample_freq);
	psdx_raw[1] = extract_psdx_features(sampleY, sample_size,sample_freq);
	psdx_raw[2] = extract_psdx_features(sampleZ, sample_size,sample_freq);
	psdx_tilt[0] = extract_psdx_features(tiltXY, sample_size,sample_freq);
	psdx_tilt[1] = extract_psdx_features(tiltXZ, sample_size,sample_freq);
	psdx_tilt[2] = extract_psdx_features(tiltYZ, sample_size,sample_freq);



	t2 = clock(); 
	diff = ((float)(t2 - t1) / 1000000.0F ) * 1000;   
    	syslog (LOG_INFO, "psdx features: %f",diff);

	
	//add the features in the predefined order
	
	//enter acf sum features
	features[0] = acf_x_sum;
	features[1] = acf_y_sum;
	features[2] = acf_z_sum;
	//enter acf min features
	features[3] = acf_x_min;
	features[4] = acf_y_min;
	features[5] = acf_z_min;
	//enter cross corr features
	//features[6] = xcf_xy_max;
	features[6] = xcf_xz_max;
	//features[8] = xcf_yz_max;
	//enter mean features
	features[7] = meanX;
	features[8] = meanY;
	features[9] = meanZ;
	//enter min features
	t1 = clock(); 
	features[10] = calc_min(sampleX,sample_size);
	features[11] = calc_min(sampleY,sample_size);
	features[12] = calc_min(sampleZ,sample_size);
	//enter max features
	features[13] = calc_max(sampleX,sample_size);
	features[14] = calc_max(sampleY,sample_size);
	features[15] = calc_max(sampleZ,sample_size);
	t2 = clock(); 
	diff = ((float)(t2 - t1) / 1000000.0F ) * 1000;   
    	syslog (LOG_INFO, "min/max raw: %f",diff); 
	//möjlighet att optimera, dumt att beräka moment flera gånger, men bör vara marginell skillnad
	//enter kurtosis for raw data
	t1 = clock(); 
	//enter skewness for raw data
	features[16] = calc_skewness(sampleX,sample_size,meanX,varianceX);
	features[17] = calc_skewness(sampleY,sample_size,meanY,varianceY);
	features[18] = calc_skewness(sampleZ,sample_size,meanZ,varianceZ);
	//enter kurtosis for acor data
	features[19] = calc_kurtosis(acf_x,AUTO_LAG+1,acor_meanX,acor_varianceX);
	features[20] = calc_kurtosis(acf_y,AUTO_LAG+1,acor_meanY,acor_varianceY);
	features[21] = calc_kurtosis(acf_z,AUTO_LAG+1,acor_meanZ,acor_varianceZ);
	//enter skewness for acor data
	features[22] = calc_skewness(acf_x,AUTO_LAG+1,acor_meanX,acor_varianceX);
	features[23] = calc_skewness(acf_y,AUTO_LAG+1,acor_meanY,acor_varianceY);
	features[24] = calc_skewness(acf_z,AUTO_LAG+1,acor_meanZ,acor_varianceZ);
	t2 = clock(); 
	diff = ((float)(t2 - t1) / 1000000.0F ) * 1000;   
    	syslog (LOG_INFO, "kurtosis/skewness for acf/raw: %f",diff); 
	//enter sum changes
	t1 = clock(); 
	features[25] = calc_sum_changes(sampleX,sample_size);
	features[26] = calc_sum_changes(sampleY,sample_size);
	features[27] = calc_sum_changes(sampleZ,sample_size);
	t2 = clock(); 
	diff = ((float)(t2 - t1) / 1000000.0F ) * 1000;   
    	syslog (LOG_INFO, "sum/mean changes: %f",diff); 
	// enter sum changes mean, probably dumb, remove later

	//enter freq bin for psdx tilt
	for(i = 0; i < 5; i++)
	{
		//features[28+i]= psdx_freq_bins_tilt_xy[i];
		features[28+i]= psdx_tilt[0]->psdx_freq_bins[i];
	}
	for(i = 0; i < 5; i++)
	{
		features[33+i]= psdx_tilt[1]->psdx_freq_bins[i];
	}
	for(i = 0; i < 5; i++)
	{
		features[38+i]= psdx_tilt[2]->psdx_freq_bins[i];
	}

	//calc skewness for psdx
	t1 = clock(); 
	double psdx_x_mean = calc_mean(psdx_raw[0]->psdx,psdx_raw[0]->size);
	double psdx_x_variance = calc_variance(psdx_raw[0]->psdx,psdx_raw[0]->size,psdx_x_mean);
	features[43] = calc_skewness(psdx_raw[0]->psdx,psdx_raw[0]->size,psdx_x_mean,psdx_x_variance);

	double psdx_y_mean = calc_mean(psdx_raw[1]->psdx,psdx_raw[1]->size);
	double psdx_y_variance = calc_variance(psdx_raw[1]->psdx,psdx_raw[1]->size,psdx_y_mean);
	features[44] = calc_skewness(psdx_raw[1]->psdx,psdx_raw[1]->size,psdx_y_mean,psdx_y_variance);

	double psdx_z_mean = calc_mean(psdx_raw[2]->psdx,psdx_raw[2]->size);
	double psdx_z_variance = calc_variance(psdx_raw[2]->psdx,psdx_raw[2]->size,psdx_z_mean);
	features[45] = calc_skewness(psdx_raw[2]->psdx,psdx_raw[2]->size,psdx_z_mean,psdx_z_variance);
	t2 = clock(); 
	diff = ((float)(t2 - t1) / 1000000.0F ) * 1000;   
    	syslog (LOG_INFO, "skewness for psdx: %f",diff); 

	features[46] = psdx_tilt[0]->nbr_freq_peaks;
	features[47] = psdx_tilt[1]->nbr_freq_peaks;
	features[48] = psdx_tilt[2]->nbr_freq_peaks;
	
	for(i = 0; i < 5; i++)
	{
		features[49+i]= psdx_raw[0]->psdx_freq_bins[i];
	}
	for(i = 0; i < 5; i++)
	{
		features[54+i]= psdx_raw[1]->psdx_freq_bins[i];
	}	
	
	for(i = 0; i< NBR_FEATURES ; i++)
	{
		
		features[i] = (features[i]-svm_model->features_mean[i]) / svm_model->features_std[i];
		if(isnan(features[i]))
		{
			syslog (LOG_INFO, "-- CAUGHT NAN -- feature[%d]: %f",i,features[i]);
		}
		
		
		 
	}

	free(tiltXY);
	free(tiltXZ);
	free(tiltYZ);

	free_psdx_feature_data_type(psdx_raw[0]);
	free_psdx_feature_data_type(psdx_raw[1]);
	free_psdx_feature_data_type(psdx_raw[2]);
	free_psdx_feature_data_type(psdx_tilt[0]);
	free_psdx_feature_data_type(psdx_tilt[1]);
	free_psdx_feature_data_type(psdx_tilt[2]);


	return features;
}



