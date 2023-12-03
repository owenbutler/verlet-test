#pragma once

#include <stdbool.h>
#include <SDL.h>
#include <SDL_image.h>

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

SDL_Window* gWindow;
SDL_Renderer* gRenderer;

void doInput(void);

void init_SDL();

Uint64 gTickStart;
Uint64 gTickCount;
float elapsedMS;

typedef struct {
	bool dumpImages;
	bool resetBalls;
	bool start;
} inputs;

inputs gInputs;

void tick_start();

void tick_end();

// Utilities
float sqrt7(float x);
double remap(double n, double start1, double stop1, double start2, double stop2);
int remapi(double n, double start1, double stop1, double start2, double stop2);