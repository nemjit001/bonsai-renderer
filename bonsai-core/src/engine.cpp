#include "bonsai/engine.hpp"

#include <imgui.h>
#include "bonsai/core/assert.hpp"
#include "bonsai/core/logger.hpp"
#include "bonsai/core/platform.hpp"
#include "bonsai/render_backend/render_backend.hpp"
#include "bonsai/application.hpp"
#include "bonsai/engine_api.hpp"

static ImGuiContext* s_imgui_context = nullptr;
static Platform* s_platform = nullptr;
static PlatformSurface* s_main_surface = nullptr;
static RenderBackend* s_render_backend = nullptr;
static EngineAPI* s_engine_api = nullptr;

Engine::Engine()
{
    Logger* logger = Logger::get();
    logger->set_min_log_level(LogLevel::Trace);
    BONSAI_ENGINE_LOG_INFO("Initializing Bonsai Engine");

    BONSAI_ENGINE_LOG_TRACE("Initializing ImGui");
    IMGUI_CHECKVERSION();
    s_imgui_context = ImGui::CreateContext();
    ImGuiIO& imgui_io = ImGui::GetIO();
    imgui_io.IniFilename = nullptr;
    imgui_io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    BONSAI_ENGINE_LOG_TRACE("Initializing Platform");
    s_platform = new Platform();

    BONSAI_ENGINE_LOG_TRACE("Initializing main surface");
    PlatformSurfaceConfig main_surface_config{};
    main_surface_config.resizable = true;
    main_surface_config.high_dpi = true;
    s_main_surface = s_platform->create_surface("Bonsai Application", 1600, 900, main_surface_config);

    BONSAI_ENGINE_LOG_TRACE("Initializing Render Backend");
    s_render_backend = RenderBackend::create(s_main_surface);
    BONSAI_ASSERT(s_render_backend != nullptr && "No Render Backend selected for Bonsai");

    BONSAI_ENGINE_LOG_TRACE("Initializing Engine API");
    s_engine_api = new EngineAPI();
    s_engine_api->register_loggger(logger);
    s_engine_api->register_imgui_context(s_imgui_context);
    s_engine_api->register_platform(s_platform);
    s_EngineAPI = s_engine_api;

    s_platform->set_surface_resized_callback([](PlatformSurface*, uint32_t width, uint32_t height) {
        BONSAI_ENGINE_LOG_TRACE("Window resized ({} x {})", width, height);
    });

    BONSAI_ENGINE_LOG_INFO("Initialized Bonsai Engine");
}

Engine::~Engine()
{
    BONSAI_ENGINE_LOG_INFO("Shutting down...");
    delete s_engine_api;

    BONSAI_ENGINE_LOG_TRACE("Shutting down Render Backend");
    delete s_render_backend;

    BONSAI_ENGINE_LOG_TRACE("Shutting down Platform");
    s_platform->destroy_surface(s_main_surface);
    delete s_platform;

    BONSAI_ENGINE_LOG_TRACE("Shutting down ImGui");
    ImGui::DestroyContext(s_imgui_context);

    BONSAI_ENGINE_LOG_INFO("Goodbye!");
}

void Engine::run(char const* app_name)
{
    // Load app module
    ApplicationModule const app_module = load_application_module(app_name);
    if (app_module.library == nullptr
        || app_module.create_application == nullptr
        || app_module.destroy_application == nullptr)
    {
        BONSAI_ENGINE_LOG_ERROR("Failed to load application module: \"{}\"", app_name);
        return;
    }

    // Create application
    Application* app = app_module.create_application(s_engine_api);
    BONSAI_ASSERT(app != nullptr);

    // Enter the engine main loop
    bool running = true;
    while (running)
    {
        running = s_platform->pump_messages();
        app->update(0.0);
    }

    // Clean up app module
    app_module.destroy_application(app);
    unload_application_module(app_module);
}

