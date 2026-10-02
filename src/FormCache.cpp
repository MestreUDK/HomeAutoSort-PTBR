#include "pch.h"
#include "FormCache.h"
#include "Settings.h"

namespace
{
    std::atomic<bool> g_built{ false };

    RE::BGSKeyword*      g_kwRestoreHealth = nullptr;
    RE::BGSKeyword*      g_kwRestoreMagicka = nullptr;
    RE::BGSKeyword*      g_kwRestoreStamina = nullptr;
    RE::BGSKeyword*      g_kwVendorItemPotion = nullptr;
    RE::BGSKeyword*      g_kwVendorItemPoison = nullptr;
    RE::BGSKeyword*      g_kwVendorItemScroll = nullptr;
    RE::BGSKeyword*      g_kwVendorItemFood = nullptr;
    RE::BGSKeyword*      g_kwVendorItemBook = nullptr;
    RE::BGSKeyword*      g_kwVendorItemGem = nullptr;
    RE::BGSKeyword*      g_kwVendorItemAnimalPart = nullptr;
    RE::BGSKeyword*      g_kwVendorItemAnimalHide = nullptr;
    RE::BGSKeyword*      g_kwFoodMeat = nullptr;
    RE::BGSKeyword*      g_kwFoodVegan = nullptr;
    RE::BGSKeyword*      g_kwFoodVegetarian = nullptr;
    RE::BGSKeyword*      g_kwFoodCheese = nullptr;
    RE::BGSKeyword*      g_kwWeapTypeBow = nullptr;
    RE::BGSKeyword*      g_kwWeapTypeStaff = nullptr;
    RE::BGSKeyword*      g_kwArmorHeavy = nullptr;
    RE::BGSKeyword*      g_kwArmorLight = nullptr;
    RE::BGSKeyword*      g_kwArmorClothing = nullptr;
    RE::BGSKeyword*      g_kwCraftingCookpot = nullptr;
    RE::BGSKeyword*      g_kwCraftingOven = nullptr;
    RE::BGSKeyword*      g_kwCampfireCooking = nullptr;
    RE::BGSKeyword*      g_kwVendorItemOreIngot = nullptr;
    RE::BGSKeyword*      g_kwVendorItemKey = nullptr;
    RE::BGSKeyword*      g_kwMagicAlchRestoreHealth = nullptr;
    RE::BGSKeyword*      g_kwMagicAlchRestoreMagicka = nullptr;
    RE::BGSKeyword*      g_kwMagicAlchRestoreStamina = nullptr;
    RE::BGSKeyword*      g_kwArmorJewelry = nullptr;
    RE::BGSKeyword*      g_kwVendorItemJewelry = nullptr;
    RE::BGSKeyword*      g_kwClothingCirclet = nullptr;
    RE::BGSKeyword*      g_kwJewelryExpensive = nullptr;
    RE::BGSKeyword*      g_kwClothingNecklace = nullptr;
    RE::BGSKeyword*      g_kwClothingRing = nullptr;

    RE::BGSEquipSlot*    g_equipRightHand = nullptr;
    RE::BGSEquipSlot*    g_equipLeftHand = nullptr;
    RE::BGSEquipSlot*    g_equipBothHands = nullptr;

    RE::BGSSoundDescriptorForm* g_sndrGoldUp = nullptr;
    RE::BGSSoundDescriptorForm* g_sndrDrinkSound = nullptr;

    RE::EffectSetting*   g_mgefThirstRestore = nullptr;
    RE::EffectSetting*   g_mgefWeakStomach = nullptr;
    RE::EffectSetting*   g_mgefAlcohol = nullptr;
    RE::EffectSetting*   g_mgefHungerVerySmall = nullptr;
    RE::EffectSetting*   g_mgefHungerSmall = nullptr;
    RE::EffectSetting*   g_mgefHungerMedium = nullptr;
    RE::EffectSetting*   g_mgefHungerLarge = nullptr;

