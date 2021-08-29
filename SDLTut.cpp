#include <SDL.h>
#include <stdio.h>
#include <iostream>
#include "Header.h"
#include <SDL_video.h>
#include <SDL_render.h>
#include <SDL_events.h>


enum KeyPressSurfaces
{
	KEY_PRESS_SURFACE_DEFAULT,
	KEY_PRESS_SURFACE_UP,
	KEY_PRESS_SURFACE_DOWN,
	KEY_PRESS_SURFACE_LEFT,
	KEY_PRESS_SURFACE_RIGHT,
	KEY_PRESS_SURFACE_TOTAL
};

SDL_Surface* gKeyPressSurfaces[KEY_PRESS_SURFACE_TOTAL];
//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 320;

int main(int argc, char* args[])
{
	memset(V, 0, sizeof(V));
	memset(stack, 0, sizeof(stack));
	memset(memory, 0, sizeof(memory));
	memset(gfx, 0, sizeof(gfx));
	PC = 0x200;
	sp = 0;
	if (argc <= 1) {
		std::cerr << "Path to ROM required";
		exit(1);
	}
	if (!load_rom(args[1])) {
		std::cerr << "Error: Invalid ROM path";
		std::cout << "\t\t\tROM path loaded:" << args[1] << "\n";
		exit(1);
	}

	std::cout << "ROM LOADED: " << args[1] << "\n";

	//The window we'll be rendering to
	SDL_Window* window = NULL;
	SDL_Renderer* renderer = NULL;
	//The surface contained by the window
	SDL_Surface* screenSurface = NULL;
	SDL_Texture* texture = NULL;
	
	//Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
	}
	else
	{
		//Create window
		window = SDL_CreateWindow("CHIP-8 EMULATOR", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		if (window == NULL)
		{
			printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
			SDL_Quit();
			exit(1);
		}

		renderer = SDL_CreateRenderer(window, -1, 0);
		if (renderer == nullptr)
		{
			std::cerr << "Error in initializing rendering " << SDL_GetError() << std::endl;
			SDL_Quit();
			exit(1);
		}

		SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);

		texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 64, 32);
		if (texture == nullptr)
		{
			std::cerr << "Error in setting up texture " << SDL_GetError() << std::endl;
			SDL_Quit();
			exit(1);
		}


	}

	bool loop = true;
	SDL_Event sdl_event;
	Uint32* pixels = new Uint32[640 * 480];
	memset(pixels, 255, 640 * 480 * sizeof(Uint32));

	while (loop) {
		
		int opcode = (memory[PC] << 8) | (memory[PC + 1]); //sets opcode to current instruction

		

		//std::cout << "Current Opcode: " << std::hex << opcode << "\n";
		/*

		std::cout << PC << "\n";
		std::cout << (PC-2) << "\n";
		std::cout << "----------" << "\n";
		std::dec;
		*/
		executeInstr(opcode);
		PC += 2; //sets program counter to next instruction
		while (SDL_PollEvent(&sdl_event) != 0)
		{
			if (sdl_event.type == SDL_QUIT)
			{
				loop = false;
			}



		}

		if (draw) {
			//std::cout << "drawing";
			SDL_UpdateTexture(texture, NULL, pixels, 64 * sizeof(Uint32));
			draw = false;

			
			for (int i = 0; i < 2048; i++) {//2048 == 32* 64
				if (!gfx[i]) { //if the pixel is 0
					pixels[i] = 0;
				}
				else {
					std::cout << "setting pixel: " << i << "\n";
					pixels[i] = 255;
				}
			}
			

			SDL_RenderCopy(renderer, texture, NULL, NULL);
			SDL_RenderClear(renderer);
			SDL_RenderPresent(renderer);


			if (delay_timer > 0)
				delay_timer--;
			if (sound_timer > 0)
				sound_timer--;
		}
		//SDL_BlitSurface(screenSurface, NULL, screenSurface, NULL);

		//Update the surface
		SDL_UpdateWindowSurface(window);
		std::chrono::milliseconds timespan(10);
		std::this_thread::sleep_for(timespan);
	}


	//Destroy window
	delete[] pixels;
	SDL_DestroyWindow(window);

	//Quit SDL subsystems
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}