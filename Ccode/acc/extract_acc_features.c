#include <math.h>
#include <stdlib.h>
#include "extract_acc_features.h"
#include <float.h>
#include "kiss_fft.h"

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
#define PEAK_DECI_THRESHOLD 6
#define PEAK_NBR_BINS(size) ((size/BIN_SIZE) +1)
#define BIN_SIZE 30

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


static double* calculate_spectrum(double* window, int size,int freq)
{
  //TODO: sanity check size before allocation.
  int i;
  double *spectrum = (double *) malloc((size/2+1)*sizeof(double));
  kiss_fft_cpx *inp_data = (kiss_fft_cpx *)malloc(size*sizeof(kiss_fft_cpx));
  kiss_fft_cpx *fft_out = (kiss_fft_cpx *)malloc(size*sizeof(kiss_fft_cpx));
  kiss_fft_cfg kiss_fft_state;
  double coeff;

  // Prepare the data. Only real parts are present. Complex part is 0.
  for (i = 0; i < size; i++) {
    inp_data[i].r = window[i];
    inp_data[i].i = 0.0f;
  }

  // Compute the FFT
  kiss_fft_state = kiss_fft_alloc(size, 0, NULL, NULL);
  kiss_fft(kiss_fft_state, inp_data, fft_out);

  //xdft = xdft(1:N/2+1);
  //psdx = (1/(Fs*N)) * abs(xdft).^2;
  coeff = 1.0/(size*freq);
  for(i = 0; i < size/2+1; i++)
  {
		spectrum[i] = (fft_out[i].r*fft_out[i].r+fft_out[i].i*fft_out[i].i) * coeff;
  }
  
 
  //psdx(2:end-1) = 2*psdx(2:end-1);
  for(i = 1 ; i < size/2; i++)
  {
	spectrum[i] *= 2;
  }
  
  for(i = 0 ; i < size/2+1; i++)
  {
	spectrum[i] = 10*log10(spectrum[i]);
  }

  free(kiss_fft_state);
  free(fft_out);
  free(inp_data);

  return spectrum;
}

/*
static void calc_periodogram(double* sample,int sample_size,int freq, double* periodogram)
{
	clock_t t1, t2;
	double real, imag,diff;
	int i,j;

	double temp = -2*MATH_PI/sample_size;
	
	for(i = 0; i < sample_size/2+1; i++)
	{
		real = 0;
		imag = 0;
		//fft
		if(i == 0)
		{
			t1 = clock(); 
				real = cos(temp * (3-1) * (i-1));
			t2 = clock(); 
			diff = ((float)(t2 - t1) / 1000000.0F ) * 1000;   
    			syslog (LOG_INFO, "psdx loop: %f",diff); 
		}
		for(j= 0; j < sample_size;j++)
		{
			real += sample[j]* cos(temp * (j-1) * (i-1));
			imag += sample[j] * sin(temp * (j-1) * (i-1));
		}

		
		//psdx = (1/(Fs*N)) * abs(xdft).^2;
		//abs = sqrt(real*real+imag*imag);
		periodogram[i] = sqrt(real*real+imag*imag) / freq / sample_size;
	}
	t1 = clock(); 
	//psdx(2:end-1) = 2*psdx(2:end-1);
	for(i = 1 ; i < sample_size/2; i++)
	{
		periodogram[i] *= 2;
	}

	for(i = 0 ; i < sample_size/2+1; i++)
	{
		periodogram[i] = 10*log10(periodogram[i]);
	}
	t2 = clock(); 
	diff = ((float)(t2 - t1) / 1000000.0F ) * 1000;   
    	syslog (LOG_INFO, "psdx part2: %f",diff); 

	
}
*/

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


static double calc_skewness(double* sample, int sample_size,double mean, double variance)
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

static double calc_kurtosis(double* sample, int sample_size, double mean,double variance)
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

/*
//E(X-mean)^k /std^k where k = 3 -> skewness and k = 4 -> kurtosis
static double calc_normalized_moment(double* sample, int sample_size,double mean, double std, int k)
{
	//calc E(X-mean)
	int i;	
	double sum = 0;
	for(i = 0; i < sample_size; i++)
	{
		sum += sample[i] - mean; 
	}
	sum /= sample_size;
	//return E(X-mean)^k /std^k
	return pow(sum,k) / pow(std,k);
}
*/

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
		//sum += (sample[i]-sample_mean) * (sample[i]-sample_mean)
		
	}
	//matlab normalizes with n-1
	return sum / (sample_size-1);
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

