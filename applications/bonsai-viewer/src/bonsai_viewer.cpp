#include "bonsai_viewer.hpp"

#include <imgui.h>
#include <bonsai/core/assert.hpp>

extern "C" BONSAI_API Application* BONSAI_APICALL create_application(EngineAPI* engine_api)
{
    EngineAPI::set_instance(engine_api);
    return new BonsaiViewer();
}

extern "C" BONSAI_API void BONSAI_APICALL destroy_application(Application const* app)
{
    delete app;
}

BonsaiViewer::BonsaiViewer()
    :
    Application()
{
    BONSAI_LOG_TRACE("Setting Bonsai Viewer ImGui context");
    IMGUI_CHECKVERSION();
    ImGui::SetCurrentContext(EngineAPI::get()->get_imgui_context());

    BONSAI_LOG_INFO("Initialized Bonsai Viewer!");
}

void BonsaiViewer::update([[maybe_unused]] double delta)
{
    //
}
