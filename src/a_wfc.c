#include "a_wfc.h"
#include "render.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

typedef struct State {
    bool collapsed;
    uint32_t possibleStates;
    int32_t currentState;
} State;

typedef struct TileData {
    int32_t uniqueStates;
    int32_t *leftEdge;
    int32_t *rightEdge;
    int32_t *topEdge;
    int32_t *bottomEdge;
} TileData;

typedef struct WFC {
    State *map;
    int32_t mapWidth;
    int32_t mapHeight;

    bool shouldGenerate;
    bool failed;
    int32_t percentageComplete;

    TileData tileData;
} WFC;

static bool ResetMap();
static bool UpdateMap();
static void CollapseSurroundingStates(int32_t index);
static int32_t GetEntropy(uint8_t possibleStates);
static void DetermineState(int32_t index, int32_t entropy);

static const uint32_t blank_texture[9] = {0xFF9999FF, 0xFF9999FF, 0xFF9999FF,
                                          0xFF9999FF, 0xFF9999FF, 0xFF9999FF,
                                          0xFF9999FF, 0xFF9999FF, 0xFF9999FF};

static const uint32_t triangle1_texture[9] = {0xFF9999FF, 0xFF99FF99, 0xFF9999FF,
                                              0xFF9999FF, 0xFF99FF99, 0xFF99FF99,
                                              0xFF9999FF, 0xFF99FF99, 0xFF9999FF};

static const uint32_t triangle2_texture[9] = {0xFF9999FF, 0xFF9999FF, 0xFF9999FF,
                                              0xFF99FF99, 0xFF99FF99, 0xFF99FF99,
                                              0xFF9999FF, 0xFF99FF99, 0xFF9999FF};

static const uint32_t triangle3_texture[9] = {0xFF9999FF, 0xFF99FF99, 0xFF9999FF,
                                              0xFF99FF99, 0xFF99FF99, 0xFF9999FF,
                                              0xFF9999FF, 0xFF99FF99, 0xFF9999FF};

static const uint32_t triangle4_texture[9] = {0xFF9999FF, 0xFF99FF99, 0xFF9999FF,
                                              0xFF99FF99, 0xFF99FF99, 0xFF99FF99,
                                              0xFF9999FF, 0xFF9999FF, 0xFF9999FF};

static const uint32_t *textures[5] = {blank_texture, triangle1_texture, triangle2_texture,
                                      triangle3_texture, triangle4_texture};

static const uint8_t keyRestartWFC = 0x52; // R

static WFC wfc;

void NewWFC() {
    srand(time(NULL));
    rand();

    wfc.mapWidth = 40;
    wfc.mapHeight = 40;
    wfc.tileData.uniqueStates = 5;

    wfc.map = malloc(sizeof(State) * wfc.mapWidth * wfc.mapHeight);

    wfc.tileData.leftEdge = malloc(sizeof(int32_t) * wfc.tileData.uniqueStates);
    wfc.tileData.rightEdge = malloc(sizeof(int32_t) * wfc.tileData.uniqueStates);
    wfc.tileData.topEdge = malloc(sizeof(int32_t) * wfc.tileData.uniqueStates);
    wfc.tileData.bottomEdge = malloc(sizeof(int32_t) * wfc.tileData.uniqueStates);

    const int32_t leftEdgeData[5] = {0, 0, 1, 1, 1};
    const int32_t rightEdgeData[5] = {0, 1, 1, 0, 1};
    const int32_t topEdgeData[5] = {0, 1, 0, 1, 1};
    const int32_t bottomEdgeData[5] = {0, 1, 1, 1, 0};

    for (int32_t i = 0; i < wfc.tileData.uniqueStates; i++) {
        wfc.tileData.leftEdge[i] = leftEdgeData[i];
        wfc.tileData.rightEdge[i] = rightEdgeData[i];
        wfc.tileData.topEdge[i] = topEdgeData[i];
        wfc.tileData.bottomEdge[i] = bottomEdgeData[i];
    }

    wfc.shouldGenerate = ResetMap();
}

