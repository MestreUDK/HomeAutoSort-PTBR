#include "pch.h"
#include "Settings.h"
#include "CellManager.h"

#include <cctype>
#include <cstdio>
#include <filesystem>

namespace fs = std::filesystem;

static std::unordered_map<std::string, CellSettings> g_presetCache;
static CellSettings g_activePresets[5];
static bool g_hasActivePreset[5]{ false, false, false, false, false };

void Settings::SetActivePreset(int cellIndex, const CellSettings& a_preset)
{
    if (cellIndex < 0 || cellIndex >= 5) return;
    g_activePresets[cellIndex] = a_preset;
    g_hasActivePreset[cellIndex] = true;
}

void Settings::ClearActivePreset(int cellIndex)
{
    if (cellIndex < 0 || cellIndex >= 5) return;
    g_hasActivePreset[cellIndex] = false;
    g_activePresets[cellIndex] = CellSettings{};
}

const CellSettings* Settings::GetActivePreset(int cellIndex)
{
    if (cellIndex < 0 || cellIndex >= 5 || !g_hasActivePreset[cellIndex]) return nullptr;
    auto& preset = g_activePresets[cellIndex];
    if (preset.homeEditorID.empty()) return nullptr;
    for (int i = 0; i < 5; ++i)
    {
        if (GetSingleton().cells[i].homeSet)
        {
            if (GetSingleton().cells[i].homeEditorID == preset.homeEditorID)
            {
                if (GetSingleton().debug.load())
                    logger::info("GetActivePreset: blocked cell {} preset '{}' by cell {} homeSet '{}'", cellIndex + 1, preset.homeEditorID, i + 1, GetSingleton().cells[i].homeEditorID);
                return nullptr;
            }
        }
    }
    return &g_activePresets[cellIndex];
}

static bool IsBareHexKey(const std::string& key)
{
    if (key.empty()) return false;
    if (key.find('|') != std::string::npos) return false;
    if (key.size() != 6 && key.size() != 8) return false;
    for (char c : key) {
        if (!std::isxdigit(static_cast<unsigned char>(c))) return false;
    }
    return true;
}

static void WipeBareHexKeys(CellSettings& cs)
{
    auto wipe = [](std::string& key) {
        if (IsBareHexKey(key)) key.clear();
    };
    wipe(cs.masterChestEditorID);
    wipe(cs.weapons);
    wipe(cs.weapons_OneHanded);
    wipe(cs.weapons_TwoHanded);
    wipe(cs.weapons_Archery);
    wipe(cs.weapons_Staves);
    wipe(cs.armor);
    wipe(cs.armor_Light);
    wipe(cs.armor_Heavy);
    wipe(cs.armor_Clothing);
    wipe(cs.armor_Shield);
    wipe(cs.jewelry);
    wipe(cs.jewelry_Rings);
    wipe(cs.jewelry_Amulets);
    wipe(cs.jewelry_Circlets);
    wipe(cs.potions);
    wipe(cs.poisons);
    wipe(cs.scrolls);
    wipe(cs.consumables);
    wipe(cs.consumables_Raw);
    wipe(cs.consumables_Cooked);
    wipe(cs.consumables_Drinks);
    wipe(cs.consumables_Alcohol);
    wipe(cs.consumables_NonAlcoholicDrinks);
    wipe(cs.consumables_RawMeat);
    wipe(cs.consumables_CookedMeat);
    wipe(cs.consumables_ProduceGrains);
    wipe(cs.consumables_Cheese);
    wipe(cs.consumables_Soups);
    wipe(cs.ingredients);
    wipe(cs.writtenWorks);
    wipe(cs.writtenWorks_Books);
    wipe(cs.writtenWorks_Notes);
    wipe(cs.writtenWorks_SkillBooks);
    wipe(cs.writtenWorks_SpellBooks);
    wipe(cs.keys);
    wipe(cs.ammo);
    wipe(cs.ammo_Arrows);
    wipe(cs.ammo_Bolts);
    wipe(cs.misc);
    wipe(cs.misc_Ore);
    wipe(cs.misc_Ingot);
    wipe(cs.misc_Gem);
    wipe(cs.misc_EmptySoulGem);
    wipe(cs.misc_FilledSoulGem);
    wipe(cs.misc_AnimalParts);
    wipe(cs.misc_HidesPelts);
    wipe(cs.misc_Valuables);
    wipe(cs.misc_NonValuables);
    wipe(cs.misc_Leather);
    wipe(cs.misc_BuildingMaterials);
}

