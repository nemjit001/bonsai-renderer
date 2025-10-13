#include "asset_cache.hpp"

std::unordered_map<std::string, AssetHandle<Asset>> AssetCache::s_assets;
std::unordered_map<AssetHandle<Asset>, std::string> AssetCache::s_paths;
