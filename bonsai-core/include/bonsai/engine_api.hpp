#pragma once
#ifndef BONSAI_RENDERER_ENGINE_API_HPP
#define BONSAI_RENDERER_ENGINE_API_HPP

#include <imgui.h>
#include "core/logger.hpp"
#include "core/platform.hpp"

/// @brief The Engine API class contains engine services. The interface ensures it may be shared across dynamic library
/// boundaries.
class EngineAPI
{
private:
    EngineAPI() = default;
public:
    ~EngineAPI() = default;

    EngineAPI(EngineAPI const&) = delete;
    EngineAPI& operator=(EngineAPI const&) = delete;

    /// @brief Get or create the active engine API instance.
    /// @return The engine API instance.
    static EngineAPI* get();

    /// @brief Replace the current engine API instance.
    /// @param instance Instance to replace the API with.
    static void set_instance(EngineAPI* instance);

    void register_loggger(Logger* logger) { m_logger = logger; }

    void register_imgui_context(ImGuiContext* context) { m_context = context; }

    void register_platform(Platform* platform) { m_platform = platform; }

    Logger* get_logger() { return m_logger; }

    ImGuiContext* get_imgui_context() { return m_context; }

    Platform* get_platform() { return m_platform; }

private:
    /// @brief Static declaration accessible by both the Engine and Applications.
    static EngineAPI* s_instance;

    Logger* m_logger = nullptr;
    ImGuiContext* m_context = nullptr;
    Platform* m_platform = nullptr;
};

#define BONSAI_LOG_TRACE(...)       (EngineAPI::get()->get_logger()->trace(__VA_ARGS__))
#define BONSAI_LOG_DEBUG(...)       (EngineAPI::get()->get_logger()->debug(__VA_ARGS__))
#define BONSAI_LOG_INFO(...)        (EngineAPI::get()->get_logger()->info(__VA_ARGS__))
#define BONSAI_LOG_WARN(...)        (EngineAPI::get()->get_logger()->warn(__VA_ARGS__))
#define BONSAI_LOG_ERROR(...)       (EngineAPI::get()->get_logger()->error(__VA_ARGS__))
#define BONSAI_LOG_CRITICAL(...)    (EngineAPI::get()->get_logger()->critical(__VA_ARGS__))

#endif //BONSAI_RENDERER_ENGINE_API_HPP