#include "pch.h"
#include "ResupplyLogic.h"
#include "CellManager.h"
#include "Categorizer.h"
#include "Settings.h"
#include "FormCache.h"

namespace
{
    struct ScoredItem
    {
        RE::TESBoundObject* form;
        float               score;
        int                 available;
    };

    using InventoryMap = std::unordered_map<RE::TESBoundObject*, int>;

    struct SuppliedItem
    {
        RE::ObjectRefHandle containerRef;
        RE::FormID         formID;
        int                count;
    };
    std::vector<SuppliedItem> g_supplyQueue;
    size_t                    g_supplyIndex = 0;

    InventoryMap BuildInventoryMap(std::vector<RE::TESObjectREFR*>& containers)
    {
        InventoryMap result;
        for (auto* container : containers)
        {
            if (!container || !container->Is3DLoaded() || container->IsDeleted()) continue;
            auto inv = container->GetInventory();
            for (auto& [item, data] : inv)
            {
                if (item && !IsPhantomItem(item) && data.second && data.first > 0)
                    result[item] += data.first;
            }
        }
        return result;
    }

    int QueueFromContainers(RE::TESBoundObject* form, int count,
                            std::vector<RE::TESObjectREFR*>& containers,
                            InventoryMap* invMap = nullptr)
    {
        int remaining = count;
        int queued = 0;
        for (auto* container : containers)
        {
            if (remaining <= 0) break;
            if (!container || !container->Is3DLoaded() || container->IsDeleted()) continue;

            int available = container->GetInventoryCount(form);
            if (available <= 0) continue;

            int toTake = std::min(remaining, available);
            g_supplyQueue.push_back({ container->GetHandle(), form->GetFormID(), toTake });
            remaining -= toTake;
            queued += toTake;
            if (invMap) (*invMap)[form] -= toTake;
        }
        return queued;
    }

    void ProcessResupplyTick()
    {
        auto& s = Settings::GetSingleton();
        int perTick = s.perTickResupply.load();
        if (perTick < 1) perTick = 1;

        auto* player = RE::PlayerCharacter::GetSingleton();
        if (!player)
        {
            g_supplyQueue.clear();
            g_operationRunning.store(false);
            return;
        }

        auto hex = [](std::uint32_t id) { char b[16]; snprintf(b, sizeof(b), "%08X", id); return std::string(b); };

        size_t end = (std::min)(g_supplyIndex + static_cast<size_t>(perTick), g_supplyQueue.size());

        for (; g_supplyIndex < end; ++g_supplyIndex)
        {
            auto& itm = g_supplyQueue[g_supplyIndex];

            auto contPtr = itm.containerRef.get();
            auto* cont = contPtr.get();
            auto* form = RE::TESForm::LookupByID<RE::TESBoundObject>(itm.formID);

            if (!cont || !form) continue;

            if (!cont->Is3DLoaded() || cont->IsDeleted())
            {
                if (s.debug.load())
                    spdlog::default_logger_raw()->warn(std::string("Resupply: Container 0x") + hex(cont->GetFormID()) + " not 3D loaded or deleted");
                continue;
            }

            cont->RemoveItem(form, itm.count, RE::ITEM_REMOVE_REASON::kStoreInContainer, nullptr, player);
            if (s.debug.load())
                spdlog::default_logger_raw()->info(std::string("Resupply: Gave ") + std::to_string(itm.count) + "x " + form->GetName() + " from 0x" + hex(cont->GetFormID()));
        }

        if (g_supplyIndex >= g_supplyQueue.size())
        {
            if (s.debug.load())
                spdlog::default_logger_raw()->info(std::string("Resupply: Complete. ") + std::to_string(g_supplyQueue.size()) + " item groups given.");
            g_supplyQueue.clear();
#pragma push_macro("PlaySound")
#undef PlaySound
            RE::PlaySound("ITMGenericUp");
#pragma pop_macro("PlaySound")
            g_operationRunning.store(false);
            return;
        }

        auto* task = SKSE::GetTaskInterface();
        if (task)
        {
            task->AddTask([]() { ProcessResupplyTick(); });
        }
        else
        {
            g_supplyQueue.clear();
            g_operationRunning.store(false);
        }
    }

