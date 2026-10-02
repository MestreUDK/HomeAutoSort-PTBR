#include "pch.h"
#include "UI.h"
#include "Settings.h"
#include "FormCache.h"
#include "CellManager.h"
#include "StashLogic.h"
#include "ResupplyLogic.h"
#include "PromptManager.h"
#include "Localization.h"

namespace
{
    #define TR(section, key, fallback) Localization::Get(section, key, fallback)

    const char* dxKbNames[] = {
        "[NONE]", "Escape", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0",
        "Minus", "Equals", "Backspace", "Tab", "Q", "W", "E", "R", "T", "Y",
        "U", "I", "O", "P", "Left Bracket", "Right Bracket", "Enter", "Left Ctrl",
        "A", "S", "D", "F", "G", "H", "J", "K", "L", "Semicolon", "Apostrophe",
        "Grave", "Left Shift", "Backslash", "Z", "X", "C", "V", "B", "N", "M",
        "Comma", "Period", "Slash", "Right Shift", "Numpad *", "Left Alt", "Space",
        "Caps Lock", "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10",
        "Num Lock", "Scroll Lock", "Numpad 7", "Numpad 8", "Numpad 9", "Numpad -",
        "Numpad 4", "Numpad 5", "Numpad 6", "Numpad +", "Numpad 1", "Numpad 2",
        "Numpad 3", "Numpad 0", "Numpad .", "Mouse Left", "Mouse Right",
        "Mouse Middle", "Mouse 4", "Mouse 5", "Mouse Wheel Up", "Mouse Wheel Down"
    };

