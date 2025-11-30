#include "bonsai/engine_api.hpp"

#include "bonsai/core/assert.hpp"

EngineAPI* EngineAPI::s_instance = nullptr;

EngineAPI* EngineAPI::get()
{
    if (s_instance == nullptr)
    {
        s_instance = new EngineAPI();
    }

    BONSAI_ASSERT(s_instance != nullptr && "EngineAPI instance must not be NULL");
    return s_instance;
}

void EngineAPI::set_instance(EngineAPI* instance)
{
    BONSAI_ASSERT(instance != nullptr && "EngineAPI instance must not be NULL");
    delete s_instance;
    s_instance = instance;
}
