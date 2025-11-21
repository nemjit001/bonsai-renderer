#include "bonsai_viewer.hpp"

extern "C" BONSAI_API Application* BONSAI_APICALL create_application()
{
    return new BonsaiViewer();
}

extern "C" BONSAI_API void BONSAI_APICALL destroy_application(Application* app)
{
    delete app;
}
