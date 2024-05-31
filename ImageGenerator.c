#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "ImageGenerator.h"

int calcSuite(comp c, int nMax) {
    comp Z;
    Z.re = c.re;
    Z.im = c.im;
    int n = 1;

    while(n <= nMax && sqrt(Z.re * Z.re + Z.im * Z.im) <= SEUIL_DIV) {
        double re = Z.re;
        Z.re = re*re - Z.im*Z.im + c.re;
        Z.im = 2*re*Z.im + c.im;
        n++;
    }

    return n > nMax ? -1 : n;
}

FILE* initBMP(char *filename) {
    int width = TAILLE_R;
    int height = TAILLE_I;
    FILE *file = fopen(filename, "wb");
    if (!file) {
        printf("initBMP : Unable to open '%s'\n", filename);
        exit(1);
    }

    BMPHeader bmpHeader = {0};
    int bytesPerPixel = 3;
    int padding = (4 - (width * bytesPerPixel) % 4) % 4;
    int rowSize = (width * bytesPerPixel) + padding;
    bmpHeader.fileType = 0x4D42;
    bmpHeader.fileSize = sizeof(BMPHeader) + rowSize * height;
    bmpHeader.offsetData = sizeof(BMPHeader);
    bmpHeader.size = 40;
    bmpHeader.width = width;
    bmpHeader.height = height;
    bmpHeader.planes = 1;
    bmpHeader.bitCount = 24;
    bmpHeader.compression = 0;
    bmpHeader.imageSize = rowSize * height;

    fwrite(&bmpHeader, sizeof(BMPHeader), 1, file);

    return file;
}

void recordImageBMP(int val, FILE* BMPfile, int nMax) {
    uint8_t colorValue = (uint8_t)(float)val/(float)nMax*255;
    uint8_t colorValues[3];
    if(val == -1) {
        colorValues[0] = 255 * ENSEMBLE_B;
        colorValues[1] = 255 * ENSEMBLE_G;
        colorValues[2] = 255 * ENSEMBLE_R;
    }
    else {
        colorValues[0] = (uint8_t)((float) colorValue * BLUE);
        colorValues[1] = (uint8_t)((float) colorValue * GREEN);
        colorValues[2] = (uint8_t)((float) colorValue * RED);
    }
    fwrite(&colorValues, sizeof(uint8_t), 3, BMPfile);
}

void paddingBMP(FILE* BMPfile) {
    int padding = (4 - (TAILLE_R * 3) % 4) % 4;
    for (int p = 0; p < padding; p++) {
        fputc(0x00, BMPfile);
    }
}

int genImage(comp centre, double zoom, int nMax) {

    // printf("%lf + %lfi ; x%i ; N max = %i", centre.re, centre.im, (int)zoom, nMax);


    FILE* BMPfile = initBMP("image.bmp");

    comp c;
    for(int i = -TAILLE_I/2; i < TAILLE_I/2; i++) {
        // if(TAILLE_I >= 1000 && ((i + TAILLE_I/2) % (TAILLE_I / 1000)) == 0) {
        //     // printf("\rGenerating ... %.1f%%", (float)(i+TAILLE_I/2) / (float)TAILLE_I * 100.);
        //     fflush(stdout);
        // }

        c.im = centre.im + (i * RES_I / zoom); 
        for(int r = -TAILLE_R/2; r < TAILLE_R/2; r++) {
            c.re = centre.re + (r * RES_R / zoom);
            recordImageBMP(calcSuite(c, nMax), BMPfile, nMax);
        }
        paddingBMP(BMPfile);
    }
    // printf("\rGenerating ... 100.0%%\n");
    fclose(BMPfile);
    // printf("Done.\n");

    return 0;
}