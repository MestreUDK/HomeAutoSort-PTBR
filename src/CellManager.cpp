#include "pch.h"
#include "CellManager.h"
#include "Settings.h"
#include "FormCache.h"

namespace
{
    CellHomeData g_cellData[5];
    std::mutex g_cellMutex[5];
    std::atomic<bool> g_homeSet[5]{ false, false, false, false, false };
    std::atomic<bool> g_scanned[5]{ false, false, false, false, false };
}

void CellManager::ScanContainers(int cellIndex, bool setHomeInSettings)
{
    if (cellIndex < 0 || cellIndex >= 5) return;

    std::lock_guard<std::mutex> lock(g_cellMutex[cellIndex]);
    auto& data = g_cellData[cellIndex];
    data.containers.clear();

    auto* player = RE::PlayerCharacter::GetSingleton();
    if (!player) return;

    auto* cell = player->GetParentCell();
    if (!cell) return;

    data.cellName = GetCellDisplayName(cell);
    const char* edid = cell->GetFormEditorID();
    data.cellEditorID = edid ? edid : "";
    data.cellFormID = cell->GetFormID();
    data.isInterior = cell->IsInteriorCell();
    data.homeSet = true;

    g_homeSet[cellIndex].store(true);
    g_scanned[cellIndex].store(true);

    cell->ForEachReference([&](RE::TESObjectREFR* ref) {
        if (!ref) return RE::BSContainer::ForEachResult::kContinue;

        if (ref->IsDisabled() || ref->IsDeleted())
            return RE::BSContainer::ForEachResult::kContinue;

        auto* base = ref->GetBaseObject();
        if (!base) return RE::BSContainer::ForEachResult::kContinue;

        if (base->GetFormType() != RE::FormType::Container)
            return RE::BSContainer::ForEachResult::kContinue;

        if (ref->GetFormFlags() & static_cast<std::uint32_t>(RE::TESObjectREFR::RecordFlags::kRespawns))
            return RE::BSContainer::ForEachResult::kContinue;

        auto* cont = base->As<RE::TESObjectCONT>();
        if (cont && cont->data.flags.any(RE::CONT_DATA::Flag::kRespawn))
            return RE::BSContainer::ForEachResult::kContinue;

        ContainerInfo info;
        info.name = ref->GetName();
        if (info.name.empty())
        {
            info.name = base->GetName();
        }
        const char* baseEdid = base->GetFormEditorID();
        info.editorID = FormCache::GetEditorIDForForm(base->GetFormID());
        if (info.editorID.empty() && baseEdid && baseEdid[0] != '\0')
            info.editorID = baseEdid;
        info.baseFormID = base->GetFormID();
        info.refFormID = ref->GetFormID();

        if (info.name.empty())
        {
            if (!info.editorID.empty())
                info.name = info.editorID;
            else
                return RE::BSContainer::ForEachResult::kContinue;
        }

        auto* owner = ref->GetOwner();
        if (owner)
        {
            auto& s = Settings::GetSingleton();
            if (s.debug.load())
                logger::warn("CellManager: Container '{}' is owned, may mark items stolen", info.name);
        }

        data.containers.push_back(std::move(info));
        return RE::BSContainer::ForEachResult::kContinue;
    });

    auto& settings = Settings::GetSingleton();
    auto& cellSettings = settings.cells[cellIndex];
    if (setHomeInSettings)
    {
        cellSettings.homeEditorID = data.cellEditorID;
        cellSettings.homeFormID = data.cellFormID;
        cellSettings.homeSet = true;
    }

    std::string masterEdid = cellSettings.masterChestEditorID;
    if (masterEdid.empty())
    {
        auto* preset = Settings::GetActivePreset(cellIndex);
        if (preset) masterEdid = preset->masterChestEditorID;
    }
    if (settings.debug.load())
        logger::info("Cell {}: masterEdid='{}'", cellIndex + 1, masterEdid);
    if (!masterEdid.empty())
    {
        bool matched = false;
        for (auto& ci : data.containers)
        {
            if (CellManager::MatchContainer(ci, masterEdid))
            {
                matched = true;
                if (settings.debug.load())
                    logger::info("Cell {}: Master chest matched: '{}' [0x{:06X}] editorID='{}'", cellIndex + 1, ci.name, ci.refFormID & 0xFFFFFF, ci.editorID);
                break;
            }
        }
        if (!matched && settings.debug.load())
            logger::warn("Cell {}: Master chest NOT matched for key '{}'", cellIndex + 1, masterEdid);
    }
    else if (settings.debug.load())
        logger::warn("Cell {}: masterEdid empty", cellIndex + 1);

    std::sort(data.containers.begin(), data.containers.end(),
        [](const ContainerInfo& a, const ContainerInfo& b) {
            return _stricmp(a.name.c_str(), b.name.c_str()) < 0;
        });

    if (settings.debug.load())
    {
        for (auto& ci : data.containers)
        {
            const char* edidStr = ci.editorID.empty() ? "(none)" : ci.editorID.c_str();
            logger::info("  Container '{}' ref=0x{:08X} partial={:06X} edid={}", ci.name, ci.refFormID, ci.refFormID & 0xFFFFFF, edidStr);
        }
    }

    if (settings.debug.load())
        logger::info("Cell {}: Scanned {} containers in '{}'", cellIndex + 1, data.containers.size(), data.cellName);
}

