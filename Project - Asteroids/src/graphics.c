#include "graphics.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vcard.h"

char * get_filepath (char * folder, char * file){
	char *file_location = (char *) malloc (50);
	strcpy(file_location, folder);
	strcat(file_location, file);
	
	return file_location;
}

uint32_t* read_pixmap24 (const char *map[], int *wd, int *ht){
	int width, height, colors;
	
	/* read width, height, colors */
	
	if (sscanf(map[0], "%d %d %d", &width, &height, &colors) != 3) {
		printf("read_xpm: incorrect width, height, colors\n");
		return NULL;
	}
	
	unsigned int size = width * height;
	
	uint32_t *graphic = (uint32_t*) malloc (size * 4);
	uint32_t color [colors];
	char symbol [colors];
	
	for (int i = 0; i < colors; i++){
		sscanf (map [i+1], "%c %x", &symbol[i], &color[i]);
	}
	
	*wd = width;
	*ht = height;
	
	char *line = (char *) malloc (width);
	
	
	for (int i = 0; i < height; i++){
		memcpy (line, map [ i + colors + 1], width);
		for (int j = 0; j < width; j++){
			char sym = line[j];
			
			int index = 0;
			for (int k = 0; k < colors; k++){
				if (sym == symbol[k]){
					index = k;
					break;
				}
			}
			uint32_t colr = color[index];
			graphic[(i * width) + j] = colr;
		}
	}
	return graphic;
}

void draw_pixmap (pixmap *xpm, uint16_t gx, uint16_t gy, double rotation_degrees){
	
	int center_x = (xpm->width / 2) - 1;
	int center_y = (xpm->height / 2) - 1;
	int x = gx;
	int y = gy;
	double degrees = 180 - rotation_degrees;
	
	for (int i = 0; i < xpm->height; i++)
		for (int j = 0; j < xpm->width; j++){
			uint32_t color = xpm->map [(i * xpm->width) + j];
			mvector pivot;
			pivot.x = center_x - j;
			pivot.y = center_y - i;
			mvector_rotate(&pivot, degrees);
			pivot.x += center_x; 
			pivot.y += center_y;
			x = round(pivot.x + gx - center_x);
			y = round(pivot.y + gy - center_y);
		
			if (color &&color != COLOR_IGNORED){
				if ( draw_pixel(x, y, color) )
						continue;
			}
		}
}

Bitmap* loadBitmap(const char* filename)
{
	//allocating size
	Bitmap* bmp = (Bitmap*) malloc(sizeof(Bitmap));
	
	//open file in binary mode
	FILE *filePtr;
	filePtr = fopen(filename, "rb");
	
	if(filePtr == NULL)
		return NULL;
	
	
	//read the bitmap's file header
	BitmapFileHeader bitmapFileHeader;
	fread(&bitmapFileHeader, 2, 1, filePtr);
	
	//verify correct file type
	if (bitmapFileHeader.type != 0x4D42)
	{
		fclose(filePtr);
		return NULL;
	}
	
	int rd;
	
	do
	{
		if ( (rd = fread(&bitmapFileHeader.size, 4, 1, filePtr)) != 1)
			  break;
		
		if ( (rd = fread(&bitmapFileHeader.reserved, 4, 1, filePtr)) != 1)
			break;
		
		if ( (rd = fread(&bitmapFileHeader.offset, 4, 1, filePtr)) != 1)
			break;
	}while (0);
	
	if (rd != 1)
	{
		fprintf(stderr, "Error reading file\n");
		exit(-1);
	}
	
	//read bitmap's info header
	BitmapInfoHeader bitmapInfoHeader;
	fread(&bitmapInfoHeader, sizeof(BitmapInfoHeader), 1, filePtr);

	//move file pointer to beginning of bitmap data
	fseek(filePtr, bitmapFileHeader.offset, SEEK_SET);
	
	//alocate memory to image data
	unsigned char* bitmapImage = (unsigned char*) malloc(bitmapInfoHeader.imageSize);
	
	
	
	//verify allocation
	if (!bitmapImage)
	{
		free(bitmapImage);
		fclose(filePtr);
		return NULL;
	}
	
	//read image data
	fread(bitmapImage, bitmapInfoHeader.imageSize, 1, filePtr);
	
	//making sure data was read
	if (bitmapImage == NULL)
	{
		fclose(filePtr);
		return NULL;
	}
	
	//close file and return image data
	fclose(filePtr);
	
	bmp->bitmapData = bitmapImage;
	bmp->bitmapInfoHeader = bitmapInfoHeader;
	
	return bmp;
}

void drawBitmap(Bitmap* bmp, int x, int y) {
	
	if (bmp == NULL)
		return;
	
	int width = bmp->bitmapInfoHeader.width;
	int height = bmp->bitmapInfoHeader.height;
	
	if (x + width < 0 || x > hres || y + height < 0 || y > vres )
		return;
	
	unsigned char* imgStartPos;
	imgStartPos = bmp->bitmapData;
	
	for (int i = height; i != 0; i--)
		for (int j = 0; j < width; j++){
			uint32_t color = (imgStartPos [(i * width * 3) + (j * 3 + 2)] << 16);
			color += (imgStartPos [(i * width * 3) + (j * 3 + 1)] << 8);
			color += (imgStartPos [(i * width * 3) + (j * 3)]);
			if (color && color != COLOR_IGNORED)
				if (draw_pixel(x + j, y + height - i, color) )
					continue;
		}
}

void deleteBitmap(Bitmap* bmp)
{
	if (bmp == NULL)
		return;
	
	free(bmp->bitmapData);
	free(bmp);
}


