#include "pch.h"
#include "Localization.h"

namespace
{
    struct TranslationStore
    {
        CSimpleIniA ini;
        bool loaded{ false };

        TranslationStore()
        {
            ini.SetUnicode();
            constexpr auto path = "Data/SKSE/Plugins/HomeAutoSort_Translation.ini";
            const auto result = ini.LoadFile(path);
            loaded = (result >= 0);

            if (!loaded) {
                logger::warn("Localization: could not load HomeAutoSort_Translation.ini; using English fallbacks");
            }
        }
    };

    TranslationStore& GetStore()
    {
        static TranslationStore store;
        return store;
    }
}

const char* Localization::Get(const char* section, const char* key, const char* fallback)
{
    auto& store = GetStore();
    if (!store.loaded) {
        return fallback;
    }

    const char* value = store.ini.GetValue(section, key, nullptr);
    if (!value || value[0] == '\0') {
        return fallback;
    }

    return value;
}
