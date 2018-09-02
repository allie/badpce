#include <SDL2/SDL.h>
#include <math.h>
#include "cpu/cpu.h"
#include "psg/psg.h"
#include "core/graphics.h"
// #include "core/audio.h"
#include "core/config.h"
// #include "core/input.h"
#include "core/debugger.h"
#include "memory/memory.h"
#include "memory/cart.h"
#include "system/pce.h"

extern CPU cpu;
// extern PSG psg;

int main(int argc, char* argv[]) {
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("SDL_Init error: %s\n", SDL_GetError());
		return 0;
	}

	if (!Config_Load("config.json")) {
		Config_LoadDefaults();
	}

	if (!Graphics_Init()) {
		return 0;
	}

	// if (!Audio_Init()) {
	// 	return 0;
	// }

	if (!Debugger_Init()) {
		return 0;
	};

	if (argc > 1) {
		Cart_Load(argv[1]);
		PCE_Reset();
	} else {
		printf("NOTE: No ROM file supplied.\n");
	}

	SDL_Thread* system_thread = SDL_CreateThread(PCE_Emulate, "system", (void*)NULL);
	if (system_thread == NULL) {
		printf("SDL_Thread error: %s\n", SDL_GetError());
		return 0;
	}

	int running = 1;
	while (running) {
		SDL_Event e;
		while (SDL_PollEvent(&e) != 0) {
			if (e.type == SDL_QUIT) {
				running = 0;
			}
		}

		Graphics_Clear();
		Debugger_Draw();
		Graphics_Present();
	}

	Graphics_Destroy();
	Audio_Destroy();
	Debugger_Destroy();
	Config_Destroy();

	SDL_Quit();

	return 0;
}
