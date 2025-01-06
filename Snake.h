#include"./SDL2-2.0.10/include/SDL.h"
#include"./SDL2-2.0.10/include/SDL_main.h"
#include<string.h>

#pragma once
#define GRID_WIDTH 32
#define GRID_HEIGHT 24
#define STARTING_LENGTH 2
#define STARTING_SPEED 1
#define MIN_TIME_TO_RED_DOT 5
#define MAX_TIME_TO_RED_DOT 10
#define HOW_LONG_RED_DOT_LASTS 10
#define POINTS_FOR_RED 5
#define POINTS_FOR_BLUE 1
#define SLOW_MULTIPLIER 1.1
#define RED_DOT_LENGTH_LOSS 3
#define TIME_TO_SPEED 30
#define SPEED_MULTIPLIER 0.8
#define SAVE_FILE_NAME "savegame.txt"
#define CELL_SIZE 20
#define PULSATING_SCALE 0.2
#define MAX_TELEPORTS 10
#define NUM_TELEPORTS 2
#define CONFIG_FILE_NAME "config.txt"

typedef struct {
	int grid_height;
	int grid_width;
	int starting_length;
	float starting_speed,slow_multiplier;
	int min_time_to_red_dot, max_time_to_red_dot, points_for_red, points_for_blue;
	float speed_multiplier;
} Config;

struct Teleport {
	int x1, y1, x2, y2;
	int id;
	bool active;
};

class Snake
{
private:
	int grid_height,grid_width,starting_length,min_time_to_red_dot, max_time_to_red_dot, points_for_red, points_for_blue;
	float starting_speed,  slow_multiplier;
	Teleport teleports[MAX_TELEPORTS];
	int numTeleports;
	double animationPhase = 0.0;
	int grid[GRID_HEIGHT][GRID_WIDTH];
	int headX, headY, directionX, directionY;
	bool checkCollision();
	bool checkIncomingCollision();
	void spawnBlueDot();
	void spawnRedDot();
	void autoPilot();
	bool redDotLoading;
	int  snakeLength;
	void generateTeleports();
	void loadConfig(const char* filename);	
	int targetX, targetY;
	
public:
	float speed_multiplier;
	bool autoplayer=0;
	int  points=0;
	double speed=1;
	void setSpeed(double speed);
	double timeToRedDot;
	bool redDot;
	int gameOver;
	Snake();
	void reset();
	void changeDirection(int dx, int dy);
	void update(double WorldTime);
	void draw(SDL_Surface* screen, int cellSize, Uint32 color, SDL_Surface *charset);
	void saveGame(double time);
	void loadGame(double &time);
	
};