    const uint32_t dxKbValues[] = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
        20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37,
        38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55,
        56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73,
        74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 256, 257, 258, 259, 260, 264, 265
    };

    const int kbComboCount = sizeof(dxKbValues) / sizeof(dxKbValues[0]);

    const char* dxGpNames[] = {
        "[NONE]", "DPAD UP", "DPAD DOWN", "DPAD LEFT", "DPAD RIGHT",
        "START", "BACK", "LEFT THUMB", "RIGHT THUMB",
        "LEFT SHOULDER", "RIGHT SHOULDER",
        "A", "B", "X", "Y", "LT", "RT"
    };

    const uint32_t dxGpValues[] = {
        0, 266, 267, 268, 269, 270, 271, 272, 273, 274, 275, 276, 277, 278, 279, 280, 281
    };

    const int gpComboCount = sizeof(dxGpValues) / sizeof(dxGpValues[0]);

    int FindComboIndex(const uint32_t* values, int count, uint32_t target)
    {
        for (int i = 0; i < count; ++i)
            if (values[i] == target) return i;
        return 0;
    }

    bool ContainsIgnoreCase(std::string_view haystack, std::string_view needle)
    {
        if (needle.empty()) return true;
        auto it = std::search(haystack.begin(), haystack.end(), needle.begin(), needle.end(),
            [](char a, char b) { return std::tolower(static_cast<unsigned char>(a)) == std::tolower(static_cast<unsigned char>(b)); });
        return it != haystack.end();
    }

    static std::map<std::string, std::string> s_comboFilterCache;

    bool MatchContainerKey(const ContainerInfo& ci, const std::string& key)
    {
        return CellManager::MatchContainer(ci, key);
    }

    std::string ContainerKey(const ContainerInfo& ci)
    {
        return CellManager::ContainerKey(ci);
    }

    std::string ContainerLabel(const ContainerInfo& ci)
    {
        return CellManager::ContainerLabel(ci);
    }

    void RenderContainerCombo(const char* label, std::string& currentEditorID,
                              const CellHomeData& cellData, bool enabled,
                              const ImGuiMCP::ImVec4& color = ImGuiMCP::ImVec4(1.0f, 1.0f, 1.0f, 1.0f))
    {
        ImGuiMCP::PushID(label);

        auto& filterStr = s_comboFilterCache[label];
        char filterBuf[256];
        strncpy_s(filterBuf, filterStr.c_str(), sizeof(filterBuf) - 1);
        filterBuf[sizeof(filterBuf) - 1] = '\0';
        ImGuiMCP::InputTextWithHint("##cf", TR("$UI", "filterContainers", "Filter containers..."), filterBuf, sizeof(filterBuf));
        ImGuiMCP::SameLine();
        UI::HelpMarker(TR("$UI", "filterContainersHelp",
            "Type to filter. Case-insensitive partial match on container name, base form ID, or reference ID.\n\nTo find a container's IDs: open the console (~), click the container to see its reference (e.g. 000ABC12), then type that number into the filter."));
        filterStr = filterBuf;

        std::string preview = TR("$UI", "none", "[None]");
        int selectedIdx = 0;

        if (!currentEditorID.empty())
        {
            for (size_t i = 0; i < cellData.containers.size(); ++i)
            {
                if (MatchContainerKey(cellData.containers[i], currentEditorID))
                {
                    preview = ContainerLabel(cellData.containers[i]);
                    selectedIdx = static_cast<int>(i) + 1;
                    break;
                }
            }
        }

        ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_PopupBg, ImGuiMCP::ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
        ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_Text, color);

        ImGuiMCP::BeginDisabled(!enabled);

        if (ImGuiMCP::BeginCombo("##co", preview.c_str()))
        {
            bool filterActive = !filterStr.empty();

            if (ImGuiMCP::Selectable(TR("$UI", "none", "[None]"), selectedIdx == 0))
            {
                currentEditorID.clear();
                Settings::GetSingleton().Save();
            }
            if (selectedIdx == 0) ImGuiMCP::SetItemDefaultFocus();

            for (size_t i = 0; i < cellData.containers.size(); ++i)
            {
                char hexID[9];
                snprintf(hexID, sizeof(hexID), "%06X", cellData.containers[i].refFormID & 0xFFFFFF);
                if (filterActive &&
                    !ContainsIgnoreCase(cellData.containers[i].name, filterStr) &&
                    !ContainsIgnoreCase(cellData.containers[i].editorID, filterStr) &&
                    !ContainsIgnoreCase(hexID, filterStr))
                    continue;

                char buf[512];
                snprintf(buf, sizeof(buf), "%s [0x%06X]",
                    cellData.containers[i].name.c_str(),
                    cellData.containers[i].refFormID & 0xFFFFFF);
                bool isSel = (static_cast<int>(i) + 1 == selectedIdx);
                if (ImGuiMCP::Selectable(buf, isSel))
                {
                    currentEditorID = ContainerKey(cellData.containers[i]);
                    Settings::GetSingleton().Save();
                }
                if (isSel) ImGuiMCP::SetItemDefaultFocus();
            }
            ImGuiMCP::EndCombo();
        }

        ImGuiMCP::EndDisabled();

        ImGuiMCP::PopStyleColor(2);
        ImGuiMCP::PopID();
    }

    void RenderSubLabel(const char* text, const ImGuiMCP::ImVec4& color)
    {
        ImGuiMCP::Bullet();
        ImGuiMCP::SameLine();
        ImGuiMCP::TextColored(color, "%s", text);
    }

    void RenderCellContent(int cellIndex, const char* tabName)
    {
        auto& s = Settings::GetSingleton();
        auto& cs = s.cells[cellIndex];
        auto& cellData = CellManager::GetCellData(cellIndex);
        bool homeSet = cs.homeSet;
        bool inCell = (CellManager::FindActiveCellIndex() == cellIndex);
        bool enabled = homeSet && inCell;
        bool l5 = s.lorerim5Compat.load();

        const ImGuiMCP::ImVec4 COL_WHITE = ImGuiMCP::ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
        const ImGuiMCP::ImVec4 COL_WEAPONS = ImGuiMCP::ImVec4(1.0f, 0.4f, 0.4f, 1.0f);
        const ImGuiMCP::ImVec4 COL_ARMOR = ImGuiMCP::ImVec4(0.4f, 0.6f, 1.0f, 1.0f);
        const ImGuiMCP::ImVec4 COL_JEWELRY = ImGuiMCP::ImVec4(1.0f, 0.8f, 0.2f, 1.0f);
        const ImGuiMCP::ImVec4 COL_POTIONS = ImGuiMCP::ImVec4(0.4f, 1.0f, 0.4f, 1.0f);
        const ImGuiMCP::ImVec4 COL_POISONS = ImGuiMCP::ImVec4(0.8f, 0.3f, 0.9f, 1.0f);
        const ImGuiMCP::ImVec4 COL_SCROLLS = ImGuiMCP::ImVec4(0.3f, 0.9f, 1.0f, 1.0f);
        const ImGuiMCP::ImVec4 COL_CONSUMABLES = ImGuiMCP::ImVec4(1.0f, 0.7f, 0.3f, 1.0f);
        const ImGuiMCP::ImVec4 COL_INGREDIENTS = ImGuiMCP::ImVec4(0.6f, 1.0f, 0.6f, 1.0f);
        const ImGuiMCP::ImVec4 COL_WRITTEN = ImGuiMCP::ImVec4(0.9f, 0.8f, 0.6f, 1.0f);
        const ImGuiMCP::ImVec4 COL_KEYS = ImGuiMCP::ImVec4(0.9f, 0.9f, 0.0f, 1.0f);
        const ImGuiMCP::ImVec4 COL_AMMO = ImGuiMCP::ImVec4(0.6f, 0.8f, 1.0f, 1.0f);
        const ImGuiMCP::ImVec4 COL_MISC = ImGuiMCP::ImVec4(0.5f, 0.9f, 0.9f, 1.0f);

        ImGuiMCP::TextColored(ImGuiMCP::ImVec4(0.7f, 0.85f, 1.0f, 1.0f), "%s", tabName);

        if (ImGuiMCP::Button(TR("$UI", "selectHome", "Select this Cell as Home for Storage System")))
        {
            SKSE::GetTaskInterface()->AddTask([cellIndex]() {
                CellManager::ScanContainers(cellIndex);
                Settings::GetSingleton().Save();
            });
        }

        if (homeSet)
        {
            ImGuiMCP::SameLine();
            ImGuiMCP::TextColored(ImGuiMCP::ImVec4(0.3f, 1.0f, 0.3f, 1.0f), TR("$UI", "setToHome", "Set to Home"));
            ImGuiMCP::SameLine();
            ImGuiMCP::TextColored(ImGuiMCP::ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "%s", cellData.cellName.c_str());
            if (!cellData.cellEditorID.empty())
            {
                ImGuiMCP::SameLine();
                ImGuiMCP::TextColored(ImGuiMCP::ImVec4(0.3f, 1.0f, 0.3f, 1.0f), " (%s)", cellData.cellEditorID.c_str());
            }
        }
        else
        {
            ImGuiMCP::SameLine();
            ImGuiMCP::TextColored(ImGuiMCP::ImVec4(1.0f, 0.3f, 0.3f, 1.0f), TR("$UI", "homeUnset", "Home Unset"));
        }

        ImGuiMCP::Spacing();
        ImGuiMCP::Separator();

        static bool s_showImport = false;
        ImGuiMCP::Spacing();
        if (ImGuiMCP::Button(TR("$UI", "importPreset", "Import preset")))
        {
            s_showImport = !s_showImport;
        }
        ImGuiMCP::SameLine();
        ImGuiMCP::SetItemTooltip(TR("$UI", "importPresetHelp", "Load a preset that matches the current cell."));

        if (homeSet)
        {
            if (ImGuiMCP::Button(TR("$UI", "clear", "Clear")))
            {
                cs = CellSettings{};
                Settings::GetSingleton().Save();
                CellManager::ClearCellData(cellIndex);
                Settings::ClearActivePreset(cellIndex);
            }
            ImGuiMCP::SetItemTooltip(TR("$UI", "clearHelp", "Clear all SKSE menu settings for this cell (including home setup)."));

            ImGuiMCP::SameLine();
            if (ImGuiMCP::Button(TR("$UI", "exportPreset", "Export as preset")))
            {
                Settings::ExportPreset(cellIndex);
                Settings::LoadPresetCache();
            }
            ImGuiMCP::SetItemTooltip(TR("$UI", "exportPresetHelp", "Save this cell\'s container setup to a preset JSON file for reuse."));
        }

        ImGuiMCP::Spacing();
        if (s_showImport)
        {
                auto* playerCell = CellManager::GetPlayerParentCell();
                std::string currentCellEdid;
                if (playerCell)
                {
                    const char* c = playerCell->GetFormEditorID();
                    if (c) currentCellEdid = c;
                }

                std::vector<std::pair<std::string, std::string>> matches;
                for (auto& [cacheKey, cacheVal] : Settings::GetAllCachedPresets())
                {
                    if (!currentCellEdid.empty() && cacheKey != currentCellEdid) continue;
                    std::string label = cacheKey + " (" + cacheVal.homeEditorID + ")";
                    matches.push_back({cacheKey, label});
                }

                if (matches.empty())
                {
                    ImGuiMCP::TextDisabled(TR("$UI", "noMatchingPresets", "(no matching presets for this cell)"));
                }
                else
                {
                    static int selectedPreset = -1;
                    if (selectedPreset >= (int)matches.size()) selectedPreset = -1;
                    const char* preview = (selectedPreset >= 0) ? matches[selectedPreset].second.c_str() : TR("$UI", "none", "[None]");
                    ImGuiMCP::SetNextItemWidth(ImGuiMCP::GetWindowWidth() * 0.5f);
                    if (ImGuiMCP::BeginCombo("##presetDrop", preview))
                    {
                        if (ImGuiMCP::Selectable(TR("$UI", "none", "[None]"), selectedPreset < 0))
                        {
                            selectedPreset = -1;
                        }
                        if (selectedPreset < 0) ImGuiMCP::SetItemDefaultFocus();
                        for (size_t i = 0; i < matches.size(); ++i)
                        {
                            bool sel = (static_cast<int>(i) == selectedPreset);
                            if (ImGuiMCP::Selectable(matches[i].second.c_str(), sel))
                            {
                                selectedPreset = static_cast<int>(i);
                                auto* presetPtr = Settings::FindPresetInCache(matches[i].first);
                                if (presetPtr)
                                {
                                    cs = *presetPtr;
                                    cs.homeSet = true;
                                    cs.homeEditorID = matches[i].first;
                                    Settings::GetSingleton().Save();
                                    CellManager::ClearCellData(cellIndex);
            std::string currentCellEdid = cs.homeEditorID;
            if (currentCellEdid.empty()) currentCellEdid = cellData.cellEditorID;
            SKSE::GetTaskInterface()->AddTask([cellIndex, currentCellEdid]() {
                                        CellManager::ScanContainers(cellIndex);
                                    });
                                }
                            }
                            if (sel) ImGuiMCP::SetItemDefaultFocus();
                        }
                        ImGuiMCP::EndCombo();
                    }
                    ImGuiMCP::SameLine();
                    UI::HelpMarker(TR("$UI", "presetListHelp", "Only shows presets matching the cell you are currently in. Select one to apply it."));
                }
            }

        ImGuiMCP::Spacing();
        ImGuiMCP::Separator();

        ImGuiMCP::TextWrapped(TR("$UI", "protectedItems", "Equipped, favorited, and quest items are never moved."));

        if (homeSet)
        {
            ImGuiMCP::TextColored(ImGuiMCP::ImVec4(1.0f, 0.6f, 0.3f, 1.0f), TR("$UI", "needMasterChest", "Set a Master Chest below, then go to it and use the SkyPrompt to stash or resupply."));
        }
        else
        {
            ImGuiMCP::TextColored(ImGuiMCP::ImVec4(1.0f, 0.6f, 0.3f, 1.0f), TR("$UI", "selectHomeFirst", "Select this cell as your home first, then set a Master Chest."));
        }

        if (!enabled && homeSet)
        {
            ImGuiMCP::TextColored(ImGuiMCP::ImVec4(1.0f, 0.6f, 0.3f, 1.0f), TR("$UI", "standInHome", "Stand in the home cell to configure containers."));
        }

        ImGuiMCP::Spacing();
        ImGuiMCP::TextColored(COL_WHITE, TR("$Categories", "masterChest", "Master Chest"));
        ImGuiMCP::SetItemTooltip(TR("$UI", "masterChestHelp", "The primary container for this cell. Items that don\'t fit any category go here."));
        RenderContainerCombo("##masterChest", cs.masterChestEditorID, cellData, enabled, COL_WHITE);

        ImGuiMCP::Spacing();
        ImGuiMCP::Separator();
        ImGuiMCP::TextWrapped(TR("$UI", "priorityHelp", "Priority: items first go into their specific subcategory container. If no subcategory container is assigned, items fall back to the main category container."));

        ImGuiMCP::Spacing();
        ImGuiMCP::Separator();
        ImGuiMCP::TextColored(COL_WEAPONS, TR("$Categories", "weapons", "Weapons"));
        RenderContainerCombo("##weapons", cs.weapons, cellData, enabled, COL_WEAPONS);
        ImGuiMCP::Indent();
        RenderSubLabel(TR("$Categories", "oneHanded", "One-Handed"), COL_WEAPONS);
        RenderContainerCombo("##weapons1H", cs.weapons_OneHanded, cellData, enabled, COL_WEAPONS);
        RenderSubLabel(TR("$Categories", "twoHanded", "Two-Handed"), COL_WEAPONS);
        RenderContainerCombo("##weapons2H", cs.weapons_TwoHanded, cellData, enabled, COL_WEAPONS);
        RenderSubLabel(TR("$Categories", "archery", "Archery"), COL_WEAPONS);
        RenderContainerCombo("##weaponsBow", cs.weapons_Archery, cellData, enabled, COL_WEAPONS);
        RenderSubLabel(TR("$Categories", "staves", "Staves"), COL_WEAPONS);
        RenderContainerCombo("##weaponsStaves", cs.weapons_Staves, cellData, enabled, COL_WEAPONS);
        ImGuiMCP::Unindent();

        ImGuiMCP::Spacing();
        ImGuiMCP::Separator();
        ImGuiMCP::TextColored(COL_ARMOR, TR("$Categories", "armor", "Armor"));
        RenderContainerCombo("##armor", cs.armor, cellData, enabled, COL_ARMOR);
        ImGuiMCP::Indent();
        RenderSubLabel(TR("$Categories", "light", "Light"), COL_ARMOR);
        RenderContainerCombo("##armorLight", cs.armor_Light, cellData, enabled, COL_ARMOR);
        RenderSubLabel(TR("$Categories", "heavy", "Heavy"), COL_ARMOR);
        RenderContainerCombo("##armorHeavy", cs.armor_Heavy, cellData, enabled, COL_ARMOR);
        RenderSubLabel(TR("$Categories", "clothing", "Clothing"), COL_ARMOR);
        RenderContainerCombo("##armorClothing", cs.armor_Clothing, cellData, enabled, COL_ARMOR);
        RenderSubLabel(TR("$Categories", "shield", "Shield"), COL_ARMOR);
        RenderContainerCombo("##armorShield", cs.armor_Shield, cellData, enabled, COL_ARMOR);
        ImGuiMCP::Unindent();

        ImGuiMCP::Spacing();
        ImGuiMCP::Separator();
        ImGuiMCP::TextColored(COL_JEWELRY, TR("$Categories", "jewelry", "Jewelry"));
        RenderContainerCombo("##jewelry", cs.jewelry, cellData, enabled, COL_JEWELRY);
        ImGuiMCP::Indent();
        RenderSubLabel(TR("$Categories", "rings", "Rings"), COL_JEWELRY);
        RenderContainerCombo("##jewelryRings", cs.jewelry_Rings, cellData, enabled, COL_JEWELRY);
        RenderSubLabel(TR("$Categories", "amulets", "Amulets"), COL_JEWELRY);
        RenderContainerCombo("##jewelryAmulets", cs.jewelry_Amulets, cellData, enabled, COL_JEWELRY);
        RenderSubLabel(TR("$Categories", "circlets", "Circlets"), COL_JEWELRY);
        RenderContainerCombo("##jewelryCirclets", cs.jewelry_Circlets, cellData, enabled, COL_JEWELRY);
        ImGuiMCP::Unindent();

        ImGuiMCP::Spacing();
        ImGuiMCP::Separator();
        ImGuiMCP::TextColored(COL_POTIONS, TR("$Categories", "potions", "Potions"));
        RenderContainerCombo("##potions", cs.potions, cellData, enabled, COL_POTIONS);
        ImGuiMCP::TextColored(COL_POISONS, TR("$Categories", "poisons", "Poisons"));
        RenderContainerCombo("##poisons", cs.poisons, cellData, enabled, COL_POISONS);
        ImGuiMCP::TextColored(COL_SCROLLS, TR("$Categories", "scrolls", "Scrolls"));
        RenderContainerCombo("##scrolls", cs.scrolls, cellData, enabled, COL_SCROLLS);

        ImGuiMCP::Spacing();
        ImGuiMCP::Separator();
        ImGuiMCP::TextColored(COL_CONSUMABLES, TR("$Categories", "consumables", "Consumables"));
        RenderContainerCombo("##consumables", cs.consumables, cellData, enabled, COL_CONSUMABLES);
        ImGuiMCP::Indent();
        RenderSubLabel(TR("$Categories", "rawFood", "Raw Food"), COL_CONSUMABLES);
        RenderContainerCombo("##consumablesRaw", cs.consumables_Raw, cellData, enabled, COL_CONSUMABLES);
        RenderSubLabel(TR("$Categories", "cookedFood", "Cooked Food"), COL_CONSUMABLES);
        RenderContainerCombo("##consumablesCooked", cs.consumables_Cooked, cellData, enabled, COL_CONSUMABLES);
        RenderSubLabel(TR("$Categories", "drinks", "Drinks"), COL_CONSUMABLES);
        RenderContainerCombo("##consumablesDrinks", cs.consumables_Drinks, cellData, enabled, COL_CONSUMABLES);

        if (l5)
        {
            RenderSubLabel(TR("$Categories", "alcoholLorerim", "Alcohol - Lorerim 5"), COL_CONSUMABLES);
            RenderContainerCombo("##consumablesAlc", cs.consumables_Alcohol, cellData, enabled, COL_CONSUMABLES);
            RenderSubLabel(TR("$Categories", "nonAlcoholicLorerim", "Non-Alcoholic Drinks - Lorerim 5"), COL_CONSUMABLES);
            RenderContainerCombo("##consumablesNADrink", cs.consumables_NonAlcoholicDrinks, cellData, enabled, COL_CONSUMABLES);
            RenderSubLabel(TR("$Categories", "rawMeatLorerim", "Raw Meat - Lorerim 5"), COL_CONSUMABLES);
            RenderContainerCombo("##consumablesRawMeat", cs.consumables_RawMeat, cellData, enabled, COL_CONSUMABLES);
            RenderSubLabel(TR("$Categories", "cookedMeatLorerim", "Cooked Meat - Lorerim 5"), COL_CONSUMABLES);
            RenderContainerCombo("##consumablesCookedMeat", cs.consumables_CookedMeat, cellData, enabled, COL_CONSUMABLES);
            RenderSubLabel(TR("$Categories", "produceGrainsLorerim", "Produce and Grains - Lorerim 5"), COL_CONSUMABLES);
            RenderContainerCombo("##consumablesProduce", cs.consumables_ProduceGrains, cellData, enabled, COL_CONSUMABLES);
            RenderSubLabel(TR("$Categories", "cheeseLorerim", "Cheese - Lorerim 5"), COL_CONSUMABLES);
            RenderContainerCombo("##consumablesCheese", cs.consumables_Cheese, cellData, enabled, COL_CONSUMABLES);
            RenderSubLabel(TR("$Categories", "soupsLorerim", "Soups - Lorerim 5"), COL_CONSUMABLES);
            RenderContainerCombo("##consumablesSoups", cs.consumables_Soups, cellData, enabled, COL_CONSUMABLES);
        }
        ImGuiMCP::Unindent();

        ImGuiMCP::Spacing();
        ImGuiMCP::Separator();
        ImGuiMCP::TextColored(COL_INGREDIENTS, TR("$Categories", "ingredients", "Ingredients"));
        RenderContainerCombo("##ingredients", cs.ingredients, cellData, enabled, COL_INGREDIENTS);

        ImGuiMCP::Spacing();
        ImGuiMCP::Separator();
        ImGuiMCP::TextColored(COL_WRITTEN, TR("$Categories", "writtenWorks", "Written Works"));
        RenderContainerCombo("##writtenWorks", cs.writtenWorks, cellData, enabled, COL_WRITTEN);
        ImGuiMCP::Indent();
        RenderSubLabel(TR("$Categories", "books", "Books"), COL_WRITTEN);
        RenderContainerCombo("##wwBooks", cs.writtenWorks_Books, cellData, enabled, COL_WRITTEN);
        RenderSubLabel(TR("$Categories", "notesLettersJournals", "Notes / Letters / Journals"), COL_WRITTEN);
        RenderContainerCombo("##wwNotes", cs.writtenWorks_Notes, cellData, enabled, COL_WRITTEN);
        RenderSubLabel(TR("$Categories", "skillBooks", "Skill Books"), COL_WRITTEN);
        RenderContainerCombo("##wwSkill", cs.writtenWorks_SkillBooks, cellData, enabled, COL_WRITTEN);
        RenderSubLabel(TR("$Categories", "spellBooks", "Spell Books"), COL_WRITTEN);
        RenderContainerCombo("##wwSpell", cs.writtenWorks_SpellBooks, cellData, enabled, COL_WRITTEN);
        ImGuiMCP::Unindent();

        ImGuiMCP::Spacing();
        ImGuiMCP::Separator();
        ImGuiMCP::TextColored(COL_KEYS, TR("$Categories", "keys", "Keys"));
        RenderContainerCombo("##keys", cs.keys, cellData, enabled, COL_KEYS);

        ImGuiMCP::Spacing();
        ImGuiMCP::Separator();
        ImGuiMCP::TextColored(COL_AMMO, TR("$Categories", "ammunition", "Ammunition"));
        RenderContainerCombo("##ammo", cs.ammo, cellData, enabled, COL_AMMO);
        ImGuiMCP::Indent();
        RenderSubLabel(TR("$Categories", "arrows", "Arrows"), COL_AMMO);
        RenderContainerCombo("##ammoArrows", cs.ammo_Arrows, cellData, enabled, COL_AMMO);
        RenderSubLabel(TR("$Categories", "bolts", "Bolts"), COL_AMMO);
        RenderContainerCombo("##ammoBolts", cs.ammo_Bolts, cellData, enabled, COL_AMMO);
        ImGuiMCP::Unindent();


        ImGuiMCP::Spacing();
        ImGuiMCP::Separator();
        ImGuiMCP::TextColored(COL_MISC, TR("$Categories", "miscellaneous", "Miscellaneous"));
        RenderContainerCombo("##misc", cs.misc, cellData, enabled, COL_MISC);
        ImGuiMCP::Indent();
        RenderSubLabel(TR("$Categories", "ore", "Ore"), COL_MISC);
        RenderContainerCombo("##miscOre", cs.misc_Ore, cellData, enabled, COL_MISC);
        RenderSubLabel(TR("$Categories", "ingot", "Ingot"), COL_MISC);
        RenderContainerCombo("##miscIngot", cs.misc_Ingot, cellData, enabled, COL_MISC);
        RenderSubLabel(TR("$Categories", "gem", "Gem"), COL_MISC);
        RenderContainerCombo("##miscGem", cs.misc_Gem, cellData, enabled, COL_MISC);
        RenderSubLabel(TR("$Categories", "emptySoulGem", "Empty Soul Gem"), COL_MISC);
        RenderContainerCombo("##miscEmptySG", cs.misc_EmptySoulGem, cellData, enabled, COL_MISC);
        RenderSubLabel(TR("$Categories", "filledSoulGem", "Filled Soul Gem"), COL_MISC);
        RenderContainerCombo("##miscFilledSG", cs.misc_FilledSoulGem, cellData, enabled, COL_MISC);
        RenderSubLabel(TR("$Categories", "animalParts", "Animal Parts"), COL_MISC);
        RenderContainerCombo("##miscAnimalP", cs.misc_AnimalParts, cellData, enabled, COL_MISC);
        RenderSubLabel(TR("$Categories", "hidesPelts", "Hides / Pelts"), COL_MISC);
        RenderContainerCombo("##miscHides", cs.misc_HidesPelts, cellData, enabled, COL_MISC);
        RenderSubLabel(TR("$Categories", "miscValuables", "Misc. Valuables"), COL_MISC);
        RenderContainerCombo("##miscValuables", cs.misc_Valuables, cellData, enabled, COL_MISC);
        RenderSubLabel(TR("$Categories", "miscNonValuables", "Misc. Non-Valuables"), COL_MISC);
        RenderContainerCombo("##miscNonVal", cs.misc_NonValuables, cellData, enabled, COL_MISC);
        RenderSubLabel(TR("$Categories", "leather", "Leather"), COL_MISC);
        RenderContainerCombo("##miscLeather", cs.misc_Leather, cellData, enabled, COL_MISC);
        RenderSubLabel(TR("$Categories", "buildingMaterials", "Building Materials"), COL_MISC);
        RenderContainerCombo("##miscBuilding", cs.misc_BuildingMaterials, cellData, enabled, COL_MISC);
        ImGuiMCP::Unindent();
    }
}

