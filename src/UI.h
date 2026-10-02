#pragma once

#include "plugin.h"

namespace UI
{
    void Register();

    void __stdcall RenderGeneral();
    void __stdcall RenderResupply();
    void __stdcall RenderCell1();
    void __stdcall RenderCell2();
    void __stdcall RenderCell3();
    void __stdcall RenderCell4();
    void __stdcall RenderCell5();

    void HelpMarker(const char* desc);

    inline char _itemFilterBuf[256]{};
    inline char _containerFilterBuf[256]{};
}
