#include "svm.h"
#include <stdio.h>	
#include <stdlib.h>
#include <math.h>

double predict(double* features, double* beta, double bias)
{
	double score = 0;
	int i;
	for(i = 0; i< 3780; i++)
	{
		score += features[i]*beta[i];
	//	printf("score at index %d with beta %f and feature %f  = %f\n", i,beta[i],features[i], score);	
	}
	return score+bias;
}

double* read_svm_model()
{

	FILE *fp;
	double* beta= malloc(sizeof(double)*3780);
	int i;
	fp = fopen("beta-first.txt", "r");
	for(i=0; i <3780; i++)
	{
		fscanf(fp, "%lf\n", &beta[i]);
		//printf("%lf\n",beta[i]);
	}
   	fclose(fp);	
	return beta;
}
