#include "Snake.h"
#include "./SDL2-2.0.10/include/SDL.h"
#include<stdio.h>
#include<cstdlib>



Snake::Snake() {
	reset();
}
void Snake::reset() {

	grid_height = GRID_HEIGHT;
	grid_width = GRID_WIDTH;
	starting_length = STARTING_LENGTH;
	starting_speed = STARTING_SPEED;
	min_time_to_red_dot = MIN_TIME_TO_RED_DOT;
	max_time_to_red_dot = MAX_TIME_TO_RED_DOT;
	points_for_red = POINTS_FOR_RED;
	points_for_blue = POINTS_FOR_BLUE;
	speed_multiplier = SPEED_MULTIPLIER;
	slow_multiplier = SLOW_MULTIPLIER;

	loadConfig(CONFIG_FILE_NAME);

	// Zerowanie siatki
	for (int i = 0; i < grid_height; i++)
		for (int j = 0; j < grid_width; j++)
			grid[i][j] = 0;

	// Ustawienie pocz¹tkowej pozycji wê¿a
	headX = grid_width / 2;
	headY = grid_height / 2;
	directionX = 1; // Pocz¹tkowy kierunek: prawo
	directionY = 0;
	snakeLength = starting_length;
	gameOver = 0;
	points = 0;
	speed = starting_speed;

	redDot = false;
	redDotLoading = 0;
	// Oznaczenie pocz¹tkowych segmentów wê¿a
	for (int i = 0; i < snakeLength; i++)
		grid[headY][headX - i] = snakeLength - i;

	spawnBlueDot();
	generateTeleports();
}

void Snake::changeDirection(int dx, int dy) {
	// Zmiana kierunku wê¿a 
	if (directionX + dx != 0 || directionY + dy != 0) {
		directionX = dx;
		directionY = dy;
	}
}
bool Snake::checkIncomingCollision() {
	return headX + directionX <0 || headX + directionX > grid_width - 1 || headY + directionY < 2 || headY + directionY > grid_height - 1;
}
void DrawFilledCircleOnSurface(SDL_Surface* surface, int centerX, int centerY, int radius, Uint32 color) {
	for (int w = 0; w < radius * 2; w++) {
		for (int h = 0; h < radius * 2; h++) {
			int dx = radius - w; // Odleg³oœæ od œrodka w poziomie
			int dy = radius - h; // Odleg³oœæ od œrodka w pionie
			if ((dx * dx + dy * dy) <= (radius * radius)) {
				int x = centerX + dx;
				int y = centerY + dy;
				if (x >= 0 && x < surface->w && y >= 0 && y < surface->h) {
					Uint32* pixels = (Uint32*)surface->pixels;
					pixels[y * surface->w + x] = color;
				}
			}
		}
	}
}

void Snake::spawnBlueDot() {
	int x, y;
	do {
		x = (rand() % 32) - 1;
		y = (rand() % 22) + 2;
	} while (grid[y][x] != 0);



	grid[y][x] = -1;
	targetX = x;
	targetY = y;
	
}

void Snake::spawnRedDot() {
	int x, y;
	do {
		x = (rand() % 32) - 1;
		y = (rand() % 22) + 2;
	} while (grid[y][x] != 0);

	grid[y][x] = -2;
	targetX = x;
	targetY = y;
	printf("%d\n", x);
	printf("%d\n", y);
	
}
void Snake::setSpeed(double x) {
	speed *= x;
}