void UI::HelpMarker(const char* desc)
{
    ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_Text, ImGuiMCP::ImVec4{ 0.55f, 0.55f, 0.55f, 1.0f });
    ImGuiMCP::TextUnformatted("(?)");
    ImGuiMCP::PopStyleColor();
    if (ImGuiMCP::IsItemHovered())
    {
        ImGuiMCP::BeginTooltip();
        ImGuiMCP::PushTextWrapPos(ImGuiMCP::GetFontSize() * 28.0f);
        ImGuiMCP::TextUnformatted(desc);
        ImGuiMCP::PopTextWrapPos();
        ImGuiMCP::EndTooltip();
    }
}

void UI::Register()
{
    if (!SKSEMenuFramework::IsInstalled())
    {
        logger::warn("SKSE Menu Framework not found; install for in-game options");
        return;
    }

    SKSEMenuFramework::SetSection("Home Auto Sort");
    SKSEMenuFramework::AddSectionItem(TR("$Menu", "generalSettings", "General Settings"), RenderGeneral);
    SKSEMenuFramework::AddSectionItem(TR("$Menu", "resupply", "Resupply"), RenderResupply);
    SKSEMenuFramework::AddSectionItem(TR("$Menu", "cell1", "Cell 1"), RenderCell1);
    SKSEMenuFramework::AddSectionItem(TR("$Menu", "cell2", "Cell 2"), RenderCell2);
    SKSEMenuFramework::AddSectionItem(TR("$Menu", "cell3", "Cell 3"), RenderCell3);
    SKSEMenuFramework::AddSectionItem(TR("$Menu", "cell4", "Cell 4"), RenderCell4);
    SKSEMenuFramework::AddSectionItem(TR("$Menu", "cell5", "Cell 5"), RenderCell5);
}

