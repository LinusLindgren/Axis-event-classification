#include <stdio.h>
#include <stdlib.h>
#include "bmp-reader.h"

unsigned char getColor(int row,int col,int color, unsigned char* image)
{
        return image[64*3*row+col*3+color];
}

void setRGB(int row, int col, unsigned char* image, unsigned char r, unsigned char g, unsigned char b)
{
	image[64*3*row+col*3]=r;
	image[64*3*row+col*3+1]=g;
	image[64*3*row+col*3+2]=b;
}

void drawWindow(int row, int col, unsigned char* image)
{
	int i;	
	for(i = col; i < col+64; i++)
	{
		setRGB(row,i,image,0,255,0);
	}
	for(i = col; i < col+64; i++)
	{
		setRGB(row+127,i,image,0,255,0);
	}
	for(i = row; i < row+128; i++)
	{
		setRGB(i,col,image,0,255,0);
	}
	for(i = row; i < row+128; i++)
	{
		setRGB(i,col+63,image,0,255,0);
	}
}

void readBMPImage(unsigned char** header, unsigned char** rbg_values)
{




	//int image[8192][3]; // first number here is 8192 pixels in my image, 3 is for RGB values
	
	//138 bytes for header, hardcoded right now fix later
	*header = malloc(54*sizeof(char));

	 FILE *streamIn;
	 streamIn = fopen("../data/pedestrian.bmp", "rb");
	 if (streamIn == (FILE *)0){
	   printf("File opening error ocurred. Exiting program.\n");
	   exit(0);
	 }
	
	 int i,j;
	 for(i=0;i<54 ;i++)  // strip out BMP header
	{
		(*header)[i] = getc(streamIn);
	}
	
	//unsigned long int height = (*header)[22];
	//unsigned long int width = (*header)[18];
	//unsigned long int offset = (*header)[10];
	//int test;
	//isscanf((*header)+18,"%d",&test);
	int width = *((int *) &(*header)[18]);
	int height = *((int *) &(*header)[22]);
	int offset = *((int *) &(*header)[10]);
	//int test = 0;
	//long int heighttest = ((*header)[10]<<24)|
	//((*header)[11]<<16)|
	//((*header)[12]<<8)|
	//((*header)[13]);
	//sscanf((*header)+10, "%d", &test);
	/*for (i = 1; i < 4; i++ ) {
		offset <= 8;
		offset += (*header)[10+i];
		width << 8;    
		width += (*header)[18+i];
		height << 8;
		height += (*header)[22+i];
	
	}
	printf("test %d",test);
*/
	for(i=0; i < offset-54;i++)getc(streamIn);
	unsigned char* image = malloc(height*width*3*sizeof(char));
	 for(i=127; i>= 0;i--){    // foreach pixel
	 	for(j=0; j < 64; j++)
		{
	    		image[i*64*3+j*3+2] = getc(streamIn);  // reverse-order array indexing fixes RGB issue...
	    		image[i*64*3+j*3+1] = getc(streamIn);  // reverse-order array indexing fixes RGB issue...
	    		image[i*64*3+j*3+0] = getc(streamIn);  // reverse-order array indexing fixes RGB issue...
	    		//printf("pixel %d : [%d,%d,%d]\n",i+1,image[i*3],image[i*3+1],image[i*3+2]);
		} 
	}

	 fclose(streamIn);
	*rbg_values = image;

}


void writeBMPImage(unsigned char* header, unsigned char* image)
{

	
	FILE* streamOut = fopen("output.bmp","wb");
	 int i,j;

	 for(i=0;i<138 ;i++)  // strip out BMP header
	{
		fprintf(streamOut,"%c",header[i]);
	}
	 for(i=127; i>= 0;i--){    // foreach pixel
	 	for(j=0; j < 64; j++)
		{
	    		fprintf(streamOut,"%c%c%c",image[i*64*3+j*3+2], image[i*64*3+j*3+1] ,image[i*64*3+j*3+0]);  // reverse-order array indexing fixes RGB issue...
	    		//printf("pixel %d : [%d,%d,%d]\n",i+1,image[i*3],image[i*3+1],image[i*3+2]);
		} 
	}

	 fclose(streamOut);
}


