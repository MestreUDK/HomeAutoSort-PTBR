#include "pch.h"
#include "InputHandler.h"
#include "Settings.h"

namespace
{
    class HotkeyInputSink : public RE::BSTEventSink<RE::InputEvent*>
    {
    public:
        static HotkeyInputSink& GetSingleton()
        {
            static HotkeyInputSink instance;
            return instance;
        }

        RE::BSEventNotifyControl ProcessEvent(RE::InputEvent* const* a_event,
            RE::BSTEventSource<RE::InputEvent*>*) override
        {
            if (!a_event) return RE::BSEventNotifyControl::kContinue;

            auto* ui = RE::UI::GetSingleton();
            if (ui && ui->GameIsPaused()) return RE::BSEventNotifyControl::kContinue;
            if (ui && ui->IsApplicationMenuOpen()) return RE::BSEventNotifyControl::kContinue;

            auto& s = Settings::GetSingleton();
            uint32_t kb = s.keyboardStashHotkey.load();
            uint32_t gp = s.gamepadStashHotkey.load();

            for (auto* e = *a_event; e; e = e->next)
            {
                if (e->GetEventType() != RE::INPUT_EVENT_TYPE::kButton) continue;
                auto* btn = static_cast<RE::ButtonEvent*>(e);

                uint32_t key = btn->GetIDCode();
                auto device = btn->GetDevice();

                if (device == RE::INPUT_DEVICE::kMouse) key += 256;
                else if (device == RE::INPUT_DEVICE::kGamepad) key = SKSE::InputMap::GamepadMaskToKeycode(key);

                bool match = false;
                if (kb != 0 && key == kb && device != RE::INPUT_DEVICE::kGamepad) match = true;
                if (gp != 0 && device == RE::INPUT_DEVICE::kGamepad && key == gp) match = true;
                if (!match) continue;

                if (btn->IsDown())
                {
                    if (!m_isHolding.load())
                    {
                        m_isHolding.store(true);
                        m_holdStart.store(std::chrono::steady_clock::now().time_since_epoch().count());
                    }
                    m_pressedEdge.store(true);
                }
                else if (btn->IsUp())
                {
                    m_isHolding.store(false);
                }
            }

            return RE::BSEventNotifyControl::kContinue;
        }

        std::atomic<bool> m_pressedEdge{ false };
        std::atomic<bool> m_isHolding{ false };
        std::atomic<std::chrono::steady_clock::time_point::rep> m_holdStart{ 0 };

    private:
        HotkeyInputSink() = default;
    };
}

void InputHandler::Register()
{
    auto* inputMgr = RE::BSInputDeviceManager::GetSingleton();
    if (inputMgr)
        inputMgr->AddEventSink(&HotkeyInputSink::GetSingleton());
}

bool InputHandler::IsPressed()
{
    bool val = HotkeyInputSink::GetSingleton().m_pressedEdge.load();
    return val;
}

bool InputHandler::IsHolding()
{
    return HotkeyInputSink::GetSingleton().m_isHolding.load();
}

float InputHandler::GetHoldDuration()
{
    auto& sink = HotkeyInputSink::GetSingleton();
    if (!sink.m_isHolding.load()) return 0.0f;
    auto start = std::chrono::steady_clock::time_point(std::chrono::steady_clock::duration(sink.m_holdStart.load()));
    return std::chrono::duration<float>(std::chrono::steady_clock::now() - start).count();
}

void InputHandler::ResetHold()
{
    auto& sink = HotkeyInputSink::GetSingleton();
    sink.m_isHolding.store(false);
    sink.m_pressedEdge.store(false);
}

void InputHandler::ConsumePress()
{
    HotkeyInputSink::GetSingleton().m_pressedEdge.store(false);
}
