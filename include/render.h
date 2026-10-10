#ifndef HEADER_FILE_RENDER
#define HEADER_FILE_RENDER

#include <stdint.h>

void SetBitmapValues(uint32_t* data, int32_t width, int32_t height);
void DrawBackground(uint32_t colour);
void DrawRectangle(int32_t x, int32_t y, int32_t width, int32_t height, uint32_t colour);
void DrawRectangleBorder(int32_t x, int32_t y, int32_t width, int32_t height, uint32_t colour);
void DrawString(char* buffer, int32_t x, int32_t y, uint32_t colour, int32_t scale);
void DrawInteger(int32_t number, int32_t x, int32_t y, uint32_t colour, int32_t scale);
void DrawTexture(const uint32_t* s_texture, int32_t s_width, int32_t s_height, int32_t d_x,
                 int32_t d_y, int32_t d_width, int32_t d_height);

#endif
