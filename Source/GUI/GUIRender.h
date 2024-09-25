#pragma once

struct MTGUIFontTextureDesc
{
    unsigned char* Pixels = nullptr;
    MTInt32 Width = 0;
    MTInt32 Height = 0;
};

struct MTGUIVertexDesc
{
    MTInt32 PosOffset = 0;
    MTInt32 UVOffset = 0;
    MTInt32 ColorOffset = 0;
    MTInt32 VertexSize = 0;
};