const CellHomeData& CellManager::GetCellData(int cellIndex)
{
    static CellHomeData empty;
    if (cellIndex < 0 || cellIndex >= 5) return empty;
    return g_cellData[cellIndex];
}

void CellManager::ClearCellData(int cellIndex)
{
    if (cellIndex < 0 || cellIndex >= 5) return;
    std::lock_guard<std::mutex> lock(g_cellMutex[cellIndex]);
    g_cellData[cellIndex] = CellHomeData{};
    g_homeSet[cellIndex].store(false);
    g_scanned[cellIndex].store(false);
}

bool CellManager::IsHomeSet(int cellIndex)
{
    if (cellIndex < 0 || cellIndex >= 5) return false;
    return g_homeSet[cellIndex].load();
}

bool CellManager::IsScanned(int cellIndex)
{
    if (cellIndex < 0 || cellIndex >= 5) return false;
    return g_scanned[cellIndex].load();
}

int CellManager::FindActiveCellIndex()
{
    auto* player = RE::PlayerCharacter::GetSingleton();
    if (!player) return -1;
    auto* cell = player->GetParentCell();
    if (!cell) return -1;

    std::string currentEdid;
    const char* cedid = cell->GetFormEditorID();
    if (cedid) currentEdid = cedid;
    RE::FormID currentFormID = cell->GetFormID();

    auto& s = Settings::GetSingleton();
    for (int i = 0; i < 5; ++i)
    {
        auto& cs = s.cells[i];
        if (!cs.homeSet) continue;
        if (!cs.homeEditorID.empty() && cs.homeEditorID == currentEdid) return i;
        if (cs.homeFormID != 0 && cs.homeFormID == currentFormID) return i;
    }
    for (int i = 0; i < 5; ++i)
    {
        auto* preset = Settings::GetActivePreset(i);
        if (!preset) continue;
        if (!preset->homeEditorID.empty() && preset->homeEditorID == currentEdid)
            return i;
    }
    auto* cachedPreset = Settings::FindPresetInCache(currentEdid);
    if (cachedPreset && !cachedPreset->homeEditorID.empty())
    {
        for (int i = 0; i < 5; ++i)
        {
            auto* active = Settings::GetActivePreset(i);
            if (active && active->homeEditorID == currentEdid)
                return i;
        }
        for (int i = 0; i < 5; ++i)
        {
            if (!s.cells[i].homeSet) return i;
        }
        for (int i = 0; i < 5; ++i)
        {
            auto& data = GetCellData(i);
            if (data.cellEditorID != s.cells[i].homeEditorID)
            {
                ClearCellData(i);
                return i;
            }
        }
    }
    return -1;
}

RE::TESObjectREFR* CellManager::FindMasterChest(int cellIndex)
{
    if (cellIndex < 0 || cellIndex >= 5) return nullptr;

    auto& s = Settings::GetSingleton();
    auto& cs = s.cells[cellIndex];

    std::string masterID = cs.masterChestEditorID;
    if (masterID.empty())
    {
        auto* preset = Settings::GetActivePreset(cellIndex);
        if (preset) masterID = preset->masterChestEditorID;
    }
    if (masterID.empty()) return nullptr;

    auto* player = RE::PlayerCharacter::GetSingleton();
    if (!player) return nullptr;

    auto* currentCell = player->GetParentCell();
    if (!currentCell) return nullptr;

    std::string currentEdid;
    const char* cedid = currentCell->GetFormEditorID();
    if (cedid) currentEdid = cedid;

    if (currentEdid != g_cellData[cellIndex].cellEditorID &&
        currentCell->GetFormID() != g_cellData[cellIndex].cellFormID)
        return nullptr;

    auto& data = g_cellData[cellIndex];
    for (auto& ci : data.containers)
    {
        if (CellManager::MatchContainer(ci, masterID))
        {
            return RE::TESForm::LookupByID<RE::TESObjectREFR>(ci.refFormID);
        }
    }

    return nullptr;
}

RE::TESObjectCELL* CellManager::GetPlayerParentCell()
{
    auto* player = RE::PlayerCharacter::GetSingleton();
    if (!player) return nullptr;
    return player->GetParentCell();
}

std::string CellManager::GetCellDisplayName(RE::TESObjectCELL* cell)
{
    if (!cell) return "Unknown";

    if (cell->IsInteriorCell())
    {
        auto* name = cell->GetName();
        if (name && strlen(name) > 0) return name;

        const char* edid = cell->GetFormEditorID();
        if (edid) return edid;

        return "Interior";
    }

    auto* coords = cell->GetCoordinates();
    if (coords)
    {
        char buf[128];
        snprintf(buf, sizeof(buf), "Exterior (%d, %d)", coords->cellX, coords->cellY);
        return buf;
    }

    return "Exterior";
}