static void RenderBlacklistSection(bool& a_changed);

void __stdcall UI::RenderGeneral()
{
    auto& s = Settings::GetSingleton();
    bool changed = false;
    float winWidth = ImGuiMCP::GetWindowWidth();

    ImGuiMCP::TextColored(ImGuiMCP::ImVec4(0.7f, 0.85f, 1.0f, 1.0f), TR("$GeneralUI", "title", "General Settings"));

    ImGuiMCP::Separator();
    ImGuiMCP::TextUnformatted(TR("$GeneralUI", "debugSection", "-- Debug --"));
    ImGuiMCP::SameLine(); HelpMarker(TR("$GeneralUI", "debugHelp", "Enable detailed logging for troubleshooting."));
    ImGuiMCP::Separator();

    bool debug = s.debug.load();
    if (ImGuiMCP::Checkbox(TR("$GeneralUI", "enableDebug", "Enable Debug Logging"), &debug)) { s.debug.store(debug); changed = true; }

    ImGuiMCP::Spacing();
    ImGuiMCP::Separator();
    ImGuiMCP::TextUnformatted(TR("$GeneralUI", "hotkeysSection", "-- Hotkeys --"));
    ImGuiMCP::SameLine(); HelpMarker(TR("$GeneralUI", "hotkeysHelp", "Set separate keys for stash and resupply. Hold for 500ms to activate."));
    ImGuiMCP::Separator();

    ImGuiMCP::SetNextItemWidth(winWidth * 0.45f);
    {
        int idx = FindComboIndex(dxKbValues, kbComboCount, s.keyboardStashHotkey.load());
        if (ImGuiMCP::Combo(TR("$GeneralUI", "keyboardStash", "Keyboard Stash Hotkey"), &idx, dxKbNames, kbComboCount))
        {
            s.keyboardStashHotkey.store(dxKbValues[idx]);
            changed = true;
        }
    }

    ImGuiMCP::SetNextItemWidth(winWidth * 0.45f);
    {
        int idx = FindComboIndex(dxKbValues, kbComboCount, s.keyboardResupplyHotkey.load());
        if (ImGuiMCP::Combo(TR("$GeneralUI", "keyboardResupply", "Keyboard Resupply Hotkey"), &idx, dxKbNames, kbComboCount))
        {
            s.keyboardResupplyHotkey.store(dxKbValues[idx]);
            changed = true;
        }
    }

    ImGuiMCP::SetNextItemWidth(winWidth * 0.45f);
    {
        int idx = FindComboIndex(dxGpValues, gpComboCount, s.gamepadStashHotkey.load());
        if (ImGuiMCP::Combo(TR("$GeneralUI", "gamepadStash", "Gamepad Stash Hotkey"), &idx, dxGpNames, gpComboCount))
        {
            s.gamepadStashHotkey.store(dxGpValues[idx]);
            changed = true;
        }
    }

    ImGuiMCP::SetNextItemWidth(winWidth * 0.45f);
    {
        int idx = FindComboIndex(dxGpValues, gpComboCount, s.gamepadResupplyHotkey.load());
        if (ImGuiMCP::Combo(TR("$GeneralUI", "gamepadResupply", "Gamepad Resupply Hotkey"), &idx, dxGpNames, gpComboCount))
        {
            s.gamepadResupplyHotkey.store(dxGpValues[idx]);
            changed = true;
        }
    }

    ImGuiMCP::Spacing();
    ImGuiMCP::Separator();
    ImGuiMCP::TextUnformatted(TR("$GeneralUI", "compatSection", "-- Compatibility --"));
    ImGuiMCP::SameLine(); HelpMarker(TR("$GeneralUI", "compatHelp", "Enable support for mods with extra item identifiers."));
    ImGuiMCP::Separator();

    bool l5 = s.lorerim5Compat.load();
    if (ImGuiMCP::Checkbox(TR("$GeneralUI", "lorerim5", "Lorerim 5 Compatiblity"), &l5))
    {
        s.lorerim5Compat.store(l5);
        changed = true;
    }
    ImGuiMCP::SameLine(); UI::HelpMarker(TR("$GeneralUI", "lorerim5Help", "Lorerim 5 adds extra identifiers that can be used to have more specific categorization."));

    ImGuiMCP::Spacing();
    ImGuiMCP::Separator();
    ImGuiMCP::TextUnformatted(TR("$GeneralUI", "resupplyPrioritySection", "-- Resupply Priority --"));
    ImGuiMCP::SameLine(); HelpMarker(TR("$GeneralUI", "resupplyPriorityHelp", "When enabled, the strongest (or most filling/hydrating) items are given first during resupply. When disabled, the weakest items are given first."));
    ImGuiMCP::Separator();

    bool b = s.prioritizeStrongestHealing.load();
    if (ImGuiMCP::Checkbox(TR("$GeneralUI", "priorityHealing", "Prioritize strongest healing potion"), &b)) { s.prioritizeStrongestHealing.store(b); changed = true; }
    ImGuiMCP::SameLine(); UI::HelpMarker(TR("$GeneralUI", "priorityHealingHelp", "When checked, resupply gives the highest magnitude healing potions first."));

    b = s.prioritizeStrongestMagicka.load();
    if (ImGuiMCP::Checkbox(TR("$GeneralUI", "priorityMagicka", "Prioritize strongest magicka restoration potion"), &b)) { s.prioritizeStrongestMagicka.store(b); changed = true; }
    ImGuiMCP::SameLine(); UI::HelpMarker(TR("$GeneralUI", "priorityMagickaHelp", "When checked, resupply gives the highest magnitude magicka potions first."));

    b = s.prioritizeStrongestStamina.load();
    if (ImGuiMCP::Checkbox(TR("$GeneralUI", "priorityStamina", "Prioritize strongest stamina restoration potion"), &b)) { s.prioritizeStrongestStamina.store(b); changed = true; }
    ImGuiMCP::SameLine(); UI::HelpMarker(TR("$GeneralUI", "priorityStaminaHelp", "When checked, resupply gives the highest magnitude stamina potions first."));

    b = s.prioritizeStrongestArrow.load();
    if (ImGuiMCP::Checkbox(TR("$GeneralUI", "priorityArrow", "Prioritize strongest arrow"), &b)) { s.prioritizeStrongestArrow.store(b); changed = true; }
    ImGuiMCP::SameLine(); UI::HelpMarker(TR("$GeneralUI", "priorityArrowHelp", "When checked, resupply gives the highest damage arrows first."));

    b = s.prioritizeStrongestBolt.load();
    if (ImGuiMCP::Checkbox(TR("$GeneralUI", "priorityBolt", "Prioritize strongest bolt"), &b)) { s.prioritizeStrongestBolt.store(b); changed = true; }
    ImGuiMCP::SameLine(); UI::HelpMarker(TR("$GeneralUI", "priorityBoltHelp", "When checked, resupply gives the highest damage bolts first."));

    ImGuiMCP::BeginDisabled(!l5);
    b = s.prioritizeFillingFood.load();
    if (ImGuiMCP::Checkbox(TR("$GeneralUI", "priorityFood", "Prioritize most filling food item - Lorerim 5 only"), &b)) { s.prioritizeFillingFood.store(b); changed = true; }
    ImGuiMCP::SameLine(); UI::HelpMarker(TR("$GeneralUI", "priorityFoodHelp", "When checked, resupply gives the most filling food items first. Requires Lorerim 5."));

    b = s.prioritizeHydratingDrink.load();
    if (ImGuiMCP::Checkbox(TR("$GeneralUI", "priorityDrink", "Prioritize most hydrating drink item - Lorerim 5 only"), &b)) { s.prioritizeHydratingDrink.store(b); changed = true; }
    ImGuiMCP::SameLine(); UI::HelpMarker(TR("$GeneralUI", "priorityDrinkHelp", "When checked, resupply gives the most hydrating drink items first. Requires Lorerim 5."));

    b = s.allowAlcohol.load();
    if (ImGuiMCP::Checkbox(TR("$GeneralUI", "allowAlcohol", "Allow alcohol as drink - Lorerim 5 only"), &b)) { s.allowAlcohol.store(b); changed = true; }
    ImGuiMCP::SameLine(); UI::HelpMarker(TR("$GeneralUI", "allowAlcoholHelp", "When enabled, alcohol will be used for drink resupply if no non-alcoholic drinks are available. Requires Lorerim 5."));
    ImGuiMCP::EndDisabled();

    ImGuiMCP::Spacing();
    ImGuiMCP::Separator();
    ImGuiMCP::TextUnformatted(TR("$GeneralUI", "stashSection", "-- Stash --"));
    ImGuiMCP::SameLine(); HelpMarker(TR("$GeneralUI", "stashHelp", "Settings for item stashing behavior."));
    ImGuiMCP::Separator();

    b = s.stashUnmatchedToMaster.load();
    if (ImGuiMCP::Checkbox(TR("$GeneralUI", "stashUnmatched", "Stash items that don\'t fit any category into the Master Chest"), &b)) { s.stashUnmatchedToMaster.store(b); changed = true; }
    ImGuiMCP::SameLine(); UI::HelpMarker(TR("$GeneralUI", "stashUnmatchedHelp", "When checked, items that don\'t match any category are placed into the Master Chest."));

    ImGuiMCP::Spacing();
    int minVal = s.minimumValueValuable.load();
    ImGuiMCP::SetNextItemWidth(winWidth * 0.45f);
    if (ImGuiMCP::SliderInt(TR("$GeneralUI", "minimumValuable", "Minimum value to be considered a Valuable"), &minVal, 1, 5000))
    {
        s.minimumValueValuable.store(minVal);
        changed = true;
    }
    ImGuiMCP::SameLine(); UI::HelpMarker(TR("$GeneralUI", "minimumValuableHelp", "Items in the Misc category worth this many gold or more are classified as Valuables."));

    ImGuiMCP::Spacing();
    ImGuiMCP::Separator();
    ImGuiMCP::TextUnformatted(TR("$GeneralUI", "blacklistSection", "-- Blacklist --"));
    ImGuiMCP::SameLine(); UI::HelpMarker(TR("$GeneralUI", "blacklistHelp", "Items matching these terms will never be stashed from your inventory."));
    ImGuiMCP::Separator();

    RenderBlacklistSection(changed);

    if (changed) s.Save();
}

