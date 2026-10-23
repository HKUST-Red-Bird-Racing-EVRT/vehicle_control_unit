#ifndef BITRANGE_HPP
#define BITRANGE_HPP

#include <stdint.h>
#include <stddef.h>

/**
 * @brief Utility to pack and unpack bits within an integer type at compile-time.
 * @tparam START_BIT The lowest bit index of the range (0-indexed).
 * @tparam WIDTH    How many bits the field occupies.
 * @tparam T        The underlying storage type (defaults to uint8_t).
 */
template <size_t START_BIT, size_t WIDTH, typename T = uint8_t>
struct BitRange 
{
    static_assert(START_BIT + WIDTH <= sizeof(T) * 8, "Bit range exceeds type boundaries");

    // Generate a compile-time mask for just the WIDTH of the field (e.g., WIDTH=2 -> 0x03)
    static constexpr T field_mask = (1ULL << WIDTH) - 1;
    
    // Generate the mask in place (e.g., START_BIT=2, WIDTH=2 -> 0x0C)
    static constexpr T storage_mask = field_mask << START_BIT;

    /**
     * @brief Packs a value into the specific bit range of an existing storage byte.
     */
    static constexpr void pack(T& storage, T value) 
    {
        storage = (storage & ~storage_mask) | ((value & field_mask) << START_BIT);
    }

    /**
     * @brief Unpacks/reads the value from the specific bit range.
     */
    static constexpr T unpack(T storage) 
    {
        return (storage >> START_BIT) & field_mask;
    }
};

#endif // BITRANGE_HPP