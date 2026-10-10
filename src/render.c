#include "render.h"
#include <stdint.h>
#include <stdio.h>

extern const uint32_t customFont[128];

static uint32_t* bitmapData;
static int32_t bitmapWidth;
static int32_t bitmapHeight;

void SetBitmapValues(uint32_t* data, int32_t width, int32_t height) {
    bitmapData = data;
    bitmapWidth = width;
    bitmapHeight = height;
}

static inline int32_t ClampWidthCoords(int32_t position) {
    if (position < 0) {
        return 0;
    } else if (position > bitmapWidth) {
        return bitmapWidth;
    }

    return position;
}

static inline int32_t ClampHeightCoords(int32_t position) {
    if (position < 0) {
        return 0;
    } else if (position > bitmapHeight) {
        return bitmapHeight;
    }

    return position;
}

static inline void DrawPixel(int32_t x, int32_t y, uint32_t colour) {
    if (x >= 0 && x < bitmapWidth && y >= 0 && y < bitmapHeight) {
        bitmapData[y * bitmapWidth + x] = colour;
    }
}

void DrawBackground(uint32_t colour) {
    for (int32_t i = 0; i < bitmapWidth * bitmapHeight; i++) {
        bitmapData[i] = colour;
    }
}

void DrawRectangle(int32_t x, int32_t y, int32_t width, int32_t height, uint32_t colour) {
    const int32_t x2 = ClampWidthCoords(x + width);
    const int32_t y2 = ClampHeightCoords(y + height);

    for (int32_t y1 = ClampHeightCoords(y); y1 < y2; y1++) {
        for (int32_t x1 = ClampWidthCoords(x); x1 < x2; x1++) {
            bitmapData[y1 * bitmapWidth + x1] = colour;
        }
    }
}

void DrawRectangleBorder(int32_t x, int32_t y, int32_t width, int32_t height, uint32_t colour) {
    const int32_t x1 = ClampWidthCoords(x);
    const int32_t y1 = ClampHeightCoords(y);
    const int32_t x2 = ClampWidthCoords(x + width);
    const int32_t y2 = ClampHeightCoords(y + height);

    if (y == y1) {
        for (int32_t tempX = x1; tempX < x2; tempX++) {
            bitmapData[y1 * bitmapWidth + tempX] = colour;
        }
    }

    if (y + height == y2) {
        for (int32_t tempX = x1; tempX < x2; tempX++) {
            bitmapData[(y2 - 1) * bitmapWidth + tempX] = colour;
        }
    }

    if (x == x1) {
        for (int32_t tempY = y1; tempY < y2; tempY++) {
            bitmapData[tempY * bitmapWidth + x1] = colour;
        }
    }

    if (x + width == x2) {
        for (int32_t tempY = y1; tempY < y2; tempY++) {
            bitmapData[tempY * bitmapWidth + x2 - 1] = colour;
        }
    }
}

void DrawString(char* buffer, int32_t x, int32_t y, uint32_t colour, int32_t scale) {
    uint32_t charFont;

    int32_t tempX;
    int32_t tempY;

    for (; *buffer != '\0'; buffer++) {
        charFont = customFont[(int)*buffer];

        for (int32_t charY = 0; charY < 5; charY++) {
            for (int32_t charX = 0; charX < 5; charX++) {
                if (charFont & 1) {
                    tempX = x + charX * scale;
                    tempY = y + charY * scale;

                    for (int32_t b = 0; b < scale; b++) {
                        for (int32_t a = 0; a < scale; a++) {
                            DrawPixel(tempX + a, tempY + b, colour);
                        }
                    }
                }

                charFont >>= 1;
            }
        }

        x += 5 * scale + 1;
    }
}

void DrawInteger(int32_t number, int32_t x, int32_t y, uint32_t colour, int32_t scale) {
    char buffer[8];

    snprintf(buffer, 8, "%d", number);

    DrawString(buffer, x, y, colour, scale);
}

void DrawTexture(const uint32_t* s_texture, int32_t s_width, int32_t s_height, int32_t d_x,
                 int32_t d_y, int32_t d_width, int32_t d_height) {
    int32_t tempX;
    int32_t tempY;

    uint32_t colour;

    for (int32_t y = 0; y < d_height; y++) {
        for (int32_t x = 0; x < d_width; x++) {
            tempX = (float)(x * s_width) / (float)d_width;
            tempY = (float)(y * s_height) / (float)d_height;

            colour = s_texture[tempY * s_width + tempX];

            DrawPixel(d_x + x, d_y + y, colour);
        }
    }
}