static CellSettings ParsePresetJson(const std::string& json)
{
    CellSettings cs;
    auto findStr = [&](const std::string& key) -> std::string {
        auto pos = json.find("\"" + key + "\"");
        if (pos == std::string::npos) return "";
        auto colon = json.find(':', pos);
        if (colon == std::string::npos) return "";
        auto start = json.find('"', colon);
        if (start == std::string::npos) return "";
        start++;
        auto end = json.find('"', start);
        if (end == std::string::npos) return "";
        return json.substr(start, end - start);
    };

    cs.homeEditorID = findStr("cellEditorID");
    cs.masterChestEditorID = findStr("masterChestEditorID");
    cs.weapons = findStr("weapons");
    cs.weapons_OneHanded = findStr("weapons_OneHanded");
    cs.weapons_TwoHanded = findStr("weapons_TwoHanded");
    cs.weapons_Archery = findStr("weapons_Archery");
    cs.weapons_Staves = findStr("weapons_Staves");
    cs.armor = findStr("armor");
    cs.armor_Light = findStr("armor_Light");
    cs.armor_Heavy = findStr("armor_Heavy");
    cs.armor_Clothing = findStr("armor_Clothing");
    cs.armor_Shield = findStr("armor_Shield");
    cs.jewelry = findStr("jewelry");
    cs.jewelry_Rings = findStr("jewelry_Rings");
    cs.jewelry_Amulets = findStr("jewelry_Amulets");
    cs.jewelry_Circlets = findStr("jewelry_Circlets");
    cs.potions = findStr("potions");
    cs.poisons = findStr("poisons");
    cs.scrolls = findStr("scrolls");
    cs.consumables = findStr("consumables");
    cs.consumables_Raw = findStr("consumables_Raw");
    cs.consumables_Cooked = findStr("consumables_Cooked");
    cs.consumables_Drinks = findStr("consumables_Drinks");
    cs.consumables_Alcohol = findStr("consumables_Alcohol");
    cs.consumables_NonAlcoholicDrinks = findStr("consumables_NonAlcoholicDrinks");
    cs.consumables_RawMeat = findStr("consumables_RawMeat");
    cs.consumables_CookedMeat = findStr("consumables_CookedMeat");
    cs.consumables_ProduceGrains = findStr("consumables_ProduceGrains");
    cs.consumables_Cheese = findStr("consumables_Cheese");
    cs.consumables_Soups = findStr("consumables_Soups");
    cs.ingredients = findStr("ingredients");
    cs.writtenWorks = findStr("writtenWorks");
    cs.writtenWorks_Books = findStr("writtenWorks_Books");
    cs.writtenWorks_Notes = findStr("writtenWorks_Notes");
    cs.writtenWorks_SkillBooks = findStr("writtenWorks_SkillBooks");
    cs.writtenWorks_SpellBooks = findStr("writtenWorks_SpellBooks");
    cs.keys = findStr("keys");
    cs.ammo = findStr("ammo");
    cs.ammo_Arrows = findStr("ammo_Arrows");
    cs.ammo_Bolts = findStr("ammo_Bolts");
    cs.misc = findStr("misc");
    cs.misc_Ore = findStr("misc_Ore");
    cs.misc_Ingot = findStr("misc_Ingot");
    cs.misc_Gem = findStr("misc_Gem");
    cs.misc_EmptySoulGem = findStr("misc_EmptySoulGem");
    cs.misc_FilledSoulGem = findStr("misc_FilledSoulGem");
    cs.misc_AnimalParts = findStr("misc_AnimalParts");
    cs.misc_HidesPelts = findStr("misc_HidesPelts");
    cs.misc_Valuables = findStr("misc_Valuables");
    cs.misc_NonValuables = findStr("misc_NonValuables");
    cs.misc_Leather = findStr("misc_Leather");
    cs.misc_BuildingMaterials = findStr("misc_BuildingMaterials");
    WipeBareHexKeys(cs);
    return cs;
}

void Settings::LoadPresetCache()
{
    g_presetCache.clear();
    fs::path dir = "Data/SKSE/Plugins";
    if (!fs::exists(dir)) return;
    for (auto& entry : fs::directory_iterator(dir))
    {
        auto filename = entry.path().filename().string();
        if (filename.size() < 9) continue;
        if (filename.substr(filename.size() - 9) != "_HAS.json") continue;

        FILE* f = nullptr;
        if (_wfopen_s(&f, entry.path().wstring().c_str(), L"r") != 0 || !f) continue;
        fseek(f, 0, SEEK_END);
        long len = ftell(f);
        fseek(f, 0, SEEK_SET);
        std::string json(len, '\0');
        fread(json.data(), 1, len, f);
        fclose(f);

        auto cs = ParsePresetJson(json);
        if (Settings::GetSingleton().debug.load())
            logger::info("Settings: Parsed preset {}, homeEditorID='{}'", filename, cs.homeEditorID);
        if (!cs.homeEditorID.empty())
        {
            std::string key = cs.homeEditorID;
            g_presetCache[key] = cs;
            logger::info("Settings: Cached preset {} for cell {}", filename, key);
        }
    }
    logger::info("Settings: Loaded {} presets from cache", g_presetCache.size());
}

