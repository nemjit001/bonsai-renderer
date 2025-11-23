#include "bonsai_viewer.hpp"

#include <bonsai/core/assert.hpp>

extern "C" BONSAI_API Application* BONSAI_APICALL create_application(EngineAPI* engine_api)
{
    s_EngineAPI = engine_api;
    BONSAI_ASSERT(s_EngineAPI != nullptr);
    return new BonsaiViewer();
}

extern "C" BONSAI_API void BONSAI_APICALL destroy_application(Application const* app)
{
    delete app;
}

BonsaiViewer::BonsaiViewer()
{
    BONSAI_LOG_TRACE("Initialized Bonsai Viewer!");
}

void BonsaiViewer::update([[maybe_unused]] double delta)
{
    //
}
