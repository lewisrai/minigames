#include "a_menu.h"
#include "a_gol.h"
#include "a_wfc.h"
#include "render.h"
#include <stdbool.h>
#include <stdint.h>

typedef enum ProgramState { stateMenu, stateGOL, stateWFC } ProgramState;

static const uint8_t keySwitchProgramLeft = 0x51;  // Q
static const uint8_t keySwitchProgramRight = 0x45; // E

static bool allowProgramSwitch = true;
static ProgramState programState = stateMenu;

void InitialiseApp(uint32_t* bitmapData, int32_t bitmapWidth, int32_t bitmapHeight) {
    SetBitmapValues(bitmapData, bitmapWidth, bitmapHeight);
}

void UpdateApp(uint8_t* keyboardState, float cursorPositionX, float cursorPositionY) {
    switch (programState) {
    case stateMenu:
        if (keyboardState[keySwitchProgramRight] && allowProgramSwitch) {
            programState = stateGOL;
            NewGOL();
            break;
        }

        DrawBackground(0xFF111111);
        DrawRectangleBorder(0, 0, 640, 400, 0xFFFF99FF);
        DrawString("MENU", 7, 7, 0xFFFF99FF, 5);
        DrawString("Q - PREVIOUS PROGRAM", 31, 62, 0xFFFF99FF, 4);
        DrawString("E - NEXT PROGRAM", 31, 86, 0xFFFF99FF, 4);
        break;

    case stateGOL:
        if (keyboardState[keySwitchProgramLeft] && allowProgramSwitch) {
            programState = stateMenu;
            CloseGOL();
            break;
        }
        if (keyboardState[keySwitchProgramRight] && allowProgramSwitch) {
            programState = stateWFC;
            CloseGOL();
            NewWFC();
            break;
        }

        UpdateGOL(keyboardState, cursorPositionX, cursorPositionY);
        RenderGOL();
        break;

    case stateWFC:
        if (keyboardState[keySwitchProgramLeft] && allowProgramSwitch) {
            programState = stateGOL;
            CloseWFC();
            NewGOL();
            break;
        }

        UpdateWFC(keyboardState);
        RenderWFC();
        break;
    }

    allowProgramSwitch =
        !(keyboardState[keySwitchProgramLeft] || keyboardState[keySwitchProgramRight]);
}
