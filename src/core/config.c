#include "core/config.h"
#include "core/debugger.h"
#include "core/graphics.h"
#include "io/io.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <jsmn.h>

Config config;

// From the jsmn example files
static int jsoncmp(const char* json, jsmntok_t* token, const char* str) {
	if (token->type == JSMN_STRING && (int)strlen(str) == token->end - token->start &&
		strncmp(json + token->start, str, token->end - token->start) == 0) {
		return 0;
	}
	return -1;
}

static char* extract_string(char* str, size_t size) {
	char* buf = (char*)malloc(size + 1);

	if (buf) {
		int i;

		for (i = 0; i < size && str[i] != '\0'; i++) {
			buf[i] = str[i];
		}

		buf[i] = '\0';
	}

	return buf;
}

int Config_Load(const char* path) {
	FILE* fp = fopen(path, "rb");
	if (fp == NULL) {
		printf("Error opening config file.\n");
		return -1;
	}

	long length;
	char* buf;

	fseek(fp, 0, SEEK_END);
	length = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	buf = (char*)malloc(length);
	fread(buf, 1, length, fp);
	fclose(fp);

	jsmn_parser parser;
	jsmntok_t tokens[256];

	jsmn_init(&parser);

	int count = jsmn_parse(&parser, buf, strlen(buf), tokens, 256);

	if (count < 0) {
		printf("Error: malformed config file; ");
		switch (count) {
		case JSMN_ERROR_INVAL:
			printf("Bad token.\n");
			break;
		case JSMN_ERROR_NOMEM:
			printf("Not enough tokens.\n");
			break;
		case JSMN_ERROR_PART:
			printf("Expected more JSON data.\n");
			break;
		default:
			printf("\n");
			break;
		}
		return -1;
	}

	if (count < 1 || tokens[0].type != JSMN_OBJECT) {
		printf("Error: root object expected.\n");
		return -1;
	}

	// Parse JSON keys in the root object
	for (int i = 1; i < count; i++) {
		// Window position
		if (jsoncmp(buf, &tokens[i], "pos") == 0) {
			i++;
			if (tokens[i].type == JSMN_ARRAY && tokens[i].size == 2) {
				char* x = extract_string(buf + tokens[i + 1].start, tokens[i + 1].end - tokens[i + 1].start);
				char* y = extract_string(buf + tokens[i + 2].start, tokens[i + 2].end - tokens[i + 2].start);
				config.window_pos.x = atoi(x);
				config.window_pos.y = atoi(y);
				free(x);
				free(y);
				i += 2;
			}
		}

		// Window scale
		else if (jsoncmp(buf, &tokens[i], "scale") == 0) {
			i++;
			char* scale = extract_string(buf + tokens[i].start, tokens[i].end - tokens[i].start);
			config.window_scale = atoi(scale);
			free(scale);
		}

		// Volume
		else if (jsoncmp(buf, &tokens[i], "volume") == 0) {
			i++;
			char* volume = extract_string(buf + tokens[i].start, tokens[i].end - tokens[i].start);
			config.volume = atoi(volume) / 100.0f;
			free(volume);
		}

		// TODO keybindings
	}

	return 1;
}

void Config_LoadDefaults() {
	config.window_pos.x = 100;
	config.window_pos.y = 100;
	config.window_scale = 1;
}

void Config_Write(const char* path) {

}

void Config_Destroy() {
}
