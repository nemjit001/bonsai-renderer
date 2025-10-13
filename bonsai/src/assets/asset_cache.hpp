#pragma once
#ifndef BONSAI_RENDERER_ASSET_CACHE_HPP
#define BONSAI_RENDERER_ASSET_CACHE_HPP

#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>

/// @brief Templated asset handle, returned by the asset cache.
/// @tparam AssetType
template<typename AssetType>
using AssetHandle = std::shared_ptr<AssetType>;

/// @brief Base asset type, used to implement asset loading behaviour.
class Asset
{
public:
    virtual ~Asset() = default;
};

/// @brief The AssetCache handles loading assets from disk.
class AssetCache
{
public:
    /// @brief Load an asset from disk, returning the cached version if it was already loaded.
    /// @tparam AssetType Type of asset to load.
    /// @param path Path to the asset file to load.
    template<typename AssetType>
    static AssetHandle<AssetType> load(std::filesystem::path const& path);

    /// @brief Unload an asset file, removing it from the asset cache.
    static void unload(std::filesystem::path const& path);

private:
    static std::unordered_map<std::string, AssetHandle<Asset>> s_assets;
};

/// @brief The AssetLoader structure can be specialized in order to implement asset loading for
/// an asset type.
/// @param AssetType Type of asset to load.
template<typename AssetType>
struct AssetLoader
{
    /// @brief Load an asset from disk.
    static AssetType load(std::filesystem::path const& path);
};

#pragma region implementation

template<typename AssetType>
AssetHandle<AssetType> AssetCache::load(std::filesystem::path const& path)
{
    static_assert(std::is_base_of_v<Asset, AssetType> && "AssetType must inherit from Asset!");
    std::string const key = path.lexically_normal().string();

    auto const iter = s_assets.find(key);
    if (iter != s_assets.end())
    {
        return std::dynamic_pointer_cast<AssetType>(iter->second);
    }

    AssetHandle<AssetType> const asset = std::make_shared<AssetType>(AssetLoader<AssetType>::load(path.lexically_normal()));
    s_assets[key] = asset;
    return asset;
}

#pragma endregion

#endif //BONSAI_RENDERER_ASSET_CACHE_HPP