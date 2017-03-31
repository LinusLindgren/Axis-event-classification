#include <math.h>
#include <stdlib.h>
#include <float.h>
#include "kiss_fft.h"
#include "feature_util.h"
#include "calc_periodogram_features.h"

#include <time.h>
#include <glib.h>
#include <glib-object.h>
#include <glib/gstdio.h>
#include <gio/gio.h>
#include <stdlib.h>
#include <event2.h>
#include <syslog.h>

#define PEAK_DECI_THRESHOLD 6
#define PEAK_NBR_BINS(size) ((size/BIN_SIZE) +1)
#define BIN_SIZE 30


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
    inp_data[i].r = (float) window[i];
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
/*
psdx_feature_data_type* extract_psdx_features(double* samples, int nbr_samples, int sample_size,int sample_freq)
{
	psdx_feature_data_type* data = malloc(sizeof(psdx_feature_data_type)*nbr_samples);
	
	for(i = 0; i < nbr_samples;i++)
	{
		data[i]->size = sample_size/2+1;
	}
	for(i = 0; i < nbr_samples;i++)
	{
		data[i]->psdx_freq_bins = calloc(PEAK_NBR_BINS(data->size),sizeof(int));
	}
	for(i = 0; i < nbr_samples;i++)
	{
		data[i]->psdx = calculate_spectrum(sample, sample_size,sample_freq);
		calc_psdx_features(data[i]->psdx, data[i]->size, &(data[i]->nbr_freq_peaks), &(data[i]->peak_freq_ratio), data[i]->psdx_freq_bins);
	}			
	return data;
}
*/

optimize perhaps
psdx_feature_data_type* extract_psdx_features(double* sample, int sample_size,int sample_freq)
{
	psdx_feature_data_type* data = malloc(sizeof(psdx_feature_data_type));	
	
	data->size = sample_size/2+1;
	data->psdx_freq_bins = calloc(PEAK_NBR_BINS(data->size),sizeof(int));
	data->psdx = calculate_spectrum(sample, sample_size,sample_freq);
	calc_psdx_features(data->psdx, data->size, &(data->nbr_freq_peaks), &(data->peak_freq_ratio), data->psdx_freq_bins);
	
	return data;
}

void free_psdx_feature_data_type(psdx_feature_data_type* data)
{
	free(data->psdx_freq_bins);
	free(data->psdx);
	free(data);
}