    int GetTotalAvailable(RE::TESBoundObject* form, InventoryMap& invMap)
    {
        auto it = invMap.find(form);
        return it != invMap.end() ? it->second : 0;
    }

    std::vector<RE::TESObjectREFR*> ResolveContainers(int cellIndex,
        const std::vector<std::string>& keys)
    {
        std::vector<RE::TESObjectREFR*> result;
        auto& data = CellManager::GetCellData(cellIndex);

        for (auto& key : keys)
        {
            if (key.empty()) continue;
            for (auto& ci : data.containers)
            {
                if (CellManager::MatchContainer(ci, key))
                {
                    auto* ref = RE::TESForm::LookupByID<RE::TESObjectREFR>(ci.refFormID);
                    if (ref && ref->Is3DLoaded())
                        result.push_back(ref);
                    break;
                }
            }
        }
        return result;
    }

    std::vector<RE::TESObjectREFR*> ResolveAllAssignedContainers(int cellIndex,
        const std::vector<std::string>& skipKeys)
    {
        auto& s = Settings::GetSingleton();
        auto& cs = s.cells[cellIndex];
        auto& data = CellManager::GetCellData(cellIndex);

        std::unordered_set<std::string> skipSet(skipKeys.begin(), skipKeys.end());

        std::vector<std::string> allAssigned = {
            cs.masterChestEditorID,
            cs.weapons, cs.weapons_OneHanded, cs.weapons_TwoHanded, cs.weapons_Archery, cs.weapons_Staves,
            cs.armor, cs.armor_Light, cs.armor_Heavy, cs.armor_Clothing, cs.armor_Shield,
            cs.jewelry, cs.jewelry_Rings, cs.jewelry_Amulets, cs.jewelry_Circlets,
            cs.potions, cs.poisons, cs.scrolls,
            cs.consumables, cs.consumables_Alcohol, cs.consumables_NonAlcoholicDrinks,
            cs.consumables_RawMeat, cs.consumables_CookedMeat,
            cs.consumables_ProduceGrains, cs.consumables_Cheese,
            cs.consumables_Raw, cs.consumables_Cooked, cs.consumables_Drinks,
            cs.ingredients,
            cs.writtenWorks, cs.writtenWorks_Books, cs.writtenWorks_Notes,
            cs.writtenWorks_SkillBooks, cs.writtenWorks_SpellBooks,
            cs.keys, cs.ammo, cs.ammo_Arrows, cs.ammo_Bolts,
            cs.misc, cs.misc_Ore, cs.misc_Ingot,
            cs.misc_Gem, cs.misc_EmptySoulGem, cs.misc_FilledSoulGem,
            cs.misc_AnimalParts, cs.misc_HidesPelts,
            cs.misc_Valuables, cs.misc_NonValuables,
            cs.misc_Leather, cs.misc_BuildingMaterials
        };

        std::vector<RE::TESObjectREFR*> result;
        for (auto& key : allAssigned)
        {
            if (key.empty() || skipSet.count(key)) continue;
            for (auto& ci : data.containers)
            {
                if (CellManager::MatchContainer(ci, key))
                {
                    auto* ref = RE::TESForm::LookupByID<RE::TESObjectREFR>(ci.refFormID);
                    if (ref && ref->Is3DLoaded())
                        result.push_back(ref);
                    break;
                }
            }
        }
        return result;
    }

    void MergeContainers(std::vector<RE::TESObjectREFR*>& primary,
                         std::vector<RE::TESObjectREFR*>& fallback)
    {
        primary.insert(primary.end(), fallback.begin(), fallback.end());
    }

    void ResupplyFromList(const std::vector<CachedItemEntry>& list, int amount,
                          std::vector<RE::TESObjectREFR*>& containers)
    {
        if (amount <= 0) return;

        auto invMap = BuildInventoryMap(containers);

        int remaining = amount;
        for (auto& entry : list)
        {
            if (remaining <= 0) break;

            auto* form = RE::TESForm::LookupByID<RE::TESBoundObject>(entry.formID);
            if (!form || IsPhantomItem(form)) continue;

            int count = GetTotalAvailable(form, invMap);
            if (count <= 0) continue;

            int toTake = std::min(remaining, count);
            int queued = QueueFromContainers(form, toTake, containers, &invMap);
            remaining -= queued;
        }
    }

