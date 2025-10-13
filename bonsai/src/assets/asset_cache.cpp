#include "asset_cache.hpp"

std::unordered_map<std::string, AssetHandle<Asset>> AssetCache::s_assets;

void AssetCache::unload(std::filesystem::path const& path)
{
    std::string const key = path.lexically_normal().string();
    s_assets.erase(key);
}
