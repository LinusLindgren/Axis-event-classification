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
#define NBR_FEATURES 41
#define MATH_PI 3.14159265358979323846


/**
 * calc_tilt:
 * @sample_A: An array containing the Y values
 * @sample_B: An array containing the X values
 * @size the length of the arrays
 * calculates the pairwise tilt values of the arrays
 * with filename acc_sample + timestamp
 * 
 * Returns: an array containing the calcuated tilt values
 */
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

/**
 * calc_sum_changes:
 * @sample_A: An array containing the sample values
 * @sample_size the length of the array
 * calculates the sum of all changes between neighbouring data points
 * 
 * Returns: the calculated sum
 */
static double calc_sum_changes(double* sample, int sample_size)
{
	//syslog (LOG_INFO,"ENTER SUM CHANGES\n");
	int i;	
	double sum = 0;
	for(i = 1; i < sample_size; i++)
	{
		sum += fabs(sample[i]-sample[i-1]);
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
	for(i=0; i <= AUTO_LAG; i++)
	{	
		sum = 0;
		for(j=0;j<sample_size-i;j++)
		{
			sum +=(samples[j]-mean)*(samples[j+i]-mean);		
		}
		acf[i]= sum / ((sample_size-1)*variance);			
	}	
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
 * @sample_size: The length of the sample.
 * @svm_model The support vector machine model used for the classification.
 * @sample_freq The frequency that the sample was recorded with.
 *
 * Returns: the features extracted from the signal.    
 */


// Extracts the accelerometer features from a sample. The number of features are stated in NBR_FEATURES.
double* extract_features(double* sampleX,double* sampleY, double* sampleZ,int sample_size,svm_linear_model_data_type* svm_model, int sample_freq)
{
	int i,index_max;

	//calc tilt values
	double* tiltXY = calc_tilt(sampleY,sampleX,sample_size);
	double* tiltXZ = calc_tilt(sampleZ,sampleX,sample_size);
	double* tiltYZ = calc_tilt(sampleZ,sampleY,sample_size);
	

	//calc mean and variance
	double* features = malloc(sizeof(double)*NBR_FEATURES);
	double meanX = calc_mean(sampleX,sample_size);
	double meanY = calc_mean(sampleY,sample_size);
	double meanZ = calc_mean(sampleZ,sample_size);

	double varianceX = calc_variance(sampleX,sample_size,meanX);
	double varianceY = calc_variance(sampleY,sample_size,meanY);
	double varianceZ = calc_variance(sampleZ,sample_size,meanZ);
	
	//auto correlation for every dimension
	 
	double acf_x[AUTO_LAG+1];
	double acf_y[AUTO_LAG+1];
	double acf_z[AUTO_LAG+1];
	calc_acf(sampleX,sample_size,meanX,varianceX,acf_x);	
	calc_acf(sampleY,sample_size,meanY,varianceY,acf_y);	
	calc_acf(sampleZ,sample_size,meanZ,varianceZ,acf_z);
	//extract features

	//acf features sum and min for every dimension
	double acf_y_min = calc_min(acf_y,AUTO_LAG+1);
	double acf_z_min = calc_min(acf_z,AUTO_LAG+1);
	double acf_z_sum = calc_sum(acf_z,AUTO_LAG+1);

	//mean and variance for acor,used for skewness and kurtosis
	double acor_meanX = calc_mean(acf_x,AUTO_LAG+1);
	double acor_meanY = calc_mean(acf_y,AUTO_LAG+1);
	double acor_meanZ = calc_mean(acf_z,AUTO_LAG+1);

	double acor_varianceX = calc_variance(acf_x,AUTO_LAG+1,acor_meanX);
	double acor_varianceY = calc_variance(acf_y,AUTO_LAG+1,acor_meanY);
	double acor_varianceZ = calc_variance(acf_z,AUTO_LAG+1,acor_meanZ);


	psdx_feature_data_type* psdx_raw[3];
	psdx_feature_data_type* psdx_tilt[3];

	


	psdx_raw[0] = extract_psdx_features(sampleX, sample_size,sample_freq);
	psdx_raw[1] = extract_psdx_features(sampleY, sample_size,sample_freq);
	psdx_raw[2] = extract_psdx_features(sampleZ, sample_size,sample_freq);
	psdx_tilt[0] = extract_psdx_features(tiltXY, sample_size,sample_freq);
	psdx_tilt[1] = extract_psdx_features(tiltXZ, sample_size,sample_freq);
	psdx_tilt[2] = extract_psdx_features(tiltYZ, sample_size,sample_freq);


	//add the features in the predefined order
	//enter acf sum features
	features[0] = acf_z_sum;
	//enter acf min features
	features[1] = acf_y_min;
	features[2] = acf_z_min;
	//enter mean features
	features[3] = meanX;
	//enter min features
	features[4] = calc_min(sampleY,sample_size);
	features[5] = calc_min(sampleZ,sample_size);
	//enter max features
	features[6] = calc_max(sampleX,&index_max,sample_size);
	features[7] = calc_max(sampleY,&index_max,sample_size);
	//enter skewness for raw data
	features[8] = calc_skewness(sampleY,sample_size,meanY,varianceY);
	features[9] = calc_skewness(sampleZ,sample_size,meanZ,varianceZ);
	//enter kurtosis for acor data
	features[10] = calc_kurtosis(acf_x,AUTO_LAG+1,acor_meanX,acor_varianceX);
	features[11] = calc_kurtosis(acf_z,AUTO_LAG+1,acor_meanZ,acor_varianceZ);
	//enter skewness for acor data
	features[12] = calc_skewness(acf_x,AUTO_LAG+1,acor_meanX,acor_varianceX);
	features[13] = calc_skewness(acf_y,AUTO_LAG+1,acor_meanY,acor_varianceY);
	features[14] = calc_skewness(acf_z,AUTO_LAG+1,acor_meanZ,acor_varianceZ);
	//enter sum changes
	features[15] = calc_sum_changes(sampleX,sample_size);
	features[16] = calc_sum_changes(sampleY,sample_size);
	features[17] = calc_sum_changes(sampleZ,sample_size);
	//add peaks distribution using bins for tilt psdx
	features[18]= psdx_tilt[0]->psdx_freq_bins[0];
	for(i = 0; i < 5; i++)
	{
		features[19+i]= psdx_tilt[2]->psdx_freq_bins[i];
	}


	features[24] = psdx_raw[0]->skewness;
	features[25] = psdx_raw[1]->skewness;
	features[26] = psdx_raw[2]->skewness;

	//add nbr peaks for psdx tilt
	features[27] = psdx_tilt[0]->nbr_freq_peaks;
	features[28] = psdx_tilt[1]->nbr_freq_peaks;
	features[29] = psdx_tilt[2]->nbr_freq_peaks;
	
	// add peaks distribution using bins for raw psdx
	features[30]= psdx_raw[0]->psdx_freq_bins[0];
	features[31]= psdx_raw[0]->psdx_freq_bins[2];
	features[32]= psdx_raw[0]->psdx_freq_bins[3];
	features[33]= psdx_raw[0]->psdx_freq_bins[4];

	features[34]= psdx_raw[1]->psdx_freq_bins[0];
	features[35]= psdx_raw[1]->psdx_freq_bins[4];

	calc_max(sampleZ,&index_max,sample_size);
	//index of max z
	features[36] = index_max;
	//mean tilt y 
	features[37] = calc_mean(tiltXZ,sample_size);
	//std y
	features[38] = sqrt(varianceY);
	//sum changes auto x y
	features[39] = calc_sum_changes(acf_x,AUTO_LAG+1);
	features[40] = calc_sum_changes(acf_y,AUTO_LAG+1);
	
	for(i = 0; i< NBR_FEATURES ; i++)
	{
		//syslog (LOG_INFO,"feature[%d]: %f\n",i,features[i]);
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



