#include <stdio.h>
#include <stdlib.h>


void test()
{
	FILE* fd;
	fd = fopen("pedestrian.bmp","rb");
	int i;
	for(i = 0; i < 9000; i++)
	{
		unsigned char c = getc(fd);
		printf("byte %d = %u\n", i, c);
	}
}




 void readBmp()
{


	unsigned char *texels;
	int width, height;


	FILE *fd;
	fd = fopen("pedestrian.bmp", "rb");
	if (fd == NULL)
	{
		printf("Error: fopen failed\n");
		return;
	}

	unsigned char header[54];

	// Read header
	fread(header, sizeof(unsigned char), 54, fd);

	// Capture dimensions
	width = *(int*)&header[18];
	height = *(int*)&header[22];

	int padding = 0;

	// Calculate padding
	while ((width * 3 + padding) % 4 != 0)
	{
	padding++;
	}

	// Compute new width, which includes padding
	int widthnew = width * 3 + padding;

	// Allocate memory to store image data (non-padded)
	texels = (unsigned char *)malloc(width * height * 3 * sizeof(unsigned char));
	if (texels == NULL)
	{
	printf("Error: Malloc failed\n");
	return;
	}

	// Allocate temporary memory to read widthnew size of data
	unsigned char* data = (unsigned char *)malloc(widthnew * sizeof (unsigned int));
	int i, j;
	// Read row by row of data and remove padded data.
	for (i = 0; i<height; i++)
	{
	// Read widthnew length of data
	fread(data, sizeof(unsigned char), widthnew, fd);

	// Retain width length of data, and swizzle RB component.
	// BMP stores in BGR format, my usecase needs RGB format
	for (j = 0; j < width * 3; j += 3)
	{
	int index = (i * width * 3) + (j);
	texels[index + 0] = data[j + 2];
	texels[index + 1] = data[j + 1];
	texels[index + 2] = data[j + 0];
	printf("RGB value pixel(%d,%d) =(%d,%d,%d)\n",i,j/3,texels[index+0],texels[index+1],texels[index+2]);
	}
	}

	
	free(data);
	fclose(fd);

	









}




















/*
int imageLoad(char *filename, Image *image) {
    FILE *file;
    unsigned long size;                 // size of the image in bytes.
    unsigned long i;                    // standard counter.
    unsigned short int planes;          // number of planes in image (must be 1) 
    unsigned short int bpp;             // number of bits per pixel (must be 24)
    char temp;                          // temporary color storage for bgr-rgb conversion.

    // make sure the file is there.
    if ((file = fopen(filename, "rb"))==NULL)
    {
	printf("File Not Found : %s\n",filename);
	return 0;
    }
    
    // seek through the bmp header, up to the width/height:
    fseek(file, 18, SEEK_CUR);

    // read the width
    if (!(image->sizeX = endianReadInt(file))) {
	printf("Error reading width from %s.\n", filename);
	return 0;
    }
    printf("Width of %s: %lu\n", filename, image->sizeX);
    
    // read the height 
    if (!(image->sizeY = endianReadInt(file))) {
	printf("Error reading height from %s.\n", filename);
	return 0;
    }
    printf("Height of %s: %lu\n", filename, image->sizeY);
    
    // calculate the size (assuming 24 bits or 3 bytes per pixel).
    size = image->sizeX * image->sizeY * 3;

    // read the planes
    if (!(planes=endianReadShort(file))) {
	printf("Error reading planes from %s.\n", filename);
	return 0;
    }
    if (planes != 1) {
	printf("Planes from %s is not 1: %u\n", filename, planes);
	return 0;
    }

    // read the bits per pixel
    if (!(bpp = endianReadShort(file))) {
	printf("Error reading bpp from %s.\n", filename);
	return 0;
    }
    if (bpp != 24) {
	printf("Bpp from %s is not 24: %u\n", filename, bpp);
	return 0;
    }
	
    // seek past the rest of the bitmap header.
    fseek(file, 24, SEEK_CUR);

    // read the data. 
    image->data = (char *) malloc(size);
    if (image->data == NULL) {
	printf("Error allocating memory for color-corrected image data");
	return 0;	
    }

    if ((i = fread(image->data, size, 1, file)) != 1) {
	printf("Error reading image data from %s.\n", filename);
	return 0;
    }

    for (i=0;i<size;i+=3) { // reverse all of the colors. (bgr -> rgb)
	temp = image->data[i];
	image->data[i] = image->data[i+2];
	image->data[i+2] = temp;
    }
    
    // we're done.
    return 1;
}
*/
/*int main()
{	
test();	
//	readBmp();
}
*/