    RE::TESForm*         g_formLockpick = nullptr;

    std::vector<CachedItemEntry> g_allItems;
    std::vector<CachedItemEntry> g_healingPotions;
    std::vector<CachedItemEntry> g_magickaPotions;
    std::vector<CachedItemEntry> g_staminaPotions;
    std::vector<CachedItemEntry> g_curePoisonPotions;
    std::vector<CachedItemEntry> g_cureDiseasePotions;
    std::vector<CachedItemEntry> g_arrows;
    std::vector<CachedItemEntry> g_bolts;
    std::vector<CachedItemEntry> g_foodItems;
    std::vector<CachedItemEntry> g_drinkItems;
    std::vector<CachedItemEntry> g_cookedFoodItems;
    std::vector<CachedItemEntry> g_rawFoodItems;

    std::unordered_set<RE::FormID> g_cookedFoodOutputs;
    std::unordered_set<RE::FormID> g_cookRecipeInputs;
    std::unordered_set<RE::FormID> g_smelterInputs;
    std::unordered_set<RE::FormID> g_tanningRackOutputs;
    std::unordered_set<RE::FormID> g_carpenterTableInputs;

    std::unordered_map<RE::FormID, std::string> g_formIDToEditorID;
}

void FormCache::Build()
{
    auto* dh = RE::TESDataHandler::GetSingleton();

    g_kwRestoreHealth    = dh->LookupForm<RE::BGSKeyword>(0x00042503, "Skyrim.esm");
    g_kwRestoreMagicka   = dh->LookupForm<RE::BGSKeyword>(0x00042508, "Skyrim.esm");
    g_kwRestoreStamina   = dh->LookupForm<RE::BGSKeyword>(0x00042504, "Skyrim.esm");

    g_kwVendorItemPotion    = dh->LookupForm<RE::BGSKeyword>(0x0008CDEC, "Skyrim.esm");
    g_kwVendorItemPoison    = dh->LookupForm<RE::BGSKeyword>(0x0008CDED, "Skyrim.esm");
    g_kwVendorItemScroll    = dh->LookupForm<RE::BGSKeyword>(0x000A0E57, "Skyrim.esm");
    g_kwVendorItemFood      = dh->LookupForm<RE::BGSKeyword>(0x0008CDEA, "Skyrim.esm");
    g_kwVendorItemBook      = dh->LookupForm<RE::BGSKeyword>(0x000937A2, "Skyrim.esm");
    g_kwVendorItemGem       = dh->LookupForm<RE::BGSKeyword>(0x000914ED, "Skyrim.esm");
    g_kwVendorItemAnimalPart = dh->LookupForm<RE::BGSKeyword>(0x000914EB, "Skyrim.esm");
    g_kwVendorItemAnimalHide = dh->LookupForm<RE::BGSKeyword>(0x000914EA, "Skyrim.esm");

    g_kwWeapTypeBow   = dh->LookupForm<RE::BGSKeyword>(0x0001E715, "Skyrim.esm");
    g_kwWeapTypeStaff = dh->LookupForm<RE::BGSKeyword>(0x0001E716, "Skyrim.esm");

    g_kwArmorHeavy    = dh->LookupForm<RE::BGSKeyword>(0x0006BBD2, "Skyrim.esm");
    g_kwArmorLight    = dh->LookupForm<RE::BGSKeyword>(0x0006BBD3, "Skyrim.esm");
    g_kwArmorClothing = dh->LookupForm<RE::BGSKeyword>(0x0006BBD4, "Skyrim.esm");
    g_kwCraftingCookpot = dh->LookupForm<RE::BGSKeyword>(0x000A5CB3, "Skyrim.esm");
    g_kwCraftingOven = dh->LookupForm<RE::BGSKeyword>(0x0117F7, "HearthFires.esm");
    g_kwCampfireCooking = dh->LookupForm<RE::BGSKeyword>(0x00314B, "Update.esm");
    g_kwVendorItemOreIngot = dh->LookupForm<RE::BGSKeyword>(0x000914EC, "Skyrim.esm");
    g_kwVendorItemKey = dh->LookupForm<RE::BGSKeyword>(0x000914EF, "Skyrim.esm");

    g_kwArmorJewelry = dh->LookupForm<RE::BGSKeyword>(0x0006BBE9, "Skyrim.esm");
    g_kwVendorItemJewelry = dh->LookupForm<RE::BGSKeyword>(0x0008F95A, "Skyrim.esm");
    g_kwClothingCirclet = dh->LookupForm<RE::BGSKeyword>(0x0010CD08, "Skyrim.esm");
    g_kwJewelryExpensive = dh->LookupForm<RE::BGSKeyword>(0x000A8664, "Skyrim.esm");
    g_kwClothingNecklace = dh->LookupForm<RE::BGSKeyword>(0x0010CD0A, "Skyrim.esm");
    g_kwClothingRing = dh->LookupForm<RE::BGSKeyword>(0x0010CD09, "Skyrim.esm");

    g_kwMagicAlchRestoreHealth  = dh->LookupForm<RE::BGSKeyword>(0x00042503, "Skyrim.esm");
    g_kwMagicAlchRestoreMagicka = dh->LookupForm<RE::BGSKeyword>(0x00042508, "Skyrim.esm");
    g_kwMagicAlchRestoreStamina = dh->LookupForm<RE::BGSKeyword>(0x00042504, "Skyrim.esm");

    g_equipRightHand = dh->LookupForm<RE::BGSEquipSlot>(0x00013F42, "Skyrim.esm");
    g_equipLeftHand  = dh->LookupForm<RE::BGSEquipSlot>(0x00013F43, "Skyrim.esm");
    g_equipBothHands = dh->LookupForm<RE::BGSEquipSlot>(0x00013F45, "Skyrim.esm");

    g_sndrGoldUp = dh->LookupForm<RE::BGSSoundDescriptorForm>(0x0003E952, "Skyrim.esm");
    g_sndrDrinkSound = dh->LookupForm<RE::BGSSoundDescriptorForm>(0x000B6435, "Skyrim.esm");

    g_formLockpick = dh->LookupForm(0x0000000A, "Skyrim.esm");

    g_kwFoodMeat       = RE::TESForm::LookupByEditorID<RE::BGSKeyword>("FoodMeat");
    g_kwFoodVegan      = RE::TESForm::LookupByEditorID<RE::BGSKeyword>("FoodVegan");
    g_kwFoodVegetarian = RE::TESForm::LookupByEditorID<RE::BGSKeyword>("FoodVegetarian");
    g_kwFoodCheese     = RE::TESForm::LookupByEditorID<RE::BGSKeyword>("FoodCheese");

    g_mgefThirstRestore = RE::TESForm::LookupByEditorID<RE::EffectSetting>("Survival_ThirstRestoreEffect");
    g_mgefWeakStomach   = RE::TESForm::LookupByEditorID<RE::EffectSetting>("REQ_Effect_Food_DamageAttributes");
    g_mgefAlcohol       = RE::TESForm::LookupByEditorID<RE::EffectSetting>("REQ_Effect_Alcohol");

    g_mgefHungerVerySmall = RE::TESForm::LookupByEditorID<RE::EffectSetting>("Survival_FoodRestoreHungerVerySmall");
    g_mgefHungerSmall     = RE::TESForm::LookupByEditorID<RE::EffectSetting>("Survival_FoodRestoreHungerSmall");
    g_mgefHungerMedium    = RE::TESForm::LookupByEditorID<RE::EffectSetting>("Survival_FoodRestoreHungerMedium");
    g_mgefHungerLarge     = RE::TESForm::LookupByEditorID<RE::EffectSetting>("Survival_FoodRestoreHungerLarge");

    auto cacheFormArray = [&](auto& arr) {
        for (auto* form : arr)
        {
            if (!form) continue;
            CachedItemEntry entry;
            entry.name = form->GetName();
            if (entry.name.empty())
            {
                const char* edid = form->GetFormEditorID();
                if (edid) entry.name = edid;
            }
            if (entry.name.empty()) continue;
            const char* edid = form->GetFormEditorID();
            if (edid) entry.editorID = edid;
            entry.formID = form->GetFormID();
            entry.formType = form->GetFormType();
            g_allItems.push_back(std::move(entry));
        }
    };

    cacheFormArray(dh->GetFormArray<RE::AlchemyItem>());
    cacheFormArray(dh->GetFormArray<RE::TESObjectWEAP>());
    cacheFormArray(dh->GetFormArray<RE::TESObjectARMO>());
    cacheFormArray(dh->GetFormArray<RE::ScrollItem>());
    cacheFormArray(dh->GetFormArray<RE::IngredientItem>());
    cacheFormArray(dh->GetFormArray<RE::TESObjectBOOK>());
    cacheFormArray(dh->GetFormArray<RE::TESObjectMISC>());
    cacheFormArray(dh->GetFormArray<RE::TESAmmo>());
    cacheFormArray(dh->GetFormArray<RE::TESSoulGem>());
    cacheFormArray(dh->GetFormArray<RE::TESKey>());

    std::sort(g_allItems.begin(), g_allItems.end(),
        [](const CachedItemEntry& a, const CachedItemEntry& b) { return a.name < b.name; });

    {
        auto* kwSmelter = dh->LookupForm<RE::BGSKeyword>(0x000A5CCE, "Skyrim.esm");
        auto* kwTanning = dh->LookupForm<RE::BGSKeyword>(0x0007866A, "Skyrim.esm");
        auto* kwCarpenter = dh->LookupForm<RE::BGSKeyword>(0x00014353, "HearthFires.esm");

        for (auto* form : dh->GetFormArray<RE::BGSConstructibleObject>())
        {
            if (!form || !form->createdItem) continue;

            bool isCookStation =
                (g_kwCraftingCookpot && form->benchKeyword == g_kwCraftingCookpot) ||
                (g_kwCraftingOven && form->benchKeyword == g_kwCraftingOven) ||
                (g_kwCampfireCooking && form->benchKeyword == g_kwCampfireCooking);

            if (isCookStation)
                g_cookedFoodOutputs.insert(form->createdItem->GetFormID());

            bool createsFood = false;
            if (auto* alch = form->createdItem->As<RE::AlchemyItem>())
                createsFood = alch->IsFood();

            if (isCookStation || createsFood)
                form->requiredItems.ForEachContainerObject([&](RE::ContainerObject& entry) {
                    if (entry.obj) g_cookRecipeInputs.insert(entry.obj->GetFormID());
                    return RE::BSContainer::ForEachResult::kContinue;
                });

            if (kwSmelter && form->benchKeyword == kwSmelter)
                form->requiredItems.ForEachContainerObject([&](RE::ContainerObject& entry) {
                    if (entry.obj) g_smelterInputs.insert(entry.obj->GetFormID());
                    return RE::BSContainer::ForEachResult::kContinue;
                });
            if (kwTanning && form->benchKeyword == kwTanning)
                g_tanningRackOutputs.insert(form->createdItem->GetFormID());
            if (kwCarpenter && form->benchKeyword == kwCarpenter)
                form->requiredItems.ForEachContainerObject([&](RE::ContainerObject& entry) {
                    if (entry.obj) g_carpenterTableInputs.insert(entry.obj->GetFormID());
                    return RE::BSContainer::ForEachResult::kContinue;
                });
        }

        if (Settings::GetSingleton().debug.load())
            logger::info("FormCache: {} cooked outputs, {} cook inputs, {} smelter, {} tanning, {} carpenter",
                g_cookedFoodOutputs.size(), g_cookRecipeInputs.size(), g_smelterInputs.size(),
                g_tanningRackOutputs.size(), g_carpenterTableInputs.size());
    }

    {
        const auto& [map, lock] = RE::TESForm::GetAllFormsByEditorID();
        RE::BSReadLockGuard l{ lock };
        if (map)
        {
            for (auto& [key, form] : *map)
            {
                if (form) g_formIDToEditorID[form->GetFormID()] = key.c_str();
            }
        }
    }

    for (auto* form : dh->GetFormArray<RE::AlchemyItem>())
    {
        if (!form) continue;
        auto* alch = form->As<RE::AlchemyItem>();
        if (!alch || alch->IsPoison() || alch->IsFood()) continue;
        const char* edid = alch->GetFormEditorID();
        std::string edidStr = edid ? edid : "";
        std::string name = alch->GetName();
        if (name.empty() && !edidStr.empty()) name = edidStr;
        if (name.empty()) continue;

        CachedItemEntry entry{name, edidStr, alch->GetFormID(), RE::FormType::AlchemyItem};

        bool hasHeal = false, hasMag = false, hasStam = false;
        bool hasCureP = false, hasCureD = false;
        for (auto* effect : alch->effects)
        {
            if (!effect || !effect->baseEffect) continue;
            auto* mgef = effect->baseEffect;
            if (g_kwMagicAlchRestoreHealth && mgef->HasKeyword(g_kwMagicAlchRestoreHealth)) hasHeal = true;
            if (g_kwMagicAlchRestoreMagicka && mgef->HasKeyword(g_kwMagicAlchRestoreMagicka)) hasMag = true;
            if (g_kwMagicAlchRestoreStamina && mgef->HasKeyword(g_kwMagicAlchRestoreStamina)) hasStam = true;
            if (mgef->GetArchetype() == RE::EffectArchetype::kCurePoison) hasCureP = true;
            if (mgef->GetArchetype() == RE::EffectArchetype::kCureDisease) hasCureD = true;
        }
        if (hasHeal) g_healingPotions.push_back(entry);
        if (hasMag) g_magickaPotions.push_back(entry);
        if (hasStam) g_staminaPotions.push_back(entry);
        if (hasCureP) g_curePoisonPotions.push_back(entry);
        if (hasCureD) g_cureDiseasePotions.push_back(entry);
    }

    for (auto* form : dh->GetFormArray<RE::TESAmmo>())
    {
        if (!form) continue;
        const char* edid = form->GetFormEditorID();
        std::string edidStr = edid ? edid : "";
        std::string name = form->GetName();
        if (name.empty() && !edidStr.empty()) name = edidStr;
        if (name.empty()) continue;

        CachedItemEntry entry{name, edidStr, form->GetFormID(), RE::FormType::Ammo};
        if (form->IsBolt())
            g_bolts.push_back(entry);
        else
            g_arrows.push_back(entry);
    }

    for (auto* form : dh->GetFormArray<RE::AlchemyItem>())
    {
        if (!form || !form->IsFood()) continue;
        const char* edid = form->GetFormEditorID();
        std::string edidStr = edid ? edid : "";
        std::string name = form->GetName();
        if (name.empty() && !edidStr.empty()) name = edidStr;
        if (name.empty()) continue;

        CachedItemEntry entry{name, edidStr, form->GetFormID(), RE::FormType::AlchemyItem};

        bool hasThirst = false;
        bool hasHunger = false;
        for (auto* effect : form->effects)
        {
            if (!effect || !effect->baseEffect) continue;
            if (g_mgefThirstRestore && effect->baseEffect == g_mgefThirstRestore) hasThirst = true;
            if (g_mgefHungerVerySmall && effect->baseEffect == g_mgefHungerVerySmall) hasHunger = true;
            if (g_mgefHungerSmall && effect->baseEffect == g_mgefHungerSmall) hasHunger = true;
            if (g_mgefHungerMedium && effect->baseEffect == g_mgefHungerMedium) hasHunger = true;
            if (g_mgefHungerLarge && effect->baseEffect == g_mgefHungerLarge) hasHunger = true;
        }
        if (hasThirst) g_drinkItems.push_back(entry);
        if (hasHunger) g_foodItems.push_back(entry);

        if (g_sndrDrinkSound && form->data.consumptionSound == g_sndrDrinkSound)
            g_drinkItems.push_back(entry);
        if (IsCookedFood(form->GetFormID()))
        {
            g_foodItems.push_back(entry);
            g_cookedFoodItems.push_back(entry);
        }
        else
        {
            g_rawFoodItems.push_back(entry);
        }
    }

    g_built.store(true);

    if (Settings::GetSingleton().debug.load())
        logger::info("FormCache: {} items cached", g_allItems.size());
}

