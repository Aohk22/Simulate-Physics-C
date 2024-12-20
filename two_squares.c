#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "SDL3/SDL.h"

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 360;

const int N_SQUARES = 2;

typedef struct {
	float x, y;
} Bector2D;

typedef struct {
	Bector2D position;
	Bector2D velocity;
	Bector2D acceleration;
	float mass;
} Object;

typedef struct {
	Object *squares;
	size_t n_squares;

} GameState;

typedef struct {
	SDL_Renderer *renderer;
	SDL_Window *window;

} ScreenState;

Bector2D vector_scalar_multiple(Bector2D vector, float n) {
	return (Bector2D) { vector.x * n, vector.y * n };
}

Bector2D vector_dot_product(Bector2D vector1, Bector2D vector2) {
	return (Bector2D) { vector2.x*vector1.x + vector2.y*vector1.x, vector2.y*vector1.x + vector2.y*vector1.y };
}

Bector2D vector_add(Bector2D vector1, Bector2D vector2) {
	return (Bector2D) { vector1.x + vector2.x, vector1.y + vector2.y };
}

void initialize_all(GameState *gameState, ScreenState *screenState);

void handler_render(GameState *gameState, ScreenState *screenState);
void handler_collision(GameState *gameState);

void collision_detect_response_window(GameState *gameState);
void collision_detect_response_object(GameState *gameState);
void render_objects(SDL_Renderer *renderer, GameState *gameState);
void render_background(SDL_Renderer *renderer, SDL_Window *window);
void collect_garbage(ScreenState *screenState);

int process_events(ScreenState *screenState);

int main(int argc, char* argv[]) {
	GameState gameState;
	ScreenState screenState;
	int done = 0;
	srand((int)time(NULL));

	initialize_all(&gameState, &screenState);
	while(!done) {
		done = process_events(&screenState);
		handler_collision(&gameState);
		handler_render(&gameState, &screenState);

		SDL_Delay(40);
	}

	collect_garbage(&screenState);
	SDL_Quit();
	return 0;
}

void initialize_all(GameState *gameState, ScreenState *screenState) {
	/* Initialize the screen. */
	SDL_Init(SDL_INIT_VIDEO);
	screenState->window = SDL_CreateWindow("Physics Simulator", SCREEN_WIDTH, SCREEN_HEIGHT, 0x22);
	screenState->renderer = SDL_CreateRenderer(screenState->window , NULL);
	if(!SDL_SetRenderVSync(screenState->renderer, 1)) printf("Set vsync unsuccessful.");

	/* Initialize objects, dynamic memory allocation. */
	gameState->n_squares = N_SQUARES;
	gameState->squares = (Object *)malloc(gameState->n_squares * sizeof(Object));
	int i;
	for(i = 0; i < gameState->n_squares; i++) {
		gameState->squares[i].position.x = (rand() % (SCREEN_WIDTH-60 - 60 + 1)) + 60;
		gameState->squares[i].position.y = (rand() % (SCREEN_HEIGHT-60 - 60 + 1)) + 60;
		float velocity_x = (rand() % (100 - 69 + 1)) + 69.; //+ (i % 20);
		float velocity_y = (rand() % (100 - 69 + 1)) + 69.; //+ (i % 20);
		gameState->squares[i].velocity.x = velocity_x;
		gameState->squares[i].velocity.y = velocity_y;
		gameState->squares[i].mass = (rand() % (100 - 50 + 1)) + 50;
		printf("Calculated velo: %f, %f\n", velocity_x, velocity_y);
	}

	for(i = 0; i < gameState->n_squares; i++) {
		printf("Square %d: mass = %f\n", i, gameState->squares[i].mass);
	}
}

void handler_render(GameState *gameState, ScreenState *screenState) {
	render_background(screenState->renderer, screenState->window);
	render_objects(screenState->renderer, gameState); // Ugh..., ugly.

	SDL_RenderPresent(screenState->renderer);
}

void render_background(SDL_Renderer *renderer, SDL_Window *window) {
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
	SDL_RenderClear(renderer);
}

