#ifndef HEADER_FILE_A_GOL
#define HEADER_FILE_A_GOL

#include <stdint.h>

void NewGOL();
void UpdateGOL(uint8_t *keyboardState, float cursorPositionX, float cursorPositionY);
void RenderGOL();
void CloseGOL();

#endif