void UpdateWFC(uint8_t *keyboardState) {
    if (keyboardState[keyRestartWFC]) {
        wfc.shouldGenerate = ResetMap();
    }
    wfc.shouldGenerate = UpdateMap();

    int32_t totalCollapsed = 0;

    for (int32_t i = 0; i < wfc.mapWidth * wfc.mapHeight; i++) {
        if (wfc.map[i].collapsed) {
            totalCollapsed++;
        }
    }

    wfc.percentageComplete = 100 * totalCollapsed / (wfc.mapWidth * wfc.mapHeight);

    if (wfc.percentageComplete != 100 && wfc.shouldGenerate == false) {
        wfc.failed = true;
    }
}

void RenderWFC() {
    DrawBackground(0xFF111111);

    DrawRectangleBorder(0, 0, 640, 400, 0xFFFF99FF);
    DrawRectangleBorder(0, 38, 362, 362, 0xFFFF99FF);

    DrawString("WFC", 7, 7, 0xFFFF99FF, 5);
    DrawString("R - RESET", 392, 42, 0xFFFF99FF, 4);

    DrawString("STATE:", 392, 92, 0xFFFF99FF, 2);

    if (wfc.percentageComplete == 100) {
        DrawString("COMPLETE!", 392, 104, 0xFFFF99FF, 4);
    } else if (wfc.failed == true) {
        DrawString("FAILED!", 392, 104, 0xFFFF99FF, 4);
    } else {
        DrawString("RUNNING", 392, 104, 0xFFFF99FF, 4);
    }

    DrawString("PERCENTAGE COMPLETE:", 392, 128, 0xFFFF99FF, 2);

    DrawInteger(wfc.percentageComplete, 392, 140, 0xFFFF99FF, 4);

    for (int32_t y = 0; y < wfc.mapHeight; y++) {
        for (int32_t x = 0; x < wfc.mapWidth; x++) {
            if (wfc.map[y * wfc.mapWidth + x].collapsed) {
                DrawTexture(textures[wfc.map[y * wfc.mapWidth + x].currentState], 3, 3, 9 * x + 1,
                            9 * y + 39, 9, 9);
            } else if (wfc.map[y * wfc.mapWidth + x].possibleStates == 0) {
                DrawRectangle(9 * x + 1, 9 * y + 39, 9, 9, 0xFFFF1111);
            }
        }
    }
}

void CloseWFC() {
    free(wfc.map);

    free(wfc.tileData.leftEdge);
    free(wfc.tileData.rightEdge);
    free(wfc.tileData.topEdge);
    free(wfc.tileData.bottomEdge);
}

int32_t GenerateRandomValue(int32_t low, int32_t high) { return (rand() % (high - low + 1)) + low; }

static bool ResetMap() {
    uint8_t allPossibleStates = 0;

    for (int32_t i = 0; i < wfc.tileData.uniqueStates; i++) {
        allPossibleStates <<= 1;
        allPossibleStates += 1;
    }

    for (int32_t i = 0; i < wfc.mapWidth * wfc.mapHeight; i++) {
        wfc.map[i].collapsed = false;
        wfc.map[i].possibleStates = allPossibleStates;
        wfc.map[i].currentState = 0;
    }

    const int32_t randomIndex = GenerateRandomValue(0, wfc.mapWidth * wfc.mapHeight - 1);

    wfc.map[randomIndex].collapsed = true;
    wfc.map[randomIndex].possibleStates = 0x00;
    wfc.map[randomIndex].currentState = GenerateRandomValue(0, wfc.tileData.uniqueStates - 1);

    wfc.failed = false;

    return true;
}