CellSettings* Settings::FindPresetInCache(const std::string& a_cellEditorID)
{
    auto it = g_presetCache.find(a_cellEditorID);
    return it != g_presetCache.end() ? &it->second : nullptr;
}

const std::unordered_map<std::string, CellSettings>& Settings::GetAllCachedPresets()
{
    return g_presetCache;
}

Settings& Settings::GetSingleton()
{
    static Settings instance;
    return instance;
}

void Settings::Load()
{
    constexpr auto path = L"Data/SKSE/Plugins/HomeAutoSort.ini";

    try
    {
        CSimpleIniA ini;
        ini.SetUnicode();
        ini.LoadFile(path);

    debug.store(ini.GetBoolValue("General", "debug", false));
    {
        uint32_t kb = static_cast<uint32_t>(ini.GetLongValue("General", "keyboardStashHotkey", 0));
        if (kb == 0) kb = static_cast<uint32_t>(ini.GetLongValue("General", "keyboardHotkey", 20));
        keyboardStashHotkey.store(kb);
    }
    {
        uint32_t gp = static_cast<uint32_t>(ini.GetLongValue("General", "gamepadStashHotkey", 0));
        if (gp == 0) gp = static_cast<uint32_t>(ini.GetLongValue("General", "gamepadHotkey", 278));
        gamepadStashHotkey.store(gp);
    }
    keyboardResupplyHotkey.store(static_cast<uint32_t>(ini.GetLongValue("General", "keyboardResupplyHotkey", 34)));
    gamepadResupplyHotkey.store(static_cast<uint32_t>(ini.GetLongValue("General", "gamepadResupplyHotkey", 279)));
    lorerim5Compat.store(ini.GetBoolValue("General", "lorerim5Compat", false));
    perTickStash.store(static_cast<int>(ini.GetLongValue("General", "perTickStash", 5)));
    perTickResupply.store(static_cast<int>(ini.GetLongValue("General", "perTickResupply", 5)));

    prioritizeStrongestHealing.store(ini.GetBoolValue("General", "prioritizeStrongestHealing", true));
    prioritizeStrongestMagicka.store(ini.GetBoolValue("General", "prioritizeStrongestMagicka", true));
    prioritizeStrongestStamina.store(ini.GetBoolValue("General", "prioritizeStrongestStamina", true));
    prioritizeStrongestArrow.store(ini.GetBoolValue("General", "prioritizeStrongestArrow", true));
    prioritizeStrongestBolt.store(ini.GetBoolValue("General", "prioritizeStrongestBolt", true));
    prioritizeFillingFood.store(ini.GetBoolValue("General", "prioritizeFillingFood", false));
    prioritizeHydratingDrink.store(ini.GetBoolValue("General", "prioritizeHydratingDrink", false));
    allowAlcohol.store(ini.GetBoolValue("General", "allowAlcohol", false));

    stashUnmatchedToMaster.store(ini.GetBoolValue("General", "stashUnmatchedToMaster", true));
    minimumValueValuable.store(static_cast<int>(ini.GetLongValue("General", "minimumValueValuable", 500)));

    resupplyHealingPotions.store(static_cast<int>(ini.GetLongValue("Resupply", "healingPotions", 10)));
    resupplyMagickaPotions.store(static_cast<int>(ini.GetLongValue("Resupply", "magickaPotions", 5)));
    resupplyStaminaPotions.store(static_cast<int>(ini.GetLongValue("Resupply", "staminaPotions", 5)));
    resupplyCurePoison.store(static_cast<int>(ini.GetLongValue("Resupply", "curePoison", 5)));
    resupplyCureDisease.store(static_cast<int>(ini.GetLongValue("Resupply", "cureDisease", 2)));
    resupplyArrows.store(static_cast<int>(ini.GetLongValue("Resupply", "arrows", 500)));
    resupplyBolts.store(static_cast<int>(ini.GetLongValue("Resupply", "bolts", 500)));
    resupplyLockpicks.store(static_cast<int>(ini.GetLongValue("Resupply", "lockpicks", 50)));
    resupplyFood.store(static_cast<int>(ini.GetLongValue("Resupply", "food", 0)));
    resupplyDrink.store(static_cast<int>(ini.GetLongValue("Resupply", "drink", 0)));
    resupplyCookedFood.store(static_cast<int>(ini.GetLongValue("Resupply", "cookedFood", 10)));

    for (int i = 0; i < 8; ++i)
    {
        LoadCustomItem(ini, "Resupply", i, resupplyCustom[i]);
    }

    LoadCell(ini, "Cell1", cells[0]);
    LoadCell(ini, "Cell2", cells[1]);
    LoadCell(ini, "Cell3", cells[2]);
    LoadCell(ini, "Cell4", cells[3]);
    LoadCell(ini, "Cell5", cells[4]);

    const char* blacklistRaw = ini.GetValue("Blacklist", "items", "");
    if (blacklistRaw && blacklistRaw[0] != '\0') {
        LoadStringList(blacklistRaw, blacklist);
    }
    RebuildBlacklistLower();

    Clamp();
    }
    catch (...)
    {
        logger::error("Failed to load settings, using defaults");
    }
}

