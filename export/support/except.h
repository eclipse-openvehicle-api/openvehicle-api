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

#ifndef SDV_EXCEPT_H
#define SDV_EXCEPT_H

#include <cstdint>

#ifndef except
/** Define the except keyword */
#define except struct
#endif

namespace sdv
{
    /**
     * @brief exception ID type
     */
    using exception_id = uint64_t;

    /**
     * @brief Get the exception ID.
     * @tparam TException The exception type
     * @return The ID of the exception
     */
    template <typename TExcept>
    constexpr inline exception_id GetExceptionId() noexcept
    {
        // Return the exception ID.
        return TExcept::_id;
    }
}

#endif // !defined SDV_EXCEPT_H