#include "a_gol.h"
#include "render.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct GOL {
    bool* visibleMap;
    bool* tempMap;

    int32_t mapWidth;
    int32_t mapHeight;

    bool shouldRun;
    int32_t updateCounter;
} GOL;

static void ResetMaps();
static int32_t GetNumberOfNeighbours(int32_t x, int32_t y);

static const uint8_t keyAddTile = 0x01;    // LMB
static const uint8_t keyRemoveTile = 0x02; // RMB
static const uint8_t keyStartGOL = 0x53;   // S
static const uint8_t keyStopGOL = 0x44;    // D
static const uint8_t keyResetGOL = 0x52;   // R

static GOL gol;

void NewGOL() {
    gol.mapWidth = 90;
    gol.mapHeight = 90;

    gol.visibleMap = malloc(sizeof(bool) * gol.mapWidth * gol.mapHeight);
    gol.tempMap = malloc(sizeof(bool) * gol.mapWidth * gol.mapHeight);

    ResetMaps();

    gol.shouldRun = false;
    gol.updateCounter = 3;
}

void UpdateGOL(uint8_t* keyboardState, float cursorPositionX, float cursorPositionY) {
    if (gol.shouldRun == false) {
        if (keyboardState[keyResetGOL]) {
            ResetMaps();
        }
        if (keyboardState[keyStartGOL]) {
            gol.shouldRun = true;
        }

        if (keyboardState[keyAddTile]) {
            if (cursorPositionX >= 1 && cursorPositionX < 361 && cursorPositionY >= 39 &&
                cursorPositionY < 399) {
                gol.visibleMap[(int32_t)((cursorPositionY - 39) / 4) * gol.mapWidth +
                               (int32_t)((cursorPositionX - 1) / 4)] = true;
            }
        } else if (keyboardState[keyRemoveTile]) {
            if (cursorPositionX >= 1 && cursorPositionX < 361 && cursorPositionY >= 39 &&
                cursorPositionY < 399) {
                gol.visibleMap[(int32_t)((cursorPositionY - 39) / 4) * gol.mapWidth +
                               (int32_t)((cursorPositionX - 1) / 4)] = false;
            }
        }
    } else {
        if (keyboardState[keyStopGOL]) {
            gol.shouldRun = false;
        }

        gol.updateCounter += 1;

        if (gol.updateCounter < 4) {
            return;
        }

        gol.updateCounter = 0;

        for (int32_t i = 0; i < gol.mapWidth * gol.mapHeight; i++) {
            gol.tempMap[i] = gol.visibleMap[i];
        }

        for (int32_t y = 0; y < gol.mapHeight; y++) {
            for (int32_t x = 0; x < gol.mapWidth; x++) {
                int32_t index = y * gol.mapWidth + x;
                int32_t numberOfNeighbours = GetNumberOfNeighbours(x, y);

                if (gol.tempMap[index] == false) {
                    if (numberOfNeighbours == 3) {
                        gol.visibleMap[index] = true;
                    } else {
                        gol.visibleMap[index] = false;
                    }
                } else {
                    if (numberOfNeighbours == 2 || numberOfNeighbours == 3) {
                        gol.visibleMap[index] = true;
                    } else {
                        gol.visibleMap[index] = false;
                    }
                }
            }
        }
    }
}

void RenderGOL() {
    DrawBackground(0xFF111111);

    DrawRectangleBorder(0, 0, 640, 400, 0xFFFF99FF);
    DrawRectangleBorder(0, 38, 362, 362, 0xFFFF99FF);

    DrawString("GAME OF LIFE", 7, 7, 0xFFFF99FF, 5);

    DrawString("S - START", 392, 42, 0xFFFF99FF, 4);
    DrawString("D - STOP", 392, 66, 0xFFFF99FF, 4);
    DrawString("R - RESET", 392, 90, 0xFFFF99FF, 4);

    DrawString("STATE:", 392, 140, 0xFFFF99FF, 2);

    if (gol.shouldRun) {
        DrawString("RUNNING", 392, 152, 0xFFFF99FF, 4);
    } else {
        DrawString("PAUSED", 392, 152, 0xFFFF99FF, 4);
    }

    for (int32_t y = 0; y < gol.mapHeight; y++) {
        for (int32_t x = 0; x < gol.mapWidth; x++) {
            if (gol.visibleMap[y * gol.mapWidth + x]) {
                DrawRectangle(4 * x + 1, 4 * y + 39, 4, 4, 0xFF99FF99);
            } else {
                DrawRectangle(4 * x + 1, 4 * y + 39, 4, 4, 0xFF111111);
            }
        }
    }
}

void CloseGOL() {
    free(gol.visibleMap);
    free(gol.tempMap);
}

static void ResetMaps() {
    for (int32_t i = 0; i < gol.mapWidth * gol.mapHeight; i++) {
        gol.visibleMap[i] = false;
        gol.tempMap[i] = false;
    }
}

static int32_t GetNumberOfNeighbours(int32_t x, int32_t y) {
    const int32_t xOffset[8] = {0, 1, 1, 1, 0, -1, -1, -1};
    const int32_t yOffset[8] = {1, 1, 0, -1, -1, -1, 0, 1};

    int32_t numberOfNeighbours = 0;

    for (int32_t i = 0; i < 8; i++) {
        if (x + xOffset[i] >= 0 && x + xOffset[i] < gol.mapWidth && y + yOffset[i] >= 0 &&
            y + yOffset[i] < gol.mapHeight) {
            if (gol.tempMap[(y + yOffset[i]) * gol.mapWidth + x + xOffset[i]]) {
                numberOfNeighbours++;
            }
        }
    }

    return numberOfNeighbours;
}
