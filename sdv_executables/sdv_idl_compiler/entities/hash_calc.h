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

#ifndef HASH_CALC_H
#define HASH_CALC_H

#ifdef _MSC_VER
    // Prevent warnings for XXHash during static code analysis.
    #pragma warning(push)
    #pragma warning(disable : 26812 26451)
#endif

#define XXH_INLINE_ALL
#include <xxhash.h>

#ifdef _MSC_VER
    #pragma warning(pop)
#endif

#include <string>

/**
 * @brief Calculate a hash value
 */
class CHashObject
{
public:
    /**
     * @brief Constructor
     */
    CHashObject();

    /**
     * @brief Destructor
     */
    ~CHashObject();

    /**
     * @brief Shift operator adding a string to the hash.
     * @param[in] rssString Reference to the string.
     * @return 
    */
    CHashObject& operator<<(const std::string& rssString);

    /**
     * @brief Get the current hash value.
     * @return The hash value.
    */
    uint64_t GetHash() const;

private:
    XXH64_state_t* m_state = nullptr;   ///< Hash state
};

#endif // !defined(HASH_CALC_H)