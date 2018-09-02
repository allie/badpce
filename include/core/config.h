#ifndef CONFIG_H
#define CONFIG_H

#include "../common.h"
#include <SDL2/SDL.h>

typedef struct {
	Vector2 window_pos;
	int window_scale;

	float volume;

	char recent_files[10][256];
} Config;

int Config_Load(const char*);
void Config_LoadDefaults();
void Config_Write(const char*);
void Config_Destroy();

#endif
