#pragma once
#ifndef BONSAI_RENDERER_ENTRYPOINT_HPP
#define BONSAI_RENDERER_ENTRYPOINT_HPP

#include "engine.hpp"

#define BONSAI_MARK_ENTRYPOINT(app_name)    \
    int main(int argc, char** argv)         \
    {                                       \
        Engine().run((app_name));           \
        return 0;                           \
    }

#endif //BONSAI_RENDERER_ENTRYPOINT_HPP