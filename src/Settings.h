#pragma once

#include "plugin.h"

struct CellSettings
{
    std::string homeEditorID;
    uint32_t    homeFormID = 0;
    bool        homeSet = false;

    std::string masterChestEditorID;

    std::string weapons;
    std::string weapons_OneHanded;
    std::string weapons_TwoHanded;
    std::string weapons_Archery;
    std::string weapons_Staves;

    std::string armor;
    std::string armor_Light;
    std::string armor_Heavy;
    std::string armor_Clothing;
    std::string armor_Shield;

    std::string jewelry;
    std::string jewelry_Rings;
    std::string jewelry_Amulets;
    std::string jewelry_Circlets;

    std::string potions;
    std::string poisons;
    std::string scrolls;

    std::string consumables;
    std::string consumables_Alcohol;
    std::string consumables_NonAlcoholicDrinks;
    std::string consumables_RawMeat;
    std::string consumables_CookedMeat;
    std::string consumables_ProduceGrains;
    std::string consumables_Cheese;
    std::string consumables_Soups;

    std::string consumables_Raw;
    std::string consumables_Cooked;
    std::string consumables_Drinks;

    std::string ingredients;

    std::string writtenWorks;
    std::string writtenWorks_Books;
    std::string writtenWorks_Notes;
    std::string writtenWorks_SkillBooks;
    std::string writtenWorks_SpellBooks;

    std::string keys;

    std::string ammo;
    std::string ammo_Arrows;
    std::string ammo_Bolts;

    std::string misc;
    std::string misc_Ore;
    std::string misc_Ingot;
    std::string misc_Gem;
    std::string misc_EmptySoulGem;
    std::string misc_FilledSoulGem;
    std::string misc_AnimalParts;
    std::string misc_HidesPelts;
    std::string misc_Valuables;
    std::string misc_NonValuables;
    std::string misc_Leather;
    std::string misc_BuildingMaterials;
};

struct ResupplyCustomItem
{
    std::string filterText;
    std::string selectedEditorID;
    int         count = 0;
};

class Settings
{
public:
    static Settings& GetSingleton();

    Settings(const Settings&)            = delete;
    Settings& operator=(const Settings&) = delete;

    void Load();
    void Save();
    void ApplyLogLevel() const;
    void Clamp();

    std::atomic<bool>     debug{ false };
    std::atomic<uint32_t> keyboardStashHotkey{ 20 };
    std::atomic<uint32_t> gamepadStashHotkey{ 278 };
    std::atomic<uint32_t> keyboardResupplyHotkey{ 34 };
    std::atomic<uint32_t> gamepadResupplyHotkey{ 279 };
    std::atomic<bool>     lorerim5Compat{ false };
    std::atomic<int>      perTickStash{ 5 };
    std::atomic<int>      perTickResupply{ 5 };

    std::atomic<bool>     prioritizeStrongestHealing{ true };
    std::atomic<bool>     prioritizeStrongestMagicka{ true };
    std::atomic<bool>     prioritizeStrongestStamina{ true };
    std::atomic<bool>     prioritizeStrongestArrow{ true };
    std::atomic<bool>     prioritizeStrongestBolt{ true };
    std::atomic<bool>     prioritizeFillingFood{ false };
    std::atomic<bool>     prioritizeHydratingDrink{ false };
    std::atomic<bool>     allowAlcohol{ false };

    std::atomic<bool>     stashUnmatchedToMaster{ true };
    std::atomic<int>      minimumValueValuable{ 500 };

    std::atomic<int>      resupplyHealingPotions{ 10 };
    std::atomic<int>      resupplyMagickaPotions{ 5 };
    std::atomic<int>      resupplyStaminaPotions{ 5 };
    std::atomic<int>      resupplyCurePoison{ 5 };
    std::atomic<int>      resupplyCureDisease{ 2 };
    std::atomic<int>      resupplyArrows{ 500 };
    std::atomic<int>      resupplyBolts{ 500 };
    std::atomic<int>      resupplyLockpicks{ 50 };
    std::atomic<int>      resupplyFood{ 0 };
    std::atomic<int>      resupplyDrink{ 0 };
    std::atomic<int>      resupplyCookedFood{ 10 };

    ResupplyCustomItem    resupplyCustom[8];

    CellSettings          cells[5];

    std::vector<std::string> blacklist;
    std::vector<std::string> blacklistLower;

    void RebuildBlacklistLower();

    static void LoadPresetCache();
    static CellSettings* FindPresetInCache(const std::string& a_cellEditorID);
    static const std::unordered_map<std::string, CellSettings>& GetAllCachedPresets();
    static void SetActivePreset(int cellIndex, const CellSettings& a_preset);
    static void ClearActivePreset(int cellIndex);
    static const CellSettings* GetActivePreset(int cellIndex);

private:
    Settings() = default;

    void LoadCell(CSimpleIniA& ini, const char* section, CellSettings& cell);
    void SaveCell(CSimpleIniA& ini, const char* section, const CellSettings& cell);
    void LoadCustomItem(CSimpleIniA& ini, const char* section, int index, ResupplyCustomItem& item);
    void SaveCustomItem(CSimpleIniA& ini, const char* section, int index, const ResupplyCustomItem& item);
    std::string SaveStringList(const std::vector<std::string>& a_list) const;
    void LoadStringList(const std::string& a_value, std::vector<std::string>& a_out);

public:
    static void ExportPreset(int cellIndex);
};