void Settings::Save()
{
    constexpr auto path = L"Data/SKSE/Plugins/HomeAutoSort.ini";

    CSimpleIniA ini;
    ini.SetUnicode();

    ini.SetBoolValue("General", "debug", debug.load());
    ini.SetLongValue("General", "keyboardStashHotkey", static_cast<long>(keyboardStashHotkey.load()));
    ini.SetLongValue("General", "gamepadStashHotkey", static_cast<long>(gamepadStashHotkey.load()));
    ini.SetLongValue("General", "keyboardResupplyHotkey", static_cast<long>(keyboardResupplyHotkey.load()));
    ini.SetLongValue("General", "gamepadResupplyHotkey", static_cast<long>(gamepadResupplyHotkey.load()));
    ini.SetBoolValue("General", "lorerim5Compat", lorerim5Compat.load());
    ini.SetLongValue("General", "perTickStash", static_cast<long>(perTickStash.load()));
    ini.SetLongValue("General", "perTickResupply", static_cast<long>(perTickResupply.load()));

    ini.SetBoolValue("General", "prioritizeStrongestHealing", prioritizeStrongestHealing.load());
    ini.SetBoolValue("General", "prioritizeStrongestMagicka", prioritizeStrongestMagicka.load());
    ini.SetBoolValue("General", "prioritizeStrongestStamina", prioritizeStrongestStamina.load());
    ini.SetBoolValue("General", "prioritizeStrongestArrow", prioritizeStrongestArrow.load());
    ini.SetBoolValue("General", "prioritizeStrongestBolt", prioritizeStrongestBolt.load());
    ini.SetBoolValue("General", "prioritizeFillingFood", prioritizeFillingFood.load());
    ini.SetBoolValue("General", "prioritizeHydratingDrink", prioritizeHydratingDrink.load());
    ini.SetBoolValue("General", "allowAlcohol", allowAlcohol.load());

    ini.SetBoolValue("General", "stashUnmatchedToMaster", stashUnmatchedToMaster.load());
    ini.SetLongValue("General", "minimumValueValuable", static_cast<long>(minimumValueValuable.load()));

    ini.SetLongValue("Resupply", "healingPotions", static_cast<long>(resupplyHealingPotions.load()));
    ini.SetLongValue("Resupply", "magickaPotions", static_cast<long>(resupplyMagickaPotions.load()));
    ini.SetLongValue("Resupply", "staminaPotions", static_cast<long>(resupplyStaminaPotions.load()));
    ini.SetLongValue("Resupply", "curePoison", static_cast<long>(resupplyCurePoison.load()));
    ini.SetLongValue("Resupply", "cureDisease", static_cast<long>(resupplyCureDisease.load()));
    ini.SetLongValue("Resupply", "arrows", static_cast<long>(resupplyArrows.load()));
    ini.SetLongValue("Resupply", "bolts", static_cast<long>(resupplyBolts.load()));
    ini.SetLongValue("Resupply", "lockpicks", static_cast<long>(resupplyLockpicks.load()));
    ini.SetLongValue("Resupply", "food", static_cast<long>(resupplyFood.load()));
    ini.SetLongValue("Resupply", "drink", static_cast<long>(resupplyDrink.load()));
    ini.SetLongValue("Resupply", "cookedFood", static_cast<long>(resupplyCookedFood.load()));

    for (int i = 0; i < 8; ++i)
    {
        SaveCustomItem(ini, "Resupply", i, resupplyCustom[i]);
    }

    SaveCell(ini, "Cell1", cells[0]);
    SaveCell(ini, "Cell2", cells[1]);
    SaveCell(ini, "Cell3", cells[2]);
    SaveCell(ini, "Cell4", cells[3]);
    SaveCell(ini, "Cell5", cells[4]);

    ini.SetValue("Blacklist", "items", SaveStringList(blacklist).c_str(),
        "; Comma-separated list of case-insensitive partial match strings. Items with names/editorIDs containing these are never stashed.");

    RebuildBlacklistLower();
    ini.SaveFile(path);
    ApplyLogLevel();
}

