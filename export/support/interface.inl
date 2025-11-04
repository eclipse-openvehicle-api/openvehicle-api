#ifndef SDV_INTERFACE_INL
#define SDV_INTERFACE_INL

#ifndef SDV_INTERFACE_H
#error Do not include "interface.inl" directly. Include "interface.h" instead!
#endif //!defined SDV_INTERFACE_H

#include <cstddef>

namespace sdv
{
    inline interface_t::interface_t() noexcept
    {
        // Ensure the layout is the same on all platforms.
        static_assert(offsetof(interface_t, m_id) == 0);
        static_assert(offsetof(interface_t, m_pInterface) == 8);
        static_assert(sizeof(interface_t) == 16);
    }

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
    inline interface_t::interface_t(const interface_t& rifc) noexcept :
        m_id(rifc.m_id), m_pInterface(rifc.m_pInterface)
    {}

    inline interface_t::interface_t(interface_t&& rifc) noexcept :
        m_id(rifc.m_id), m_pInterface(rifc.m_pInterface)
    {
        rifc.reset();
    }
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

    inline interface_t::interface_t(std::nullptr_t) noexcept
    {}

    template <typename TInterface>
    inline interface_t::interface_t(TInterface* pInterface) noexcept :
        m_id(GetInterfaceId<TInterface>()), m_pInterface(pInterface)
    {}

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
    inline interface_t& interface_t::operator=(const interface_t& rifc) noexcept
    {
        m_id = rifc.m_id;
        m_pInterface = rifc.m_pInterface;
        return *this;
    }

    inline interface_t& interface_t::operator=(interface_t&& rifc) noexcept
    {
        m_id = rifc.m_id;
        m_pInterface = rifc.m_pInterface;
        rifc.reset();
        return *this;
    }
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

    inline interface_t& interface_t::operator=(std::nullptr_t) noexcept
    {
        reset();
        return *this;
    }

    template <typename TInterface>
    inline interface_t& interface_t::operator=(TInterface* pInterface) noexcept
    {
        m_id = GetInterfaceId<TInterface>();
        m_pInterface = pInterface;
        return *this;
    }

    inline interface_t::operator bool() const noexcept
    {
        return m_pInterface ? true : false;
    }

    inline void interface_t::reset() noexcept
    {
        m_id = 0ull;
        m_pInterface = nullptr;
    }

    inline interface_id interface_t::id() const noexcept
    {
        return m_id;
    }

    template <typename TInterface>
    inline TInterface* interface_t::get() noexcept
    {
        return GetInterfaceId<TInterface>() == m_id ? reinterpret_cast<TInterface*>(m_pInterface) : nullptr;
    }

    template <typename TInterface>
    inline const TInterface* interface_t::get() const noexcept
    {
        return GetInterfaceId<TInterface>() == m_id ? reinterpret_cast<const TInterface*>(m_pInterface) : nullptr;
    }

    inline int interface_t::compare(const interface_t& rifc) const noexcept
    {
        if (m_id < rifc.m_id) return -1;
        if (m_id > rifc.m_id) return 1;
        if (m_pInterface < rifc.m_pInterface) return -1;
        if (m_pInterface > rifc.m_pInterface) return 1;
        return 0;
    }

    inline bool operator==(const interface_t& rifcLeft, const interface_t& rifcRight) noexcept
    {
        return rifcLeft.compare(rifcRight) == 0;
    }

    inline bool operator==(const interface_t& rifcLeft, std::nullptr_t) noexcept
    {
        return !rifcLeft;
    }

    inline bool operator==(std::nullptr_t, const interface_t& rifcRight) noexcept
    {
        return !rifcRight;
    }

    inline bool operator!=(const interface_t& rifcLeft, const interface_t& rifcRight) noexcept
    {
        return rifcLeft.compare(rifcRight) != 0;
    }

    inline bool operator!=(const interface_t& rifcLeft, std::nullptr_t) noexcept
    {
        return static_cast<bool>(rifcLeft);
    }

    inline bool operator!=(std::nullptr_t, const interface_t& rifcRight) noexcept
    {
        return static_cast<bool>(rifcRight);
    }

    inline bool operator<(const interface_t& rifcLeft, const interface_t& rifcRight) noexcept
    {
        return rifcLeft.compare(rifcRight) < 0;
    }

    inline bool operator<(const interface_t& /*rifcLeft*/, std::nullptr_t) noexcept
    {
        return false;
    }

    inline bool operator<(std::nullptr_t, const interface_t& rifcRight) noexcept
    {
        return static_cast<bool>(rifcRight);
    }

    inline bool operator<=(const interface_t& rifcLeft, const interface_t& rifcRight) noexcept
    {
        return rifcLeft.compare(rifcRight) <= 0;
    }
    inline bool operator<=(const interface_t& rifcLeft, std::nullptr_t) noexcept
    {
        return !rifcLeft;
    }

    inline bool operator<=(std::nullptr_t, const interface_t& /*rifcRight*/) noexcept
    {
        return true;
    }

    inline bool operator>(const interface_t& rifcLeft, const interface_t& rifcRight) noexcept
    {
        return rifcLeft.compare(rifcRight) > 0;
    }

    inline bool operator>(const interface_t& rifcLeft, std::nullptr_t) noexcept
    {
        return static_cast<bool>(rifcLeft);
    }

    inline bool operator>(std::nullptr_t, const interface_t& /*rifcRight*/) noexcept
    {
        return false;
    }

    inline bool operator>=(const interface_t& rifcLeft, const interface_t& rifcRight) noexcept
    {
        return rifcLeft.compare(rifcRight) >= 0;
    }
    
    inline bool operator>=(const interface_t& /*rifcLeft*/, std::nullptr_t) noexcept
    {
        return true;
    }

    inline bool operator>=(std::nullptr_t, const interface_t& rifcRight) noexcept
    {
        return !rifcRight;
    }
}

#endif // !defined SDV_INTERFACE_INL