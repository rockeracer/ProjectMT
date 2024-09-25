#pragma once

struct MTGUIInputState
{
    bool bMousePressed[3] = {false, false, false};
    float MouseWheel = 0.0f;
    MTInt32 MousePosX = 0;
    MTInt32 MousePosY = 0;
    
    bool bKeyPressed[512] = {0};
    bool bKeyShiftPressed = false;
    bool bKeyCtrlPressed = false;
    bool bKeyAltPressed = false;
    bool bKeySuperPressed = false;
    
    MTArray<MTString> Text;
    
    void InitLoop()
    {
        bMousePressed[0] = bMousePressed[1] = bMousePressed[2] = false;
        MouseWheel = 0.0f;
        Text.Clear();
    }
};
