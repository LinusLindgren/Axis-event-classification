#include <stdio.h>	
#include <stdlib.h>
#include <math.h>
#include "extract_acc_features.h"
#include "read_sample.h"
#include <float.h>
#include <time.h>
#include <glib.h>
#include <glib-object.h>
#include <glib/gstdio.h>
#include <gio/gio.h>
#include <event2.h>
#include <syslog.h>
#include <dirent.h> 
#include <string.h>

sample_data_type* read_sample(char* path, int nbr_samples)
{

	sample_data_type* sample = malloc(sizeof(sample_data_type));
	FILE *fp;
	sample->x = malloc(sizeof(double)*nbr_samples);
	sample->y = malloc(sizeof(double)*nbr_samples);
	sample->z = malloc(sizeof(double)*nbr_samples);
	
	int i;
	fp = fopen(path, "rb");
	//read startup values
	fscanf(fp, "startup(x,y,z):(%d,%d,%d)\n", &(sample->start_x), &(sample->start_y), &(sample->start_z));
	
	for(i=0; i <nbr_samples; i++)
	{
		fscanf(fp, "%lf %lf %lf\n", &(sample->x[i]),&(sample->y[i]),&(sample->z[i]));
		
	}
   	fclose(fp);
	return sample;
}
/*
static void test_all_files(void)
{
	svm_linear_model_data_type* svm_model = read_linear_svm_model("/tmp/svm_params", get_nbr_features());
  DIR           *d;
  struct dirent *dir;
  d = opendir(".");
  if (d)
  {
    while ((dir = readdir(d)) != NULL)
    {
	if(strstr(dir->d_name,"acc") != NULL)
	{
		printf("%s\n", dir->d_name);
		int nbr_samples = 512;
		int i;
		sample_data_type* sample = read_sample(dir->d_name,nbr_samples);
		for(i=0;i<nbr_samples; i++){
			sample->x[i]=sample->x[i] - sample->start_x;
			sample->y[i]=sample->y[i] - sample->start_y;
			sample->z[i]=sample->z[i] - sample->start_z;
		}
		double* features = extract_features(sample->x,sample->y,sample->z,nbr_samples,svm_model,400);
		free(features);
	}
      
    }

    closedir(d);
  }
}
*/

void test_calc_time()
{
	//test_all_files();	
	svm_linear_model_data_type* svm_model = read_linear_svm_model("/tmp/svm_params", get_nbr_features());
	int nbr_samples = 256;
	int i;
	sample_data_type* sample = read_sample("acc_sample",nbr_samples);

	/*
	syslog (LOG_INFO, "start(x,y,z) = (%d,%d,%d)\n",sample->start_x,sample->start_y,sample->start_z);
	for(i = 0; i<nbr_samples; i++)
	{
		syslog (LOG_INFO, "%f %f %f \n",sample->x[i],sample->y[i],sample->z[i]);
	}
	*/
	for(i=0;i<nbr_samples; i++){
		sample->x[i]=sample->x[i] - sample->start_x;
		sample->y[i]=sample->y[i] - sample->start_y;
		sample->z[i]=sample->z[i] - sample->start_z;
	}

	double* features = extract_features(sample->x,sample->y,sample->z,nbr_samples,svm_model,200);
	double score = predict(features, svm_model, 41);
	syslog (LOG_INFO, "score: %f\n",score);
	free(features);

	/*
	clock_t t1, t2;
	
	int nbr_steps = 10;
	int step_size = 50;
	int nbr_iterations = 10;
	int current_amount_of_samples = nbr_samples;
	float time[nbr_steps];
	for(i = 0; i < nbr_steps;i++)
	{
		float sum = 0;
		for(j = 0; j < nbr_iterations; j++)
		{
			t1 = clock();
			double* features = extract_features(sample->x,sample->y,sample->z,current_amount_of_samples,svm_model,400);
			t2 = clock(); 
			free(features);
			float diff = ((float)(t2 - t1) / 1000000.0F ) * 1000; 
			syslog (LOG_INFO, "time : %f for nbr-samples: %d",diff,nbr_samples - step_size*i);
			sum +=   diff;
		}
		time[i] = sum / nbr_iterations;
		current_amount_of_samples -= step_size;
	}

	for(i = 0; i < nbr_steps;i++)
	{
		syslog (LOG_INFO, "Classifying time : %f for nbr-samples: %d",time[i],nbr_samples - step_size*i);
	}
	*/
	free(svm_model);
	free(sample->x);
	free(sample->y);
	free(sample->z);
	free(sample);



	
    	


}