void Settings::LoadCell(CSimpleIniA& ini, const char* section, CellSettings& cell)
{
    cell.homeEditorID = ini.GetValue(section, "homeEditorID", "");
    cell.homeFormID = static_cast<uint32_t>(ini.GetLongValue(section, "homeFormID", 0));
    cell.homeSet = ini.GetBoolValue(section, "homeSet", false);

    cell.masterChestEditorID = ini.GetValue(section, "masterChestEditorID", "");

    cell.weapons = ini.GetValue(section, "weapons", "");
    cell.weapons_OneHanded = ini.GetValue(section, "weapons_OneHanded", "");
    cell.weapons_TwoHanded = ini.GetValue(section, "weapons_TwoHanded", "");
    cell.weapons_Archery = ini.GetValue(section, "weapons_Archery", "");
    cell.weapons_Staves = ini.GetValue(section, "weapons_Staves", "");

    cell.armor = ini.GetValue(section, "armor", "");
    cell.armor_Light = ini.GetValue(section, "armor_Light", "");
    cell.armor_Heavy = ini.GetValue(section, "armor_Heavy", "");
    cell.armor_Clothing = ini.GetValue(section, "armor_Clothing", "");
    cell.armor_Shield = ini.GetValue(section, "armor_Shield", "");

    cell.jewelry = ini.GetValue(section, "jewelry", "");
    cell.jewelry_Rings = ini.GetValue(section, "jewelry_Rings", "");
    cell.jewelry_Amulets = ini.GetValue(section, "jewelry_Amulets", "");
    cell.jewelry_Circlets = ini.GetValue(section, "jewelry_Circlets", "");

    cell.potions = ini.GetValue(section, "potions", "");
    cell.poisons = ini.GetValue(section, "poisons", "");
    cell.scrolls = ini.GetValue(section, "scrolls", "");

    cell.consumables = ini.GetValue(section, "consumables", "");
    cell.consumables_Alcohol = ini.GetValue(section, "consumables_Alcohol", "");
    cell.consumables_NonAlcoholicDrinks = ini.GetValue(section, "consumables_NonAlcoholicDrinks", "");
    cell.consumables_RawMeat = ini.GetValue(section, "consumables_RawMeat", "");
    cell.consumables_CookedMeat = ini.GetValue(section, "consumables_CookedMeat", "");
    cell.consumables_ProduceGrains = ini.GetValue(section, "consumables_ProduceGrains", "");
    cell.consumables_Cheese = ini.GetValue(section, "consumables_Cheese", "");
    cell.consumables_Soups = ini.GetValue(section, "consumables_Soups", "");

    cell.consumables_Raw = ini.GetValue(section, "consumables_Raw", "");
    cell.consumables_Cooked = ini.GetValue(section, "consumables_Cooked", "");
    cell.consumables_Drinks = ini.GetValue(section, "consumables_Drinks", "");

    cell.ingredients = ini.GetValue(section, "ingredients", "");

    cell.writtenWorks = ini.GetValue(section, "writtenWorks", "");
    cell.writtenWorks_Books = ini.GetValue(section, "writtenWorks_Books", "");
    cell.writtenWorks_Notes = ini.GetValue(section, "writtenWorks_Notes", "");
    cell.writtenWorks_SkillBooks = ini.GetValue(section, "writtenWorks_SkillBooks", "");
    cell.writtenWorks_SpellBooks = ini.GetValue(section, "writtenWorks_SpellBooks", "");

    cell.keys = ini.GetValue(section, "keys", "");

    cell.ammo = ini.GetValue(section, "ammo", "");
    cell.ammo_Arrows = ini.GetValue(section, "ammo_Arrows", "");
    cell.ammo_Bolts = ini.GetValue(section, "ammo_Bolts", "");

    cell.misc = ini.GetValue(section, "misc", "");
    cell.misc_Ore = ini.GetValue(section, "misc_Ore", "");
    cell.misc_Ingot = ini.GetValue(section, "misc_Ingot", "");
    cell.misc_Gem = ini.GetValue(section, "misc_Gem", "");
    cell.misc_EmptySoulGem = ini.GetValue(section, "misc_EmptySoulGem", "");
    cell.misc_FilledSoulGem = ini.GetValue(section, "misc_FilledSoulGem", "");
    cell.misc_AnimalParts = ini.GetValue(section, "misc_AnimalParts", "");
    cell.misc_HidesPelts = ini.GetValue(section, "misc_HidesPelts", "");
    cell.misc_Valuables = ini.GetValue(section, "misc_Valuables", "");
    cell.misc_NonValuables = ini.GetValue(section, "misc_NonValuables", "");
    cell.misc_Leather = ini.GetValue(section, "misc_Leather", "");
    cell.misc_BuildingMaterials = ini.GetValue(section, "misc_BuildingMaterials", "");
    WipeBareHexKeys(cell);
}

