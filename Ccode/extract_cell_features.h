#ifndef extract_cell_features_h/* Include guard */
#define extract_cell_features_h
	/*
	*I 64x128 image where the first pixel have its rbg values on index 0,1,2
	*/
	double* extract_cell_features(unsigned char* I);
	int isBorder(int cj, int ci, int i, int j);
	void calcGradient(int cj, int ci, int i, int j, unsigned char* I, double* maxMag, double* maxAng);
#endif// FOO_H_

