#pragma once

#include <stdbool.h>

#include "system.h"

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

// Game data
#define MAX_BALLS 16850
#define BALL_SIZE 8
#define NUM_STEPS 8



ball gBalls[MAX_BALLS] = { 0 };
int gTotalBalls = 0;

const int limitFramerate = 0;

bool spawnBalls = false;

#define MAX_GRID_INDICES 4

typedef struct collision_grid_cell {
	int num_balls;
	int ball_indices[MAX_GRID_INDICES];
} collision_grid_cell;

#define GRID_WIDTH (SCREEN_WIDTH / BALL_SIZE + 2)
#define GRID_HEIGHT (SCREEN_HEIGHT / BALL_SIZE + 2)

collision_grid_cell collision_grid[(GRID_WIDTH) * (GRID_HEIGHT)];

const float quick_check = (BALL_SIZE) * (BALL_SIZE);

int nextBallSpawn = 0;


void update_all(float delta_time);

int grid_index_for(ball* ball);
void place_ball_in_grid_at(int ball_index, int grid_index);

void collisions();
void check_collisions(collision_grid_cell* cell1, collision_grid_cell* cell2);
void check_collision(ball* ball1, ball* ball2);

void dumpColors();
void check_spawn_new_ball();