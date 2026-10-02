#include "pch.h"
#include "Categorizer.h"
#include "FormCache.h"
#include "Settings.h"

namespace
{
    bool StringContainsIgnoreCase(std::string_view haystack, std::string_view needle)
    {
        if (needle.empty()) return false;
        auto it = std::search(haystack.begin(), haystack.end(), needle.begin(), needle.end(),
            [](char a, char b) { return std::tolower(static_cast<unsigned char>(a)) == std::tolower(static_cast<unsigned char>(b)); });
        return it != haystack.end();
    }

    std::string ToLower(std::string_view sv)
    {
        std::string result(sv);
        for (auto& c : result) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        return result;
    }

    template <typename T>
    bool FormHasKeyword(T* form, RE::BGSKeyword* kw)
    {
        if (!form || !kw) return false;
        return form->HasKeyword(kw);
    }

    bool IsBook(RE::TESObjectBOOK* book)
    {
        if (!book) return false;
        auto* kwBook = FormCache::kwVendorItemBook();
        if (!kwBook) return false;
        return FormHasKeyword(book, kwBook);
    }

    bool IsNoteOrLetterOrJournal(RE::TESObjectBOOK* book)
    {
        if (!book) return false;
        auto name = ToLower(book->GetName());
        std::string edid;
        const char* rawEdid = book->GetFormEditorID();
        if (rawEdid) edid = ToLower(rawEdid);

        if (book->GetWeight() <= 0.0f) return true;

        if (StringContainsIgnoreCase(name, "note") || StringContainsIgnoreCase(name, "letter"))
            return true;
        if (!edid.empty() && (StringContainsIgnoreCase(edid, "note") || StringContainsIgnoreCase(edid, "letter")))
            return true;

        if (StringContainsIgnoreCase(name, "journal") || (!edid.empty() && StringContainsIgnoreCase(edid, "journal")))
            return true;

        return false;
    }

    bool IsSkillBook(RE::TESObjectBOOK* book)
    {
        if (!book) return false;
        return book->TeachesSkill();
    }

    bool IsSpellBook(RE::TESObjectBOOK* book)
    {
        if (!book) return false;
        return book->TeachesSpell();
    }
}

bool Categorizer::HasAlchemyEffectKeyword(RE::AlchemyItem* alch, RE::BGSKeyword* kw)
{
    if (!alch || !kw) return false;
    for (auto* effect : alch->effects)
    {
        if (!effect || !effect->baseEffect) continue;
        if (effect->baseEffect->HasKeyword(kw))
            return true;
    }
    return false;
}

bool Categorizer::HasAlchemyEffectMGEF(RE::AlchemyItem* alch, RE::EffectSetting* mgef)
{
    if (!alch || !mgef) return false;
    for (auto* effect : alch->effects)
    {
        if (!effect) continue;
        if (effect->baseEffect == mgef)
            return true;
    }
    return false;
}

bool Categorizer::HasAlchemyEffectArchetype(RE::AlchemyItem* alch, RE::EffectArchetype arch)
{
    if (!alch) return false;
    for (auto* effect : alch->effects)
    {
        if (!effect || !effect->baseEffect) continue;
        if (effect->baseEffect->GetArchetype() == arch)
            return true;
    }
    return false;
}

float Categorizer::GetAlchemyEffectMagnitude(RE::AlchemyItem* alch, RE::BGSKeyword* kw)
{
    if (!alch || !kw) return 0.0f;
    for (auto* effect : alch->effects)
    {
        if (!effect || !effect->baseEffect) continue;
        if (effect->baseEffect->HasKeyword(kw))
            return effect->effectItem.magnitude;
    }
    return 0.0f;
}

float Categorizer::GetAlchemyEffectMagnitudeByMGEF(RE::AlchemyItem* alch, RE::EffectSetting* mgef)
{
    if (!alch || !mgef) return 0.0f;
    for (auto* effect : alch->effects)
    {
        if (!effect) continue;
        if (effect->baseEffect == mgef)
            return effect->effectItem.magnitude;
    }
    return 0.0f;
}