void Settings::SaveCell(CSimpleIniA& ini, const char* section, const CellSettings& cell)
{
    ini.SetValue(section, "homeEditorID", cell.homeEditorID.c_str());
    ini.SetLongValue(section, "homeFormID", static_cast<long>(cell.homeFormID));
    ini.SetBoolValue(section, "homeSet", cell.homeSet);

    ini.SetValue(section, "masterChestEditorID", cell.masterChestEditorID.c_str());

    ini.SetValue(section, "weapons", cell.weapons.c_str());
    ini.SetValue(section, "weapons_OneHanded", cell.weapons_OneHanded.c_str());
    ini.SetValue(section, "weapons_TwoHanded", cell.weapons_TwoHanded.c_str());
    ini.SetValue(section, "weapons_Archery", cell.weapons_Archery.c_str());
    ini.SetValue(section, "weapons_Staves", cell.weapons_Staves.c_str());

    ini.SetValue(section, "armor", cell.armor.c_str());
    ini.SetValue(section, "armor_Light", cell.armor_Light.c_str());
    ini.SetValue(section, "armor_Heavy", cell.armor_Heavy.c_str());
    ini.SetValue(section, "armor_Clothing", cell.armor_Clothing.c_str());
    ini.SetValue(section, "armor_Shield", cell.armor_Shield.c_str());

    ini.SetValue(section, "jewelry", cell.jewelry.c_str());
    ini.SetValue(section, "jewelry_Rings", cell.jewelry_Rings.c_str());
    ini.SetValue(section, "jewelry_Amulets", cell.jewelry_Amulets.c_str());
    ini.SetValue(section, "jewelry_Circlets", cell.jewelry_Circlets.c_str());

    ini.SetValue(section, "potions", cell.potions.c_str());
    ini.SetValue(section, "poisons", cell.poisons.c_str());
    ini.SetValue(section, "scrolls", cell.scrolls.c_str());

    ini.SetValue(section, "consumables", cell.consumables.c_str());
    ini.SetValue(section, "consumables_Alcohol", cell.consumables_Alcohol.c_str());
    ini.SetValue(section, "consumables_NonAlcoholicDrinks", cell.consumables_NonAlcoholicDrinks.c_str());
    ini.SetValue(section, "consumables_RawMeat", cell.consumables_RawMeat.c_str());
    ini.SetValue(section, "consumables_CookedMeat", cell.consumables_CookedMeat.c_str());
    ini.SetValue(section, "consumables_ProduceGrains", cell.consumables_ProduceGrains.c_str());
    ini.SetValue(section, "consumables_Cheese", cell.consumables_Cheese.c_str());
    ini.SetValue(section, "consumables_Soups", cell.consumables_Soups.c_str());

    ini.SetValue(section, "consumables_Raw", cell.consumables_Raw.c_str());
    ini.SetValue(section, "consumables_Cooked", cell.consumables_Cooked.c_str());
    ini.SetValue(section, "consumables_Drinks", cell.consumables_Drinks.c_str());

    ini.SetValue(section, "ingredients", cell.ingredients.c_str());

    ini.SetValue(section, "writtenWorks", cell.writtenWorks.c_str());
    ini.SetValue(section, "writtenWorks_Books", cell.writtenWorks_Books.c_str());
    ini.SetValue(section, "writtenWorks_Notes", cell.writtenWorks_Notes.c_str());
    ini.SetValue(section, "writtenWorks_SkillBooks", cell.writtenWorks_SkillBooks.c_str());
    ini.SetValue(section, "writtenWorks_SpellBooks", cell.writtenWorks_SpellBooks.c_str());

    ini.SetValue(section, "keys", cell.keys.c_str());

    ini.SetValue(section, "ammo", cell.ammo.c_str());
    ini.SetValue(section, "ammo_Arrows", cell.ammo_Arrows.c_str());
    ini.SetValue(section, "ammo_Bolts", cell.ammo_Bolts.c_str());

    ini.SetValue(section, "misc", cell.misc.c_str());
    ini.SetValue(section, "misc_Ore", cell.misc_Ore.c_str());
    ini.SetValue(section, "misc_Ingot", cell.misc_Ingot.c_str());
    ini.SetValue(section, "misc_Gem", cell.misc_Gem.c_str());
    ini.SetValue(section, "misc_EmptySoulGem", cell.misc_EmptySoulGem.c_str());
    ini.SetValue(section, "misc_FilledSoulGem", cell.misc_FilledSoulGem.c_str());
    ini.SetValue(section, "misc_AnimalParts", cell.misc_AnimalParts.c_str());
    ini.SetValue(section, "misc_HidesPelts", cell.misc_HidesPelts.c_str());
    ini.SetValue(section, "misc_Valuables", cell.misc_Valuables.c_str());
    ini.SetValue(section, "misc_NonValuables", cell.misc_NonValuables.c_str());
    ini.SetValue(section, "misc_Leather", cell.misc_Leather.c_str());
    ini.SetValue(section, "misc_BuildingMaterials", cell.misc_BuildingMaterials.c_str());
}

