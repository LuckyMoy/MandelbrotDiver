#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include "ImageGenerator.h"

#define NMAX_PAS 0.1
#define ZOOM_PAS 0.1
#define FIRST_N_MAX 50
#define FIRST_CENTRE_R -0.65
#define FIRST_CENTRE_I 0

// gcc -Wall -Werror MandelbrotDiver_parallel.c $(pkg-config --cflags --libs sdl2 SDL2_image SDL2_gfx) ImageGenerator_parallel.c -o MandelbrotDiver_parallel 

comp calcNewCentreZoom(int cursorX, int cursorY, comp currentCenter, double currentZoom, double newZoom) {
    // Conversion des coordonnées du curseur en coordonnées complexes
    double cursor_reel = currentCenter.re + (cursorX - (TAILLE_R / 2)) * RES_R / currentZoom;
    double cursor_imaginaire = currentCenter.im - (cursorY - (TAILLE_I / 2)) * RES_I / currentZoom;
    
    // Calcul du vecteur de déplacement dans le plan complexe
    double deplacement_reel = cursor_reel - currentCenter.re;
    double deplacement_imaginaire = cursor_imaginaire - currentCenter.im;

    // Ajustement du vecteur de déplacement en fonction du changement de zoom
    deplacement_reel *= (currentZoom / newZoom);
    deplacement_imaginaire *= (currentZoom / newZoom);

    // Calcul des nouvelles coordonnées complexes du centre
    comp newCenter;
    newCenter.re = cursor_reel - deplacement_reel;
    newCenter.im = cursor_imaginaire - deplacement_imaginaire;

    return newCenter;
}

int main(int argc, char *argv[]) {
    const int WINDOW_WIDTH = TAILLE_R;
    const int WINDOW_HEIGHT = TAILLE_I;
    double zoom = 1, lastZoom = zoom;
    int nMax = FIRST_N_MAX, last_nMax = nMax;
    int cursorMooveX, cursorMooveY;
    comp centre = {FIRST_CENTRE_R, FIRST_CENTRE_I};

    int nprocs = (int)sysconf(_SC_NPROCESSORS_ONLN);
    if (nprocs < 1) {
        printf("sysconf error\n");
        return EXIT_FAILURE;
    }
    printf("Number of available logical CPU cores: %i\n", nprocs);


    // Initialisation de SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_Log("Erreur lors de l'initialisation de SDL : %s", SDL_GetError());
        return 1;
    }

    // Création de la fenêtre
    SDL_Window *window = SDL_CreateWindow("Mandelbrot Diver", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    if (!window) {
        SDL_Log("Erreur lors de la création de la fenêtre : %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Création du renderer
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        SDL_Log("Erreur lors de la création du renderer : %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Charger l'image de fond
    genImage(centre, zoom, nMax, nprocs);
    SDL_Surface *backgroundSurface = SDL_LoadBMP("image.bmp");
    SDL_Texture *backgroundTexture = SDL_CreateTextureFromSurface(renderer, backgroundSurface);
    SDL_FreeSurface(backgroundSurface);

    // Dessiner l'arrière-plan
    SDL_RenderCopy(renderer, backgroundTexture, NULL, NULL);

    // Mise à jour de l'écran
    SDL_RenderPresent(renderer);
    remove("image.bmp");

    // Boucle principale
    bool running = true, mooved = false;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) { // Ou SDL_WaitEvent(&event) si vous n'avez pas besoin de boucler rapidement
            switch (event.type) {
                case SDL_QUIT:
                    // Gérer la sortie de l'application
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    SDL_GetMouseState(&cursorMooveX, &cursorMooveY);
                    printf("mouse down\n");
                    mooved = true;
                    break;
                case SDL_MOUSEBUTTONUP:
                    printf("mouse up\n");
                    mooved = false;
                    break;
                case SDL_MOUSEWHEEL:
                    if (event.wheel.y > 0) { // défilement vers le haut
                        zoom*= 1.+ ZOOM_PAS; // Augmenter le niveau de zoom
                    } else if (event.wheel.y < 0) { // défilement vers le bas
                        zoom = zoom*(1. - ZOOM_PAS); // Diminuer le niveau de zoom
                    }
                    break;
                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym) {
                        case SDLK_ESCAPE:
                            running = false;
                            break;

                        case SDLK_LEFT:
                            nMax = fmax(5, nMax* (1-NMAX_PAS));
                            // printf("N max -= %i\n", NMAX_PAS);
                            break;

                        case SDLK_RIGHT:
                            nMax *= 1.1 + NMAX_PAS;
                            printf("N max += %f\n", NMAX_PAS);
                            break;

                        case SDLK_SPACE:
                            zoom = 1;
                            nMax = FIRST_N_MAX;
                            centre.re = FIRST_CENTRE_R;
                            centre.im = FIRST_CENTRE_I;
                            // astuce pour le pas déclencher calcNewCentre()
                            lastZoom = zoom;
                            last_nMax = 0;
                            break;
                    }
                    break;
            }
        }
        int mouseX, mouseY;
        if(mooved || nMax != last_nMax || zoom != lastZoom) {
            int endX, endY;
            SDL_GetMouseState(&endX, &endY);
            if(zoom != lastZoom) {
                SDL_GetMouseState(&mouseX, &mouseY);
                centre = calcNewCentreZoom(mouseX, mouseY, centre, lastZoom, zoom);
                lastZoom = zoom;
            }
            else if(mooved ) {
                if( !(endX!=cursorMooveX || endY!=cursorMooveY)) continue;
                // mooved = false;
                centre.re = centre.re - (endX - cursorMooveX) * RES_R / zoom;
                centre.im = centre.im + (endY - cursorMooveY) * RES_I / zoom;
                cursorMooveX = endX;
                cursorMooveY = endY;
            }
            else {
                last_nMax = nMax;
            }

            printf("%lf + %lfi ; x%i ; N max = %i", centre.re, centre.im, (int)zoom, nMax);
            clock_t start = clock();
            genImage(centre, zoom, nMax, nprocs);
            clock_t end = clock();
            double time_spent = (double)(end - start) * 1000.0 / CLOCKS_PER_SEC / nprocs;
            printf(" gen time : %.3lf ms FPS:%i\n", time_spent, (int)(1/(time_spent/1000.)));

            backgroundSurface = SDL_LoadBMP("image.bmp");
            backgroundTexture = SDL_CreateTextureFromSurface(renderer, backgroundSurface);
            SDL_FreeSurface(backgroundSurface);
            
            // Effacement de l'écran
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Fond noir
            SDL_RenderClear(renderer);

            // Dessiner l'arrière-plan
            SDL_RenderCopy(renderer, backgroundTexture, NULL, NULL);

            // Mise à jour de l'écran
            SDL_RenderPresent(renderer);
            remove("image.bmp");
        }
    }

    // Nettoyage
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
