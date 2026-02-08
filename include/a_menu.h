#ifndef HEADER_FILE_A_MENU
#define HEADER_FILE_A_MENU

#include <stdint.h>

void InitialiseApp(uint32_t *bitmapData, int32_t bitmapWidth, int32_t bitmapHeight);
void UpdateApp(uint8_t *keyboardState, float cursorPositionX, float cursorPositionY);

#endif