RE::BGSKeyword*      FormCache::kwRestoreHealth()     { return g_kwRestoreHealth; }
RE::BGSKeyword*      FormCache::kwRestoreMagicka()    { return g_kwRestoreMagicka; }
RE::BGSKeyword*      FormCache::kwRestoreStamina()    { return g_kwRestoreStamina; }
RE::BGSKeyword*      FormCache::kwVendorItemPotion()  { return g_kwVendorItemPotion; }
RE::BGSKeyword*      FormCache::kwVendorItemPoison()  { return g_kwVendorItemPoison; }
RE::BGSKeyword*      FormCache::kwVendorItemScroll()  { return g_kwVendorItemScroll; }
RE::BGSKeyword*      FormCache::kwVendorItemFood()    { return g_kwVendorItemFood; }
RE::BGSKeyword*      FormCache::kwVendorItemBook()    { return g_kwVendorItemBook; }
RE::BGSKeyword*      FormCache::kwVendorItemGem()     { return g_kwVendorItemGem; }
RE::BGSKeyword*      FormCache::kwVendorItemAnimalPart() { return g_kwVendorItemAnimalPart; }
RE::BGSKeyword*      FormCache::kwVendorItemAnimalHide() { return g_kwVendorItemAnimalHide; }
RE::BGSKeyword*      FormCache::kwFoodMeat()          { return g_kwFoodMeat; }
RE::BGSKeyword*      FormCache::kwFoodVegan()         { return g_kwFoodVegan; }
RE::BGSKeyword*      FormCache::kwFoodVegetarian()    { return g_kwFoodVegetarian; }
RE::BGSKeyword*      FormCache::kwFoodCheese()        { return g_kwFoodCheese; }
RE::BGSKeyword*      FormCache::kwWeapTypeBow()       { return g_kwWeapTypeBow; }
RE::BGSKeyword*      FormCache::kwWeapTypeStaff()     { return g_kwWeapTypeStaff; }
RE::BGSKeyword*      FormCache::kwArmorHeavy()        { return g_kwArmorHeavy; }
RE::BGSKeyword*      FormCache::kwArmorLight()        { return g_kwArmorLight; }
RE::BGSKeyword*      FormCache::kwArmorClothing()     { return g_kwArmorClothing; }
RE::BGSKeyword*      FormCache::kwCraftingCookpot()   { return g_kwCraftingCookpot; }
RE::BGSKeyword*      FormCache::kwVendorItemOreIngot(){ return g_kwVendorItemOreIngot; }
RE::BGSKeyword*      FormCache::kwVendorItemKey()     { return g_kwVendorItemKey; }
RE::BGSKeyword*      FormCache::kwArmorJewelry()     { return g_kwArmorJewelry; }
RE::BGSKeyword*      FormCache::kwVendorItemJewelry(){ return g_kwVendorItemJewelry; }
RE::BGSKeyword*      FormCache::kwClothingCirclet()  { return g_kwClothingCirclet; }
RE::BGSKeyword*      FormCache::kwJewelryExpensive() { return g_kwJewelryExpensive; }
RE::BGSKeyword*      FormCache::kwClothingNecklace() { return g_kwClothingNecklace; }
RE::BGSKeyword*      FormCache::kwClothingRing()     { return g_kwClothingRing; }
RE::BGSKeyword*      FormCache::kwMagicAlchRestoreHealth()  { return g_kwMagicAlchRestoreHealth; }
RE::BGSKeyword*      FormCache::kwMagicAlchRestoreMagicka() { return g_kwMagicAlchRestoreMagicka; }
RE::BGSKeyword*      FormCache::kwMagicAlchRestoreStamina() { return g_kwMagicAlchRestoreStamina; }
RE::BGSEquipSlot*    FormCache::equipRightHand()      { return g_equipRightHand; }
RE::BGSEquipSlot*    FormCache::equipLeftHand()       { return g_equipLeftHand; }
RE::BGSEquipSlot*    FormCache::equipBothHands()      { return g_equipBothHands; }
RE::BGSSoundDescriptorForm* FormCache::sndrGoldUp()   { return g_sndrGoldUp; }
RE::BGSSoundDescriptorForm* FormCache::sndrDrinkSound(){ return g_sndrDrinkSound; }
RE::EffectSetting*   FormCache::mgefThirstRestore()   { return g_mgefThirstRestore; }
RE::EffectSetting*   FormCache::mgefWeakStomach()     { return g_mgefWeakStomach; }
RE::EffectSetting*   FormCache::mgefAlcohol()         { return g_mgefAlcohol; }
RE::EffectSetting*   FormCache::mgefHungerVerySmall() { return g_mgefHungerVerySmall; }
RE::EffectSetting*   FormCache::mgefHungerSmall()     { return g_mgefHungerSmall; }
RE::EffectSetting*   FormCache::mgefHungerMedium()    { return g_mgefHungerMedium; }
RE::EffectSetting*   FormCache::mgefHungerLarge()     { return g_mgefHungerLarge; }
RE::TESForm*         FormCache::formLockpick()        { return g_formLockpick; }
bool                 FormCache::IsBuilt()             { return g_built.load(); }

