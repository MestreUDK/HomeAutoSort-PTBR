#pragma once

#include "plugin.h"
#include "SkyPrompt/API.hpp"

namespace PromptManager
{
    void Init();
    void Shutdown();
    void ShowForRef(RE::TESObjectREFR* ref);
    void Hide();
    void UpdateStashProgress(float progress);
    bool IsShowing();
    SkyPromptAPI::ClientID GetClientID();
}
