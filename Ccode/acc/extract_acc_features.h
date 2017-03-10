#ifndef extract_acc_features_h/* Include guard */
#define extract_acc_features_h

double* calc_acf(double* samples, int sample_size, double mean, double variance);
double* calc_xcf(double* samplesX,double* samplesY, int sample_size,double stdX, double stdY);
double* extract_features(double* sampleX,double* sampleY, double* sampleZ,int sample_size);
#endif// FOO_H_
