#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include <SDL_image.h>

#include "system.h"
#include "colors.h"

// Game data
typedef struct ball {
	float x;
	float y;
	float lastX;
	float lastY;
	int spawnCounter;
	int w;
	int h;
	Uint8 r;
	Uint8 g;
	Uint8 b;
	Uint8 a;
} ball;

#define MAX_BALLS 16850
#define BALL_SIZE 8
#define NUM_STEPS 8

const int field_width = 1280 * 2;
const int field_height = 720 * 2;

ball gBalls[MAX_BALLS] = { 0 };
int gTotalBalls = 0;

const int limitFramerate = 0;

bool spawnBalls = false;

void update_all(float delta_time) {
	float accelX = 0.0f;
	float accelY = 1000.0f;

	float dts = delta_time * delta_time;

	int maxBalls = MAX_BALLS;
	for (int i = 0; i < maxBalls; i++) {
		ball* pBall = gBalls + i;

		float lastMoveX = pBall->x - pBall->lastX;
		float lastMoveY = pBall->y - pBall->lastY;

		float new_x = pBall->x + lastMoveX + (accelX * dts);
		float new_y = pBall->y + lastMoveY + (accelY * dts);

		pBall->lastX = pBall->x;
		pBall->lastY = pBall->y;

		float half_size = BALL_SIZE / 2.0f;
		new_x = new_x > half_size ? new_x : half_size;
		new_y = new_y > half_size ? new_y : half_size;
		new_x = new_x < SCREEN_WIDTH - half_size ? new_x : SCREEN_WIDTH - half_size;
		new_y = new_y < SCREEN_HEIGHT - half_size ? new_y : SCREEN_HEIGHT - half_size;

		pBall->x = new_x;
		pBall->y = new_y;
	}
}

#define MAX_GRID_INDICES 4

typedef struct collision_grid_cell {
	int num_balls;
	int ball_indices[MAX_GRID_INDICES];
} collision_grid_cell;

#define GRID_WIDTH (SCREEN_WIDTH / BALL_SIZE + 2)
#define GRID_HEIGHT (SCREEN_HEIGHT / BALL_SIZE + 2)

collision_grid_cell collision_grid[(GRID_WIDTH) * (GRID_HEIGHT)];

int grid_index_for(ball* ball) {
	int index_x = ((int)(ball->x) / BALL_SIZE) + 1;
	int index_y = ((int)(ball->y) / BALL_SIZE) + 1;

	return index_y * GRID_WIDTH + index_x;
}

void place_ball_in_grid_at(int ball_index, int grid_index) {
	collision_grid_cell* grid_cell = collision_grid + grid_index;

	if (grid_cell->num_balls < MAX_GRID_INDICES) {
		grid_cell->ball_indices[grid_cell->num_balls] = ball_index;
		grid_cell->num_balls++;
	}
}

const float quick_check = (BALL_SIZE) * (BALL_SIZE);

void check_collision(ball* ball1, ball* ball2) {

	if (ball1 == ball2) {
		return;
	}

	float dx = ball1->x - ball2->x;
	float dy = ball1->y - ball2->y;

	float d2 = dx * dx + dy * dy;
	if (d2 < quick_check) {
		float distance = sqrt7(d2);

		float delta = (float) ((float)BALL_SIZE - distance) * 0.5f;
		float hx = (dx / distance) * delta;
		float hy = (dy / distance) * delta;

		ball1->x += hx;
		ball1->y += hy;
		ball2->x -= hx;
		ball2->y -= hy;
	}
}

void check_collisions(collision_grid_cell* cell1, collision_grid_cell* cell2) {

	for (int i = 0; i < cell1->num_balls; i++) {
		for (int j = 0; j < cell2->num_balls; j++) {
			check_collision(gBalls + cell1->ball_indices[i], gBalls + cell2->ball_indices[j]);
		}
	}
}

void collisions() {

	// clear the collision grid
	int totalGridCells = GRID_WIDTH * GRID_HEIGHT;
	for (int i = 0; i < totalGridCells; i++) {
		collision_grid[i].num_balls = 0;
	}

	// place an index of each ball in the grid
	for (int ball_index = 0; ball_index < gTotalBalls; ball_index++) {
		int grid_index = grid_index_for(gBalls + ball_index);
		place_ball_in_grid_at(ball_index, grid_index);
	}
	
	for (int col = 1; col < GRID_WIDTH - 1; col++) {
		for (int row = 1; row < GRID_HEIGHT - 1; row++) {

			collision_grid_cell* center_cell = collision_grid + (row * GRID_WIDTH + col);
			if (center_cell->num_balls == 0) {
				continue;
			}

			check_collisions(center_cell, center_cell);

			check_collisions(center_cell, collision_grid + ((row + 1) * GRID_WIDTH + col)); // n
			check_collisions(center_cell, collision_grid + ((row + 1) * GRID_WIDTH + col + 1)); // ne
			check_collisions(center_cell, collision_grid + (row * GRID_WIDTH + col + 1)); // e
			check_collisions(center_cell, collision_grid + ((row - 1) * GRID_WIDTH + col + 1)); // se
			check_collisions(center_cell, collision_grid + ((row - 1) * GRID_WIDTH + col)); // s
			check_collisions(center_cell, collision_grid + ((row - 1) * GRID_WIDTH + col - 1)); // sw
			check_collisions(center_cell, collision_grid + (row * GRID_WIDTH + col - 1)); // w
			check_collisions(center_cell, collision_grid + ((row + 1) * GRID_WIDTH + col - 1)); // nw
		}
	}
}

