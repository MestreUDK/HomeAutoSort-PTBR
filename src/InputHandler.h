#pragma once

#include "plugin.h"

namespace InputHandler
{
    void Register();
    bool IsPressed();
    bool IsHolding();
    float GetHoldDuration();
    void ResetHold();
    void ConsumePress();
}
