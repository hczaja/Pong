#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <utility>

#define MAX_SPEED	64
#define ACCELERATION	8

static constexpr auto SCREEN_WIDTH = 960;
static constexpr auto SCREEN_HEIGHT = 640;
static constexpr auto OBJ_SIZE = 16;

static bool quit_flag = false;

const int FPS = 30;
const int FRAMEDELAY = 1000/FPS;

class Pong {
	SDL_Surface* surface;
	SDL_Window* window;
	SDL_Event event;
	SDL_Thread* thread;

	SDL_Color text_color = { 0xFA, 0xFA, 0xFA };
	//TTF_Font* font = TTF_OpenFont("Marlboro.ttf",32);
	//SDL_Surface* text = TTF_RenderText_Solid(font,"HELLO",text_color);

	struct PLAYER_HANDLER{
		typedef enum {
			BLUE, RED
		} COLOR;

		COLOR color;
		SDL_Rect rect;
		int speed;
		int score;

		void init(COLOR p) {
			color = p;
			switch(color) {
				case COLOR::BLUE: {
					rect.x = 0;
					break;
				}
				default: {
					rect.x = SCREEN_WIDTH - OBJ_SIZE;
					break;
				}
			}
			rect.y = SCREEN_HEIGHT/2;
			rect.w = OBJ_SIZE;
			rect.h = OBJ_SIZE*5;
			speed = 0;
			score = 0;
		}

		void draw(SDL_Surface* s) {
			switch(color) {
				case COLOR::BLUE: {
					SDL_FillRect(s, &rect, SDL_MapRGB(s->format, 0,0,0xFF));
					break;
				}
				case COLOR::RED: {
					SDL_FillRect(s, &rect, SDL_MapRGB(s->format, 0xFF,0,0));
					break;
				}
				default:
					break;
			}
		}

		void update(SDL_Event event) {
				if(event.type == SDL_QUIT)
					quit_flag = true;
				else if(event.type == SDL_KEYDOWN){
					if(event.key.keysym.sym == SDLK_t)
						quit_flag = true;
					switch(color) {
						case COLOR::BLUE: {
							if(event.key.keysym.sym == SDLK_s) {
								speed += ACCELERATION;
								if(speed >= MAX_SPEED)
									speed = MAX_SPEED;
							} else if(event.key.keysym.sym == SDLK_w) {
								speed -= ACCELERATION;
								if(speed <= -MAX_SPEED)
									speed = -MAX_SPEED;
							} 
							break;
						}
						case COLOR::RED: {
							if(event.key.keysym.sym == SDLK_k) {
								speed += ACCELERATION;
								if(speed >= MAX_SPEED)
									speed = MAX_SPEED;
							} else if(event.key.keysym.sym == SDLK_i) {
								speed -= ACCELERATION;
								if(speed <= -MAX_SPEED)
									speed = -MAX_SPEED;
							}
							break;
						}
					}
				}
			rect.y += speed;

			if(rect.y <= 0) rect.y = 0;
			if(rect.y >= SCREEN_HEIGHT-rect.h) rect.y = SCREEN_HEIGHT-rect.h;

			if(speed!=0) {
				if(speed>0) {
					speed -= ACCELERATION;
					if(speed <= 0)
						speed = 0;
				} else {
					speed += ACCELERATION;
					if(speed >= 0)
						speed = 0;
				}
			}
		}

	};

	typedef struct {
		SDL_Rect rect;
		std::pair<int, int> speeds;

		void init() {
			rect.x = SCREEN_WIDTH/2 - OBJ_SIZE/2;
			rect.y = SCREEN_HEIGHT/2 + OBJ_SIZE/2;
			rect.w = rect.h = OBJ_SIZE;

			speeds.first = speeds.second = 8;
		}

		void draw(SDL_Surface *s) {
			SDL_FillRect(s, &rect, SDL_MapRGB(s->format,0xFF,0xFF,0xFF));
		}

		void update() {
			rect.x += speeds.first;
			rect.y += speeds.second;

			if(rect.y<=0 || rect.y>=SCREEN_HEIGHT-OBJ_SIZE)
				speeds.second = -speeds.second;

			if(rect.x<=0) rect.x = 0;
			if(rect.x>=SCREEN_WIDTH-OBJ_SIZE) rect.x = SCREEN_WIDTH-OBJ_SIZE;
		}

		void reset_ball() {
			SDL_Delay(1500);
			rect.x = SCREEN_WIDTH/2 - OBJ_SIZE/2;
			rect.y = SCREEN_HEIGHT/2 + OBJ_SIZE/2;
			
			speeds.first = -speeds.first;
		}

	} BALL;

