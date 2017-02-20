#include <stdio.h>
#include <stdlib.h>
#include "bmp-reader.h"

unsigned char getColor(int row,int col,int color, unsigned char* image)
{
	return image[64*3*row+col*3+color];
}

unsigned char* readImage()
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
	 streamIn = fopen("./pedestrian.bmp", "r");
	 if (streamIn == (FILE *)0){
	   printf("File opening error ocurred. Exiting program.\n");
	   exit(0);
	 }
	
	 int i;
	// int byte; 
	 for(i=0;i<54;i++) getc(streamIn);  // strip out BMP header

	 for(i=0;i<8192;i++){    // foreach pixel
	    //image[i][2] = getc(streamIn);  // use BMP 24bit with no alpha channel
	    //image[i][1] = getc(streamIn);  // BMP uses BGR but we want RGB, grab byte-by-byte
	    //image[i][0] = getc(streamIn);  // reverse-order array indexing fixes RGB issue...
	    image[i*3+2] = getc(streamIn);  // reverse-order array indexing fixes RGB issue...
	    image[i*3+1] = getc(streamIn);  // reverse-order array indexing fixes RGB issue...
	    image[i*3] = getc(streamIn);  // reverse-order array indexing fixes RGB issue...
	    printf("pixel %d : [%d,%d,%d]\n",i+1,image[i*3],image[i*3+1],image[i*3+2]);
	 }

	 fclose(streamIn);
	 return image;
}