void __stdcall UI::RenderResupply()
{
    auto& s = Settings::GetSingleton();
    bool changed = false;
    float winWidth = ImGuiMCP::GetWindowWidth();
    bool l5 = s.lorerim5Compat.load();

    ImGuiMCP::TextColored(ImGuiMCP::ImVec4(0.7f, 0.85f, 1.0f, 1.0f), TR("$ResupplyUI", "title", "Resupply Settings"));
    ImGuiMCP::TextWrapped(TR("$ResupplyUI", "description", "Choose the items and amount to give the player during resupply from storage."));

    ImGuiMCP::Separator();
    ImGuiMCP::Spacing();

    auto renderSlider = [&](const char* label, std::atomic<int>& value, int max, const char* help) {
        int v = value.load();
        ImGuiMCP::SetNextItemWidth(winWidth * 0.45f);
        if (ImGuiMCP::SliderInt(label, &v, 0, max))
        {
            value.store(v);
            changed = true;
        }
        ImGuiMCP::SameLine(); UI::HelpMarker(help);
    };

    renderSlider(TR("$ResupplyUI", "healingPotions", "Healing Potions"), s.resupplyHealingPotions, 100,
        TR("$ResupplyUI", "healingPotionsHelp", "Number of healing potions to give during resupply."));
    renderSlider(TR("$ResupplyUI", "magickaPotions", "Magicka Potions"), s.resupplyMagickaPotions, 100,
        TR("$ResupplyUI", "magickaPotionsHelp", "Number of magicka potions to give during resupply."));
    renderSlider(TR("$ResupplyUI", "staminaPotions", "Stamina Potions"), s.resupplyStaminaPotions, 100,
        TR("$ResupplyUI", "staminaPotionsHelp", "Number of stamina potions to give during resupply."));
    renderSlider(TR("$ResupplyUI", "curePoison", "Cure Poison Potions"), s.resupplyCurePoison, 100,
        TR("$ResupplyUI", "curePoisonHelp", "Number of cure poison potions to give during resupply."));
    renderSlider(TR("$ResupplyUI", "cureDisease", "Cure Disease Potions"), s.resupplyCureDisease, 100,
        TR("$ResupplyUI", "cureDiseaseHelp", "Number of cure disease potions to give during resupply."));
    renderSlider(TR("$Categories", "arrows", "Arrows"), s.resupplyArrows, 1000,
        TR("$ResupplyUI", "arrowsHelp", "Number of arrows to give during resupply."));
    renderSlider(TR("$Categories", "bolts", "Bolts"), s.resupplyBolts, 1000,
        TR("$ResupplyUI", "boltsHelp", "Number of bolts to give during resupply."));
    renderSlider(TR("$ResupplyUI", "lockpicks", "Lockpicks"), s.resupplyLockpicks, 500,
        TR("$ResupplyUI", "lockpicksHelp", "Number of lockpicks to give during resupply."));

    renderSlider(TR("$ResupplyUI", "food", "Food"), s.resupplyCookedFood, 100,
        TR("$ResupplyUI", "foodHelp", "Number of food items to give during resupply. Cooked food is used first, then raw if cooked runs out."));

    ImGuiMCP::BeginDisabled(!l5);
    renderSlider(TR("$ResupplyUI", "foodLorerim", "Food - Lorerim 5 only"), s.resupplyFood, 100,
        TR("$ResupplyUI", "foodLorerimHelp", "Number of food items to give during resupply. Requires Lorerim 5."));
    renderSlider(TR("$ResupplyUI", "drinksLorerim", "Drinks - Lorerim 5 only"), s.resupplyDrink, 100,
        TR("$ResupplyUI", "drinksLorerimHelp", "Number of drink items to give during resupply. Requires Lorerim 5."));
    ImGuiMCP::EndDisabled();

    ImGuiMCP::Spacing();
    ImGuiMCP::Separator();
    ImGuiMCP::TextColored(ImGuiMCP::ImVec4(0.9f, 0.9f, 0.6f, 1.0f), TR("$ResupplyUI", "customItems", "Custom Items"));
    ImGuiMCP::SameLine(); UI::HelpMarker(TR("$ResupplyUI", "customItemsHelp", "Select specific items to give during resupply."));

    for (int i = 0; i < 8; ++i)
    {
        ImGuiMCP::PushID(300 + i);

        char label[64];
        snprintf(label, sizeof(label), TR("$ResupplyUI", "customItemFormat", "Custom Item %d"), i + 1);

        if (ImGuiMCP::TreeNode(label))
        {
            ImGuiMCP::SetNextItemWidth(winWidth * 0.4f);
            char filterBuf[256];
            strncpy_s(filterBuf, s.resupplyCustom[i].filterText.c_str(), sizeof(filterBuf) - 1);
            filterBuf[sizeof(filterBuf) - 1] = '\0';

            if (ImGuiMCP::InputTextWithHint("##customFilter", TR("$ResupplyUI", "filterItems", "Filter items..."), filterBuf, sizeof(filterBuf)))
            {
                s.resupplyCustom[i].filterText = filterBuf;
                changed = true;
            }

            const auto& cache = FormCache::GetAllItems();
            std::string filterStr(filterBuf);

            static int selectedCustomIdx[8] = { -1, -1, -1, -1, -1, -1, -1, -1 };
            static std::string prevCustomFilter[8];

            if (filterStr != prevCustomFilter[i])
            {
                prevCustomFilter[i] = filterStr;
                selectedCustomIdx[i] = -1;
            }

            struct FilteredEntry { int cacheIndex; std::string label; };
            std::vector<FilteredEntry> filtered;
            for (size_t j = 0; j < cache.size(); ++j)
            {
                if (ContainsIgnoreCase(cache[j].name, filterStr) ||
                    (!cache[j].editorID.empty() && ContainsIgnoreCase(cache[j].editorID, filterStr)))
                {
                    char buf[384];
                    snprintf(buf, sizeof(buf), "%s [0x%08X]", cache[j].name.c_str(), cache[j].formID);
                    filtered.push_back({ static_cast<int>(j), buf });
                }
            }

            const char* preview = TR("$ResupplyUI", "selectItem", "Select item...");
            if (selectedCustomIdx[i] >= 0 && static_cast<size_t>(selectedCustomIdx[i]) < filtered.size())
                preview = filtered[selectedCustomIdx[i]].label.c_str();
            else if (!s.resupplyCustom[i].selectedEditorID.empty())
            {
                for (size_t j = 0; j < cache.size(); ++j)
                {
                    if (cache[j].editorID == s.resupplyCustom[i].selectedEditorID ||
                        std::format("{:08X}", cache[j].formID) == s.resupplyCustom[i].selectedEditorID)
                    {
                        selectedCustomIdx[i] = static_cast<int>(j);
                        break;
                    }
                }
            }

            if (ImGuiMCP::BeginCombo("##customDropdown", preview))
            {
                if (ImGuiMCP::Selectable(TR("$UI", "none", "[None]"), selectedCustomIdx[i] < 0))
                {
                    selectedCustomIdx[i] = -1;
                    s.resupplyCustom[i].selectedEditorID.clear();
                    changed = true;
                }
                if (selectedCustomIdx[i] < 0) ImGuiMCP::SetItemDefaultFocus();

                for (size_t j = 0; j < filtered.size(); ++j)
                {
                    bool isSel = (static_cast<int>(j) == selectedCustomIdx[i]);
                    if (ImGuiMCP::Selectable(filtered[j].label.c_str(), isSel))
                    {
                        selectedCustomIdx[i] = static_cast<int>(j);
                        auto& entry = cache[filtered[j].cacheIndex];
                        if (!entry.editorID.empty())
                            s.resupplyCustom[i].selectedEditorID = entry.editorID;
                        else
                            s.resupplyCustom[i].selectedEditorID = std::format("{:08X}", entry.formID);
                        changed = true;
                    }
                    if (isSel) ImGuiMCP::SetItemDefaultFocus();
                }
                ImGuiMCP::EndCombo();
            }

            int countVal = s.resupplyCustom[i].count;
            ImGuiMCP::SetNextItemWidth(winWidth * 0.3f);
            if (ImGuiMCP::SliderInt("##customCount", &countVal, 0, 100))
            {
                s.resupplyCustom[i].count = countVal;
                changed = true;
            }

            ImGuiMCP::TreePop();
        }
        ImGuiMCP::PopID();
    }

    if (changed) s.Save();
}

    void RenderBlacklistSection(bool& a_changed)
    {
        auto& bl = Settings::GetSingleton().blacklist;

        static char inputBuf[256] = "";
        ImGuiMCP::SetNextItemWidth(ImGuiMCP::GetWindowWidth() * 0.55f);
        ImGuiMCP::InputTextWithHint("##blacklistInput", TR("$GeneralUI", "blacklistInput", "Item name or EditorID..."), inputBuf, sizeof(inputBuf));
        ImGuiMCP::SameLine();
        if (ImGuiMCP::Button(TR("$GeneralUI", "add", "Add"))) {
            std::string trimmed = inputBuf;
            trimmed.erase(0, trimmed.find_first_not_of(" \t"));
            trimmed.erase(trimmed.find_last_not_of(" \t") + 1);
            if (!trimmed.empty() && trimmed.find(',') == std::string::npos) {
                bool exists = false;
                for (auto& e : bl) {
                    if (_stricmp(e.c_str(), trimmed.c_str()) == 0) { exists = true; break; }
                }
                if (!exists) {
                    bl.push_back(trimmed);
                    a_changed = true;
                }
                memset(inputBuf, 0, sizeof(inputBuf));
            }
        }
        ImGuiMCP::SameLine();
        UI::HelpMarker(TR("$GeneralUI", "blacklistInputHelp", "Type part of an item\'s name or EditorID (case-insensitive partial match). Items matching any term here will never be stashed."));

        if (!bl.empty()) {
            ImGuiMCP::Spacing();
            ImGuiMCP::TextUnformatted(TR("$GeneralUI", "blacklistedItems", "Blacklisted items:"));
            std::string toRemove;
            for (auto& item : bl) {
                ImGuiMCP::BulletText("%s", item.c_str());
                ImGuiMCP::SameLine();
                ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_Button, ImGuiMCP::ImVec4(0.6f, 0.1f, 0.1f, 1.0f));
                ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_ButtonHovered, ImGuiMCP::ImVec4(0.8f, 0.1f, 0.1f, 1.0f));
                ImGuiMCP::PushID(item.c_str());
                if (ImGuiMCP::SmallButton("X")) {
                    toRemove = item;
                }
                ImGuiMCP::PopID();
                ImGuiMCP::PopStyleColor(2);
            }
            if (!toRemove.empty()) {
                auto it = std::find(bl.begin(), bl.end(), toRemove);
                if (it != bl.end()) bl.erase(it);
                a_changed = true;
            }
        }
    }

void __stdcall UI::RenderCell1() { RenderCellContent(0, TR("$Menu", "cell1", "Cell 1")); }
void __stdcall UI::RenderCell2() { RenderCellContent(1, TR("$Menu", "cell2", "Cell 2")); }
void __stdcall UI::RenderCell3() { RenderCellContent(2, TR("$Menu", "cell3", "Cell 3")); }
void __stdcall UI::RenderCell4() { RenderCellContent(3, TR("$Menu", "cell4", "Cell 4")); }
void __stdcall UI::RenderCell5() { RenderCellContent(4, TR("$Menu", "cell5", "Cell 5")); }
