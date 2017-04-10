#ifndef read_sample_h
#define read_sample_h
typedef struct{
  		double* x;
		double* y;
		double* z;
		int start_x;
		int start_y;
		int start_z;
	}sample_data_type;

sample_data_type* read_sample(char* path, int nbr_samples);
void test_calc_time(void);
#endif