void Settings::LoadCustomItem(CSimpleIniA& ini, const char* section, int index, ResupplyCustomItem& item)
{
    char keyFilter[64];
    char keyItem[64];
    char keyCount[64];
    snprintf(keyFilter, sizeof(keyFilter), "customFilter%d", index + 1);
    snprintf(keyItem, sizeof(keyItem), "customItem%d", index + 1);
    snprintf(keyCount, sizeof(keyCount), "customCount%d", index + 1);

    item.filterText = ini.GetValue(section, keyFilter, "");
    item.selectedEditorID = ini.GetValue(section, keyItem, "");
    item.count = static_cast<int>(ini.GetLongValue(section, keyCount, 0));
}

void Settings::SaveCustomItem(CSimpleIniA& ini, const char* section, int index, const ResupplyCustomItem& item)
{
    char keyFilter[64];
    char keyItem[64];
    char keyCount[64];
    snprintf(keyFilter, sizeof(keyFilter), "customFilter%d", index + 1);
    snprintf(keyItem, sizeof(keyItem), "customItem%d", index + 1);
    snprintf(keyCount, sizeof(keyCount), "customCount%d", index + 1);

    ini.SetValue(section, keyFilter, item.filterText.c_str());
    ini.SetValue(section, keyItem, item.selectedEditorID.c_str());
    ini.SetLongValue(section, keyCount, static_cast<long>(item.count));
}

void Settings::Clamp()
{
    int val = minimumValueValuable.load();
    if (val < 1) val = 1;
    if (val > 5000) val = 5000;
    minimumValueValuable.store(val);

    for (int i = 0; i < 8; ++i)
    {
        if (resupplyCustom[i].count < 0) resupplyCustom[i].count = 0;
        if (resupplyCustom[i].count > 100) resupplyCustom[i].count = 100;
    }

    auto clampCount = [](std::atomic<int>& count, int max) {
        int v = count.load();
        if (v < 0) v = 0;
        if (v > max) v = max;
        count.store(v);
    };

    clampCount(resupplyHealingPotions, 100);
    clampCount(resupplyMagickaPotions, 100);
    clampCount(resupplyStaminaPotions, 100);
    clampCount(resupplyCurePoison, 100);
    clampCount(resupplyCureDisease, 100);
    clampCount(resupplyArrows, 1000);
    clampCount(resupplyBolts, 1000);
    clampCount(resupplyLockpicks, 500);
    clampCount(resupplyFood, 100);
    clampCount(resupplyDrink, 100);
    clampCount(resupplyCookedFood, 100);

    auto clampRange = [](std::atomic<int>& val, int lo, int hi) {
        int v = val.load();
        if (v < lo) v = lo;
        if (v > hi) v = hi;
        val.store(v);
    };
    clampRange(perTickStash, 1, 50);
    clampRange(perTickResupply, 1, 50);
}

void Settings::ApplyLogLevel() const
{
    auto log = spdlog::default_logger();
    if (!log) return;

    if (debug.load())
    {
        log->set_level(spdlog::level::debug);
        log->flush_on(spdlog::level::debug);
    }
    else
    {
        log->set_level(spdlog::level::info);
        log->flush_on(spdlog::level::info);
    }
}