int nextBallSpawn = 0;

void check_spawn_new_ball() {

	if (!spawnBalls) {
		return;
	}

	if (gTotalBalls >= MAX_BALLS) {
		return;
	}

	if (nextBallSpawn > gTickCount) {
		return;
	}

	if (elapsedMS > 16 && limitFramerate) {
		return;
	}

	int r = remapi(sin(gTickCount / 77.0), -1, 1, 70, 255);
	int g = remapi(cos(gTickCount / 57.0), -1, 1, 80, 255);
	int b = remapi(sin(gTickCount / 52.0), -1, 1, 60, 255);

	float velX = 1.9f;

	int y = 10;
	int yDiff = 0;
	int yInc = -9;
	int numSpawn = 12;
	for (int i = 0; i < numSpawn; i++, yDiff += yInc) {

		gBalls[gTotalBalls].x = 5;
		gBalls[gTotalBalls].y = (float)y - yDiff;
		gBalls[gTotalBalls].lastX = (float)5 - velX;
		gBalls[gTotalBalls].lastY = (float)y - yDiff;
		gBalls[gTotalBalls].w = BALL_SIZE;
		gBalls[gTotalBalls].h = BALL_SIZE;
		gBalls[gTotalBalls].r = colors[gTotalBalls].r;
		gBalls[gTotalBalls].g = colors[gTotalBalls].g;
		gBalls[gTotalBalls].b = colors[gTotalBalls].b;

		gTotalBalls++;

		if (gTotalBalls >= MAX_BALLS) {
			return;
		}

		gBalls[gTotalBalls].x = 1275;
		gBalls[gTotalBalls].y = (float)y - yDiff;
		gBalls[gTotalBalls].lastX = (float)(1275 + 1.2);
		gBalls[gTotalBalls].lastY = (float)y - yDiff;
		gBalls[gTotalBalls].w = BALL_SIZE;
		gBalls[gTotalBalls].h = BALL_SIZE;
		gBalls[gTotalBalls].r = colors[gTotalBalls].r;
		gBalls[gTotalBalls].g = colors[gTotalBalls].g;
		gBalls[gTotalBalls].b = colors[gTotalBalls].b;

		gTotalBalls++;

		if (gTotalBalls >= MAX_BALLS) {
			return;
		}
	}

}

void dumpColors();

int main(int argc, char* argv[]) {

	init_SDL();

	SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 255);

	SDL_Texture* ballTexture = IMG_LoadTexture(gRenderer, "circle.png");
	if (ballTexture == NULL) {
		printf("Failed to load PNG: %s\n", SDL_GetError());
		exit(1);
	}

	float scaleFactor = (float)SCREEN_WIDTH / 1280.0f;

	SDL_Rect dstrect = {.x = 1280/2, .y = 720/2, .w = (int) (BALL_SIZE / scaleFactor), .h = (int) (BALL_SIZE / scaleFactor)};

	while (1) {
		tick_start();

		SDL_RenderClear(gRenderer);

		doInput();

		check_spawn_new_ball();

		if (gInputs.resetBalls) {
			gTotalBalls = 0;
		}

		if (gInputs.dumpImages) {
			dumpColors();
		}

		if (gInputs.start) {
			spawnBalls = true;
		}

		// update
		float delta_time = 16.0f / (float) NUM_STEPS / 1000.0f;		
		for (int step = 0; step < NUM_STEPS; step++) {
			collisions();
			update_all(delta_time);
		}

		// render
		for (int i = 0; i < gTotalBalls; i++) {
			ball* pBall = gBalls + i;

			int half = pBall->w / 2;

			SDL_SetTextureColorMod(ballTexture, pBall->r, pBall->g, pBall->b);
			dstrect.x = (int) ((pBall->x - half) / scaleFactor);
			dstrect.y = (int) ((pBall->y - half) / scaleFactor);
			SDL_RenderCopy(gRenderer, ballTexture, NULL, &dstrect);
		}

		SDL_RenderPresent(gRenderer);

		tick_end();
	}

	return 0;
}

void dumpColors() {

	SDL_Surface* image = IMG_Load("unicorn.png");
	SDL_Surface* formattedImage = SDL_ConvertSurfaceFormat(image, SDL_PIXELFORMAT_RGBA8888, 0);

	SDL_LockSurface(formattedImage);

	Uint32* pixels = (Uint32*)formattedImage->pixels;

	FILE* f = fopen("colors.h", "w");
	fprintf(f, "#pragma once\n\n\n");
	fprintf(f, "typedef struct {\n");
	fprintf(f, "\tUint8 r;\n");
	fprintf(f, "\tUint8 g;\n");
	fprintf(f, "\tUint8 b;\n");
	fprintf(f, "} color;\n\n\n");
	fprintf(f, "color colors[] = {\n");


	for (int i = 0; i < gTotalBalls; i++) {
		ball* pBall = gBalls + i;

		int ballX = (int) pBall->x;
		int ballY = (int) pBall->y;

		Uint32 pixel = pixels[ballY * 1280 + ballX];

		Uint8 r = (pixel & 0xFF000000) >> 24;
		Uint8 g = (pixel & 0x00FF0000) >> 16;
		Uint8 b = (pixel & 0x0000FF00) >> 8;

		pBall->r = r;
		pBall->g = g;
		pBall->b = b;

		fprintf(f, "{%u, %u, %u},\n", r, g, b);
	}

	fprintf(f, "}\n");

	fclose(f);

	SDL_UnlockSurface(formattedImage);

	SDL_FreeSurface(image);
	SDL_FreeSurface(formattedImage);
}