Categorizer::CategoryResult Categorizer::ClassifyItem(RE::TESBoundObject* item)
{
    CategoryResult result;
    if (!item) return result;

    auto* settings = &Settings::GetSingleton();
    auto formType = item->GetFormType();

    if (formType == RE::FormType::Weapon)
    {
        auto* weap = item->As<RE::TESObjectWEAP>();
        if (weap)
        {
            auto* kwBow = FormCache::kwWeapTypeBow();
            auto* kwStaff = FormCache::kwWeapTypeStaff();

            if (kwBow && FormHasKeyword(weap, kwBow))
            {
                result.main = MainCategory::Weapons;
                result.sub = SubCategory::Weapons_Archery;
            }
            else if (kwStaff && FormHasKeyword(weap, kwStaff))
            {
                result.main = MainCategory::Weapons;
                result.sub = SubCategory::Weapons_Staves;
            }
            else
            {
                auto* slot = weap->GetEquipSlot();
                result.main = MainCategory::Weapons;
                auto* rh = FormCache::equipRightHand();
                auto* lh = FormCache::equipLeftHand();
                auto* bh = FormCache::equipBothHands();
                if ((rh && slot == rh) || (lh && slot == lh))
                    result.sub = SubCategory::Weapons_OneHanded;
                else if (bh && slot == bh)
                    result.sub = SubCategory::Weapons_TwoHanded;
                else
                    result.sub = SubCategory::Weapons_OneHanded;
            }
        }
        return result;
    }

    if (formType == RE::FormType::Armor)
    {
        auto* armo = item->As<RE::TESObjectARMO>();
        if (armo)
        {
            if (armo->IsShield())
            {
                result.main = MainCategory::Armor;
                result.sub = SubCategory::Armor_Shield;
                return result;
            }

            if (armo->IsHeavyArmor() || armo->IsLightArmor())
            {
                result.main = MainCategory::Armor;
                if (armo->IsHeavyArmor())
                    result.sub = SubCategory::Armor_Heavy;
                else
                    result.sub = SubCategory::Armor_Light;
                return result;
            }

            auto HasJewelryKeyword = [&](RE::BGSKeyword* kw) {
                return kw && FormHasKeyword(armo, kw);
            };

            auto* kwAJ = FormCache::kwArmorJewelry();
            auto* kwVJ = FormCache::kwVendorItemJewelry();
            auto* kwRing = FormCache::kwClothingRing();
            auto* kwNeck = FormCache::kwClothingNecklace();
            auto* kwCirc = FormCache::kwClothingCirclet();

            bool hasRingSlot = armo->HasPartOf(RE::BIPED_MODEL::BipedObjectSlot::kRing);
            bool hasAmuletSlot = armo->HasPartOf(RE::BIPED_MODEL::BipedObjectSlot::kAmulet);
            bool hasCircletSlot = armo->HasPartOf(RE::BIPED_MODEL::BipedObjectSlot::kCirclet);

            if (hasRingSlot && (HasJewelryKeyword(kwAJ) || HasJewelryKeyword(kwVJ) || HasJewelryKeyword(kwRing)))
            {
                result.main = MainCategory::Jewelry;
                result.sub = SubCategory::Jewelry_Rings;
                return result;
            }
            if (hasAmuletSlot && (HasJewelryKeyword(kwAJ) || HasJewelryKeyword(kwVJ) || HasJewelryKeyword(kwNeck)))
            {
                result.main = MainCategory::Jewelry;
                result.sub = SubCategory::Jewelry_Amulets;
                return result;
            }
            if (hasCircletSlot && (HasJewelryKeyword(kwAJ) || HasJewelryKeyword(kwVJ) || HasJewelryKeyword(kwCirc)))
            {
                result.main = MainCategory::Jewelry;
                result.sub = SubCategory::Jewelry_Circlets;
                return result;
            }

            result.main = MainCategory::Armor;
            result.sub = SubCategory::Armor_Clothing;
            return result;
        }
        return result;
    }

    if (formType == RE::FormType::AlchemyItem)
    {
        auto* alch = item->As<RE::AlchemyItem>();
        if (!alch) return result;

        auto* kwPotion = FormCache::kwVendorItemPotion();
        auto* kwPoison = FormCache::kwVendorItemPoison();
        auto* kwFood = FormCache::kwVendorItemFood();

        bool isPoison = alch->IsPoison();
        if (isPoison || (kwPoison && FormHasKeyword(alch, kwPoison)))
        {
            result.main = MainCategory::Poisons;
            result.sub = SubCategory::Poisons_General;
            return result;
        }

        if (kwPotion && FormHasKeyword(alch, kwPotion))
        {
            result.main = MainCategory::Potions;
            result.sub = SubCategory::Potions_General;
            return result;
        }

        auto* sndrDrink = FormCache::sndrDrinkSound();
        if (sndrDrink && alch->data.consumptionSound == sndrDrink)
        {
            result.main = MainCategory::Consumables;
            if (settings->lorerim5Compat.load())
            {
                auto* mgefAlcohol = FormCache::mgefAlcohol();
                if (mgefAlcohol && HasAlchemyEffectMGEF(alch, mgefAlcohol))
                    result.sub = SubCategory::Consumables_Alcohol;
                else
                    result.sub = SubCategory::Consumables_NonAlcoholicDrinks;
            }
            else
            {
                result.sub = SubCategory::Consumables_Drinks;
            }
            return result;
        }

        bool isFood = alch->IsFood();

        if (isFood || (kwFood && FormHasKeyword(alch, kwFood)))
        {
            result.main = MainCategory::Consumables;

            if (settings->lorerim5Compat.load())
            {
                auto* mgefAlcohol = FormCache::mgefAlcohol();
                if (mgefAlcohol && HasAlchemyEffectMGEF(alch, mgefAlcohol))
                {
                    result.sub = SubCategory::Consumables_Alcohol;
                    return result;
                }

                auto* mgefThirst = FormCache::mgefThirstRestore();
                bool hasThirst = mgefThirst && HasAlchemyEffectMGEF(alch, mgefThirst);
                bool hasHunger = false;
                if (hasThirst)
                {
                    hasHunger =
                        (FormCache::mgefHungerVerySmall() && HasAlchemyEffectMGEF(alch, FormCache::mgefHungerVerySmall())) ||
                        (FormCache::mgefHungerSmall() && HasAlchemyEffectMGEF(alch, FormCache::mgefHungerSmall())) ||
                        (FormCache::mgefHungerMedium() && HasAlchemyEffectMGEF(alch, FormCache::mgefHungerMedium())) ||
                        (FormCache::mgefHungerLarge() && HasAlchemyEffectMGEF(alch, FormCache::mgefHungerLarge()));
                }
                if (hasThirst && hasHunger)
                {
                    result.sub = SubCategory::Consumables_Soups;
                    return result;
                }
                if (hasThirst)
                {
                    result.sub = SubCategory::Consumables_NonAlcoholicDrinks;
                    return result;
                }

                auto* mgefWeak = FormCache::mgefWeakStomach();
                bool hasWeak = mgefWeak && HasAlchemyEffectMGEF(alch, mgefWeak);
                auto* kwMeat = FormCache::kwFoodMeat();
                bool hasMeat = kwMeat && FormHasKeyword(alch, kwMeat);

                if (hasWeak && hasMeat)
                {
                    result.sub = SubCategory::Consumables_RawMeat;
                    return result;
                }

                if (hasMeat)
                {
                    result.sub = SubCategory::Consumables_CookedMeat;
                    return result;
                }

                if (hasWeak)
                {
                    result.sub = SubCategory::Consumables_Raw;
                    return result;
                }

                auto* kwVegan = FormCache::kwFoodVegan();
                auto* kwVegetarian = FormCache::kwFoodVegetarian();
                if ((kwVegan && FormHasKeyword(alch, kwVegan)) ||
                    (kwVegetarian && FormHasKeyword(alch, kwVegetarian)))
                {
                    result.sub = SubCategory::Consumables_ProduceGrains;
                    return result;
                }

                auto* kwCheese = FormCache::kwFoodCheese();
                if (kwCheese && FormHasKeyword(alch, kwCheese))
                {
                    result.sub = SubCategory::Consumables_Cheese;
                    return result;
                }

                if (FormCache::IsCookedFood(alch->GetFormID()))
                {
                    result.sub = SubCategory::Consumables_Cooked;
                    return result;
                }
            }

            if (FormCache::IsCookedFood(alch->GetFormID()))
            {
                result.sub = SubCategory::Consumables_Cooked;
                return result;
            }

            result.sub = SubCategory::Consumables_Raw;
            return result;
        }

        result.main = MainCategory::Potions;
        result.sub = SubCategory::Potions_General;
        return result;
    }

    if (formType == RE::FormType::Scroll)
    {
        result.main = MainCategory::Scrolls;
        result.sub = SubCategory::Scrolls_General;
        return result;
    }

    if (formType == RE::FormType::Ingredient)
    {
        result.main = MainCategory::Ingredients;
        result.sub = SubCategory::Ingredients_General;
        return result;
    }

    if (formType == RE::FormType::Book)
    {
        auto* book = item->As<RE::TESObjectBOOK>();
        if (book)
        {
            result.main = MainCategory::WrittenWorks;

            if (IsSkillBook(book))
            {
                result.sub = SubCategory::WrittenWorks_SkillBooks;
                return result;
            }
            if (book->GetSpell() != nullptr)
            {
                result.sub = SubCategory::WrittenWorks_SpellBooks;
                return result;
            }
            if (IsNoteOrLetterOrJournal(book))
            {
                result.sub = SubCategory::WrittenWorks_Notes;
                return result;
            }
            result.sub = SubCategory::WrittenWorks_Books;
        }
        return result;
    }

    if (formType == RE::FormType::KeyMaster)
    {
        result.main = MainCategory::Keys;
        result.sub = SubCategory::Keys_General;
        return result;
    }

    if (formType == RE::FormType::SoulGem)
    {
        auto* sg = item->As<RE::TESSoulGem>();
        if (sg)
        {
            result.main = MainCategory::Misc;
            auto soul = sg->GetContainedSoul();
            if (soul == RE::SOUL_LEVEL::kNone)
                result.sub = SubCategory::Misc_EmptySoulGem;
            else
                result.sub = SubCategory::Misc_FilledSoulGem;
        }
        return result;
    }

    if (formType == RE::FormType::KeyMaster)
    {
        result.main = MainCategory::Keys;
        result.sub = SubCategory::Keys_General;
        return result;
    }

    if (formType == RE::FormType::Misc)
    {
        auto* misc = item->As<RE::TESObjectMISC>();
        if (!misc) return result;

        auto* kwKey = FormCache::kwVendorItemKey();
        if (kwKey && FormHasKeyword(misc, kwKey))
        {
            result.main = MainCategory::Keys;
            result.sub = SubCategory::Keys_General;
            return result;
        }

        auto* kwOreIngot = FormCache::kwVendorItemOreIngot();
        if (kwOreIngot && FormHasKeyword(misc, kwOreIngot))
        {
            result.main = MainCategory::Misc;
            if (FormCache::IsSmelterInput(misc->GetFormID()))
                result.sub = SubCategory::Misc_Ore;
            else
                result.sub = SubCategory::Misc_Ingot;
            return result;
        }

        if (FormCache::IsTanningRackOutput(misc->GetFormID()))
        {
            result.main = MainCategory::Misc;
            result.sub = SubCategory::Misc_Leather;
            return result;
        }

        if (FormCache::IsCarpenterTableInput(misc->GetFormID()))
        {
            result.main = MainCategory::Misc;
            result.sub = SubCategory::Misc_BuildingMaterials;
            return result;
        }

        result.main = MainCategory::Misc;

        auto* kwGem = FormCache::kwVendorItemGem();
        if (kwGem && FormHasKeyword(misc, kwGem))
        {
            result.sub = SubCategory::Misc_Gem;
            return result;
        }

        auto* kwAnimalPart = FormCache::kwVendorItemAnimalPart();
        if (kwAnimalPart && FormHasKeyword(misc, kwAnimalPart))
        {
            result.sub = SubCategory::Misc_AnimalParts;
            return result;
        }

        auto* kwAnimalHide = FormCache::kwVendorItemAnimalHide();
        if (kwAnimalHide && FormHasKeyword(misc, kwAnimalHide))
        {
            result.sub = SubCategory::Misc_HidesPelts;
            return result;
        }

        int minValue = settings->minimumValueValuable.load();
        if (misc->GetGoldValue() >= minValue)
            result.sub = SubCategory::Misc_Valuables;
        else
            result.sub = SubCategory::Misc_NonValuables;

        return result;
    }

    if (formType == RE::FormType::Ammo)
    {
        auto* ammo = item->As<RE::TESAmmo>();
        if (ammo)
        {
            result.main = MainCategory::Ammo;
            result.sub = ammo->IsBolt() ? SubCategory::Ammo_Bolts : SubCategory::Ammo_Arrows;
        }
        return result;
    }

    return result;
}

