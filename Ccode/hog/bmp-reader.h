#ifndef bmp_reader_h/* Include guard */
#define bmp_reader_h
unsigned char getColor(int row,int col,int color, unsigned char* image);
void readBMPImage(unsigned char** header, unsigned char** rbg_values);
void writeBMPImage(unsigned char* header, unsigned char* image);
void setRGB(int row, int col, unsigned char* image, unsigned char r, unsigned char g, unsigned char b);
void drawWindow(int row, int col, unsigned char* image);

#endif// FOO_H_
