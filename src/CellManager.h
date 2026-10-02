#pragma once

#include "plugin.h"

struct ContainerInfo
{
    std::string       name;
    std::string       editorID;
    RE::FormID        baseFormID = 0;
    RE::FormID        refFormID = 0;
};

struct CellHomeData
{
    bool              homeSet = false;
    std::string       cellName;
    std::string       cellEditorID;
    RE::FormID        cellFormID = 0;
    bool              isInterior = false;
    std::vector<ContainerInfo> containers;
};

namespace CellManager
{
    void ScanContainers(int cellIndex, bool setHomeInSettings = true);
    const CellHomeData& GetCellData(int cellIndex);
    void ClearCellData(int cellIndex);
    bool IsHomeSet(int cellIndex);
    bool IsScanned(int cellIndex);

    RE::TESObjectREFR* FindMasterChest(int cellIndex);
    int FindActiveCellIndex();

    bool IsContainer3DLoaded(RE::TESObjectREFR* ref);

    RE::TESObjectCELL* GetPlayerParentCell();
    std::string GetCellDisplayName(RE::TESObjectCELL* cell);

    inline bool MatchContainer(const ContainerInfo& ci, const std::string& key)
    {
        if (key.empty()) return false;

        auto pipePos = key.find('|');
        if (pipePos != std::string::npos)
        {
            std::string hexPart = key.substr(0, pipePos);
            std::string rest = key.substr(pipePos + 1);
            auto pipe2 = rest.find('|');
            std::string edidPart = (pipe2 != std::string::npos) ? rest.substr(0, pipe2) : rest;
            std::string namePart = (pipe2 != std::string::npos) ? rest.substr(pipe2 + 1) : "";

            std::string live6 = std::format("{:06X}", ci.refFormID & 0xFFFFFF);
            std::string live3 = std::format("{:03X}", ci.refFormID & 0x00000FFF);

            if (!edidPart.empty())
            {
                if (live6 == hexPart && ci.editorID == edidPart)
                    return true;
                if (live3 == hexPart.substr(hexPart.length() >= 3 ? hexPart.length() - 3 : 0) && ci.editorID == edidPart)
                    return true;
                return false;
            }

            if (!namePart.empty() && ci.name == namePart)
                return true;

            return live6 == hexPart || live3 == hexPart.substr(hexPart.length() >= 3 ? hexPart.length() - 3 : 0);
        }

        if (!ci.editorID.empty() && ci.editorID == key) return true;
        if (std::format("{:06X}", ci.refFormID & 0xFFFFFF) == key) return true;
        if (std::format("{:03X}", ci.refFormID & 0x00000FFF) == key) return true;
        if (std::format("{:08X}", ci.refFormID) == key) return true;
        if (std::format("{:08X}", ci.baseFormID) == key) return true;
        return false;
    }

    inline std::string ContainerKey(const ContainerInfo& ci)
    {
        return std::format("{:06X}|{}|{}", ci.refFormID & 0xFFFFFF, ci.editorID, ci.name);
    }

    inline std::string ContainerLabel(const ContainerInfo& ci)
    {
        return ci.name + " [0x" + std::format("{:06X}", ci.refFormID & 0xFFFFFF) + "]";
    }
}