const char* Categorizer::SubCategoryToFieldName(SubCategory sub)
{
    switch (sub)
    {
    case SubCategory::Weapons_OneHanded:            return "weapons_OneHanded";
    case SubCategory::Weapons_TwoHanded:            return "weapons_TwoHanded";
    case SubCategory::Weapons_Archery:              return "weapons_Archery";
    case SubCategory::Weapons_Staves:               return "weapons_Staves";
    case SubCategory::Armor_Light:                  return "armor_Light";
    case SubCategory::Armor_Heavy:                  return "armor_Heavy";
    case SubCategory::Armor_Clothing:               return "armor_Clothing";
    case SubCategory::Armor_Shield:                 return "armor_Shield";
    case SubCategory::Jewelry_Rings:                return "jewelry_Rings";
    case SubCategory::Jewelry_Amulets:              return "jewelry_Amulets";
    case SubCategory::Jewelry_Circlets:             return "jewelry_Circlets";
    case SubCategory::Potions_General:              return "potions";
    case SubCategory::Poisons_General:              return "poisons";
    case SubCategory::Scrolls_General:              return "scrolls";
    case SubCategory::Consumables_General:          return "consumables";
    case SubCategory::Consumables_Alcohol:          return "consumables_Alcohol";
    case SubCategory::Consumables_NonAlcoholicDrinks: return "consumables_NonAlcoholicDrinks";
    case SubCategory::Consumables_RawMeat:          return "consumables_RawMeat";
    case SubCategory::Consumables_CookedMeat:       return "consumables_CookedMeat";
    case SubCategory::Consumables_ProduceGrains:    return "consumables_ProduceGrains";
    case SubCategory::Consumables_Cheese:           return "consumables_Cheese";
    case SubCategory::Consumables_Soups:            return "consumables_Soups";
    case SubCategory::Consumables_Raw:              return "consumables_Raw";
    case SubCategory::Consumables_Cooked:           return "consumables_Cooked";
    case SubCategory::Consumables_Drinks:           return "consumables_Drinks";
    case SubCategory::Ingredients_General:          return "ingredients";
    case SubCategory::WrittenWorks_Books:           return "writtenWorks_Books";
    case SubCategory::WrittenWorks_Notes:           return "writtenWorks_Notes";
    case SubCategory::WrittenWorks_SkillBooks:      return "writtenWorks_SkillBooks";
    case SubCategory::WrittenWorks_SpellBooks:      return "writtenWorks_SpellBooks";
    case SubCategory::Keys_General:                 return "keys";
    case SubCategory::Ammo_Arrows:                  return "ammo_Arrows";
    case SubCategory::Ammo_Bolts:                   return "ammo_Bolts";
    case SubCategory::Misc_General:                 return "misc";
    case SubCategory::Misc_Ore:                     return "misc_Ore";
    case SubCategory::Misc_Ingot:                   return "misc_Ingot";
    case SubCategory::Misc_Gem:                     return "misc_Gem";
    case SubCategory::Misc_EmptySoulGem:            return "misc_EmptySoulGem";
    case SubCategory::Misc_FilledSoulGem:           return "misc_FilledSoulGem";
    case SubCategory::Misc_AnimalParts:             return "misc_AnimalParts";
    case SubCategory::Misc_HidesPelts:              return "misc_HidesPelts";
    case SubCategory::Misc_Valuables:               return "misc_Valuables";
    case SubCategory::Misc_NonValuables:            return "misc_NonValuables";
    case SubCategory::Misc_Leather:                 return "misc_Leather";
    case SubCategory::Misc_BuildingMaterials:       return "misc_BuildingMaterials";
    default: return "";
    }
}
