#include "engine.hpp"

#include "core/die.hpp"
#include "core/logger.hpp"
#include "bonsai_config.hpp"

Engine::~Engine()
{
    // Clean up renderer system
    delete m_renderer;

    // Clean up world
    delete m_world;

    // Clean up platform system
    if (m_platform != nullptr)
    {
        m_platform->destroy_surface(m_surface);
    }
    delete m_platform;
}

Engine::Engine()
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
    m_surface = m_platform->create_surface("Bonsai Renderer (" BONSAI_VERSION ")", 1600, 900, surface_config);
    if (m_surface == nullptr)
    {
        bonsai::die("Failed to create application surface");
    }

    // Initialize empty world
    BONSAI_LOG_INFO("Initializing World");
    m_world = new World();

    // Initialize rendering system
    BONSAI_LOG_INFO("Initializing Renderer");
    m_renderer = new Renderer();

    // Set surface handlers
    m_surface->set_user_data(m_renderer);
    m_platform->set_platform_surface_resize_callback([]([[maybe_unused]] void* user_data, uint32_t width, uint32_t height)
    {
        BONSAI_LOG_TRACE("Window resized ({} x {})", width, height);
        Renderer* renderer = static_cast<Renderer*>(user_data);
        renderer->on_resize(width, height);
    });
    m_platform->set_platform_surface_closed_callback([]([[maybe_unused]] void* user_data)
    {
        BONSAI_LOG_TRACE("Window closed");
    });

    // Set application quit handler
    m_platform->set_user_data(this);
    m_platform->set_platform_quit_callback([](void* user_data)
    {
        Engine* that = static_cast<Engine*>(user_data);
        that->m_running = false;
    });

    m_running = true;
    BONSAI_LOG_INFO("Initialized Bonsai! (v{})", BONSAI_VERSION);
}

void Engine::run()
{
    while (m_running)
    {
        m_platform->pump_messages();
        m_renderer->render(*m_world);
    }
}