void Settings::ExportPreset(int cellIndex)
{
    auto& s = GetSingleton();
    auto& cs = s.cells[cellIndex];
    auto& cd = CellManager::GetCellData(cellIndex);
    if (cd.cellEditorID.empty()) return;

    std::string name = cd.cellEditorID;
    for (auto& c : name)
        if (!std::isalnum(static_cast<unsigned char>(c))) c = '_';
    std::string path = std::format("Data/SKSE/Plugins/{}_HAS.json", name);
    std::string json;
    json += "{\n";
    json += std::format("  \"cellEditorID\": \"{}\",\n", cd.cellEditorID);
    json += std::format("  \"cellFormID\": {},\n", cd.cellFormID);
    json += std::format("  \"cellName\": \"{}\",\n", cd.cellName);
    json += std::format("  \"masterChestEditorID\": \"{}\",\n", cs.masterChestEditorID);
    json += "  \"containers\": {\n";

    auto addField = [&](const char* key, const std::string& val) {
        json += std::format("    \"{}\": \"{}\",\n", key, val);
    };

    addField("weapons", cs.weapons);
    addField("weapons_OneHanded", cs.weapons_OneHanded);
    addField("weapons_TwoHanded", cs.weapons_TwoHanded);
    addField("weapons_Archery", cs.weapons_Archery);
    addField("weapons_Staves", cs.weapons_Staves);
    addField("armor", cs.armor);
    addField("armor_Light", cs.armor_Light);
    addField("armor_Heavy", cs.armor_Heavy);
    addField("armor_Clothing", cs.armor_Clothing);
    addField("armor_Shield", cs.armor_Shield);
    addField("jewelry", cs.jewelry);
    addField("jewelry_Rings", cs.jewelry_Rings);
    addField("jewelry_Amulets", cs.jewelry_Amulets);
    addField("jewelry_Circlets", cs.jewelry_Circlets);
    addField("potions", cs.potions);
    addField("poisons", cs.poisons);
    addField("scrolls", cs.scrolls);
    addField("consumables", cs.consumables);
    addField("consumables_Raw", cs.consumables_Raw);
    addField("consumables_Cooked", cs.consumables_Cooked);
    addField("consumables_Drinks", cs.consumables_Drinks);
    addField("consumables_Alcohol", cs.consumables_Alcohol);
    addField("consumables_NonAlcoholicDrinks", cs.consumables_NonAlcoholicDrinks);
    addField("consumables_RawMeat", cs.consumables_RawMeat);
    addField("consumables_CookedMeat", cs.consumables_CookedMeat);
    addField("consumables_ProduceGrains", cs.consumables_ProduceGrains);
    addField("consumables_Cheese", cs.consumables_Cheese);
    addField("consumables_Soups", cs.consumables_Soups);
    addField("ingredients", cs.ingredients);
    addField("writtenWorks", cs.writtenWorks);
    addField("writtenWorks_Books", cs.writtenWorks_Books);
    addField("writtenWorks_Notes", cs.writtenWorks_Notes);
    addField("writtenWorks_SkillBooks", cs.writtenWorks_SkillBooks);
    addField("writtenWorks_SpellBooks", cs.writtenWorks_SpellBooks);
    addField("keys", cs.keys);
    addField("ammo", cs.ammo);
    addField("ammo_Arrows", cs.ammo_Arrows);
    addField("ammo_Bolts", cs.ammo_Bolts);
    addField("misc", cs.misc);
    addField("misc_Ore", cs.misc_Ore);
    addField("misc_Ingot", cs.misc_Ingot);
    addField("misc_Gem", cs.misc_Gem);
    addField("misc_EmptySoulGem", cs.misc_EmptySoulGem);
    addField("misc_FilledSoulGem", cs.misc_FilledSoulGem);
    addField("misc_AnimalParts", cs.misc_AnimalParts);
    addField("misc_HidesPelts", cs.misc_HidesPelts);
    addField("misc_Valuables", cs.misc_Valuables);
    addField("misc_NonValuables", cs.misc_NonValuables);
    addField("misc_Leather", cs.misc_Leather);
    json += std::format("    \"misc_BuildingMaterials\": \"{}\"\n", cs.misc_BuildingMaterials);
    json += "  }\n}\n";

    std::wstring wpath(path.begin(), path.end());
    FILE* f = nullptr;
    if (_wfopen_s(&f, wpath.c_str(), L"w") == 0 && f)
    {
        fwrite(json.c_str(), 1, json.size(), f);
        fclose(f);
        logger::info("Settings: Exported preset to {}", path);
    }
    else
    {
        logger::error("Settings: Failed to write preset {}", path);
    }
}

std::string Settings::SaveStringList(const std::vector<std::string>& a_list) const
{
    std::string result;
    for (size_t i = 0; i < a_list.size(); ++i) {
        if (i > 0) result += ",";
        result += a_list[i];
    }
    return result;
}

void Settings::LoadStringList(const std::string& a_value, std::vector<std::string>& a_out)
{
    a_out.clear();
    if (a_value.empty()) return;
    std::stringstream ss(a_value);
    std::string token;
    while (std::getline(ss, token, ',')) {
        auto trimmed = token;
        trimmed.erase(0, trimmed.find_first_not_of(" \t"));
        trimmed.erase(trimmed.find_last_not_of(" \t") + 1);
        if (!trimmed.empty()) a_out.push_back(trimmed);
    }
}

void Settings::RebuildBlacklistLower()
{
    blacklistLower.clear();
    for (auto& term : blacklist) {
        std::string lower = term;
        std::transform(lower.begin(), lower.end(), lower.begin(),
            [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        blacklistLower.push_back(std::move(lower));
    }
}