	std::pair<PLAYER_HANDLER, PLAYER_HANDLER> players;
	BALL ball;

	void init() {
		players.first.init(PLAYER_HANDLER::COLOR::BLUE);
		players.second.init(PLAYER_HANDLER::COLOR::RED);
		ball.init();
		//SDL_BlitSurface(text, NULL, surface, NULL);
		//SDL_FreeSurface(text);

		SDL_Delay(1000);
	}


	void thread_fun(void* ptr) {
		Pong::update();
	}

	void draw() {
		SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 0, 0, 0));

		players.first.draw(surface);
		players.second.draw(surface);
		ball.draw(surface);
		SDL_UpdateWindowSurface(window);
	}

	void check_collisions(BALL *b, PLAYER_HANDLER p) {
		switch(p.color) {
			case PLAYER_HANDLER::COLOR::BLUE: {
				if(b->rect.x <= p.rect.x+OBJ_SIZE/2)
					b->reset_ball();
				if((b->rect.x == (p.rect.x+OBJ_SIZE)) && (b->rect.y >= (p.rect.y-OBJ_SIZE/2) && b->rect.y <= (p.rect.y + p.rect.h+OBJ_SIZE/2)))
					b->speeds.first = -b->speeds.first;
				break;
			}
			case PLAYER_HANDLER::COLOR::RED: {
				if(b->rect.x+OBJ_SIZE/2 >= p.rect.x)
					b->reset_ball();
				if((b->rect.x+OBJ_SIZE == p.rect.x) && (b->rect.y >= (p.rect.y-OBJ_SIZE/2) && b->rect.y <= (p.rect.y + p.rect.h + OBJ_SIZE/2)))
					b->speeds.first = -b->speeds.first;
			}
		}
	}

	void update_score() {

	}

public:
	Pong(SDL_Surface* s, SDL_Event e, SDL_Window* w) {
		surface = s;
		event = e;
		window = w;
	}

	void update() {
		while(SDL_PollEvent(&event)) {
			players.first.update(event);
			players.second.update(event);
		}
		ball.update();
	}

	void run() {

		init();

		Uint32 frameStart;
		int frameTime;

		while(!quit_flag) {
			frameStart = SDL_GetTicks();

			thread = SDL_CreateThread(thread_fun, "thread_fun", NULL);
			check_collisions(&ball, players.first);
			check_collisions(&ball, players.second);
			draw();
			//update_score();

			frameTime = SDL_GetTicks() - frameStart;
			if(FRAMEDELAY>frameTime) {
				SDL_Delay(FRAMEDELAY-frameTime);
			}
			SDL_WaitThread(thread, NULL);
	
		}
		SDL_Delay(2000);
	}
	~Pong() {
		//TTF_CloseFont(font);

		SDL_FreeSurface(surface);
	}
	
};

int main() {

	/* SDL main handles */
	SDL_Window* window = nullptr;
	SDL_Surface* surface = nullptr;
	SDL_Event event;

	if(SDL_Init(SDL_INIT_EVERYTHING) < 0)
		printf("Error:%s",SDL_GetError());
	else {
		window = SDL_CreateWindow("Pong",
					SDL_WINDOWPOS_UNDEFINED,
					SDL_WINDOWPOS_UNDEFINED,
					SCREEN_WIDTH,
					SCREEN_HEIGHT,
					SDL_WINDOW_SHOWN);

		if(!window)
			printf("Error:%s",SDL_GetError());
		else {
			surface = SDL_GetWindowSurface(window);
			SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 0xFF, 0xFF, 0xFF));
			SDL_UpdateWindowSurface(window);
			TTF_Init();
		}
	}

	SDL_Delay(1000);

	Pong* pong = new Pong(surface, event, window);
	pong->run();

	SDL_FreeSurface(surface);
	SDL_DestroyWindow(window);
	TTF_Quit();
	SDL_Quit();

	return 0;
}