const std::vector<CachedItemEntry>& FormCache::GetAllItems() { return g_allItems; }

const std::vector<CachedItemEntry>& FormCache::GetHealingPotions()   { return g_healingPotions; }
const std::vector<CachedItemEntry>& FormCache::GetMagickaPotions()   { return g_magickaPotions; }
const std::vector<CachedItemEntry>& FormCache::GetStaminaPotions()   { return g_staminaPotions; }
const std::vector<CachedItemEntry>& FormCache::GetCurePoisonPotions(){ return g_curePoisonPotions; }
const std::vector<CachedItemEntry>& FormCache::GetCureDiseasePotions(){ return g_cureDiseasePotions; }
const std::vector<CachedItemEntry>& FormCache::GetArrows()           { return g_arrows; }
const std::vector<CachedItemEntry>& FormCache::GetBolts()            { return g_bolts; }
const std::vector<CachedItemEntry>& FormCache::GetFoodItems()        { return g_foodItems; }
const std::vector<CachedItemEntry>& FormCache::GetDrinkItems()       { return g_drinkItems; }
const std::vector<CachedItemEntry>& FormCache::GetCookedFoodItems()  { return g_cookedFoodItems; }
const std::vector<CachedItemEntry>& FormCache::GetRawFoodItems()     { return g_rawFoodItems; }

bool FormCache::IsCookedFood(RE::FormID formID)
{
    return g_cookedFoodOutputs.count(formID) > 0 || g_cookRecipeInputs.count(formID) == 0;
}
bool FormCache::IsSmelterInput(RE::FormID formID)      { return g_smelterInputs.count(formID) > 0; }
bool FormCache::IsTanningRackOutput(RE::FormID formID) { return g_tanningRackOutputs.count(formID) > 0; }
bool FormCache::IsCarpenterTableInput(RE::FormID formID){ return g_carpenterTableInputs.count(formID) > 0; }

const std::string& FormCache::GetEditorIDForForm(RE::FormID formID)
{
    static const std::string empty;
    auto it = g_formIDToEditorID.find(formID);
    return it != g_formIDToEditorID.end() ? it->second : empty;
}