    void ResupplyFromListSorted(const std::vector<CachedItemEntry>& list, int amount,
                                std::vector<RE::TESObjectREFR*>& containers,
                                bool prioritizeStrongest,
                                float (*getScore)(RE::TESBoundObject*))
    {
        if (amount <= 0) return;

        auto invMap = BuildInventoryMap(containers);

        std::vector<ScoredItem> candidates;
        for (auto& entry : list)
        {
            auto* form = RE::TESForm::LookupByID<RE::TESBoundObject>(entry.formID);
            if (!form || IsPhantomItem(form)) continue;

            int count = GetTotalAvailable(form, invMap);
            if (count <= 0) continue;

            float score = getScore ? getScore(form) : 0.0f;
            candidates.push_back({form, score, count});
        }

        if (prioritizeStrongest)
            std::sort(candidates.begin(), candidates.end(),
                [](const ScoredItem& a, const ScoredItem& b) { return a.score > b.score; });
        else
            std::sort(candidates.begin(), candidates.end(),
                [](const ScoredItem& a, const ScoredItem& b) { return a.score < b.score; });

        int remaining = amount;
        for (auto& c : candidates)
        {
            if (remaining <= 0) break;
            int toTake = std::min(remaining, c.available);
            int queued = QueueFromContainers(c.form, toTake, containers, &invMap);
            remaining -= queued;
        }
    }

    float GetPotionScore(RE::TESBoundObject* form)
    {
        auto* alch = form->As<RE::AlchemyItem>();
        if (!alch) return 0.0f;
        auto* kwH = FormCache::kwMagicAlchRestoreHealth();
        auto* kwM = FormCache::kwMagicAlchRestoreMagicka();
        auto* kwS = FormCache::kwMagicAlchRestoreStamina();
        float mag = 0.0f;
        for (auto* effect : alch->effects)
        {
            if (!effect || !effect->baseEffect) continue;
            auto* mgef = effect->baseEffect;
            if ((kwH && mgef->HasKeyword(kwH)) ||
                (kwM && mgef->HasKeyword(kwM)) ||
                (kwS && mgef->HasKeyword(kwS)))
            {
                mag = effect->effectItem.magnitude;
                break;
            }
        }
        return mag;
    }

    float GetAmmoScore(RE::TESBoundObject* form)
    {
        auto* ammo = form->As<RE::TESAmmo>();
        if (!ammo) return 0.0f;
        return ammo->GetRuntimeData().data.damage;
    }

    float GetDrinkScore(RE::TESBoundObject* form)
    {
        auto* alch = form->As<RE::AlchemyItem>();
        if (!alch) return 0.0f;
        auto* mgefThirst = FormCache::mgefThirstRestore();
        return mgefThirst ? Categorizer::GetAlchemyEffectMagnitudeByMGEF(alch, mgefThirst) : 0.0f;
    }

    void ResupplyPotionsSorted(const std::vector<CachedItemEntry>& list, int amount,
                               std::vector<RE::TESObjectREFR*>& containers, bool prioritize)
    {
        ResupplyFromListSorted(list, amount, containers, prioritize, GetPotionScore);
    }

    void ResupplyAmmoSorted(const std::vector<CachedItemEntry>& list, int amount,
                            std::vector<RE::TESObjectREFR*>& containers, bool prioritize)
    {
        ResupplyFromListSorted(list, amount, containers, prioritize, GetAmmoScore);
    }

