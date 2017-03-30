#include "svm.h"
#ifndef extract_acc_features_h/* Include guard */
#define extract_acc_features_h

void calc_acf(double* samples, int sample_size, double mean, double variance,double* acf);
void calc_xcf(double* samplesX,double* samplesY, int sample_size,double stdX, double stdY,double* xcf);
double* extract_features(double* sampleX,double* sampleY, double* sampleZ,int sample_size, svm_linear_model_data_type* svm_model,int sample_freq);
int get_nbr_features(void);
#endif// FOO_H_
