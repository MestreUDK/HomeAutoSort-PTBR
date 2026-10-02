#include "pch.h"
#include "PromptManager.h"
#include "CellManager.h"
#include "Settings.h"
#include "InputHandler.h"
#include "StashLogic.h"
#include "ResupplyLogic.h"
#include "Localization.h"

extern std::atomic<int> g_sessionID;

namespace
{
    SkyPromptAPI::ClientID g_clientID = 0;
    std::atomic<bool> g_showing{ false };
    std::atomic<RE::FormID> g_currentRefID{ 0 };

    struct StashPromptSink : public SkyPromptAPI::PromptSink
    {
        SkyPromptAPI::Prompt m_prompts[2];

        StashPromptSink()
        {
            m_prompts[0].text = "Hold to Stash";
            m_prompts[0].eventID = 8733;
            m_prompts[0].actionID = 0;
            m_prompts[0].type = SkyPromptAPI::PromptType::kHold;
            m_prompts[0].refid = 0;
            m_prompts[0].text_color = 0xFFFFFFFF;
            m_prompts[0].progress = 0.0f;

            m_prompts[1].text = "Hold to Resupply";
            m_prompts[1].eventID = 8734;
            m_prompts[1].actionID = 1;
            m_prompts[1].type = SkyPromptAPI::PromptType::kHold;
            m_prompts[1].refid = 0;
            m_prompts[1].text_color = 0xFFFFFFFF;
            m_prompts[1].progress = 0.0f;
        }

        std::span<const SkyPromptAPI::Prompt> GetPrompts() const override
        {
            return std::span<const SkyPromptAPI::Prompt>(m_prompts, 2);
        }

        void ProcessEvent(SkyPromptAPI::PromptEvent a_event) const override
        {
            if (a_event.type == SkyPromptAPI::PromptEventType::kAccepted)
            {
                int sid = g_sessionID.load();
                if (a_event.prompt.eventID == 8733)
                {
                    SKSE::GetTaskInterface()->AddTask([sid]() {
                        if (g_sessionID.load() != sid) return;
                        StashLogic::ExecuteStash();
                    });
                }
                else if (a_event.prompt.eventID == 8734)
                {
                    SKSE::GetTaskInterface()->AddTask([sid]() {
                        if (g_sessionID.load() != sid) return;
                        ResupplyLogic::ExecuteResupply();
                    });
                }
            }
        }
    };

    StashPromptSink g_sink;

    std::pair<RE::INPUT_DEVICE, SkyPromptAPI::ButtonID> g_stashKeys[2];
    std::pair<RE::INPUT_DEVICE, SkyPromptAPI::ButtonID> g_resupplyKeys[2];

    void BuildButtonKeys()
    {
        auto& s = Settings::GetSingleton();
        uint32_t kbStash = s.keyboardStashHotkey.load();
        uint32_t gpStash = s.gamepadStashHotkey.load();
        uint32_t kbResupply = s.keyboardResupplyHotkey.load();
        uint32_t gpResupply = s.gamepadResupplyHotkey.load();

        g_stashKeys[0] = {RE::INPUT_DEVICE::kKeyboard, static_cast<SkyPromptAPI::ButtonID>(kbStash)};
        g_stashKeys[1] = {RE::INPUT_DEVICE::kGamepad, static_cast<SkyPromptAPI::ButtonID>(gpStash)};
        g_resupplyKeys[0] = {RE::INPUT_DEVICE::kKeyboard, static_cast<SkyPromptAPI::ButtonID>(kbResupply)};
        g_resupplyKeys[1] = {RE::INPUT_DEVICE::kGamepad, static_cast<SkyPromptAPI::ButtonID>(gpResupply)};
    }

    class CrosshairSink : public RE::BSTEventSink<SKSE::CrosshairRefEvent>
    {
    public:
        static CrosshairSink& GetSingleton()
        {
            static CrosshairSink instance;
            return instance;
        }

