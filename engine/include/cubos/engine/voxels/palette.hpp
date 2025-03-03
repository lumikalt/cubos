/// @file
/// @brief Class @ref cubos::engine::VoxelPalette.
/// @ingroup voxels-plugin

#pragma once

#include <vector>

#include <cubos/engine/voxels/material.hpp>

namespace cubos::engine
{
    class VoxelPalette;
}

namespace cubos::core::data::old
{
    void serialize(Serializer& serializer, const engine::VoxelPalette& palette, const char* name);
    void deserialize(Deserializer& deserializer, engine::VoxelPalette& palette);
} // namespace cubos::core::data::old

namespace cubos::engine
{
    /// @brief Holds a palette of materials. Supports up to 65535 materials.
    ///
    /// Instead of storing the materials themselves in voxel data, @b CUBOS.
    /// uses palettes, and stores the index of the material in the palette
    /// instead.
    ///
    /// This allows for more efficient storage of voxel data, since now instead
    /// of storing the whole material per each voxel, we just store a 16-bit
    /// integer.
    ///
    /// @ingroup voxels-plugin
    class VoxelPalette final
    {
    public:
        ~VoxelPalette() = default;

        /// @brief Constructs a palette with the given materials.
        /// @param materials Materials to add to the palette.
        VoxelPalette(std::vector<VoxelMaterial>&& materials);

        /// @brief Constructs an empty palette.
        VoxelPalette() = default;

        /// @brief Gets a pointer to the array of materials on the palette.
        /// @note The first element in the array corresponds to index 1, as index 0 is reserved.
        /// @return Pointer to the array of materials on the palette.
        const VoxelMaterial* data() const;

        /// @brief Gets the number of materials in the palette, excluding the empty material.
        /// @return Number of materials in the palette.
        uint16_t size() const;

        /// @brief Gets the material with the given index.
        /// @param index Index of the material to get (1-based, 0 is empty).
        /// @return Material at the given index.
        const VoxelMaterial& get(uint16_t index) const;

        /// @brief Sets the material at the given index.
        /// @param index Index of the material to set (1-based, 0 is empty).
        /// @param material Material to set.
        void set(uint16_t index, const VoxelMaterial& material);

        /// @brief Searches for the index of the material most similar with the given material.
        /// @param material Material to compare with.
        /// @return Index of the material.
        uint16_t find(const VoxelMaterial& material) const;

        /// @brief Adds a material to the palette, if one not similar enough already exists.
        /// @note Materials equal to @ref VoxelMaterial::Empty will be considered empty and may be
        /// replaced by the new material.
        /// @param material Material to add.
        /// @param similarity Minimum similarity for a material to be considered similar enough.
        /// @return Index of the material in the palette (1-based, 0 is empty).
        uint16_t add(const VoxelMaterial& material, float similarity = 1.0F);

        /// @brief Merges another palette into this one.
        /// @note All materials equal to @ref VoxelMaterial::Empty will be considered empty and may be
        /// overwritten.
        /// @param palette Palette to merge.
        /// @param similarity Minimum similarity for two materials to be merged.
        void merge(const VoxelPalette& palette, float similarity = 1.0F);

    private:
        friend void core::data::old::serialize(core::data::old::Serializer& /*serializer*/,
                                               const VoxelPalette& /*palette*/, const char* /*name*/);
        friend void core::data::old::deserialize(core::data::old::Deserializer& /*deserializer*/,
                                                 VoxelPalette& /*palette*/);

        std::vector<VoxelMaterial> mMaterials; ///< Materials in the palette.
    };
} // namespace cubos::engine