void Snake::update(double worldTime) {
	animationPhase = fmod(worldTime * 2 * M_PI, 2 * M_PI);

	if (redDotLoading == 0) {
		timeToRedDot = worldTime + min_time_to_red_dot + (rand() % (max_time_to_red_dot - min_time_to_red_dot + 1));
		redDotLoading = 1;
	}
	if (worldTime > timeToRedDot && redDot == 0) {
		printf("%d %d %.2f %.2f\n", redDot, redDotLoading, worldTime, timeToRedDot);
		spawnRedDot();
		redDot = 1;
		timeToRedDot += HOW_LONG_RED_DOT_LASTS;
	}
	if (worldTime > timeToRedDot && redDot == 1) {
		redDot = 0;
		redDotLoading = 0;

		for (int i = 0; i < grid_height; i++) {
			for (int j = 0; j < grid_width; j++) {
				if (grid[i][j] == -2) grid[i][j] = 0;
			}
		}
	}

	if (autoplayer == 1) {
		autoPilot();
	}
	//zmien kierunek
	if (checkIncomingCollision()) {
		if (directionX == 1) {
			changeDirection(0, 1);
		}
		else if (directionX == -1) {
			changeDirection(0, -1);
		}
		else if (directionY == 1) {
			changeDirection(-1, 0);
		}
		else if (directionY == -1) {
			changeDirection(1, 0);
		}
		if (checkIncomingCollision()) {
			directionX = -directionX;
			directionY = -directionY;
		}
	}


	headX += directionX;
	headY += directionY;

	for (int i = 0; i < numTeleports; i++) {
		if (teleports[i].active) {
			if (headX == teleports[i].x1 && headY == teleports[i].y1) {
				headX = teleports[i].x2;
				headY = teleports[i].y2;
				break;
			}
			else if (headX == teleports[i].x2 && headY == teleports[i].y2) {
				headX = teleports[i].x1;
				headY = teleports[i].y1;
				break;
			}
		}
	}


	if (checkCollision()) {
		gameOver = 1;
		return;
	}



	if (grid[headY][headX] != -1 && grid[headY][headX] != -2) {
		for (int i = 0; i < grid_height; i++) {
			for (int j = 0; j < grid_width; j++) {
				if (grid[i][j] > 0) grid[i][j]--;
			}
		}
	}
	else if (grid[headY][headX] == -2) {
		for (int i = 0; i < grid_height; i++) {
			for (int j = 0; j < grid_width; j++) {
				if (grid[i][j] == -1) {
					targetX = j;
					targetY = i;
				}
			}
		}

		points += points_for_red;
		int los = rand() % 2;
		redDot = 0;
		redDotLoading = 0;
		if (los == 1 && snakeLength >= 6) {
			for (int i = 0; i < grid_height; i++) {
				for (int j = 0; j < grid_width; j++) {
					if (grid[i][j] > RED_DOT_LENGTH_LOSS - 1) grid[i][j] -= RED_DOT_LENGTH_LOSS;
					if (grid[i][j] <= RED_DOT_LENGTH_LOSS - 1 && grid[i][j] > 0) grid[i][j] = 0;
				}
			}
			snakeLength -= RED_DOT_LENGTH_LOSS;
			printf("los 1 -3length \n");
		}
		else {
			printf("los 2 -spowolnienie\n");
			speed *= slow_multiplier;
		}

	}
	else if (grid[headY][headX] == -1) {
		points += 1;
		spawnBlueDot();
		snakeLength++;
	}

	grid[headY][headX] = snakeLength + 1;

	// Zmniejsz wartoœci w siatce (ruch wê¿a)

}
bool Snake::checkCollision() {
	return grid[headY][headX] > 0;
}

void DrawStringTeleports(SDL_Surface* screen, int x, int y, const char* text,
	SDL_Surface* charset) {
	int px, py, c;
	SDL_Rect s, d;
	s.w = 8;
	s.h = 8;
	d.w = 8;
	d.h = 8;
	while (*text) {
		c = *text & 255;
		px = (c % 16) * 8;
		py = (c / 16) * 8;
		s.x = px;
		s.y = py;
		d.x = x;
		d.y = y;
		SDL_BlitSurface(charset, &s, screen, &d);
		x += 8;
		text++;
	};
};

void Snake::draw(SDL_Surface* screen, int cellSize, Uint32 color, SDL_Surface* charset) {
	// Pulsacja kropek
	double scaleFactor = PULSATING_SCALE;
	double baseRadius = cellSize / 2;
	double dynamicRadius = baseRadius + sin(animationPhase) * baseRadius * scaleFactor;

	// Rysowanie teleportów
	for (int i = 0; i < numTeleports; ++i) {

		int x1px = teleports[i].x1 * cellSize + cellSize / 2;
		int y1px = teleports[i].y1 * cellSize + cellSize / 2;
		int x2px = teleports[i].x2 * cellSize + cellSize / 2;
		int y2px = teleports[i].y2 * cellSize + cellSize / 2;
		DrawFilledCircleOnSurface(screen, x1px, y1px, cellSize / 3, SDL_MapRGB(screen->format, 0xFF, 0xA5, 0x00));
		DrawFilledCircleOnSurface(screen, x2px, y2px, cellSize / 3, SDL_MapRGB(screen->format, 0xFF, 0xA5, 0x00));
		char text[128];

		sprintf(text, "%d", abs(teleports[i].id + 20));
		DrawStringTeleports(screen, teleports[i].x1 * cellSize + cellSize / 2 - strlen(text) * 8 / 2, teleports[i].y1 * cellSize - cellSize / 2, text, charset);
		DrawStringTeleports(screen, teleports[i].x2 * cellSize + cellSize / 2 - strlen(text) * 8 / 2, teleports[i].y2 * cellSize - cellSize / 2, text, charset);


	}


	// Rysowanie wê¿a
	for (int i = 0; i < grid_height; i++) {
		for (int j = 0; j < grid_width; j++) {
			if (grid[i][j] > 0) {
				if (grid[i][j] != snakeLength + 1 && (grid[i][j] + snakeLength % 2) % 2 == 0) {
					SDL_Rect rect = { j * cellSize, i * cellSize, cellSize, cellSize };
					SDL_FillRect(screen, &rect, color);

				}
				else if (grid[i][j] != snakeLength + 1) {
					SDL_Rect rect = { (j * cellSize) + (cellSize / 4), (i * cellSize) + (cellSize / 4), cellSize / 2, cellSize / 2 };
					SDL_FillRect(screen, &rect, color);
				}
				else if (grid[i][j] == snakeLength + 1) {
					SDL_Rect rect = { j * cellSize, i * cellSize, cellSize, cellSize };

					SDL_Rect eye1 = { (j * cellSize) + (cellSize / 8), (i * cellSize) + (cellSize / 4), cellSize / 8, cellSize / 8 };
					SDL_Rect eye2 = { ((j + 1) * cellSize) - (cellSize / 8), (i * cellSize) + (cellSize / 4), cellSize / 8, cellSize / 8 };
					SDL_FillRect(screen, &rect, color);
					SDL_FillRect(screen, &eye2, SDL_MapRGB(screen->format, 0x00, 0x00, 0x00));
					SDL_FillRect(screen, &eye1, SDL_MapRGB(screen->format, 0x00, 0x00, 0x00));
				}


			}
			if (grid[i][j] == -1) {

				DrawFilledCircleOnSurface(screen, j * cellSize + cellSize / 2, i * cellSize + cellSize / 2, dynamicRadius, SDL_MapRGB(screen->format, 0x11, 0x11, 0xCC));

			}
			if (grid[i][j] == -2) {

				DrawFilledCircleOnSurface(screen, j * cellSize + cellSize / 2, i * cellSize + cellSize / 2, dynamicRadius, SDL_MapRGB(screen->format, 0xFF, 0x00, 0x00));

			}
		}
	}

}

