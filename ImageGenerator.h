#include <stdint.h>

#define TAILLE_R 400
#define TAILLE_I TAILLE_R
#define RES_R 3./TAILLE_R
#define RES_I 3./TAILLE_I

#define RED 1.
#define GREEN .05
#define BLUE .05
#define ENSEMBLE 1.
#define ENSEMBLE_R ENSEMBLE * RED
#define ENSEMBLE_G ENSEMBLE * GREEN
#define ENSEMBLE_B ENSEMBLE * BLUE

#define SEUIL_DIV 2.0

#pragma pack(push, 1)
typedef struct {
    uint16_t fileType;        // "BM"
    uint32_t fileSize;        
    uint16_t reserved1;       
    uint16_t reserved2;       
    uint32_t offsetData;      
    uint32_t size;            
    int32_t width;            
    int32_t height;           
    uint16_t planes;          
    uint16_t bitCount;        
    uint32_t compression;     
    uint32_t imageSize;       
    int32_t xPixelsPerMeter;  
    int32_t yPixelsPerMeter;  
    uint32_t colorsUsed;      
    uint32_t colorsImportant; 
} BMPHeader;
#pragma pack(pop)

typedef struct comp_{
    double re,im;
}comp;

int genImage(comp centre, double zoom, int nMax, int);