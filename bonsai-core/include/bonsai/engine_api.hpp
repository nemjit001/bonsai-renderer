#pragma once
#ifndef BONSAI_RENDERER_ENGINE_API_HPP
#define BONSAI_RENDERER_ENGINE_API_HPP

#include "core/logger.hpp"
#include "core/platform.hpp"

/// @brief The Engine API class contains engine services. The interface ensures it may be shared across dynamic library
/// boundaries.
class EngineAPI
{
public:
    void register_loggger(Logger* logger) { m_logger = logger; }

    void register_platform(Platform* platform) { m_platform = platform; }

    Logger* get_logger() { return m_logger; }

    Platform* get_platform() { return m_platform; }

private:
    Logger* m_logger = nullptr;
    Platform* m_platform = nullptr;
};

/// @brief Static declaration accessible by both the Engine and Applications.
static EngineAPI* s_EngineAPI = nullptr;

#define BONSAI_LOG_TRACE(...)       (s_EngineAPI->get_logger()->trace(__VA_ARGS__))
#define BONSAI_LOG_DEBUG(...)       (s_EngineAPI->get_logger()->debug(__VA_ARGS__))
#define BONSAI_LOG_INFO(...)        (s_EngineAPI->get_logger()->info(__VA_ARGS__))
#define BONSAI_LOG_WARN(...)        (s_EngineAPI->get_logger()->warn(__VA_ARGS__))
#define BONSAI_LOG_ERROR(...)       (s_EngineAPI->get_logger()->error(__VA_ARGS__))
#define BONSAI_LOG_CRITICAL(...)    (s_EngineAPI->get_logger()->critical(__VA_ARGS__))

#endif //BONSAI_RENDERER_ENGINE_API_HPP