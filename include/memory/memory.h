#ifndef MEMORY_H
#define MEMORY_H

#include "../common.h"

#define MAP_CPU 0
#define MAP_VDC 1
#define MAP_ST 2

typedef struct {
} Memory;

void Memory_Reset();
BYTE Memory_ReadByte(int map, WORD addr);
WORD Memory_ReadWord(int map, WORD addr);
void Memory_WriteByte(int map, WORD addr, BYTE val);
void Memory_WriteWord(int map, WORD addr, WORD val);

#ifdef DEBUG_MODE
void Memory_Dump();
#endif

#endif