        RE::BSEventNotifyControl ProcessEvent(const SKSE::CrosshairRefEvent* a_event,
            RE::BSTEventSource<SKSE::CrosshairRefEvent>*) override
        {
            if (!a_event) return RE::BSEventNotifyControl::kContinue;

            auto ref = a_event->crosshairRef;
            if (ref)
            {
                RE::FormID refFormID = ref->GetFormID();
                auto& s = Settings::GetSingleton();
                for (int i = 0; i < 5; ++i)
                {
                    auto& cs = s.cells[i];
                    std::string masterKey = cs.masterChestEditorID;
                    if (masterKey.empty())
                    {
                        auto* preset = Settings::GetActivePreset(i);
                        if (preset) masterKey = preset->masterChestEditorID;
                    }
                    if (masterKey.empty()) continue;

                    auto& data = CellManager::GetCellData(i);
                    std::uint32_t resolvedRefID = 0;
                    for (auto& ci : data.containers)
                    {
                        if (CellManager::MatchContainer(ci, masterKey))
                        {
                            resolvedRefID = ci.refFormID;
                            break;
                        }
                    }
                    if (s.debug.load())
                        logger::debug("Crosshair: cell {} homeSet={} resolvedRefID=0x{:06X} lookingAt=0x{:06X}", i, cs.homeSet, resolvedRefID & 0xFFFFFF, refFormID & 0xFFFFFF);
                    if (resolvedRefID != 0 && resolvedRefID == refFormID)
                    {
                        if (s.debug.load())
                            logger::info("Crosshair: MATCH cell {} master chest!", i);
                        PromptManager::ShowForRef(ref.get());
                        return RE::BSEventNotifyControl::kContinue;
                    }
                }
                if (s.debug.load())
                    logger::debug("Crosshair: no match, ref=0x{:06X}", refFormID & 0xFFFFFF);
            }

            PromptManager::Hide();
            return RE::BSEventNotifyControl::kContinue;
        }
    };
}

void PromptManager::Init()
{
    g_clientID = SkyPromptAPI::RequestClientID();
    if (g_clientID == 0)
    {
        logger::warn("PromptManager: SkyPrompt not available, prompts disabled");
        return;
    }

    g_sink.m_prompts[0].text = Localization::Get("$Prompts", "stash", "Hold to Stash");
    g_sink.m_prompts[1].text = Localization::Get("$Prompts", "resupply", "Hold to Resupply");

    BuildButtonKeys();

    auto* crosshairSrc = SKSE::GetCrosshairRefEventSource();
    if (crosshairSrc)
        crosshairSrc->AddEventSink(&CrosshairSink::GetSingleton());

    if (Settings::GetSingleton().debug.load())
        logger::info("PromptManager: Init with clientID={}", g_clientID);

    (void)SkyPromptAPI::RequestTheme(g_clientID, "HomeAutoSort");
}

void PromptManager::Shutdown()
{
    if (g_showing.load())
    {
        SkyPromptAPI::RemovePrompt(&g_sink, g_clientID);
        g_showing.store(false);
    }

    auto* crosshairSrc = SKSE::GetCrosshairRefEventSource();
    if (crosshairSrc)
        crosshairSrc->RemoveEventSink(&CrosshairSink::GetSingleton());
}

void PromptManager::ShowForRef(RE::TESObjectREFR* ref)
{
    if (!ref) return;

    BuildButtonKeys();

    RE::FormID refID = ref->GetFormID();

    g_sink.m_prompts[0].refid = refID;
    g_sink.m_prompts[0].button_key = std::span<const std::pair<RE::INPUT_DEVICE, SkyPromptAPI::ButtonID>>(g_stashKeys, 2);
    g_sink.m_prompts[1].refid = refID;
    g_sink.m_prompts[1].button_key = std::span<const std::pair<RE::INPUT_DEVICE, SkyPromptAPI::ButtonID>>(g_resupplyKeys, 2);

    (void)SkyPromptAPI::SendPrompt(&g_sink, g_clientID);
    g_showing.store(true);
    g_currentRefID.store(refID);
}

void PromptManager::Hide()
{
    if (g_showing.load())
    {
        SkyPromptAPI::RemovePrompt(&g_sink, g_clientID);
        g_showing.store(false);
        g_currentRefID.store(0);
    }
}

void PromptManager::UpdateStashProgress(float progress)
{
    g_sink.m_prompts[1].progress = std::clamp(progress, 0.0f, 1.0f);
    if (g_showing.load())
        (void)SkyPromptAPI::SendPrompt(&g_sink, g_clientID);
}

bool PromptManager::IsShowing()
{
    return g_showing.load();
}

SkyPromptAPI::ClientID PromptManager::GetClientID()
{
    return g_clientID;
}
