#define _USE_MATH_DEFINES
#include<math.h>
#include<stdio.h>
#include<string.h>
#include <time.h>

#include"Snake.h"

extern "C" {
#include"./SDL2-2.0.10/include/SDL.h"
#include"./SDL2-2.0.10/include/SDL_main.h"
}

#define SCREEN_WIDTH	840
#define SCREEN_HEIGHT	480
#define LEADERBOARDS_FILE_NAME "records.txt"


// narysowanie napisu txt na powierzchni screen, zaczynaj¹c od punktu (x, y)
// charset to bitmapa 128x128 zawieraj¹ca znaki
// draw a text txt on surface screen, starting from the point (x, y)
// charset is a 128x128 bitmap containing character images
void DrawString(SDL_Surface* screen, int x, int y, const char* text,
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


// narysowanie na ekranie screen powierzchni sprite w punkcie (x, y)
// (x, y) to punkt œrodka obrazka sprite na ekranie
// draw a surface sprite on a surface screen in point (x, y)
// (x, y) is the center of sprite on screen
void DrawSurface(SDL_Surface *screen, SDL_Surface *sprite, int x, int y) {
	SDL_Rect dest;
	dest.x = x - sprite->w / 2;
	dest.y = y - sprite->h / 2;
	dest.w = sprite->w;
	dest.h = sprite->h;
	SDL_BlitSurface(sprite, NULL, screen, &dest);
	};


// rysowanie pojedynczego pixela
// draw a single pixel
void DrawPixel(SDL_Surface *surface, int x, int y, Uint32 color) {
	int bpp = surface->format->BytesPerPixel;
	Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
	*(Uint32 *)p = color;
	};


// rysowanie linii o d³ugoœci l w pionie (gdy dx = 0, dy = 1) 
// b¹dŸ poziomie (gdy dx = 1, dy = 0)
// draw a vertical (when dx = 0, dy = 1) or horizontal (when dx = 1, dy = 0) line
void DrawLine(SDL_Surface *screen, int x, int y, int l, int dx, int dy, Uint32 color) {
	for(int i = 0; i < l; i++) {
		DrawPixel(screen, x, y, color);
		x += dx;
		y += dy;
		};
	};


// rysowanie prostok¹ta o d³ugoœci boków l i k
// draw a rectangle of size l by k
void DrawRectangle(SDL_Surface *screen, int x, int y, int l, int k,
                   Uint32 outlineColor, Uint32 fillColor) {
	int i;
	DrawLine(screen, x, y, k, 0, 1, outlineColor);
	DrawLine(screen, x + l - 1, y, k, 0, 1, outlineColor);
	DrawLine(screen, x, y, l, 1, 0, outlineColor);
	DrawLine(screen, x, y + k - 1, l, 1, 0, outlineColor);
	for(i = y + 1; i < y + k - 1; i++)
		DrawLine(screen, x + 1, i, l - 2, 1, 0, fillColor);
	};

void DrawStats() {
}

// main
#ifdef __cplusplus
extern "C"
#endif
int main(int argc, char **argv) {
	Snake snake;
	int t1, t2, quit, frames, rc, points;
	double delta, worldTime, fpsTimer, fps;
	SDL_Event event;
	SDL_Surface *screen, *charset;
	SDL_Surface *eti;
	SDL_Texture *scrtex;
	SDL_Window *window;
	SDL_Renderer *renderer;

	// okno konsoli nie jest widoczne, je¿eli chcemy zobaczyæ
	// komunikaty wypisywane printf-em trzeba w opcjach:
	// project -> szablon2 properties -> Linker -> System -> Subsystem
	// zmieniæ na "Console"
	// console window is not visible, to see the printf output
	// the option:
	// project -> szablon2 properties -> Linker -> System -> Subsystem
	// must be changed to "Console"
	
	//error handling
	if(SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("SDL_Init error: %s\n", SDL_GetError());
		return 1;
		}

	// tryb pe³noekranowy / fullscreen mode
//	rc = SDL_CreateWindowAndRenderer(0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP,
//	                                 &window, &renderer);
	rc = SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0,
	                                 &window, &renderer);
	//error handling
	if(rc != 0) {
		SDL_Quit();
		printf("SDL_CreateWindowAndRenderer error: %s\n", SDL_GetError());
		return 1;
		};
	
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

	SDL_SetWindowTitle(window, "snake project");


	screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32,
	                              0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

	scrtex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
	                           SDL_TEXTUREACCESS_STREAMING,
	                           SCREEN_WIDTH, SCREEN_HEIGHT);


	// wy³¹czenie widocznoœci kursora myszy
	SDL_ShowCursor(SDL_DISABLE);

	// wczytanie obrazka cs8x8.bmp
	charset = SDL_LoadBMP("./cs8x8.bmp");
	if(charset == NULL) {
		printf("SDL_LoadBMP(cs8x8.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
		};
	SDL_SetColorKey(charset, true, 0x000000);

	eti = SDL_LoadBMP("./eti.bmp");
	if(eti == NULL) {
		printf("SDL_LoadBMP(eti.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(charset);
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
		};

	char text[128];
	int czarny = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
	int zielony = SDL_MapRGB(screen->format, 0x00, 0xFF, 0x00);
	int czerwony = SDL_MapRGB(screen->format, 0xFF, 0x00, 0x00);
	int niebieski = SDL_MapRGB(screen->format, 0x11, 0x11, 0xCC);

	t1 = SDL_GetTicks();

	frames = 0;
	fpsTimer = 0;
	fps = 0;
	quit = 0;
	worldTime = 0;
	
	int updated = 0;
	double percentage = 100;
	int pixels = 170;
	bool alreadySlowed = 0;
	

	FILE* saveFile = fopen(LEADERBOARDS_FILE_NAME, "r");
	if (saveFile == nullptr) {
		printf("Nie uda³o siê otworzyæ pliku do zapisu!\n");
		
	}
	int p1, p2, p3;
	char nazwa1[16]="", nazwa2[16]="", nazwa3[16]="";
	if (fscanf(saveFile, "%s %d\n", nazwa1, &p1) < 2) {
		strcpy(nazwa1, "");
		p1 = 0;
	}
	if (fscanf(saveFile, "%s %d\n", nazwa2, &p2) < 2) {
		strcpy(nazwa2, "");
		p2 = 0;
	}
	if (fscanf(saveFile, "%s %d\n", nazwa3, &p3) < 2) {
		strcpy(nazwa3, "");
		p3 = 0;
	}

	fclose(saveFile);
	
	
	bool editMode = 0;
	char nick[16]="";
	bool nickSubmited=0;

	while (!quit) {
		
		t2 = SDL_GetTicks();

		// w tym momencie t2-t1 to czas w milisekundach,
		// jaki uplyna³ od ostatniego narysowania ekranu
		// delta to ten sam czas w sekundach

		delta = (t2 - t1) * 0.001;
		t1 = t2;

		worldTime += delta;

		

		SDL_FillRect(screen, NULL, czarny);


		fpsTimer += delta;
		if (fpsTimer > 0.5) {
			fps = frames * 2;
			frames = 0;
			fpsTimer -= 0.5;
		};
		
		// stats
		DrawRectangle(screen, 4, 4, SCREEN_WIDTH - 208, 36, czerwony, niebieski);
		sprintf(text, "Snake, czas trwania = %.1lf s  %.0lf klatek / s  punkty = %d predk=%.2f", worldTime, fps, snake.points,snake.speed);
		DrawString(screen, (screen->w-200) / 2 - strlen(text) * 8 / 2, 10, text, charset);
		sprintf(text, "Esc - wyjscie, n - nowa gra , autopilot=%d",snake.autoplayer);
		DrawString(screen, (screen->w-200) / 2 - strlen(text) * 8 / 2, 26, text, charset);

		//requirments
		DrawRectangle(screen, SCREEN_WIDTH - 200, 4, 200, SCREEN_HEIGHT - 70, czerwony, czarny);
		sprintf(text, "Spelnione wymagania:");
		DrawString(screen, (screen->w - 196), 10, text, charset);
		sprintf(text, "Mandatory:");
		DrawString(screen, (screen->w - 196), 20, text, charset);
		sprintf(text, "1,2,3,4");
		DrawString(screen, (screen->w - 196), 30, text, charset);
		sprintf(text, "Optional:");
		DrawString(screen, (screen->w - 196), 40, text, charset);
		sprintf(text, "A(1pt) Lengthening");
		DrawString(screen, (screen->w - 196), 50, text, charset);
		sprintf(text, "B(1pt) Speedup");
		DrawString(screen, (screen->w - 196), 60, text, charset);
		sprintf(text, "C(2pts) Red dot");
		DrawString(screen, (screen->w - 196), 70, text, charset);
		sprintf(text, "D(1pt) Points");
		DrawString(screen, (screen->w - 196), 80, text, charset);
		sprintf(text, "E(2pts) Saving the game");
		DrawString(screen, (screen->w - 196), 90, text, charset);
		sprintf(text, "F(1pt) Best scores");
		DrawString(screen, (screen->w - 196), 100, text, charset);
		sprintf(text, "G(1pt) Fancy Graphic ");
		DrawString(screen, (screen->w - 196), 110, text, charset);
		sprintf(text, "H(1pt) Teleportation ");
		DrawString(screen, (screen->w - 196), 120, text, charset);
		sprintf(text, "I(1pt) Configurator ");
		DrawString(screen, (screen->w - 196), 130, text, charset);
		sprintf(text, "Extra:");
		DrawString(screen, (screen->w - 196), 140, text, charset);
		sprintf(text, "(2pts) Auto player");
		DrawString(screen, (screen->w - 196), 150, text, charset);


		DrawRectangle(screen, SCREEN_WIDTH - 200, SCREEN_HEIGHT - 70, 200, 70, czerwony, czarny);
		if (snake.redDot) {
			
			sprintf(text, "RedDot:");
			DrawString(screen, (screen->w - 196), screen->h - 64, text, charset);

			percentage = ((snake.timeToRedDot - worldTime)/HOW_LONG_RED_DOT_LASTS);
			
			pixels = (int)(170 * percentage) ;
			
			/*printf("%d", pixels);*/
			DrawRectangle(screen, SCREEN_WIDTH - 185, SCREEN_HEIGHT - 50, 170, 30, czerwony, czarny);
			DrawRectangle(screen, SCREEN_WIDTH - 185, SCREEN_HEIGHT - 50, pixels, 30, czarny, czerwony);
		}

		
		if (worldTime > TIME_TO_SPEED && alreadySlowed==0) {
			snake.setSpeed(snake.speed_multiplier);
			alreadySlowed = 1;
		}
		
		if (snake.gameOver == 0) {
			snake.draw(screen, CELL_SIZE, zielony,charset);
			if (frames % int(20*snake.speed) == 0) {
				snake.update(worldTime);
				updated = 0;
			}
		}
		else
		{
			DrawRectangle(screen, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, czarny, czarny);
			DrawRectangle(screen, 4, 4, SCREEN_WIDTH - 8, 36, czarny, czarny);
			DrawRectangle(screen, 4, 30, SCREEN_WIDTH - 8, 36, czerwony, czarny);
			if (snake.autoplayer == 0) {
				sprintf(text, "Koniec Gry");
			}
			else {
				sprintf(text, "Koniec Gry, blad autopilota");
			}
			sprintf(text, "Koniec Gry");
			DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 40, text, charset);
			sprintf(text, "Esc - wyjscie, n - nowa gra , punkty=%d", snake.points);
			DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 50, text, charset);
			if ((snake.points>p1 || snake.points>p2 || snake.points>p3) && nickSubmited==0 )
			{
				editMode = true;
			}
			if (editMode) {

				sprintf(text, "zakwalifikowano do tabeli rekordow !");
				DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 80, text, charset);
				sprintf(text, "podaj nick:");
				DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 90, text, charset);

				DrawString(screen, screen->w / 2 - strlen(nick) * 8 / 2, 100, nick, charset);

				SDL_StartTextInput();
				while (SDL_PollEvent(&event)) {
					switch (event.type)
					{
					case SDL_QUIT:
						quit = 1;
						editMode = false;
						break;
					case SDL_KEYDOWN:
						if (event.key.keysym.sym == SDLK_RETURN) { 
							nickSubmited = 1;
							editMode = false;
							if (snake.points > p1) {
								strcpy(nazwa3, nazwa2);
								p3 = p2;
								strcpy(nazwa2, nazwa1);
								p2 = p1;
								strcpy(nazwa1, nick);
								p1 = snake.points;
							}
							else if (snake.points > p2) {
								strcpy(nazwa3, nazwa2);
								p3 = p2;
								strcpy(nazwa2, nick);
								p2 = snake.points;
							}
							else if (snake.points > p3) {
								strcpy(nazwa3, nick);
								p3 = snake.points;
							}

							saveFile = fopen(LEADERBOARDS_FILE_NAME, "w");
							if (saveFile) {
							
								fprintf(saveFile, "%s %d\n", nazwa1, p1);
								fprintf(saveFile, "%s %d\n", nazwa2, p2);
								fprintf(saveFile, "%s %d\n", nazwa3, p3);
								fclose(saveFile);
							}
							else {
								printf("Nie uda³o siê otworzyæ pliku do zapisu!\n");
							}
						}
						else if (event.key.keysym.sym == SDLK_BACKSPACE && strlen(nick) > 0) {
							
							nick[strlen(nick) - 1] = '\0';
						}

						break;
					case SDL_TEXTINPUT:
						if (strlen(nick) < 15) { // Maksymalna d³ugoœæ nicku to 15 znaków
							strcat(nick, event.text.text);
						}
						break;
					
					}
				}
				SDL_StopTextInput();
			}
				else {
				DrawRectangle(screen, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, czarny, czarny);
				DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 50, text, charset);
				sprintf(text, "leaderboard");
				DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 70, text, charset);
				sprintf(text, "1.%s %d", nazwa1, p1);
				DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 80, text, charset);
				sprintf(text, "2.%s %d", nazwa2, p2);
				DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 90, text, charset);
				sprintf(text, "3.%s %d", nazwa3, p3);
				DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 100, text, charset);
			}
			
		}

		
		
		
		SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
