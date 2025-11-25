#pragma once
#ifndef BONSAI_RENDERER_FATAL_EXIT_HPP
#define BONSAI_RENDERER_FATAL_EXIT_HPP

#include <cstdlib>
#include "logger.hpp"

#define BONSAI_FATAL_EXIT(...)  (Logger::get()->critical(__VA_ARGS__), std::exit(1));

#endif //BONSAI_RENDERER_FATAL_EXIT_HPP