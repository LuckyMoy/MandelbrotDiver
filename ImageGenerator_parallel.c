#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
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

typedef struct Thread_data_{
    int start, end, id, nmax;
    int* done;
    int** matrix;
    comp centre;
    double zoom;
} Thread_data;

void* thread_function(void* arg) {
    Thread_data *data = (Thread_data*) arg;
    for(int i = data->start; i < data->end; i++) {
        for(int r = -TAILLE_R/2; r < TAILLE_R/2; r++) {
            // printf("thrd %i : %i %i\r", data->id, r, i);
            comp c;
            c.re = data->centre.re + (r * RES_R / data->zoom);
            c.im = data->centre.im + (i * RES_I / data->zoom); 
            (data->matrix)[i + TAILLE_I/2][r + TAILLE_R/2] = calcSuite(c, data->nmax);
        }
    }
    // printf("thead %i ok \n", data->id);
    (*data->done)++;
    return NULL;
}

int genImage(comp centre, double zoom, int nMax, int nprocs) {

    FILE* BMPfile = initBMP("image.bmp");
    int **matrix = (int **) malloc(sizeof(int*) *TAILLE_I);
    for(int i = 0 ; i< TAILLE_I ; i++) matrix[i] = (int *) malloc(sizeof(int)*TAILLE_R);

    int doneCtr = 0;

    Thread_data thrds_data[nprocs];
    pthread_t threads[nprocs];

    for(int i = 0; i<nprocs ; i++) {
        thrds_data[i].start = (TAILLE_I/nprocs)*i - TAILLE_I/2;
        thrds_data[i].end = (TAILLE_I/nprocs)*(i+1) - TAILLE_I/2;
        thrds_data[i].done = &doneCtr;
        thrds_data[i].matrix = matrix;
        thrds_data[i].id = i;
        thrds_data[i].centre = centre;
        thrds_data[i].nmax = nMax;
        thrds_data[i].zoom = zoom;
        
        if (pthread_create(&threads[i], NULL, thread_function, (void *) &thrds_data[i])) {
            fprintf(stderr, "Error creating thread %i\n", i);
            return EXIT_FAILURE;
        }
    }

    while(doneCtr < nprocs);

    for(int i = -TAILLE_I/2; i < TAILLE_I/2; i++) {

        for(int r = -TAILLE_R/2; r < TAILLE_R/2; r++) {
            recordImageBMP(matrix[i + TAILLE_I/2][r + TAILLE_R/2], BMPfile, nMax);
        }
        free(matrix[i + TAILLE_I/2]);
        paddingBMP(BMPfile);
    }
    free(matrix);
    fclose(BMPfile);
    // printf("Done.\n");

    return 0;
}