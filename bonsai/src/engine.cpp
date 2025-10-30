#include "engine.hpp"

#include "bonsai_config.hpp"
#include "platform/logger.hpp"
#include "core/die.hpp"

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

    // Initialize systems
    BONSAI_LOG_INFO("Initializing Renderer");
    m_renderer = new Renderer(m_surface);

    BONSAI_LOG_INFO("Initializing World Manager");
    m_world_manager = new WorldManager();

    // Load startup world
    m_world_manager->load_world("assets/CornellBox.bonsai"); // TODO(nemjit001): Show world selection GUI on startup instead of defaulting to a hardcoded world.
    BONSAI_LOG_INFO("Active world: {}", m_world_manager->get_active_world()->get_name());

    // Set surface handlers
    m_surface->set_user_data(m_renderer);
    m_platform->set_platform_surface_resize_callback([]([[maybe_unused]] void* user_data, uint32_t width, uint32_t height)
    {
        BONSAI_LOG_TRACE("Window resized ({} x {})", width, height);
        Renderer* renderer = static_cast<Renderer*>(user_data);
        renderer->on_resize(width, height);
    });

    // Set application quit handler
    m_platform->set_user_data(this);
    m_platform->set_platform_quit_callback([](void* user_data)
    {
        Engine* that = static_cast<Engine*>(user_data);
        that->m_running = false;
    });

    m_running = true;
    m_timer.reset();
    BONSAI_LOG_INFO("Initialized Bonsai! (v{})", BONSAI_VERSION);
}

Engine::~Engine()
{
    BONSAI_LOG_INFO("Shutting down...");

    // Clean up systems
    delete m_world_manager;
    delete m_renderer;

    // Clean up platform system
    m_platform->destroy_surface(m_surface);
    delete m_platform;
}

void Engine::run()
{
    while (m_running)
    {
        m_timer.tick();
        m_platform->pump_messages();

        double const delta_milliseconds = m_timer.delta_milliseconds().count();
        AssetHandle<World> const active_world = m_world_manager->get_active_world();
        active_world->update(delta_milliseconds);

        m_renderer->render();
    }
}
