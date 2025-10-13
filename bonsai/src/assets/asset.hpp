#pragma once
#ifndef BONSAI_RENDERER_ASSET_HPP
#define BONSAI_RENDERER_ASSET_HPP

#include <filesystem>

/// @brief Base asset type, used for implementing asset loading behaviour.
class Asset
{
public:
    virtual ~Asset() = default;
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

#endif //BONSAI_RENDERER_ASSET_HPP