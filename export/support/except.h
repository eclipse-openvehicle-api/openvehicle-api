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