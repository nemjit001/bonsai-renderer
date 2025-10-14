#include "world_manager.hpp"

#include "assets/asset_cache.hpp"

void WorldManager::load_world(std::filesystem::path const& path)
{
    if (m_active_world)
    {
        AssetCache::unload(m_active_world);
    }

    m_active_world = AssetCache::load<World>(path);
}