void Snake::saveGame(double time) {

	FILE* saveFile = fopen(SAVE_FILE_NAME, "w");
	if (saveFile == nullptr) {
		printf("Nie uda³o siê otworzyæ pliku do zapisu!\n");
		return;
	}
	fprintf(saveFile, "%d %d\n", grid_height, grid_width);


	for (int i = 0; i < grid_height; i++) {
		for (int j = 0; j < grid_width; j++) {
			fprintf(saveFile, "%d ", grid[i][j]);
		}
		fprintf(saveFile, "\n");
	}

	fprintf(saveFile, "%d\n", points);
	fprintf(saveFile, "%f\n", speed);
	fprintf(saveFile, "%d\n", snakeLength);
	fprintf(saveFile, "%d\n", headX);
	fprintf(saveFile, "%d\n", headY);
	fprintf(saveFile, "%d %d\n", directionX, directionY);
	fprintf(saveFile, "%d\n", redDot);
	fprintf(saveFile, "%f\n", timeToRedDot);
	fprintf(saveFile, "%d\n", gameOver);
	fprintf(saveFile, "%f\n", time);
	fprintf(saveFile, "%d\n", numTeleports);
	for (int i = 0; i < numTeleports; ++i) {
		fprintf(saveFile, "%d %d %d %d %d\n", teleports[i].x1, teleports[i].y1, teleports[i].x2, teleports[i].y2, teleports[i].id);
	}
	fclose(saveFile);
	printf("Gra zosta³a zapisana do pliku savegame.txt\n");
}

void Snake::loadGame(double& time) {

	FILE* saveFile = fopen(SAVE_FILE_NAME, "r");
	if (saveFile == nullptr) {
		printf("Nie uda³o siê otworzyæ pliku do odczytu!\n");
		return;
	}


	int gridHeight, gridWidth;
	fscanf(saveFile, "%d %d\n", &gridHeight, &gridWidth);
	if (gridHeight != grid_height || gridWidth != grid_width) {
		printf("Wymiary siatki w pliku nie pasuj¹ do aktualnych ustawieñ gry!\n");
		fclose(saveFile);
		return;
	}


	for (int i = 0; i < grid_height; i++) {
		for (int j = 0; j < grid_width; j++) {
			fscanf(saveFile, "%d", &grid[i][j]);
		}
	}


	fscanf(saveFile, "%d\n", &points);
	fscanf(saveFile, "%lf\n", &speed);
	fscanf(saveFile, "%d\n", &snakeLength);
	fscanf(saveFile, "%d\n", &headX);
	fscanf(saveFile, "%d\n", &headY);
	fscanf(saveFile, "%d %d\n", &directionX, &directionY);
	fscanf(saveFile, "%d\n", &redDot);
	fscanf(saveFile, "%lf\n", &timeToRedDot);
	printf("%lf", timeToRedDot);
	fscanf(saveFile, "%d\n", &gameOver);
	fscanf(saveFile, "%lf\n", &time);
	fscanf(saveFile, "%d\n", &numTeleports);
	for (int i = 0; i < numTeleports; ++i) {

		int x1 = 0, x2 = 0, y1 = 0, y2 = 0, id = 0;
		Teleport teleport = { x1,y1,x2,y2,id,true };

		fscanf(saveFile, "%d %d %d %d %d\n", &x1, &y1, &x2, &y2, &id);
		teleport.x1 = x1;
		teleport.x2 = x2;
		teleport.y1 = y1;
		teleport.y2 = y2;
		teleport.id = id;
		teleport.active = true;
		teleports[i] = teleport;
		grid[teleport.y1][teleport.x1] = teleport.id;
		grid[teleport.y2][teleport.x2] = teleport.id;
	}
	fclose(saveFile);
	printf("Gra zosta³a wczytana z pliku savegame.txt\n");
}




