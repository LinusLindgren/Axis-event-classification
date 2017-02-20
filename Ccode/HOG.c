#include "bmp-reader.h"
#include <stdio.h>
#include <stdlib.h>
#include "extract_cell_features.h"
#include "calc_block_features.h"
#include "svm.h"
int main()
{
        unsigned char* image= readImage();
	///*
	int i,j;
	for(i = 0; i < 128;i++)
	{
		for(j=0; j < 64;j++)
		{
        		printf("pixel value (%d,%d,0): %d \n",i,j, getColor(i,j,0,image));
        		printf("pixel value (%d,%d,1): %d \n",i,j, getColor(i,j,1,image));
        		printf("pixel value (%d,%d,2): %d \n",i,j, getColor(i,j,2,image));
        	}
        		//printf("pixel value (%d,%d,0): %d \n",0,0, image[0]);
        		//printf("pixel value (%d,%d,1): %d \n",0,0, image[1]);
        		//printf("pixel value (%d,%d,2): %d \n",0,0, image[2]);
	}
	//*/
	double* histogram = extract_cell_features(image);
	free(image);
	/*		
        int i,j,k;
	for(i = 0; i < 8 ; i++)
	{
		for(j=0; j < 16;j++)
		{
			printf("cell [%d,%d] has histogram[",i,j);
			for(k=0; k < 9; k++)
			{
				printf("%f,", histogram[72*j+i*8+k]);
			}
			printf("]\n");
		} 
	}
	*/
	double* features = calc_block_features(histogram);
	free(histogram);
	/*
	int i;
	for(i = 0; i < 3780; i++)
	{
		printf("features[%d] = %f\n",i,features[i]);
	}
	*/
	double* beta = read_svm_model();
	double score = predict(features, beta, -0.00478);	
	printf("score %lf", score);
	free(features);
	return 0;
}