void render_objects(SDL_Renderer *renderer, GameState *gameState) {
	SDL_SetRenderDrawColor(renderer, 255, 255, 255 ,255);
	int i;
	for(i = 0; i < gameState->n_squares; i++) {
		SDL_FRect square_rect = { gameState->squares[i].position.x, gameState->squares[i].position.y, 50, 50 };
		SDL_RenderFillRect(renderer, &square_rect);
	}
}

void handler_collision(GameState *gameState) {
	collision_detect_response_window(gameState);
	collision_detect_response_object(gameState);

	Bector2D p1 = gameState->squares[0].position;
	Bector2D p2 = gameState->squares[1].position;
	Bector2D v1 = gameState->squares[0].velocity;
	Bector2D v2 = gameState->squares[1].velocity;
	gameState->squares[0].position = (Bector2D) vector_add(p1, vector_scalar_multiple(v1, 1.f/60.f));
	gameState->squares[1].position = (Bector2D) vector_add(p2, vector_scalar_multiple(v2, 1.f/60.f));
}

void collision_detect_response_window(GameState *gameState) {
	int i;
	for(i = 0; i < gameState->n_squares; i++) {
		if(gameState->squares[i].position.x + 50 >= SCREEN_WIDTH || gameState->squares[i].position.x <= 0) {
			gameState->squares[i].velocity.x = -gameState->squares[i].velocity.x;
			//gameState->squares[i].position.x += gameState->squares[i].velocity.x;
		}
		if(gameState->squares[i].position.y + 50 >= SCREEN_HEIGHT || gameState->squares[i].position.y <= 0) {
			gameState->squares[i].velocity.y = -gameState->squares[i].velocity.y;
			//gameState->squares[i].position.y += gameState->squares[i].velocity.y;
		}
	}
}

void collision_detect_response_object(GameState *gameState) {
	int i;
	for(i = 0; i < gameState->n_squares; i = i + 2) {
		if(gameState->squares[i].position.x < gameState->squares[i+1].position.x + 50 &&
			gameState->squares[i].position.x + 50 > gameState->squares[i+1].position.x &&
			gameState->squares[i].position.y < gameState->squares[i+1].position.y + 50 &&
			gameState->squares[i].position.y + 50 > gameState->squares[i+1].position.y
		) {
			float m1 = gameState->squares[i].mass;
			float m2 = gameState->squares[i+1].mass;
			Bector2D v1 = gameState->squares[i].velocity;
			Bector2D v2 = gameState->squares[i+1].velocity;
			//Bector2D p1 = gameState->squares[i].position;
			//Bector2D p2 = gameState->squares[i+1].position;
			// Ewwwwwwwww...
			gameState->squares[i].velocity = (Bector2D) vector_scalar_multiple(vector_add(vector_scalar_multiple(v1, m1-m2), vector_scalar_multiple(v2, 2*m2)), 1./(m1+m2));
			gameState->squares[i+1].velocity = (Bector2D) vector_scalar_multiple(vector_add(vector_scalar_multiple(v2, m2-m1), vector_scalar_multiple(v1, 2*m1)), 1./(m1+m2));
			//gameState->squares[i].position = (Bector2D) vector_add(p1, vector_scalar_multiple(v1, 1.f/60.f));
			//gameState->squares[i+1].position = (Bector2D) vector_add(p2, vector_scalar_multiple(v2, 1.f/60.f));
		}
	}
}

int process_events(ScreenState *screenState) {
	SDL_Event event;
	int done = 0;

	/* Why is this shit so ugly. */
	while(SDL_PollEvent(&event)) {
		switch(event.type) {
			case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
				if(screenState->window) {
					SDL_DestroyWindow(screenState->window);
					screenState->window = NULL;
					done = 1;
				}
				break;
			case SDL_EVENT_QUIT:
				done = 1;
				break;
				/* Non-movement keyboard input */
				case SDL_EVENT_KEY_DOWN:
					switch(event.key.key) { // SDL_Event.key(SDL_KeyboardEvent).key(SDL_Keycode)(virtual key code)
						case SDLK_ESCAPE:
							done = 1;
							break;
					}
					break;
		}
	}
	return done;
}

void collect_garbage(ScreenState *screenState) {
	SDL_DestroyWindow(screenState->window);
	SDL_DestroyRenderer(screenState->renderer);
}