//		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, scrtex, NULL, NULL);
		SDL_RenderPresent(renderer);

		
		
		
		if (!editMode) {
			while (SDL_PollEvent(&event)) {
				switch (event.type) {
				case SDL_KEYDOWN:
					if (snake.autoplayer == 0) {
						if (event.key.keysym.sym == SDLK_UP && updated == 0) snake.changeDirection(0, -1);
						else if (event.key.keysym.sym == SDLK_DOWN && updated == 0) snake.changeDirection(0, 1);
						else if (event.key.keysym.sym == SDLK_LEFT && updated == 0) snake.changeDirection(-1, 0);
						else if (event.key.keysym.sym == SDLK_RIGHT && updated == 0) snake.changeDirection(1, 0);
					}
					if (event.key.keysym.sym == SDLK_p) snake.autoplayer=!snake.autoplayer;
					else if (event.key.keysym.sym == SDLK_s) snake.saveGame(worldTime);
					else if (event.key.keysym.sym == SDLK_l) snake.loadGame(worldTime);
					else if (event.key.keysym.sym == SDLK_ESCAPE) quit = 1;
					else if (event.key.keysym.sym == SDLK_n) {
						nickSubmited=0;
						snake.reset();
						worldTime = 0;
					}
					
					updated++;
					break;

				case SDL_QUIT:
					quit = 1;
					break;
				};
			};
			frames++;
			SDL_Delay(5);
		}
		
		};

	
	SDL_FreeSurface(charset);
	SDL_FreeSurface(screen);
	SDL_DestroyTexture(scrtex);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	SDL_Quit();
	return 0;
	};
