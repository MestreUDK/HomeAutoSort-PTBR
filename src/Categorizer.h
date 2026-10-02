#pragma once

#include "plugin.h"

namespace Categorizer
{
    enum class MainCategory
    {
        None,
        Weapons,
        Armor,
        Jewelry,
        Potions,
        Poisons,
        Scrolls,
        Consumables,
        Ingredients,
        WrittenWorks,
        Keys,
        Misc,
        Ammo
    };

    enum class SubCategory
    {
        None,
        Weapons_OneHanded,
        Weapons_TwoHanded,
        Weapons_Archery,
        Weapons_Staves,
        Armor_Light,
        Armor_Heavy,
        Armor_Clothing,
        Armor_Shield,
        Jewelry_Rings,
        Jewelry_Amulets,
        Jewelry_Circlets,
        Potions_General,
        Poisons_General,
        Scrolls_General,
        Consumables_General,
        Consumables_Alcohol,
        Consumables_NonAlcoholicDrinks,
        Consumables_RawMeat,
        Consumables_CookedMeat,
        Consumables_ProduceGrains,
        Consumables_Cheese,
        Consumables_Soups,
        Consumables_Raw,
        Consumables_Cooked,
        Consumables_Drinks,
        Ingredients_General,
        WrittenWorks_Books,
        WrittenWorks_Notes,
        WrittenWorks_SkillBooks,
        WrittenWorks_SpellBooks,
        Keys_General,
        Ammo_Arrows,
        Ammo_Bolts,
        Misc_General,
        Misc_Ore,
        Misc_Ingot,
        Misc_Gem,
        Misc_EmptySoulGem,
        Misc_FilledSoulGem,
        Misc_AnimalParts,
        Misc_HidesPelts,
        Misc_Valuables,
        Misc_NonValuables,
        Misc_Leather,
        Misc_BuildingMaterials
    };

    struct CategoryResult
    {
        MainCategory main = MainCategory::None;
        SubCategory  sub  = SubCategory::None;
    };

    CategoryResult ClassifyItem(RE::TESBoundObject* item);

    const char* SubCategoryToFieldName(SubCategory sub);
    bool        HasAlchemyEffectKeyword(RE::AlchemyItem* alch, RE::BGSKeyword* kw);
    bool        HasAlchemyEffectMGEF(RE::AlchemyItem* alch, RE::EffectSetting* mgef);
    bool        HasAlchemyEffectArchetype(RE::AlchemyItem* alch, RE::EffectArchetype arch);
    float       GetAlchemyEffectMagnitude(RE::AlchemyItem* alch, RE::BGSKeyword* kw);
    float       GetAlchemyEffectMagnitudeByMGEF(RE::AlchemyItem* alch, RE::EffectSetting* mgef);
}
