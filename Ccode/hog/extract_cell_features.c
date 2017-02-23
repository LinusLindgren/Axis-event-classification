#include "extract_cell_features.h"
#include "bmp-reader.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
/*
@input: cj: column cell index.
	ci: row cell index.
	i: pixel row index.
	j: pixel column index.
	I: Image to be processed.
	maxMag: Magnitude of the color channel with highest magnitude.
	maxAng: Angle of the color channel with highest magnitude.

Calculates which channel has the highest gradient magnitude and stores the magnitude and angle of this channel in maxMag
and maxAng.

*/
void calcGradient(int cellcol, int cellrow, int i, int j, unsigned char* I, double* maxMag, double* maxAng)
{
	///*
	int dyR =getColor(cellrow*8+i+1,cellcol*8+j,0,I) - getColor(cellrow*8+i-1, cellcol*8+j,0,I);
 	int dyG =getColor(cellrow*8+i+1,cellcol*8+j,1,I) - getColor(cellrow*8+i-1, cellcol*8+j,1,I);
	int dyB =getColor(cellrow*8+i+1,cellcol*8+j,2,I) - getColor(cellrow*8+i-1, cellcol*8+j,2,I);

	int dxR =getColor(cellrow*8+i,cellcol*8+j+1,0,I) - getColor(cellrow*8+i, cellcol*8+j-1,0,I);
	int dxG =getColor(cellrow*8+i,cellcol*8+j+1,1,I) - getColor(cellrow*8+i, cellcol*8+j-1,1,I);
	int dxB =getColor(cellrow*8+i,cellcol*8+j+1,2,I) - getColor(cellrow*8+i, cellcol*8+j-1,2,I);
	//*/
	/*
	//omvänt som i matlab test
	unsigned char dxR =getColor(ci*8+i+1,cj*8+j,0,I) - getColor(ci*8+i-1, cj*8+j,0,I);
	unsigned char dxG =getColor(ci*8+i+1,cj*8+j,1,I) - getColor(ci*8+i-1, cj*8+j,1,I);
	unsigned char dxB =getColor(ci*8+i+1,cj*8+j,2,I) - getColor(ci*8+i-1, cj*8+j,2,I);

	unsigned char dyR =getColor(ci*8+i,cj*8+j+1,0,I) - getColor(ci*8+i, cj*8+j-1,0,I);
	unsigned char dyG =getColor(ci*8+i,cj*8+j+1,1,I) - getColor(ci*8+i, cj*8+j-1,1,I);
	unsigned char dyB =getColor(ci*8+i,cj*8+j+1,2,I) - getColor(ci*8+i, cj*8+j-1,2,I);
	*/
	double magR = sqrt(pow(dyR,2)+pow(dxR,2));
	double magG = sqrt(pow(dyG,2)+pow(dxG,2));
	double magB = sqrt(pow(dyB,2)+pow(dxB,2));
	
	
	if(magR > magG && magR > magB)
	{
		*maxAng = atan2(dyR,dxR);
		*maxMag = magR;

	} else if(magG > magB)
	{

		*maxAng  = atan2(dyG,dxG);
		*maxMag = magG;
	}else
	{

		*maxAng = atan2(dyB,dxB);
		*maxMag = magB;
	}
//	if(cellrow==0 && cellcol == 0 && *maxAng > 2.44)
//	{
//		printf("(dyR,dyG,dyB) = (%d,%d,%d) (dxR,dxG,dxB) = (%d,%d,%d)\n", dyR,dyG,dyB,dxR,dxG,dxB); 	
//		printf("for cell(%d,%d) pixel (%d,%d) Ang(%f,%f,%f) Mag(%f,%f.%f)\n",cellrow,cellcol,i,j,atan2(dyR,dxR),atan2(dyG,dxG),atan2(dyB,dxB),magR,magG,magB );
//	}

}

/*

@input I: Image that is to have its cell features extracted.


Extracts the cell features of a specified image I and return them. The cell features are that of a histogram of 9 values
which represent the direction of the gradient of the cell.
*/
double* extract_cell_features(unsigned char* I)
{

double* histogram = calloc(128*9, sizeof(double));
int cellcol,cellrow,i,j;
for(cellcol = 0; cellcol < 8;cellcol++)
{
	for(cellrow = 0; cellrow < 16; cellrow++)
	{
		//for every cell

		for(i=0; i < 8 ; i++)
		{			
			for(j=0; j < 8; j++)	
			{

				//for every pixel
				if(!isBorder(cellcol,cellrow,i,j))
				{
				double maxAng;
				double maxMag;
				calcGradient(cellcol,cellrow,i,j,I,&maxMag,&maxAng);
				
				if(maxAng < 0)
				{
					maxAng +=  M_PI;
				}
				char binNbr = ((int)floor(maxAng / (M_PI/9))) % 9;
			 	double rest = (maxAng / (M_PI/9)) - floor((maxAng / (M_PI/9)));
				
				histogram[cellrow*8*9+cellcol*9+binNbr] += (1-rest)*maxMag;
				char binNbr2 = (binNbr+1) % 9;
				histogram[cellrow*8*9+cellcol*9+binNbr2]+= rest *maxMag;
				//printf("for cell(%d,%d) pixel (%d,%d) maxAng = %f\n",cellrow,cellcol,i,j, maxAng);
				//printf("binnbrs used %d %d from angle %f which gave %d\n" , binNbr, binNbr2,maxAng,((int)floor(maxAng / (M_PI/9))));
				}
			}

		}


	}
}

	return histogram;
}
/*
@input: Indexes of the cell and pixels respectively.

Evaluates wether the indices specified are on the border of the image.
*/
int isBorder(int cj, int ci, int i, int j)
{
	return (cj == 0 && j == 0) ||
	(cj == 7 && j == 7) ||
	(ci == 0 && i == 0) ||
	(ci == 15 && i == 7);
}
