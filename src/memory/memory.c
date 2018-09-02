#include "memory/memory.h"
#include "memory/cart.h"
#include "psg/psg.h"

Memory memory;
extern Cart cart;

void Memory_Reset() {

}

BYTE Memory_ReadByte(int map, WORD addr) {

}

WORD Memory_ReadWord(int map, WORD addr) {
	return (WORD)Memory_ReadByte(map, addr) | ((WORD)Memory_ReadByte(map, addr + 1) << 8);
}

void Memory_WriteByte(int map, WORD addr, BYTE val) {

}

void Memory_WriteWord(int map, WORD addr, WORD val) {
	Memory_WriteByte(map, addr, (BYTE)(val & 0x00FF));
	Memory_WriteByte(map, addr + 1, (BYTE)((val & 0xFF00) >> 8));
}

#ifdef DEBUG_MODE
void Memory_Dump() {
	FILE* fp = fopen("memory.log", "w");

	if (!fp) {
		printf("Error opening memory.log for output.\n");
		return;
	}

	fclose(fp);
}
#endif
