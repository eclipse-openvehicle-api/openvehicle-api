/********************************************************************************
 * Copyright (c) 2025-2026 ZF Friedrichshafen AG
 *
 * This program and the accompanying materials are made available under the 
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Contributors:
 *   Erik Verhoeven - initial API and implementation
 ********************************************************************************/

#ifndef UNIQUE_ID_H
#define UNIQUE_ID_H

#include <set>
#include <cstdlib>
#include <cstdint>
#include <ctime>

/**
 * @brief Create a unique ID using the random number generator.
 * @tparam TType Type of the variable that represents the ID.
 * @tparam nDepth The depth of the ID generation in bits.
 */
template <typename TType = uint64_t, size_t nDepth = sizeof(TType) * 8>
class CUniqueID
{
public:
    static_assert(nDepth <= sizeof(TType) * 8);
    static_assert(nDepth > 0);

    /**
     * @brief Constructor
     */
    CUniqueID()
    {
        std::srand(static_cast<unsigned int>(std::time(nullptr)));
    }

    /**
     * @brief Generate a unique ID.
     * @return The optional result with the ID if successful or 0 when not successful.
    */
    TType Generate()
    {
        // Create a number with the supplied bit amount.
        auto fnCreate = [](size_t nMaxDepth)
        {
            TType tNumber{};

            // Create a new number
            size_t nBitsFilled = 0;
            while (nBitsFilled < nMaxDepth)
            {
                size_t nBitsToShiftIn = std::min(nMaxDepth - nBitsFilled, m_nRandMaxBits);
                tNumber = (tNumber << nBitsToShiftIn) | static_cast<TType>(static_cast<size_t>(std::rand()) & ((1u << nBitsToShiftIn) - 1));
                nBitsFilled += nBitsToShiftIn;
            }
            return tNumber;
        };

        // Treat a depth of 48 bits and more separately
        if constexpr (nDepth >= 48)
        {
            // Combination of the time in seconds (22 bits only - can hold 48 days).
            return (static_cast<TType>(std::time(nullptr)) << (nDepth - 22)) | fnCreate(nDepth - 22);
        }
        else
        {
            // Maximum iterations is 10000 for less than 24 bits and otherwise unlimited (value 0)
            size_t nMaxIterations = nDepth < 24 ? 10000u : 0u;

            // Generate
            size_t nCnt = 0;
            while (!nMaxIterations || (++nCnt < nMaxIterations))
            {
                TType tID = fnCreate(nDepth);

                // Check whether not 0 and not generated already
                if (tID && (m_setUsedNumbers.find(tID) == m_setUsedNumbers.end()))
                {
                    m_setUsedNumbers.insert(tID);
                    return tID;
                }
            }

            // No ID found
            return TType{};
        }
    }

private:
    /**
     * @brief Recursive constexpr function to find the number of bits needed to hold the number.
     * @param uiNumber The number to check for.
     * @return The amount of bits needed to hold the number.
     */
    static constexpr size_t GetBitWidth(uint64_t uiNumber)
    {
        return (!uiNumber) ? 0u : 1u + GetBitWidth(uiNumber >> 1);
    }

    /// The bits needed to hold the random number
    static constexpr size_t m_nRandMaxBits = GetBitWidth(static_cast<uint64_t>(RAND_MAX));

    // Static assertion to verify at compile-time
    static_assert(m_nRandMaxBits > 0, "RAND_MAX bits must be greater than 0");

    std::set<TType> m_setUsedNumbers; ///< Numbers generated previously already (numbers < 48 bits only).
};

#endif // !defined UNIQUE_ID_H
