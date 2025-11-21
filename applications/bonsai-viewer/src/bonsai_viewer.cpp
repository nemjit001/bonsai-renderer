#include "bonsai_viewer.hpp"

#include <spdlog/spdlog.h>

extern "C" BONSAI_API Application* BONSAI_APICALL create_application()
{
    return new BonsaiViewer();
}

extern "C" BONSAI_API void BONSAI_APICALL destroy_application(Application const* app)
{
    delete app;
}

void BonsaiViewer::update([[maybe_unused]] double delta)
{
    //
}
