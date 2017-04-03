#ifndef feature_util_h
#define feature_util_h
	double calc_skewness(double* sample, int sample_size,double mean, double variance);
	double calc_kurtosis(double* sample, int sample_size, double mean,double variance);
	double calc_mean(double* sample,int sample_size);
	double calc_variance(double* sample,int sample_size, double sample_mean);
	double calc_min(double* sample, int sample_size);
	double calc_max(double* sample, int sample_size);
	double calc_sum(double* sample, int sample_size);

#endif


