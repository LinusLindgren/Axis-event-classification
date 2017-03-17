#include "svm.h"
#include <stdio.h>	
#include <stdlib.h>
#include <math.h>

/**
 * predict:
 *
 * Calculates score for binary classification using a linear svm model. 
 *
 * @features: an array containing the features of the data to be classified
 * @svm_model: the linear svm model used to classify the data. Contains the beta and bias values
 * @nbr_features: the length of features and beta
 *
 * Returns: The prediction score for binary classification. Negative values represents one class and positive the other
 *          
 */
double predict(double* features, svm_linear_model_data_type* svm_model, int nbr_features)
{
	double score = svm_model->bias;
	int i;
	for(i = 0; i< nbr_features; i++)
	{
		score += features[i] * svm_model->beta[i];
	}
	return score;
}
/**
 * read_svm_model:
 *
 * reads the svm model from the given path. The file consists of 1 value per row, the first being bias and the others being the beta values.
 *
 * @path: contains the path for the file 
 * @nbr_features: the number of features in the svm model
 *
 * Returns: the linear svm model read from the path
 *          
 */
svm_linear_model_data_type* read_linear_svm_model(char* path, int nbr_features)
{
	svm_linear_model_data_type* svm_model = malloc(sizeof(svm_linear_model_data_type));
	FILE *fp;
	double* beta= malloc(sizeof(double)*nbr_features);
	double* features_mean = malloc(sizeof(double)*nbr_features);
	double* features_std = malloc(sizeof(double)*nbr_features);
	int i;

	fp = fopen(path, "rb");
	for(i=0; i <nbr_features; i++)
	{
		fscanf(fp, "%lf\n", &features_mean[i]);
	}
	for(i=0; i <nbr_features; i++)
	{
		fscanf(fp, "%lf\n", &features_std[i]);
	}
	fscanf(fp,"%lf\n",&(svm_model->bias));
	for(i=0; i <nbr_features; i++)
	{
		fscanf(fp, "%lf\n", &beta[i]);
	}
   	fclose(fp);
	svm_model->beta=beta;
	svm_model->features_mean = features_mean;
	svm_model->features_std = features_std;	
	return svm_model;
}
