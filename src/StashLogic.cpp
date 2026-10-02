#include "pch.h"
#include "StashLogic.h"
#include "CellManager.h"
#include "Categorizer.h"
#include "Settings.h"
#include "FormCache.h"

namespace
{
    RE::TESObjectREFR* GetContainerRef(int cellIndex, const std::string& key)
    {
        if (key.empty()) return nullptr;
        auto& data = CellManager::GetCellData(cellIndex);
        for (auto& ci : data.containers)
            if (CellManager::MatchContainer(ci, key))
                return RE::TESForm::LookupByID<RE::TESObjectREFR>(ci.refFormID);
        return nullptr;
    }

    bool ShouldSkipItem(RE::InventoryEntryData* entryData)
    {
        if (!entryData) return true;
        if (entryData->IsQuestObject()) return true;
        if (entryData->IsFavorited()) return true;
        return false;
    }

    bool IsItemEquipped(RE::Actor* actor, RE::TESBoundObject* item)
    {
        if (!actor || !item) return false;

        auto& runtimeData = actor->GetActorRuntimeData();
        if (runtimeData.biped)
        {
            for (std::uint32_t slot = 0; slot < RE::BIPED_OBJECTS::kTotal; ++slot)
            {
                if (runtimeData.biped->objects[slot].item == item)
                    return true;
            }
        }

        auto inv = actor->GetInventory();
        auto it = inv.find(item);
        if (it != inv.end() && it->second.second && it->second.second->IsWorn())
            return true;

        return false;
    }

    bool ShouldSkipFromPlayer(RE::TESBoundObject* item, RE::Actor* player)
    {
        if (!item || !player) return true;
        if (item->IsGold()) return true;
        if (item->GetFormType() == RE::FormType::Light) return true;
        if (IsItemEquipped(player, item)) return true;
        return false;
    }

