#include <stdio.h>
#include <stdlib.h>
#include "bmp-reader.h"

unsigned char getColor(int row,int col,int color, unsigned char* image)
{
        return image[64*3*row+col*3+color];
}

unsigned char* readBMPImage()
{

	/*
    FILE *file;
    int ch;
         
    file=fopen("pedestrian.png","rb");     
     int count = 0; 
    while((ch=fgetc(file))!=EOF )
    {
	count++;
	         printf("%d ",ch);
	}
	printf("count %d", count);
	*/



	//int image[8192][3]; // first number here is 8192 pixels in my image, 3 is for RGB values
	unsigned char* image = malloc(8192*3*sizeof(char));
	

	 FILE *streamIn;
	 streamIn = fopen("../data/pedestrian.bmp", "rb");
	 if (streamIn == (FILE *)0){
	   printf("File opening error ocurred. Exiting program.\n");
	   exit(0);
	 }
	
	 int i,j;
	// int byte; 
	 for(i=0;i<138 ;i++) getc(streamIn);  // strip out BMP header

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
	 return image;
}


