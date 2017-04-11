#ifndef svm_h
#define svm_h
	typedef struct{
  		double bias;
  		double* beta;
		double* features_mean;
		double* features_std;
	}svm_linear_model_data_type;
	svm_linear_model_data_type* read_linear_svm_model(char* path, int nbr_features);
	double predict(double* features, svm_linear_model_data_type* svm_model,int nbr_features);
	
#endif


