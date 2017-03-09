#ifndef calc_acf_h/* Include guard */
#define calc_acf_h
double* calc_acf(double* samples, int sample_size,int lag, double mean, double variance)
double* calc_xcf(double* samplesX,double* samplesY, int sample_size, double meanX, double meanY, double varianceX, double varianceY);
double* extract_features(double* sampleX,double* sampleY, double* sampleZ,int sample_size);
#endif// FOO_H_