    void ResupplyFoodDrinkFromList(const std::vector<CachedItemEntry>& list, int amount,
                                   std::vector<RE::TESObjectREFR*>& containers,
                                   bool prioritize, bool isDrink)
    {
        if (amount <= 0) return;

        auto& s = Settings::GetSingleton();
        auto* mgefAlcohol = FormCache::mgefAlcohol();
        auto invMap = BuildInventoryMap(containers);

        std::vector<ScoredItem> candidates;
        std::vector<ScoredItem> alcoholCandidates;

        for (auto& entry : list)
        {
            auto* form = RE::TESForm::LookupByID<RE::AlchemyItem>(entry.formID);
            if (!form || IsPhantomItem(form)) continue;

            int count = GetTotalAvailable(form, invMap);
            if (count <= 0) continue;

            float score = isDrink ? GetDrinkScore(form) : 0.0f;

            if (isDrink && mgefAlcohol)
            {
                bool isAlc = Categorizer::HasAlchemyEffectMGEF(form, mgefAlcohol);
                if (isAlc)
                    alcoholCandidates.push_back({form, score, count});
                else
                    candidates.push_back({form, score, count});
            }
            else
            {
                candidates.push_back({form, score, count});
            }
        }

        if (isDrink && candidates.empty() && s.allowAlcohol.load())
            candidates = std::move(alcoholCandidates);

        if (prioritize)
            std::sort(candidates.begin(), candidates.end(),
                [](const ScoredItem& a, const ScoredItem& b) { return a.score > b.score; });
        else
            std::sort(candidates.begin(), candidates.end(),
                [](const ScoredItem& a, const ScoredItem& b) { return a.score < b.score; });

        int remaining = amount;
        for (auto& c : candidates)
        {
            if (remaining <= 0) break;
            int toTake = std::min(remaining, c.available);
            int queued = QueueFromContainers(c.form, toTake, containers, &invMap);
            remaining -= queued;
        }
    }

    void ResupplyLockpicks(int cellIndex, int amount, const std::string& miscKey, const std::string& masterChestKey)
    {
        if (amount <= 0) return;

        auto* lockpickForm = FormCache::formLockpick();
        if (!lockpickForm) return;

        auto* lp = lockpickForm->As<RE::TESBoundObject>();
        if (!lp) return;

        auto containers = ResolveContainers(cellIndex,
            { miscKey, masterChestKey });
        if (containers.empty()) return;

        auto invMap = BuildInventoryMap(containers);
        int available = GetTotalAvailable(lp, invMap);
        if (available <= 0) return;

        int toTake = std::min(amount, available);
        QueueFromContainers(lp, toTake, containers);
    }

    void DoResupplyCustomItem(int cellIndex, const ResupplyCustomItem& custom)
    {
        if (custom.count <= 0 || custom.selectedEditorID.empty())
        {
            if (Settings::GetSingleton().debug.load())
                logger::info("ResupplyCustom: skip count={} edid='{}'", custom.count, custom.selectedEditorID);
            return;
        }

        auto& data = CellManager::GetCellData(cellIndex);

        std::vector<RE::TESObjectREFR*> containers;
        for (auto& ci : data.containers)
        {
            auto* ref = RE::TESForm::LookupByID<RE::TESObjectREFR>(ci.refFormID);
            if (ref && ref->Is3DLoaded())
                containers.push_back(ref);
        }
        if (containers.empty())
        {
            if (Settings::GetSingleton().debug.load())
                logger::info("ResupplyCustom: no containers");
            return;
        }

        RE::TESForm* form = nullptr;
        auto& allItems = FormCache::GetAllItems();
        for (auto& entry : allItems)
        {
            if (entry.editorID == custom.selectedEditorID ||
                std::format("{:08X}", entry.formID) == custom.selectedEditorID)
            {
                form = RE::TESForm::LookupByID(entry.formID);
                if (Settings::GetSingleton().debug.load())
                    logger::info("ResupplyCustom: found form {:08X} for edid '{}'", entry.formID, custom.selectedEditorID);
                break;
            }
        }

        if (!form)
        {
            if (Settings::GetSingleton().debug.load())
                logger::info("ResupplyCustom: no form found for edid '{}'", custom.selectedEditorID);
            return;
        }

        auto* boundObj = form->As<RE::TESBoundObject>();
        if (!boundObj) { if (Settings::GetSingleton().debug.load()) logger::info("ResupplyCustom: not bound object"); return; }

        auto invMap = BuildInventoryMap(containers);
        int available = GetTotalAvailable(boundObj, invMap);
        if (Settings::GetSingleton().debug.load())
            logger::info("ResupplyCustom: need={} available={}", custom.count, available);
        if (available <= 0) return;

        int toTake = std::min(custom.count, available);
        QueueFromContainers(boundObj, toTake, containers);
        if (Settings::GetSingleton().debug.load())
            logger::info("ResupplyCustom: gave {}x {}", toTake, boundObj->GetName());
    }
}

