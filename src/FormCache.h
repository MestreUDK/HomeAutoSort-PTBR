#pragma once

#include "plugin.h"

struct CachedItemEntry
{
    std::string name;
    std::string editorID;
    RE::FormID  formID;
    RE::FormType formType;
};

namespace FormCache
{
    void Build();
    bool IsBuilt();

    RE::BGSKeyword*      kwRestoreHealth();
    RE::BGSKeyword*      kwRestoreMagicka();
    RE::BGSKeyword*      kwRestoreStamina();
    RE::BGSKeyword*      kwVendorItemPotion();
    RE::BGSKeyword*      kwVendorItemPoison();
    RE::BGSKeyword*      kwVendorItemScroll();
    RE::BGSKeyword*      kwVendorItemFood();
    RE::BGSKeyword*      kwVendorItemBook();
    RE::BGSKeyword*      kwVendorItemGem();
    RE::BGSKeyword*      kwVendorItemAnimalPart();
    RE::BGSKeyword*      kwVendorItemAnimalHide();
    RE::BGSKeyword*      kwFoodMeat();
    RE::BGSKeyword*      kwFoodVegan();
    RE::BGSKeyword*      kwFoodVegetarian();
    RE::BGSKeyword*      kwFoodCheese();
    RE::BGSKeyword*      kwWeapTypeBow();
    RE::BGSKeyword*      kwWeapTypeStaff();
    RE::BGSKeyword*      kwArmorHeavy();
    RE::BGSKeyword*      kwArmorLight();
    RE::BGSKeyword*      kwArmorClothing();
    RE::BGSKeyword*      kwCraftingCookpot();
    RE::BGSKeyword*      kwVendorItemOreIngot();
    RE::BGSKeyword*      kwVendorItemKey();

    RE::BGSKeyword*      kwArmorJewelry();
    RE::BGSKeyword*      kwVendorItemJewelry();
    RE::BGSKeyword*      kwClothingCirclet();
    RE::BGSKeyword*      kwJewelryExpensive();
    RE::BGSKeyword*      kwClothingNecklace();
    RE::BGSKeyword*      kwClothingRing();

    RE::BGSKeyword*      kwMagicAlchRestoreHealth();
    RE::BGSKeyword*      kwMagicAlchRestoreMagicka();
    RE::BGSKeyword*      kwMagicAlchRestoreStamina();

    RE::BGSEquipSlot*    equipRightHand();
    RE::BGSEquipSlot*    equipLeftHand();
    RE::BGSEquipSlot*    equipBothHands();

    RE::BGSSoundDescriptorForm* sndrGoldUp();
    RE::BGSSoundDescriptorForm* sndrDrinkSound();

    RE::EffectSetting*   mgefThirstRestore();
    RE::EffectSetting*   mgefWeakStomach();
    RE::EffectSetting*   mgefAlcohol();
    RE::EffectSetting*   mgefHungerVerySmall();
    RE::EffectSetting*   mgefHungerSmall();
    RE::EffectSetting*   mgefHungerMedium();
    RE::EffectSetting*   mgefHungerLarge();

    RE::TESForm*         formLockpick();

    const std::vector<CachedItemEntry>& GetAllItems();

    const std::vector<CachedItemEntry>& GetHealingPotions();
    const std::vector<CachedItemEntry>& GetMagickaPotions();
    const std::vector<CachedItemEntry>& GetStaminaPotions();
    const std::vector<CachedItemEntry>& GetCurePoisonPotions();
    const std::vector<CachedItemEntry>& GetCureDiseasePotions();
    const std::vector<CachedItemEntry>& GetArrows();
    const std::vector<CachedItemEntry>& GetBolts();
    const std::vector<CachedItemEntry>& GetFoodItems();
    const std::vector<CachedItemEntry>& GetDrinkItems();
    const std::vector<CachedItemEntry>& GetCookedFoodItems();
    const std::vector<CachedItemEntry>& GetRawFoodItems();

    bool IsCookedFood(RE::FormID formID);
    bool IsSmelterInput(RE::FormID formID);
    bool IsTanningRackOutput(RE::FormID formID);
    bool IsCarpenterTableInput(RE::FormID formID);

    const std::string& GetEditorIDForForm(RE::FormID formID);
}
