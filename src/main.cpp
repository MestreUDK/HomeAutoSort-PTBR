#include "pch.h"
#include "plugin.h"
#include "Settings.h"
#include "FormCache.h"
#include "CellManager.h"
#include "StashLogic.h"
#include "ResupplyLogic.h"
#include "PromptManager.h"
#include "InputHandler.h"
#include "UI.h"

std::atomic<int>  g_sessionID{ 0 };
std::atomic<bool> g_operationRunning{ false };

namespace
{
    std::atomic<bool> g_shutdown{ false };
    std::chrono::steady_clock::time_point g_lastHomeCheck;

    void MonitorLoop()
    {

        while (!g_shutdown.load())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));

            auto* ui = RE::UI::GetSingleton();
            bool paused = (ui && (ui->GameIsPaused() || ui->IsApplicationMenuOpen()));

            if (paused) continue;

            auto now = std::chrono::steady_clock::now();
            if (now - g_lastHomeCheck >= std::chrono::seconds(1))
            {
                g_lastHomeCheck = now;
                int homeIdx = CellManager::FindActiveCellIndex();
                if (homeIdx >= 0)
                {
                    if (!CellManager::IsHomeSet(homeIdx) || !CellManager::IsScanned(homeIdx))
                    {
                        SKSE::GetTaskInterface()->AddTask([homeIdx]() {
                            int idx = homeIdx;
                            auto& s = Settings::GetSingleton();
                            auto& cs = s.cells[idx];
                            bool setHome = true;
                            auto* player = RE::PlayerCharacter::GetSingleton();
                            if (player)
                            {
                                auto* cell = player->GetParentCell();
                                if (cell)
                                {
                                    const char* cedid = cell->GetFormEditorID();
                                    std::string currentEdid = cedid ? cedid : "";
                                    if (!cs.homeSet)
                                    {
                                        auto* preset = Settings::FindPresetInCache(currentEdid);
                                        if (preset)
                                        {
                                            Settings::SetActivePreset(idx, *preset);
                                            setHome = false;
                                        }
                                    }
                                    else if (currentEdid != cs.homeEditorID)
                                    {
                                        auto* preset = Settings::FindPresetInCache(currentEdid);
                                        if (preset)
                                        {
                                            Settings::SetActivePreset(idx, *preset);
                                            setHome = false;
                                        }
                                    }
                                }
                            }
                            CellManager::ScanContainers(idx, setHome);
                        });
                    }
                }
            }

            if (!PromptManager::IsShowing()) continue;

            int cellIndex = CellManager::FindActiveCellIndex();
            if (cellIndex < 0) continue;
        }
    }

    void InitLogger()
    {
        auto path = SKSE::log::log_directory();
        if (!path) return;

        *path /= std::format("{}.log", Plugin::NAME);

        auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
        auto log  = std::make_shared<spdlog::logger>(std::string(Plugin::NAME), std::move(sink));
        log->set_level(spdlog::level::info);
        log->flush_on(spdlog::level::info);
        spdlog::set_default_logger(std::move(log));
    }

    void MessageHandler(SKSE::MessagingInterface::Message* msg)
    {
        if (!msg) return;

        switch (msg->type)
        {
        case SKSE::MessagingInterface::kDataLoaded:
        {
            InitLogger();
            logger::info("{} v{} loaded (build 3)", Plugin::NAME, Plugin::VERSION);

            auto& settings = Settings::GetSingleton();
            settings.Load();

            FormCache::Build();
            UI::Register();
            PromptManager::Init();
            Settings::LoadPresetCache();

            std::thread(MonitorLoop).detach();

            if (settings.debug.load())
                logger::info("Data loaded, systems initialized");
            break;
        }

        case SKSE::MessagingInterface::kPreLoadGame:
        {
            g_sessionID.fetch_add(1);
            PromptManager::Hide();

            if (Settings::GetSingleton().debug.load())
                logger::info("Pre-load: state reset, session={}", g_sessionID.load());
            break;
        }

        case SKSE::MessagingInterface::kPostLoadGame:
        {
            g_sessionID.fetch_add(1);

            auto& settings = Settings::GetSingleton();
            auto* player = RE::PlayerCharacter::GetSingleton();
            if (player)
            {
                auto* cell = player->GetParentCell();
                if (cell)
                {
                    std::string currentEdid;
                    const char* cedid = cell->GetFormEditorID();
                    if (cedid) currentEdid = cedid;
                    RE::FormID currentFormID = cell->GetFormID();

                    for (int i = 0; i < 5; ++i)
                    {
                        auto& cs = settings.cells[i];
                        if (!cs.homeSet) continue;
                        if ((!cs.homeEditorID.empty() && cs.homeEditorID == currentEdid) ||
                            (cs.homeFormID != 0 && cs.homeFormID == currentFormID))
                        {
                            CellManager::ScanContainers(i);
                        }
                    }

                    auto* preset = Settings::FindPresetInCache(currentEdid);
                    if (preset)
                    {
                        for (int i = 0; i < 5; ++i)
                        {
                            auto& cs = settings.cells[i];
                            bool hasConfig = !cs.masterChestEditorID.empty() || !cs.weapons.empty();
                            if (!cs.homeSet && !hasConfig)
                            {
                                Settings::SetActivePreset(i, *preset);
                                CellManager::ScanContainers(i, false);
                                if (settings.debug.load())
                                    logger::info("Post-load: active preset for cell {}", currentEdid);
                                break;
                            }
                        }
                    }
                }
            }

            if (settings.debug.load())
                logger::info("Post-load: auto-rescan, session={}", g_sessionID.load());
            break;
        }

        case SKSE::MessagingInterface::kNewGame:
        {
            g_sessionID.fetch_add(1);
            break;
        }
        }
    }
}

SKSEPluginLoad(const SKSE::LoadInterface* skse)
{
    SKSE::Init(skse);

    auto* msg = SKSE::GetMessagingInterface();
    if (msg) msg->RegisterListener(MessageHandler);

    return true;
}
