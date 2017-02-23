#include "calc_block_features.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>



double calc_2_norm(double* conc_histogram)
{
	int i;
	double sum = 0;
	for(i= 0; i < 36; i++)
	{
		sum+= pow(conc_histogram[i],2);
	}

	return sqrt(sum);
}

void normalize(double* features, double* histogram, double* conc_histogram, int row, int col)
{
	double norm = calc_2_norm(conc_histogram);
	int index = 252*row +36*col ;
	int i;
	for(i = 0; i < 36;i++)
	{
		double feature = conc_histogram[i] / norm;
		if (isnan(feature))
		{
    			feature = 0.0;
		}
		features[index+i] = feature;
	}
}



void copy_cell_histogram(double* histogram, double* conc_histogram, int row, int col, int start_index)
{	
	int k;
	for(k = 0 ; k < 9; k++)
	{
		conc_histogram[start_index+k]=histogram[row*72+9*col+k];		
	}	
}

double* calc_block_features(double* histogram)
{
	//the nbr of features is always 3780
	double* features = malloc(sizeof(double)*3780);
	int i,j;
	double conc_histogram[36];	
	for(i = 0; i < 15;i++)
	{	
		for(j=0; j< 7; j++)
		{
			copy_cell_histogram(histogram,conc_histogram,i,j,0);				
			copy_cell_histogram(histogram,conc_histogram,i+1,j,9);				
			copy_cell_histogram(histogram,conc_histogram,i,j+1,18);				
			copy_cell_histogram(histogram,conc_histogram,i+1,j+1,27);				
			normalize(features,histogram,conc_histogram,i,j);
		}
	}
	return features;
}

