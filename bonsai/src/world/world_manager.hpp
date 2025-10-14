#pragma once
#ifndef BONSAI_RENDERER_WORLD_MANAGER_HPP
#define BONSAI_RENDERER_WORLD_MANAGER_HPP

#include <filesystem>
#include "assets/asset_cache.hpp"
#include "world.hpp"

/// @brief The WorldManager handles loading and unloading world assets, making sure an active world is always available.
class WorldManager
{
public:
    /// @brief Load a new world asset from disk, unloading the previous active world in the process.
    /// @param path Path to world to load.
    void load_world(std::filesystem::path const& path);

    /// @brief Get the active world.
    /// @return The world asset handle.
    AssetHandle<World> get_active_world() const { return m_active_world; };

private:
    AssetHandle<World> m_active_world = AssetCache::create<World>("default_world");
};

#endif //BONSAI_RENDERER_WORLD_MANAGER_HPP