
#include <stdio.h>
#include <SDL.h>
#include <SDL_image.h>

#include "system.h"

inputs gInputs = { 0 };
inputs emptyInputs = { 0 };

void doInput(void) {
	SDL_Event event;

	gInputs = emptyInputs;

	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_QUIT:
			exit(0);
			break;
		case SDL_KEYUP:
			switch (event.key.keysym.sym) {
			case SDLK_q:
				exit(0);
				break;
			case SDLK_p:
				gInputs.dumpImages = true;
				break;
			case SDLK_r:
				gInputs.resetBalls = true;
				break;
			case SDLK_SPACE:
				gInputs.start = true;
				break;
			}
		default:
			break;
		}
	}
}

void init_SDL() {
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("Couldn't initialize SDL: %s\n", SDL_GetError());
		exit(1);
	}

	int imgFlags = IMG_INIT_PNG;
	if (!(IMG_Init(imgFlags) & imgFlags)) {
		printf("Couldn't initialize SDL_Image: %s\n", SDL_GetError());
		exit(1);
	}

	int windowFlags = 0;
	gWindow = SDL_CreateWindow("verlet test", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 720, windowFlags);

	if (!gWindow) {
		printf("Failed to open %d x %d window: %s\n", SCREEN_WIDTH, SCREEN_HEIGHT, SDL_GetError());
		exit(1);
	}

	gRenderer = SDL_CreateRenderer(gWindow, -1, 0);
	if (!gRenderer) {
		printf("Failed to create renderer: %s\n", SDL_GetError());
		exit(1);
	}
}

Uint64 gTickStart;
Uint64 gTickCount;
float elapsedMS;

void tick_start() {
	gTickStart = SDL_GetPerformanceCounter();
	gTickCount++;
}

void tick_end() {
	Uint64 end = SDL_GetPerformanceCounter();

	elapsedMS = (end - gTickStart) / (float)SDL_GetPerformanceFrequency() * 1000.0f;

	if (elapsedMS < 16.666f) {
		// Cap to 60 FPS
		SDL_Delay((int)floor(16.666f - elapsedMS));
	}
}

// Utilities

float sqrt7(float x)
{
	unsigned int i = *(unsigned int*)&x;
	// adjust bias
	i += 127 << 23;
	// approximation of square root
	i >>= 1;
	return *(float*)&i;
}

double remap(double n, double start1, double stop1, double start2, double stop2) {
	return (n - start1) / (stop1 - start1) * (stop2 - start2) + start2;
}

int remapi(double n, double start1, double stop1, double start2, double stop2) {
	return (int)((n - start1) / (stop1 - start1) * (stop2 - start2) + start2);
}

