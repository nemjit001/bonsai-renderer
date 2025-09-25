#include "engine.hpp"

#include "core/die.hpp"
#include "core/logger.hpp"

Engine::~Engine()
{
    if (m_platform)
    {
        m_platform->destroy_surface(m_surface);
        delete m_platform;
    }
}

void Engine::init()
{
    // Initialize global logger state
    Logger& logger = Logger::get();
    logger.set_min_log_level(LogLevel::Trace);
    BONSAI_LOG_INFO("Initialized Logger");

    // Initialize platform system
    BONSAI_LOG_INFO("Initializing Platform");
    m_platform = new Platform();

    // Create a platform surface
    BONSAI_LOG_INFO("Initializing application surface");
    SurfaceConfig const surface_config{ true /* resizable */, true /* allow_high_dpi */ };
    m_surface = m_platform->create_surface("Bonsai Renderer", 1600, 900, surface_config);
    if (m_surface == nullptr)
    {
        bonsai::die("Failed to create application surface");
    }

    // Set surface handlers
    m_platform->set_platform_surface_resize_callback([]([[maybe_unused]] void* user_data, uint32_t width, uint32_t height)
    {
        BONSAI_LOG_TRACE("Window resized ({} x {})", width, height);
    });
    m_platform->set_platform_surface_closed_callback([]([[maybe_unused]] void* user_data)
    {
        BONSAI_LOG_TRACE("Window closed");
    });

    // Set application quit handler
    m_platform->set_user_data(this);
    m_platform->set_platform_quit_callback([](void* user_data)
    {
        Engine* pThat = static_cast<Engine*>(user_data);
        pThat->m_running = false;
    });

    m_running = true;
    BONSAI_LOG_INFO("Initialized Bonsai!");
}

void Engine::run()
{
    while (m_running)
    {
        m_platform->pump_messages();
    }
}

