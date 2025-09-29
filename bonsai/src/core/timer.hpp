#pragma once
#ifndef BONSAI_RENDERER_TIMER_HPP
#define BONSAI_RENDERER_TIMER_HPP

#include <chrono>

/// @brief High resolution timer implementation.
class Timer
{
public:
    using Clock = std::chrono::steady_clock;
    using DurationNanos = std::chrono::duration<double, std::nano>;
    using DurationMillis = std::chrono::duration<double, std::milli>;
    using DurationSeconds = std::chrono::duration<double>;
    using TimePoint = std::chrono::time_point<Clock, DurationSeconds>;

    /// @brief Reset the time delta tracked by the timer.
    inline void reset();

    /// @brief Tick the timer state, tracking the delta between this and the last tick or reset.
    inline void tick();

    /// @brief Read the time delta in milliseconds.
    /// @return The delta duration in milliseconds.
    [[nodiscard]] inline DurationMillis delta_milliseconds() const;

    /// @brief Read the time delta in seconds.
    /// @return The delta duration in seconds.
    [[nodiscard]] inline DurationSeconds delta_seconds() const;

private:
    TimePoint m_now = Clock::now();
    TimePoint m_prev = m_now;
};

#pragma region implementation

void Timer::reset()
{
    m_now = Clock::now();
    m_prev = m_now;
}

void Timer::tick()
{
    m_prev = m_now;
    m_now = Clock::now();
}

Timer::DurationMillis Timer::delta_milliseconds() const
{
    return std::chrono::duration_cast<DurationMillis>(m_now - m_prev);
}

Timer::DurationSeconds Timer::delta_seconds() const
{
    return std::chrono::duration_cast<DurationSeconds>(m_now - m_prev);
}

#pragma endregion

#endif //BONSAI_RENDERER_TIMER_HPP