void Snake::generateTeleports() {
	numTeleports = 0;
	for (int i = 0; i < NUM_TELEPORTS && i < MAX_TELEPORTS; ++i) {
		if (numTeleports < MAX_TELEPORTS) {
			int x1, x2, y1, y2;
			int id = -21 - i;
			do {

				x1 = (rand() % (grid_width - 2)) + 1;
				y1 = (rand() % (grid_height - 4)) + 3;
				x2 = (rand() % (grid_width - 2)) + 1;
				y2 = (rand() % (grid_height - 4)) + 3;
			} while ((x1 == x2 && y1 == y2) || grid[y1][x1] != 0 || grid[y2][x2] != 0);

			grid[y1][x1] = id;
			grid[y2][x2] = id;
			teleports[numTeleports] = { x1, y1, x2, y2, id, true };
			numTeleports++;
			printf("%d %d %d %d %d\n", id, x1, y1, x2, y2);
		}
	}
}


void Snake::loadConfig(const char* filename) {
	FILE* configFile = fopen(filename, "r");
	if (configFile == nullptr) {
		printf("Nie uda³o siê otworzyæ pliku konfiguracyjnego\n");
		return;
	}
	Config config;
	if (fscanf(configFile, "%d %d %d %f %f %d %d %d %d %f\n", &config.grid_height, &config.grid_width, &config.starting_length, &config.starting_speed, &config.slow_multiplier, &config.min_time_to_red_dot, &config.max_time_to_red_dot, &config.points_for_red, &config.points_for_blue, &config.speed_multiplier) != 10) {
		printf("B³¹d odczytu konfiguracji\n");
		fclose(configFile);
		return;
	}
	grid_height = config.grid_height;
	grid_width = config.grid_width;
	starting_length = config.starting_length;
	starting_speed = config.starting_speed;
	slow_multiplier = config.slow_multiplier;
	min_time_to_red_dot = config.min_time_to_red_dot;
	max_time_to_red_dot = config.max_time_to_red_dot;
	points_for_red = config.points_for_red;
	points_for_blue = config.points_for_blue;
	speed_multiplier = config.speed_multiplier;
	fclose(configFile);
}

void Snake::autoPilot() {
	


	int dx = targetX - headX;
	int dy = targetY - headY;


	bool path1_possible = true;
	for (int x = headX + (dx > 0 ? 1 : -1); x != targetX; x += (dx > 0 ? 1 : -1)) {
		if (grid[headY][x] > 0) {
			path1_possible = false;
			break;
		}
	}
	for (int y = headY + (dy > 0 ? 1 : -1); y != targetY; y += (dy > 0 ? 1 : -1)) {
		if (grid[y][targetX] > 0) {
			path1_possible = false;
			break;
		}
	}


	bool path2_possible = true;
	for (int y = headY + (dy > 0 ? 1 : -1); y != targetY; y += (dy > 0 ? 1 : -1)) {
		if (grid[y][headX] > 0) {
			path2_possible = false;
			break;
		}
	}
	for (int x = headX + (dx > 0 ? 1 : -1); x != targetX; x += (dx > 0 ? 1 : -1)) {
		if (grid[targetY][x] > 0) {
			path2_possible = false;
			break;
		}
	}



	if (dy == 0) changeDirection((dx > 0 ? 1 : -1), 0);
	else if (dx == 0) changeDirection(0, (dy > 0 ? 1 : -1));
	else if (path1_possible) {
		if (dx != 0) changeDirection((dx > 0 ? 1 : -1), 0);
		else if (dy != 0) changeDirection(0, (dy > 0 ? 1 : -1));
	}
	else if (path2_possible) {
		if (dy != 0) changeDirection(0, (dy > 0 ? 1 : -1));
		else if (dx != 0) changeDirection((dx > 0 ? 1 : -1), 0);
	}
	else {
		printf("AutoPilot failed: No valid path.\n");
		gameOver = 1;
	}
}