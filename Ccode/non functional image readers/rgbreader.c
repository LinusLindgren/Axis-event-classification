#include "rgbreader.h"
#include  <stdlib.h>
#include <stdio.h>
#include <ctype.h>

unsigned char readChar(FILE** streamIn)
{
		unsigned char color= 0;
                unsigned char c = getc(*streamIn);
                while(isdigit(c))
                {
                        color = color*10+c;
                        c = getc(*streamIn);
                }
		return color;

}

unsigned char getColor(int row,int col,int color, unsigned char* image)
{
        return image[64*3*row+col*3+color];
}


unsigned char* readimage()
{
        //int image[8192][3]; // first number here is 8192 pixels in my image, 3 is for RGB values
        unsigned char* image = malloc(8192*3*sizeof(unsigned char));

         FILE *streamIn;
         streamIn = fopen("./pedestrian3.txt", "rb");
         if (streamIn == (FILE *)0){
           printf("File opening error ocurred. Exiting program.\n");
           exit(0);
         }

         int i,j;
         for(i=0;i<128;i++){
		for(j=0; j <64; j++)
		{
		//fscanf(streamIn,"%d", &res);   // foreach pixel
		//image[i*3*64+3*j] = res;
		//streamIn =streamIn+4;
		//fscanf(streamIn,"%d", &res);   // foreach pixel
		//image[i*3*64+3*j+1]=res;
		//streamIn =streamIn+4;
		//fscanf(streamIn,"%d", &res);   // foreach pixel
		//image[i*64*3+3*j+2] = res;
		//streamIn =streamIn+4;
            image[64*3*i+j*3] = getc(streamIn);  // reverse-order array indexing fixes RGB issue...
		getc(streamIn);    
	//image[i][0] = getc(streamIn);  // reverse-order array indexing fixes RGB issue...
		//fscanf(streamIn,"%d",&image[i*j*3]);
             	
		//image[i*j*3] = readChar(&streamIn);
		//image[i*j*3+1] = readChar(&streamIn);
		//image[i*j*3+2] = readChar(&streamIn);
		//fscanf(streamIn,"%d",&image[i*j*3+1]);
             //fscanf(streamIn,"%d",&image[i*j*3+2]);
            //getc(streamIn);
	    image[64*3*i+3*j+1] = getc(streamIn);  // reverse-order array indexing fixes RGB issue...
            getc(streamIn);
		//getc(streamIn);
            image[64*3*i+3*j+2] = getc(streamIn);  // reverse-order array indexing fixes RGB issue...
        	getc(streamIn);    
	//getc(streamIn);
            //printf("pixel (%d,%d) : [%d,%d,%d]\n",i,j,image[i*j*3],image[i*j*3+1],image[i*j*3+2]);
        	} 
	}

         fclose(streamIn);
         return image;
	
}
