#pragma once
#ifndef BONSAI_RENDERER_KILL_HPP
#define BONSAI_RENDERER_KILL_HPP

#include <cstdint>
#include <cstdlib>
#include "platform/logger.hpp"

enum KillCodes : uint8_t
{
    eKillCodeOK = 0,
    eKillCodeFatal,
    NUM_KILL_CODES,
};

namespace bonsai
{
    /// @brief Kill the current process with an error message that is written to the logs, useful for non-recoverable errors.
    template <typename... Args>
    void die(Args&&... args)
    {
        BONSAI_LOG_CRITICAL(std::forward<Args>(args)...);
        std::exit(eKillCodeFatal);
    }
} // namespace bonsai

#endif //BONSAI_RENDERER_KILL_HPP