static bool UpdateMap() {
    if (!wfc.shouldGenerate) {
        return false;
    }

    bool stopGenerating = true;

    for (int32_t i = 0; i < wfc.mapWidth * wfc.mapHeight; i++) {
        if (!wfc.map[i].collapsed) {
            stopGenerating = false;
            break;
        }
    }

    if (stopGenerating) {
        return false;
    }

    for (int32_t i = 0; i < wfc.mapWidth * wfc.mapHeight; i++) {
        if (wfc.map[i].collapsed) {
            CollapseSurroundingStates(i);
        }
    }

    int32_t lowestEntropy = wfc.tileData.uniqueStates;
    int32_t entropy;

    for (int32_t i = 0; i < wfc.mapWidth * wfc.mapHeight; i++) {
        if (!wfc.map[i].collapsed) {
            entropy = GetEntropy(wfc.map[i].possibleStates);
            if (entropy < lowestEntropy) {
                lowestEntropy = entropy;
            }
        }
    }

    if (lowestEntropy == 0) {
        return false;
    }

    int32_t randomIndex = GenerateRandomValue(1, wfc.mapWidth * wfc.mapHeight);

    while (true) {
        for (int32_t i = 0; i < wfc.mapWidth * wfc.mapHeight; i++) {
            entropy = GetEntropy(wfc.map[i].possibleStates);
            if (entropy == lowestEntropy) {
                randomIndex--;
                if (randomIndex == 0) {
                    DetermineState(i, entropy);
                    return true;
                }
            }
        }
    }
}

static void CollapseSurroundingStates(int32_t index) {
    const uint8_t state = wfc.map[index].currentState;

    if ((index - 1 >= 0) && (!wfc.map[index - 1].collapsed) && (!(index % wfc.mapWidth == 0))) {
        for (int32_t i = 0; i < wfc.tileData.uniqueStates; i++) {
            if (wfc.tileData.leftEdge[state] != wfc.tileData.rightEdge[i]) {
                wfc.map[index - 1].possibleStates &= ~(1 << i);
            }
        }
    }
    if ((index + 1 >= 0) && (!wfc.map[index + 1].collapsed) &&
        (!((index + 1) % wfc.mapWidth == 0))) {
        for (int32_t i = 0; i < wfc.tileData.uniqueStates; i++) {
            if (wfc.tileData.rightEdge[state] != wfc.tileData.leftEdge[i]) {
                wfc.map[index + 1].possibleStates &= ~(1 << i);
            }
        }
    }
    if ((index - wfc.mapWidth >= 0) && (!wfc.map[index - wfc.mapWidth].collapsed)) {
        for (int32_t i = 0; i < wfc.tileData.uniqueStates; i++) {
            if (wfc.tileData.topEdge[state] != wfc.tileData.bottomEdge[i]) {
                wfc.map[index - wfc.mapWidth].possibleStates &= ~(1 << i);
            }
        }
    }
    if ((index + wfc.mapWidth >= 0) && (!wfc.map[index + wfc.mapWidth].collapsed)) {
        for (int32_t i = 0; i < wfc.tileData.uniqueStates; i++) {
            if (wfc.tileData.bottomEdge[state] != wfc.tileData.topEdge[i]) {
                wfc.map[index + wfc.mapWidth].possibleStates &= ~(1 << i);
            }
        }
    }
}

static int32_t GetEntropy(uint8_t possibleStates) {
    int32_t entropy = 0;

    while (possibleStates > 0) {
        entropy += possibleStates & 0x01;
        possibleStates >>= 1;
    }

    return entropy;
}

static void DetermineState(int32_t index, int32_t entropy) {
    int32_t currentState = 0;
    int32_t randomState = GenerateRandomValue(1, entropy);
    uint8_t possibleStates = wfc.map[index].possibleStates;

    while (true) {
        if (possibleStates & 0x01) {
            randomState--;
            if (randomState == 0) {
                wfc.map[index].collapsed = true;
                wfc.map[index].possibleStates = 0x00;
                wfc.map[index].currentState = currentState;
                return;
            }
        }

        currentState++;
        possibleStates >>= 1;
    }
}
