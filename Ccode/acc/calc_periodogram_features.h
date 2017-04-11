#ifndef calc_periodogram_features_h
#define calc_periodogram_features_h
	typedef struct{
  		int nbr_freq_peaks;
		double peak_freq_ratio;
		int* psdx_freq_bins;
		double* psdx;
		int size;
		double skewness;
	}psdx_feature_data_type;
	psdx_feature_data_type* extract_psdx_features(double* sample, int sample_size,int sample_freq);
	void free_psdx_feature_data_type(psdx_feature_data_type* data);
#endif