    bool IsBlacklisted(RE::TESBoundObject* a_form)
    {
        if (!a_form) return false;
        auto& s = Settings::GetSingleton();
        if (s.blacklistLower.empty()) return false;

        auto check = [&](const char* str) -> bool {
            if (!str || str[0] == '\0') return false;
            std::string lowered = str;
            std::transform(lowered.begin(), lowered.end(), lowered.begin(),
                [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
            for (auto& term : s.blacklistLower) {
                if (lowered.find(term) != std::string::npos) return true;
            }
            return false;
        };

        if (check(a_form->GetName())) return true;
        if (check(FormCache::GetEditorIDForForm(a_form->GetFormID()).c_str())) return true;
        return false;
    }

    std::string GetSubField(const CellSettings& cs, const std::string& fn)
    {
        if (fn == "weapons_OneHanded") return cs.weapons_OneHanded;
        if (fn == "weapons_TwoHanded") return cs.weapons_TwoHanded;
        if (fn == "weapons_Archery") return cs.weapons_Archery;
        if (fn == "weapons_Staves") return cs.weapons_Staves;
        if (fn == "armor_Light") return cs.armor_Light;
        if (fn == "armor_Heavy") return cs.armor_Heavy;
        if (fn == "armor_Clothing") return cs.armor_Clothing;
        if (fn == "armor_Shield") return cs.armor_Shield;
        if (fn == "jewelry_Rings") return cs.jewelry_Rings;
        if (fn == "jewelry_Amulets") return cs.jewelry_Amulets;
        if (fn == "jewelry_Circlets") return cs.jewelry_Circlets;
        if (fn == "potions") return cs.potions;
        if (fn == "poisons") return cs.poisons;
        if (fn == "scrolls") return cs.scrolls;
        if (fn == "consumables") return cs.consumables;
        if (fn == "consumables_Alcohol") return cs.consumables_Alcohol;
        if (fn == "consumables_NonAlcoholicDrinks") return cs.consumables_NonAlcoholicDrinks;
        if (fn == "consumables_RawMeat") return cs.consumables_RawMeat;
        if (fn == "consumables_CookedMeat") return cs.consumables_CookedMeat;
        if (fn == "consumables_ProduceGrains") return cs.consumables_ProduceGrains;
        if (fn == "consumables_Cheese") return cs.consumables_Cheese;
        if (fn == "consumables_Soups") return cs.consumables_Soups;
        if (fn == "consumables_Raw") return cs.consumables_Raw;
        if (fn == "consumables_Cooked") return cs.consumables_Cooked;
        if (fn == "consumables_Drinks") return cs.consumables_Drinks;
        if (fn == "ingredients") return cs.ingredients;
        if (fn == "writtenWorks_Books") return cs.writtenWorks_Books;
        if (fn == "writtenWorks_Notes") return cs.writtenWorks_Notes;
        if (fn == "writtenWorks_SkillBooks") return cs.writtenWorks_SkillBooks;
        if (fn == "writtenWorks_SpellBooks") return cs.writtenWorks_SpellBooks;
        if (fn == "keys") return cs.keys;
        if (fn == "ammo_Arrows") return cs.ammo_Arrows;
        if (fn == "ammo_Bolts") return cs.ammo_Bolts;
        if (fn == "misc") return cs.misc;
        if (fn == "misc_Ore") return cs.misc_Ore;
        if (fn == "misc_Ingot") return cs.misc_Ingot;
        if (fn == "misc_Gem") return cs.misc_Gem;
        if (fn == "misc_EmptySoulGem") return cs.misc_EmptySoulGem;
        if (fn == "misc_FilledSoulGem") return cs.misc_FilledSoulGem;
        if (fn == "misc_AnimalParts") return cs.misc_AnimalParts;
        if (fn == "misc_HidesPelts") return cs.misc_HidesPelts;
        if (fn == "misc_Valuables") return cs.misc_Valuables;
        if (fn == "misc_NonValuables") return cs.misc_NonValuables;
        if (fn == "misc_Leather") return cs.misc_Leather;
        if (fn == "misc_BuildingMaterials") return cs.misc_BuildingMaterials;
        return "";
    }

    std::string GetSubFieldWithFallback(int cellIndex, const std::string& fn)
    {
        auto& cs = Settings::GetSingleton().cells[cellIndex];
        auto val = GetSubField(cs, fn);
        if (!val.empty()) return val;
        auto* preset = Settings::GetActivePreset(cellIndex);
        if (preset) val = GetSubField(*preset, fn);
        return val;
    }

    std::string GetMainField(const CellSettings& cs, Categorizer::MainCategory main)
    {
        switch (main)
        {
        case Categorizer::MainCategory::Weapons: return cs.weapons;
        case Categorizer::MainCategory::Armor: return cs.armor;
        case Categorizer::MainCategory::Jewelry: return cs.jewelry;
        case Categorizer::MainCategory::Potions: return cs.potions;
        case Categorizer::MainCategory::Poisons: return cs.poisons;
        case Categorizer::MainCategory::Scrolls: return cs.scrolls;
        case Categorizer::MainCategory::Consumables: return cs.consumables;
        case Categorizer::MainCategory::Ingredients: return cs.ingredients;
        case Categorizer::MainCategory::WrittenWorks: return cs.writtenWorks;
        case Categorizer::MainCategory::Keys: return cs.keys;
        case Categorizer::MainCategory::Ammo: return cs.ammo;
        case Categorizer::MainCategory::Misc: return cs.misc;
        default: return "";
        }
    }

    std::string GetMainFieldWithFallback(int cellIndex, Categorizer::MainCategory main)
    {
        auto& cs = Settings::GetSingleton().cells[cellIndex];
        auto val = GetMainField(cs, main);
        if (!val.empty()) return val;
        auto* preset = Settings::GetActivePreset(cellIndex);
        if (preset) val = GetMainField(*preset, main);
        return val;
    }

    std::string GetDestEditorID(int cellIndex, Categorizer::SubCategory sub, Categorizer::MainCategory main)
    {
        if (sub == Categorizer::SubCategory::Consumables_Soups)
        {
            std::string r = GetSubFieldWithFallback(cellIndex, "consumables_Soups");
            if (!r.empty()) return r;
            r = GetSubFieldWithFallback(cellIndex, "consumables_Cooked");
            if (!r.empty()) return r;
            r = GetSubFieldWithFallback(cellIndex, "consumables_NonAlcoholicDrinks");
            if (!r.empty()) return r;
            r = GetSubFieldWithFallback(cellIndex, "consumables_Drinks");
            if (!r.empty()) return r;
            return GetMainFieldWithFallback(cellIndex, main);
        }

        if (sub == Categorizer::SubCategory::Consumables_NonAlcoholicDrinks)
        {
            std::string r = GetSubFieldWithFallback(cellIndex, "consumables_NonAlcoholicDrinks");
            if (!r.empty()) return r;
            r = GetSubFieldWithFallback(cellIndex, "consumables_Drinks");
            if (!r.empty()) return r;
            return GetMainFieldWithFallback(cellIndex, main);
        }

        if (sub == Categorizer::SubCategory::Consumables_RawMeat)
        {
            std::string r = GetSubFieldWithFallback(cellIndex, "consumables_RawMeat");
            if (!r.empty()) return r;
            r = GetSubFieldWithFallback(cellIndex, "consumables_Raw");
            if (!r.empty()) return r;
            return GetMainFieldWithFallback(cellIndex, main);
        }

        const char* fieldName = Categorizer::SubCategoryToFieldName(sub);
        std::string fn = fieldName ? fieldName : "";

        std::string result = GetSubFieldWithFallback(cellIndex, fn);
        if (!result.empty())
            return result;

        return GetMainFieldWithFallback(cellIndex, main);
    }

    struct StashMoveItem
    {
        RE::ObjectRefHandle sourceRef;
        RE::FormID         formID;
        int                count;
        RE::ObjectRefHandle destRef;
    };
    std::vector<StashMoveItem> g_stashQueue;
    size_t                     g_stashIndex = 0;

    void ProcessStashTick()
    {
        auto& s = Settings::GetSingleton();
        int perTick = s.perTickStash.load();
        if (perTick < 1) perTick = 1;

        size_t end = (std::min)(g_stashIndex + static_cast<size_t>(perTick), g_stashQueue.size());

        for (; g_stashIndex < end; ++g_stashIndex)
        {
            auto& itm = g_stashQueue[g_stashIndex];

            auto sourcePtr = itm.sourceRef.get();
            auto destPtr = itm.destRef.get();
            auto* source = sourcePtr.get();
            auto* dest = destPtr.get();
            auto* form = RE::TESForm::LookupByID<RE::TESBoundObject>(itm.formID);

            if (!source || !dest || !form) continue;

            if (!dest->Is3DLoaded() || dest->IsDeleted())
            {
                if (s.debug.load())
                    logger::warn("Stash: Destination 0x{:08X} not 3D loaded or deleted", dest->GetFormID());
                continue;
            }

            if (!source->Is3DLoaded() || source->IsDeleted())
            {
                if (s.debug.load())
                    logger::warn("Stash: Source 0x{:08X} not 3D loaded", source->GetFormID());
                continue;
            }

            source->RemoveItem(form, static_cast<std::int32_t>(itm.count),
                RE::ITEM_REMOVE_REASON::kStoreInContainer, nullptr, dest);
            if (s.debug.load())
                logger::info("Stash: Moved {}x {} to 0x{:08X}", itm.count, form->GetName(), dest->GetFormID());
        }

        if (g_stashIndex >= g_stashQueue.size())
        {
            if (s.debug.load())
                logger::info("Stash: Complete. {} item groups moved.", g_stashQueue.size());
            g_stashQueue.clear();
#pragma push_macro("PlaySound")
#undef PlaySound
            RE::PlaySound("ITMGenericDown");
#pragma pop_macro("PlaySound")
            g_operationRunning.store(false);
            return;
        }

        auto* task = SKSE::GetTaskInterface();
        if (task)
        {
            task->AddTask([]() { ProcessStashTick(); });
        }
        else
        {
            g_stashQueue.clear();
            g_operationRunning.store(false);
        }
    }
}

void StashLogic::ExecuteStash()
{
    bool expected = false;
    if (!g_operationRunning.compare_exchange_strong(expected, true))
    {
        if (Settings::GetSingleton().debug.load())
            logger::info("Stash: Skipped, resupply already running");
        return;
    }

    auto* player = RE::PlayerCharacter::GetSingleton();
    if (!player) { g_operationRunning.store(false); return; }

    int cellIndex = CellManager::FindActiveCellIndex();
    if (cellIndex < 0) { g_operationRunning.store(false); return; }

    auto& s = Settings::GetSingleton();
    auto& cs = s.cells[cellIndex];

    auto* masterChest = CellManager::FindMasterChest(cellIndex);

    if (s.debug.load())
        logger::info("Stash: Executing in cell {} (index {})", cs.homeEditorID, cellIndex + 1);

    g_stashQueue.clear();
    g_stashIndex = 0;

    auto addItem = [&](RE::TESObjectREFR* source, RE::TESBoundObject* form, int count, RE::TESObjectREFR* dest) {
        if (source && form && dest)
        {
            auto srcHandle = source->GetHandle();
            auto dstHandle = dest->GetHandle();
            g_stashQueue.push_back({ srcHandle, form->GetFormID(), count, dstHandle });
        }
    };

    auto processInventory = [&](RE::TESObjectREFR* source) {
        if (!source) return;

        auto inv = source->GetInventory();
        for (auto& [boundObj, invData] : inv)
        {
            if (!boundObj) continue;
            if (!invData.second) continue;

            if (IsPhantomItem(boundObj))
            {
                if (s.debug.load())
                    logger::info("Stash: {} {:08X} skip: phantom", boundObj->GetName(), boundObj->GetFormID());
                continue;
            }

            auto* entryData = invData.second.get();
            if (ShouldSkipItem(entryData))
            {
                if (s.debug.load())
                    logger::info("Stash: {} {:08X} skip: quest or favorited", boundObj->GetName(), boundObj->GetFormID());
                continue;
            }

            if (source == player)
            {
                if (ShouldSkipFromPlayer(boundObj, player))
                {
                    if (s.debug.load())
                        logger::info("Stash: {} {:08X} skip: gold, torch, or equipped", boundObj->GetName(), boundObj->GetFormID());
                    continue;
                }
            }

            if (IsBlacklisted(boundObj))
            {
                if (s.debug.load())
                    logger::info("Stash: {} {:08X} skip: blacklisted", boundObj->GetName(), boundObj->GetFormID());
                continue;
            }

            int count = invData.first;
            if (count <= 0) continue;

            if (s.debug.load())
                logger::info("Stash: {} {:08X} count={} (delta={})", boundObj->GetName(), boundObj->GetFormID(), count, entryData->countDelta);

            auto cat = Categorizer::ClassifyItem(boundObj);
            if (s.debug.load())
                logger::info("Stash: {} {:08X} -> main={} sub={}", boundObj->GetName(), boundObj->GetFormID(),
                    static_cast<int>(cat.main), static_cast<int>(cat.sub));
            if (cat.main == Categorizer::MainCategory::None)
            {
                bool hasMasterId = !cs.masterChestEditorID.empty();
                if (!hasMasterId)
                {
                    auto* preset = Settings::GetActivePreset(cellIndex);
                    hasMasterId = preset && !preset->masterChestEditorID.empty();
                }
                if (source == player && s.stashUnmatchedToMaster.load() &&
                    masterChest && hasMasterId)
                {
                    addItem(player, boundObj, count, masterChest);
                }
                continue;
            }

            std::string destEditorID = GetDestEditorID(cellIndex, cat.sub, cat.main);

            if (s.debug.load())
                logger::info("Stash:   destEditorID='{}'", destEditorID.empty() ? "(empty)" : destEditorID.c_str());

            auto getMasterID = [&]() -> std::string {
                if (!cs.masterChestEditorID.empty()) return cs.masterChestEditorID;
                auto* preset = Settings::GetActivePreset(cellIndex);
                return preset ? preset->masterChestEditorID : "";
            };

            std::string masterID = getMasterID();

            if (destEditorID.empty() || destEditorID == masterID)
            {
                if (source == player)
                {
                    if (s.stashUnmatchedToMaster.load() && masterChest &&
                        !masterID.empty())
                    {
                        addItem(player, boundObj, count, masterChest);
                    }
                }
                continue;
            }

            auto* destRef = GetContainerRef(cellIndex, destEditorID);
            if (!destRef) continue;

            if (source != destRef)
                addItem(source, boundObj, count, destRef);
        }
    };

    processInventory(player);

    if (masterChest) processInventory(masterChest);

    if (g_stashQueue.empty())
    {
        if (s.debug.load())
            logger::info("Stash: No items to move.");
        g_operationRunning.store(false);
        return;
    }

    ProcessStashTick();
}