static void calc_psdx_features(double* psdx, int size, int* nbr_peaks_feature, double* power_ratio_feature, int* freq_bins)
{
	int nbr_peaks = 0;
	double threshold = calc_max(psdx,size) - PEAK_DECI_THRESHOLD;
	int i;
	double power_sum = 0;
	double power_ratio = 0;
	for(i = 0; i < size ; i++)
	{
		if(psdx[i] > threshold)
		{
			nbr_peaks++; 
			power_ratio += psdx[i];
			freq_bins[i/BIN_SIZE]++;
		}
		power_sum += psdx[i];
	}
	*nbr_peaks_feature = nbr_peaks;
	*power_ratio_feature = power_ratio / power_sum;
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
	int psdx_size = sample_size/2+1;
	double* psdx_x = calculate_spectrum(sampleX, sample_size,sample_freq);
	double* psdx_y = calculate_spectrum(sampleY, sample_size,sample_freq);
	double* psdx_z =calculate_spectrum(sampleZ, sample_size,sample_freq);
	double* psdx_tilt_xy = calculate_spectrum(tiltXY, sample_size,sample_freq);
	double* psdx_tilt_xz = calculate_spectrum(tiltXZ, sample_size,sample_freq);
	double* psdx_tilt_yz =calculate_spectrum(tiltYZ, sample_size,sample_freq);
	t2 = clock(); 
	diff = ((float)(t2 - t1) / 1000000.0F ) * 1000;   
    	syslog (LOG_INFO, "psdx kiss: %f",diff);


	t1 = clock(); 
	int nbr_freq_peaks[3] = {0};
	double peak_freq_ratio[3] = {0};
	int nbr_freq_peaks_tilt[3] = {0};
	double peak_freq_ratio_tilt[3] = {0};

	int* psdx_freq_bins_x = calloc(PEAK_NBR_BINS(psdx_size),sizeof(int));
	int* psdx_freq_bins_y = calloc(PEAK_NBR_BINS(psdx_size),sizeof(int));
	int* psdx_freq_bins_z = calloc(PEAK_NBR_BINS(psdx_size),sizeof(int));
	int* psdx_freq_bins_tilt_xy = calloc(PEAK_NBR_BINS(psdx_size),sizeof(int));
	int* psdx_freq_bins_tilt_xz = calloc(PEAK_NBR_BINS(psdx_size),sizeof(int));
	int* psdx_freq_bins_tilt_yz = calloc(PEAK_NBR_BINS(psdx_size),sizeof(int));

	calc_psdx_features(psdx_x, psdx_size, &nbr_freq_peaks[0], &peak_freq_ratio[0], psdx_freq_bins_x);
	calc_psdx_features(psdx_y, psdx_size, &nbr_freq_peaks[1], &peak_freq_ratio[1], psdx_freq_bins_y);
	calc_psdx_features(psdx_z, psdx_size, &nbr_freq_peaks[2], &peak_freq_ratio[2], psdx_freq_bins_z);

	calc_psdx_features(psdx_tilt_xy, psdx_size, &nbr_freq_peaks_tilt[0], &peak_freq_ratio_tilt[0], psdx_freq_bins_tilt_xy);
	calc_psdx_features(psdx_tilt_xz, psdx_size, &nbr_freq_peaks_tilt[1], &peak_freq_ratio_tilt[1], psdx_freq_bins_tilt_xz);
	calc_psdx_features(psdx_tilt_yz, psdx_size, &nbr_freq_peaks_tilt[2], &peak_freq_ratio_tilt[2], psdx_freq_bins_tilt_yz);

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
		features[28+i]= psdx_freq_bins_tilt_xy[i];
	}
	for(i = 0; i < 5; i++)
	{
		features[33+i]= psdx_freq_bins_tilt_xz[i];
	}
	for(i = 0; i < 5; i++)
	{
		features[38+i]= psdx_freq_bins_tilt_yz[i];
	}

	//calc skewness for psdx
	t1 = clock(); 
	double psdx_x_mean = calc_mean(psdx_x,psdx_size);
	double psdx_x_variance = calc_variance(psdx_x,psdx_size,psdx_x_mean);
	features[43] = calc_skewness(psdx_x,psdx_size,psdx_x_mean,psdx_x_variance);

	double psdx_y_mean = calc_mean(psdx_y,psdx_size);
	double psdx_y_variance = calc_variance(psdx_y,psdx_size,psdx_y_mean);
	features[44] = calc_skewness(psdx_y,psdx_size,psdx_y_mean,psdx_y_variance);

	double psdx_z_mean = calc_mean(psdx_z,psdx_size);
	double psdx_z_variance = calc_variance(psdx_z,psdx_size,psdx_z_mean);
	features[45] = calc_skewness(psdx_z,psdx_size,psdx_z_mean,psdx_z_variance);
	t2 = clock(); 
	diff = ((float)(t2 - t1) / 1000000.0F ) * 1000;   
    	syslog (LOG_INFO, "skewness for psdx: %f",diff); 

	features[46] = nbr_freq_peaks_tilt[0];
	features[47] = nbr_freq_peaks_tilt[1];
	features[48] = nbr_freq_peaks_tilt[2];
	
	for(i = 0; i < 5; i++)
	{
		features[49+i]= psdx_freq_bins_x[i];
	}
	for(i = 0; i < 5; i++)
	{
		features[54+i]= psdx_freq_bins_y[i];
	}	

	for(i = 0; i< NBR_FEATURES ; i++)
	{
		
		features[i] = (features[i]-svm_model->features_mean[i]) / svm_model->features_std[i];
		//syslog (LOG_INFO, "feature[%d]: %f",i,features[i]);
		
		 
	}

	//syslog (LOG_INFO, "feature[%d]: %f",1,features[25]);

	
	free(tiltXY);
	free(tiltXZ);
	free(tiltYZ);

	free(psdx_x);
	free(psdx_y);
	free(psdx_z);
	free(psdx_tilt_xy);
	free(psdx_tilt_xz);
	free(psdx_tilt_yz);  

	free(psdx_freq_bins_x);
	free(psdx_freq_bins_y);
	free(psdx_freq_bins_z);
	free(psdx_freq_bins_tilt_xy);
	free(psdx_freq_bins_tilt_xz);
	free(psdx_freq_bins_tilt_yz);

	return features;
}



