#pragma once

namespace Localization
{
    // Reads Data/SKSE/Plugins/HomeAutoSort_Translation.ini.
    // Falls back to the original English text when a section/key is missing.
    const char* Get(const char* section, const char* key, const char* fallback);
}