void ResupplyLogic::ExecuteResupply()
{
    bool expected = false;
    if (!g_operationRunning.compare_exchange_strong(expected, true))
    {
        if (Settings::GetSingleton().debug.load())
            logger::info("Resupply: Skipped, stash already running");
        return;
    }

    auto* player = RE::PlayerCharacter::GetSingleton();
    if (!player) { g_operationRunning.store(false); return; }

    int cellIndex = CellManager::FindActiveCellIndex();
    if (cellIndex < 0) { g_operationRunning.store(false); return; }

    auto& s = Settings::GetSingleton();
    auto& cs = s.cells[cellIndex];
    if (s.debug.load())
        logger::info("Resupply: Executing in cell {}", cellIndex + 1);

    auto preset = [&](const std::string& direct, const std::string& presetField) -> std::string {
        if (!direct.empty()) return direct;
        auto* p = Settings::GetActivePreset(cellIndex);
        if (!p) return "";
        if (presetField == "potions") return p->potions;
        if (presetField == "poisons") return p->poisons;
        if (presetField == "ammo_Arrows") return p->ammo_Arrows;
        if (presetField == "ammo_Bolts") return p->ammo_Bolts;
        if (presetField == "consumables") return p->consumables;
        if (presetField == "consumables_Cooked") return p->consumables_Cooked;
        if (presetField == "consumables_Raw") return p->consumables_Raw;
        if (presetField == "consumables_CookedMeat") return p->consumables_CookedMeat;
        if (presetField == "consumables_RawMeat") return p->consumables_RawMeat;
        if (presetField == "consumables_ProduceGrains") return p->consumables_ProduceGrains;
        if (presetField == "consumables_Cheese") return p->consumables_Cheese;
        if (presetField == "consumables_Drinks") return p->consumables_Drinks;
        if (presetField == "consumables_NonAlcoholicDrinks") return p->consumables_NonAlcoholicDrinks;
        if (presetField == "consumables_Alcohol") return p->consumables_Alcohol;
        if (presetField == "misc") return p->misc;
        if (presetField == "masterChestEditorID") return p->masterChestEditorID;
        return "";
    };

    std::string pkPotions = preset(cs.potions, "potions");
    std::string pkPoisons = preset(cs.poisons, "poisons");
    std::string pkArrows = preset(cs.ammo_Arrows, "ammo_Arrows");
    std::string pkBolts = preset(cs.ammo_Bolts, "ammo_Bolts");
    std::string pkConsumables = preset(cs.consumables, "consumables");
    std::string pkCooked = preset(cs.consumables_Cooked, "consumables_Cooked");
    std::string pkRaw = preset(cs.consumables_Raw, "consumables_Raw");
    std::string pkCookedMeat = preset(cs.consumables_CookedMeat, "consumables_CookedMeat");
    std::string pkRawMeat = preset(cs.consumables_RawMeat, "consumables_RawMeat");
    std::string pkProduce = preset(cs.consumables_ProduceGrains, "consumables_ProduceGrains");
    std::string pkCheese = preset(cs.consumables_Cheese, "consumables_Cheese");
    std::string pkDrinks = preset(cs.consumables_Drinks, "consumables_Drinks");
    std::string pkNonAlc = preset(cs.consumables_NonAlcoholicDrinks, "consumables_NonAlcoholicDrinks");
    std::string pkAlcohol = preset(cs.consumables_Alcohol, "consumables_Alcohol");
    std::string pkMisc = preset(cs.misc, "misc");

    g_supplyQueue.clear();
    g_supplyIndex = 0;

    auto potionContainers = ResolveContainers(cellIndex,
        { pkPotions });
    auto potionFallback = ResolveAllAssignedContainers(cellIndex,
        { pkPotions });
    MergeContainers(potionContainers, potionFallback);

    auto poisonContainers = ResolveContainers(cellIndex,
        { pkPoisons });
    auto poisonFallback = ResolveAllAssignedContainers(cellIndex,
        { pkPoisons });
    MergeContainers(poisonContainers, poisonFallback);

    auto arrowContainers = ResolveContainers(cellIndex,
        { pkArrows });
    auto arrowFallback = ResolveAllAssignedContainers(cellIndex,
        { pkArrows });
    MergeContainers(arrowContainers, arrowFallback);

    auto boltContainers = ResolveContainers(cellIndex,
        { pkBolts });
    auto boltFallback = ResolveAllAssignedContainers(cellIndex,
        { pkBolts });
    MergeContainers(boltContainers, boltFallback);

    auto foodContainers = ResolveContainers(cellIndex,
        { pkConsumables, pkCooked, pkRaw, pkCookedMeat, pkRawMeat, pkProduce, pkCheese });
    auto foodFallback = ResolveAllAssignedContainers(cellIndex,
        { pkConsumables, pkCooked, pkRaw, pkCookedMeat, pkRawMeat, pkProduce, pkCheese });
    MergeContainers(foodContainers, foodFallback);

    auto drinkContainers = ResolveContainers(cellIndex,
        { pkConsumables, pkDrinks, pkNonAlc, pkAlcohol });
    auto drinkFallback = ResolveAllAssignedContainers(cellIndex,
        { pkConsumables, pkDrinks, pkNonAlc, pkAlcohol });
    MergeContainers(drinkContainers, drinkFallback);

    ResupplyPotionsSorted(FormCache::GetHealingPotions(),
        s.resupplyHealingPotions.load(), potionContainers, s.prioritizeStrongestHealing.load());
    ResupplyPotionsSorted(FormCache::GetMagickaPotions(),
        s.resupplyMagickaPotions.load(), potionContainers, s.prioritizeStrongestMagicka.load());
    ResupplyPotionsSorted(FormCache::GetStaminaPotions(),
        s.resupplyStaminaPotions.load(), potionContainers, s.prioritizeStrongestStamina.load());

    ResupplyFromList(FormCache::GetCurePoisonPotions(),
        s.resupplyCurePoison.load(), potionContainers);
    ResupplyFromList(FormCache::GetCureDiseasePotions(),
        s.resupplyCureDisease.load(), potionContainers);

    ResupplyAmmoSorted(FormCache::GetArrows(),
        s.resupplyArrows.load(), arrowContainers, s.prioritizeStrongestArrow.load());
    ResupplyAmmoSorted(FormCache::GetBolts(),
        s.resupplyBolts.load(), boltContainers, s.prioritizeStrongestBolt.load());

    ResupplyLockpicks(cellIndex, s.resupplyLockpicks.load(), pkMisc, preset(cs.masterChestEditorID, "masterChestEditorID"));

    ResupplyFoodDrinkFromList(FormCache::GetFoodItems(),
        s.resupplyFood.load(), foodContainers, s.prioritizeFillingFood.load(), false);
    ResupplyFoodDrinkFromList(FormCache::GetDrinkItems(),
        s.resupplyDrink.load(), drinkContainers, s.prioritizeHydratingDrink.load(), true);

    {
        int foodAmount = s.resupplyCookedFood.load();
        if (foodAmount > 0)
        {
            auto invMap = BuildInventoryMap(foodContainers);
            int remaining = foodAmount;

            auto takeFromList = [&](const std::vector<CachedItemEntry>& list) {
                for (auto& entry : list)
                {
                    if (remaining <= 0) break;
                    auto* form = RE::TESForm::LookupByID<RE::TESBoundObject>(entry.formID);
                    if (!form || IsPhantomItem(form)) continue;
                    auto it = invMap.find(form);
                    if (it == invMap.end() || it->second <= 0) continue;
                    int toTake = std::min(remaining, it->second);
                    int queued = QueueFromContainers(form, toTake, foodContainers);
                    remaining -= queued;
                    it->second -= queued;
                }
            };

            takeFromList(FormCache::GetCookedFoodItems());
            takeFromList(FormCache::GetRawFoodItems());
        }
    }

    for (int i = 0; i < 8; ++i)
        DoResupplyCustomItem(cellIndex, s.resupplyCustom[i]);

    if (g_supplyQueue.empty())
    {
        if (s.debug.load())
            logger::info("Resupply: No items to give.");
        g_operationRunning.store(false);
        return;
    }

    ProcessResupplyTick();
}
