/**
 * @file Interp.hpp
 * @author Planeson, Red Bird Racing
 * @brief Declaration and definition of the LinearInterp class template for linear interpolation
 * @version 1.3
 * @date 2026-10-20
 */

#ifndef INTERP_HPP
#define INTERP_HPP

#include <stdint.h>

/**
 * @brief Structure representing a point in the interpolation table
 * @tparam Tin Type of the input value
 * @tparam Tout Type of the output value
 */
template <typename Tin, typename Tout>
struct TablePoint
{
    Tin in;   /**< Input (x) value */
    Tout out; /**< Output (y) value */
};

/**
 * @brief Class template for performing linear interpolation using a lookup table
 * @details This class provides a method to perform linear interpolation based on a provided table of input-output pairs.
 *          The interpolation is performed using a binary search to find the correct interval in the table, followed by linear interpolation.
 *          It uses integer math to avoid floating-point operations, making it suitable for embedded systems.
 * @tparam Tin Type of the input values
 * @tparam Tout Type of the output values
 * @tparam Tmid Intermediate type for calculations to prevent overflow
 * @tparam size Number of points in the interpolation table
 */
template <typename Tin, typename Tout, typename Tmid, uint8_t size>
class LinearInterp
{
public:
    LinearInterp() = delete;                                                                        /**< Default constructor deleted to prevent instantiation without a table */
    explicit constexpr LinearInterp(const TablePoint<Tin, Tout> (&table_)[size]) : table(table_) {} /**< Normal constructor */

    /**
     * @brief Performs linear interpolation for the given input value, using the table
     * @param input The input value to interpolate
     * @return The interpolated output value
     */
    constexpr Tout interp(Tin input) const
    {
        // Clamp bounds
        if (input <= table[0].in)
            return table[0].out;
        if (input >= table[size - 1].in)
            return table[size - 1].out;

        // Binary search to find the correct interval in the table
        uint8_t index_low = 0;
        uint8_t index_high = size - 1;

        while (index_high - index_low > 1)
        {
            uint8_t index_mid = (index_low + index_high) / 2;
            if (input < table[index_mid].in)
                index_high = index_mid;
            else
                index_low = index_mid;
        }

        const TablePoint<Tin, Tout> &p0 = table[index_low];
        const TablePoint<Tin, Tout> &p1 = table[index_high];

        // Linear interpolation, all integer math
        Tmid deltaIn = p1.in - p0.in;
        Tmid deltaOut = p1.out - p0.out;
        return p0.out + ((Tmid)(input - p0.in) * deltaOut) / deltaIn; // can omit Tmid if slope is integer, but assume enough performance for now
    }

    /**
     * @brief Returns the starting input value of the interpolation table
     * @return The starting input value
     */
    constexpr Tin start() const
    {
        return table[0].in;
    }

    /**
     * @brief Returns the input range of the interpolation table (last input - first input)
     * @return The input range of the table
     */
    constexpr Tin range() const
    {
        return table[size - 1].in - table[0].in;
    }

private:
    const TablePoint<Tin, Tout> (&table)[size]; /**< Reference to the interpolation table */
};

#endif // INTERP_HPP