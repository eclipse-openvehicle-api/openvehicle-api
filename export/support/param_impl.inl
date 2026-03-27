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

#ifndef PARAM_IMPL_INL
#define PARAM_IMPL_INL

#ifndef PARAM_IMPL_H
    #include "param_impl.h"
#endif //! defined PARAM_IMPL_H

#include <regex>
#include <set>

/**
 * @brief Software Defined Vehicle framework.
 */
namespace sdv
{
    template <typename TVar, typename TDefaultType>
    inline CSdvParamInfo::CSdvParamInfo(TVar&, const std::string& rssName, TDefaultType tDefaultVal, const std::string& rssUnit,
        const std::string& rssCategory, const std::string& rssDescription, uint32_t uiFlagsParam)
    {
        EParamType eTypeLocal = EParamType::boolean_param;

        if constexpr (std::is_same_v<TVar, bool>)
            eTypeLocal = EParamType::boolean_param;
        else if constexpr (std::is_arithmetic_v<TVar>)
            eTypeLocal = EParamType::number_param; // No number limitation
        else if constexpr (TypeIsString<TVar>())
            eTypeLocal = EParamType::string_param; // No pattern
        else if constexpr (std::is_enum_v<TVar>)
            eTypeLocal = EParamType::enum_param; // No enumerator labels
        else
            static_assert(sdv::internal::always_false_v<TVar>, "The parameter type is not supported!");

        any_t anyDefaultValLocal;
        if constexpr (std::is_enum_v<TVar>)
        {
            static_assert(std::is_same_v<std::remove_reference_t<std::remove_const_t<TVar>>, TDefaultType>,
                "The enum types of the value and the default value must be identical!");
            anyDefaultValLocal = static_cast<std::underlying_type_t<TVar>>(tDefaultVal);
        }
        else
            anyDefaultValLocal = tDefaultVal;

        uint32_t uiFlagsLocal = uiFlagsParam;
        if (TypeIsReadOnly<TVar>())
            uiFlagsLocal |= static_cast<uint32_t>(EParamFlags::read_only);
        if (uiFlagsLocal & static_cast<uint32_t>(EParamFlags::read_only))
            uiFlagsLocal &= ~static_cast<uint32_t>(EParamFlags::locked); // Readonly cannot write
        Init(eTypeLocal, rssName, anyDefaultValLocal, rssUnit, rssCategory, rssDescription, uiFlagsLocal);

        if constexpr (std::is_enum_v<TVar>)
            uExtInfo.sEnumInfo.seqLabels = internal::GetLabelMapHelper().GetLabelMap<TVar>();
    }

    template <typename TVar, typename TDefaultType>
    inline CSdvParamInfo::CSdvParamInfo(TVar&, const std::string& rssName, TDefaultType tDefaultVal, const any_t& ranyLimitLow,
        bool bIncludeLow, const any_t& ranyLimitHigh, bool bIncludeHigh, const std::string& rssUnit, const std::string& rssCategory,
        const std::string& rssDescription, uint32_t uiFlagsParam)
    {
        static_assert(std::is_arithmetic_v<TVar> && !std::is_same_v<TVar, bool>,
            "The construct function for numeric types expects the type to be numeric.");
        uint32_t uiFlagsLocal = uiFlagsParam;
        if (TypeIsReadOnly<TVar>())
            uiFlagsLocal |= static_cast<uint32_t>(EParamFlags::read_only);
        if (uiFlagsLocal & static_cast<uint32_t>(EParamFlags::read_only))
            uiFlagsLocal &= ~static_cast<uint32_t>(EParamFlags::locked); // Readonly cannot write
        Init(EParamType::number_param, rssName, tDefaultVal, rssUnit, rssCategory, rssDescription, uiFlagsLocal);
        uExtInfo.sNumberInfo.anyLowerLimit = ranyLimitLow;
        uExtInfo.sNumberInfo.bIncludeLowerLinit = bIncludeLow;
        uExtInfo.sNumberInfo.anyUpperLimit = ranyLimitHigh;
        uExtInfo.sNumberInfo.bIncludeUpperLimit = bIncludeHigh;
    }

    template <typename TVar, typename TDefaultType>
    inline CSdvParamInfo::CSdvParamInfo(TVar&, const std::string& rssName, TDefaultType tDefaultVal, const std::string& rssPattern,
        const std::string& rssUnit, const std::string& rssCategory, const std::string& rssDescription, uint32_t uiFlagsParam)
    {
        static_assert(TypeIsString<TVar>(), "The construct function for string types expects the type to be a string.");
        uint32_t uiFlagsLocal = uiFlagsParam;
        if (TypeIsReadOnly<TVar>())
            uiFlagsLocal |= static_cast<uint32_t>(EParamFlags::read_only);
        if (uiFlagsLocal & static_cast<uint32_t>(EParamFlags::read_only))
            uiFlagsLocal &= ~static_cast<uint32_t>(EParamFlags::locked); // Readonly cannot write
        Init(EParamType::string_param, rssName, tDefaultVal, rssUnit, rssCategory, rssDescription, uiFlagsLocal);
        uExtInfo.sStringInfo.ssPattern = rssPattern;
    }

    template <typename TVar, typename TDefaultType>
    inline CSdvParamInfo::CSdvParamInfo(TVar&, const std::string& rssName, TDefaultType tDefaultVal,
        const sequence<SLabelInfo::SLabel>& rseqLabels, const std::string& rssCategory, const std::string& rssDescription,
        uint32_t uiFlagsParam)
    {
        static_assert(std::is_enum_v<TVar> || (std::is_integral_v<TVar> && !std::is_same_v<TVar, bool>),
            "The construct function for enumerators and bitmasks expects the type to be an enum or an integral type.");
        uint32_t uiFlagsLocal = uiFlagsParam;
        if (TypeIsReadOnly<TVar>())
            uiFlagsLocal |= static_cast<uint32_t>(EParamFlags::read_only);
        if (uiFlagsLocal & static_cast<uint32_t>(EParamFlags::read_only))
            uiFlagsLocal &= ~static_cast<uint32_t>(EParamFlags::locked); // Readonly cannot write
        if constexpr (std::is_enum_v<TVar>)
        {
            Init(EParamType::enum_param, rssName, static_cast<std::underlying_type_t<TVar>>(tDefaultVal), "",
                rssCategory, rssDescription, uiFlagsLocal);
            uExtInfo.sEnumInfo.seqLabels = rseqLabels;
        }
        else
        {
            Init(EParamType::bitmask_param, rssName, tDefaultVal, "", rssCategory, rssDescription, uiFlagsLocal);
            uExtInfo.sBitmaskInfo.seqLabels = rseqLabels;
        }
    }

    inline CSdvParamInfo::CSdvParamInfo(const SParamInfo& rInfo) : SParamInfo(rInfo)
    {}

    inline CSdvParamInfo::CSdvParamInfo(SParamInfo&& rInfo) : SParamInfo(std::move(rInfo))
    {}

    inline CSdvParamInfo& CSdvParamInfo::operator=(const SParamInfo& rsInfo)
    {
        static_cast<SParamInfo&>(*this) = rsInfo;
        return *this;
    }

    inline CSdvParamInfo& CSdvParamInfo::operator=(SParamInfo&& rsInfo)
    {
        static_cast<SParamInfo&>(*this) = std::move(rsInfo);
        return *this;
    }

    inline SParamInfo CSdvParamInfo::InfoStruct() const
    {
        return *this;
    }

    inline const u8string& CSdvParamInfo::Name() const
    {
        return SParamInfo::ssName;
    }

    inline u8string CSdvParamInfo::Path() const
    {
        sdv::u8string ssPath;
        if (!SParamInfo::ssGroup.empty())
            ssPath += ssGroup + ".";
        ssPath += ssName;
        return ssPath;
    }

    inline const any_t& CSdvParamInfo::DefaultVal() const
    {
        return SParamInfo::anyDefaultVal;
    }

    inline const u8string& CSdvParamInfo::Group() const
    {
        return SParamInfo::ssGroup;
    }

    inline const u8string& CSdvParamInfo::Description() const
    {
        return SParamInfo::ssDescription;
    }

    inline const u8string& CSdvParamInfo::Unit() const
    {
        return SParamInfo::ssUnit;
    }

    inline uint32_t CSdvParamInfo::Flags() const
    {
        return uiFlags;
    }

    inline bool CSdvParamInfo::ReadOnly() const
    {
        return (uiFlags & static_cast<uint32_t>(EParamFlags::read_only)) ? true : false;
    }

    inline bool CSdvParamInfo::Temporary() const
    {
        return (uiFlags & static_cast<uint32_t>(EParamFlags::temporary)) ? true : false;
    }

    inline bool CSdvParamInfo::Dirty() const
    {
        return (uiFlags & static_cast<uint32_t>(EParamFlags::dirty)) ? true : false;
    }

    inline bool CSdvParamInfo::Locked() const
    {
        return (uiFlags & static_cast<uint32_t>(EParamFlags::locked)) ? true : false;
    }


    inline bool CSdvParamInfo::Boolean() const
    {
        return get_switch() == EParamType::boolean_param;
    }

    inline bool CSdvParamInfo::Numeric() const
    {
        return get_switch() == EParamType::number_param;
    }

    inline bool CSdvParamInfo::String() const
    {
        return get_switch() == EParamType::string_param;
    }

    inline bool CSdvParamInfo::Enum() const
    {
        return get_switch() == EParamType::enum_param;
    }

    inline bool CSdvParamInfo::Bitmask() const
    {
        return get_switch() == EParamType::bitmask_param;
    }

    inline std::pair<any_t, bool> CSdvParamInfo::NumericLimitLow() const
    {
        if (!Numeric()) return {};
        return std::make_pair(uExtInfo.sNumberInfo.anyLowerLimit, uExtInfo.sNumberInfo.bIncludeLowerLinit);
    }

    inline std::pair<any_t, bool> CSdvParamInfo::NumericLimitHigh() const
    {
        if (!Numeric()) return {};
        return std::make_pair(uExtInfo.sNumberInfo.anyUpperLimit, uExtInfo.sNumberInfo.bIncludeUpperLimit);
    }

    inline std::string CSdvParamInfo::StringPattern() const
    {
        if (!String()) return {};
        return uExtInfo.sStringInfo.ssPattern;
    }

    inline sequence<SLabelInfo::SLabel> CSdvParamInfo::EnumBitmaskLabels() const
    {
        if (Enum())
            return uExtInfo.sEnumInfo.seqLabels;
        else if (Bitmask())
            return uExtInfo.sBitmaskInfo.seqLabels;
        else
            return {};
    }

    inline void CSdvParamInfo::Init(EParamType eTypeParam, const std::string& rssName, const any_t& ranyDefaultVal,
        const std::string& rssUnit, const std::string& rssCategory, const std::string& rssDescription, uint32_t uiFlagsParam)
    {
        switch_to(eTypeParam);
        uiFlags = uiFlagsParam;
        ssName = rssName;
        anyDefaultVal = ranyDefaultVal;
        ssUnit = rssUnit;
        ssGroup = rssCategory;
        ssDescription = rssDescription;
    }

    namespace internal
    {
        template <typename... TInfoConstruct>
        inline CParamGuardian::CParamGuardian(bool bLockable, bool bAutoDirty, TInfoConstruct... tConstruct) :
            CSdvParamInfo(tConstruct...), m_bLockable(bLockable), m_bAutoDirty(bAutoDirty)
        {}

        inline SParamInfo CParamGuardian::InfoStruct() const
        {
            sdv::SParamInfo sInfo = CSdvParamInfo::InfoStruct();
            sInfo.uiFlags = Flags();
            return sInfo;
        }

        inline uint32_t CParamGuardian::Flags() const
        {
            uint32_t uiFlagsTemp = CSdvParamInfo::Flags() & ~static_cast<uint32_t>(EParamFlags::state_mask);
            if (m_bLocked)
                uiFlagsTemp |= static_cast<uint32_t>(EParamFlags::locked);
            if (m_bDirty)
                uiFlagsTemp |= static_cast<uint32_t>(EParamFlags::dirty);
            return uiFlagsTemp;
        }

        inline bool CParamGuardian::UpdateDirty(const sdv::any_t& ranyValue)
        {
            if (!m_bAutoDirty) return false;
            bool bDirty = m_bDirty;
            m_bDirty |= ranyValue != m_anyStored;
            m_anyStored = ranyValue;
            return m_bDirty != bDirty;
        }

        inline bool CParamGuardian::SetDirty()
        {
            bool bDirty = m_bDirty;
            m_bDirty    = true;
            return m_bDirty != bDirty;
        }

        inline bool CParamGuardian::ResetDirty()
        {
            bool bDirty = m_bDirty;
            m_bDirty    = false;
            return m_bDirty != bDirty;
        }

        inline bool CParamGuardian::Dirty() const
        {
            return m_bDirty;
        }

        inline bool CParamGuardian::Lockable() const
        {
            return m_bLockable;
        }

        inline bool CParamGuardian::Lock()
        {
            if (!m_bLockable)
                return false;
            bool bLocked = m_bLocked;
            m_bLocked    = true;
            return m_bLocked != bLocked;
        }

        inline bool CParamGuardian::Unlock()
        {
            if (!m_bLockable)
                return false;
            bool bLocked = m_bLocked;
            m_bLocked    = false;
            return m_bLocked != bLocked;
        }

        inline bool CParamGuardian::Locked() const
        {
            return m_bLocked;
        }

        inline bool CParamGuardian::CheckRestrictions(const any_t& ranyValue)
        {
            if (Numeric())
            {
                // Check for lower limit
                auto prLimitLow = NumericLimitLow();
                if (!prLimitLow.first.empty())
                {
                    if (prLimitLow.second)
                    {
                        if (ranyValue < prLimitLow.first) return false;
                    } else
                    {
                        if (ranyValue <= prLimitLow.first) return false;
                    }
                }
                // Check for higher limit
                auto prLimitHigh = NumericLimitHigh();
                if (!prLimitHigh.first.empty())
                {
                    if (prLimitHigh.second)
                    {
                        if (ranyValue > prLimitHigh.first) return false;
                    }
                    else
                    {
                        if (ranyValue >= prLimitHigh.first) return false;
                    }
                }
            }
            else if (String())
            {
                // Check for compatibility to the regular expression pattern.
                if (!StringPattern().empty())
                {
                    std::regex regexPattern(StringPattern());
                    if (!std::regex_match(static_cast<std::string>(ranyValue), regexPattern)) return false;
                }
            }
            else if (Enum())
            {
                // Check for occurrence in the label sequence
                auto seqLabels = EnumBitmaskLabels();
                if (std::find_if(seqLabels.begin(), seqLabels.end(), [&](const auto& rsLabel)
                    { return ranyValue == rsLabel.anyValue; }) == seqLabels.end()) return false;
            }
            else if (Bitmask())
            {
                // Check for the occurence of each set bit in the label sequence
                auto seqLabels = EnumBitmaskLabels();
                uint64_t uiBitmask = ranyValue;
                for (const auto& rsLabel : seqLabels)
                    uiBitmask &= ~static_cast<uint64_t>(rsLabel.anyValue);
                if (uiBitmask) return false;
            }

            // All okay; restrictions were checked
            return true;
        }

        template <typename TVar>
        template <typename... TInfoConstruct>
        inline CParamValue<TVar>::CParamValue(TVar& rtVar, bool bLockable, bool bAutoDirty, TInfoConstruct... tConstruct) :
            CParamGuardian(bLockable, bAutoDirty, rtVar, tConstruct...), m_rtVar(rtVar)
        {
            // Assign the default value
            if constexpr (!CSdvParamInfo::TypeIsReadOnly<TVar>())
            {
                m_rtVar = DefaultVal().get<TVar>();
                UpdateDirty(DefaultVal());
                ResetDirty();
            }
        }

        template <typename TVar>
        inline bool CParamValue<TVar>::Set(const any_t& ranyValue)
        {
            // Assign the value
            if constexpr (!CSdvParamInfo::TypeIsReadOnly<TVar>())
            {
                if (!Locked() && !ReadOnly() && CheckRestrictions(ranyValue))
                {
                    m_rtVar = ranyValue.get<TVar>();
                    UpdateDirty(ranyValue);
                    return true;
                }
            }
            return false;
        }

        template <typename TVar>
        inline any_t CParamValue<TVar>::Get() const
        {
            // Remarks: in case of an enum, an explicit construction is needed.
            return sdv::any_t(m_rtVar);
        }
    }  // namespace internal

    inline std::shared_ptr<internal::CParamGuardian> CSdvParamMap::FindParamObject(const sdv::u8string& rssPath)
    {
        // Iterate through the registrations
        for (const SParamRegistration& rsRegistration : m_vecParamMapRegistration)
        {
            // Member parameter map entry
            if (rsRegistration.MemberMap() && rsRegistration.MemberMap()->Access())
            {
                auto ptrParameter = rsRegistration.MemberMap()->Access()->FindParamObject(rssPath);
                if (ptrParameter) return ptrParameter;
            }

            // Parameter entry
            if (rsRegistration.Parameter() && rsRegistration.Parameter()->Path() == rssPath)
                return rsRegistration.Parameter();
        }
        return {};
    }

    inline std::shared_ptr<const internal::CParamGuardian> CSdvParamMap::FindParamObject(const sdv::u8string& rssPath) const
    {
        // Iterate through the registrations
        for (const SParamRegistration& rsRegistration : m_vecParamMapRegistration)
        {
            // Member parameter map entry
            if (rsRegistration.MemberMap() && rsRegistration.MemberMap()->Access())
            {
                auto ptrParameter = rsRegistration.MemberMap()->Access()->FindParamObject(rssPath);
                if (ptrParameter)
                    return ptrParameter;
            }

            // Parameter entry
            if (rsRegistration.Parameter() && rsRegistration.Parameter()->Path() == rssPath)
                return rsRegistration.Parameter();
        }
        return {};
    }

    inline sequence<u8string> CSdvParamMap::GetParamPaths() const
    {
        // Split path function (splits the group from the parameter names)
        auto fnSplitPath = [](const sdv::u8string& rssPath) -> std::pair<sdv::u8string, sdv::u8string>
        {
            size_t nPos = rssPath.find_last_of('.');
            if (nPos == sdv::u8string::npos)
                return std::make_pair("", rssPath);
            return std::make_pair(rssPath.substr(0, nPos), rssPath.substr(nPos + 1));
        };

        std::set<u8string> setOneTime;
        sequence<u8string> seqPaths;
        for (const auto& rsRegistration : m_vecParamMapRegistration)
        {
            // Add a path to the sequence.
            auto fnAddPath = [&](const u8string& rssPath)
            {
                // Check if already present in the list
                if (setOneTime.find(rssPath) != setOneTime.end()) return;
                setOneTime.insert(rssPath);

                // Sort the paths per group, so all paths belonging to one group is returned together.
                // Find thee first location where the group is larger than the one of this path.
                auto prPath = fnSplitPath(rssPath);
                auto itPos = seqPaths.begin();
                while (itPos != seqPaths.end())
                {
                    auto prStoredPath = fnSplitPath(*itPos);
                    if (prPath.first.compare(prStoredPath.first) < 0) break;
                    ++itPos;
                }

                // Insert into the sequence
                seqPaths.insert(itPos, rssPath);
            };

            // Add the chained member to the list
            if (rsRegistration.MemberMap() && rsRegistration.MemberMap()->Access())
            {
                auto seqNamesMemberMap = rsRegistration.MemberMap()->Access()->GetParamPaths();
                for (const auto& rssPath : seqNamesMemberMap)
                    fnAddPath(rssPath);
            }

            // Add the parameter to the list
            if (rsRegistration.Parameter())
                fnAddPath(rsRegistration.Parameter()->Path());
        }

        return seqPaths;
    }

    inline any_t CSdvParamMap::GetParam(const u8string& rssPath) const
    {
        auto ptrParam = FindParamObject(rssPath);
        if (!ptrParam) return {};
        return ptrParam->Get();
    }

    inline bool CSdvParamMap::SetParam(const u8string& rssPath, any_t ranyValue)
    {
        auto ptrParam = FindParamObject(rssPath);
        if (!ptrParam) return false;
        return ptrParam->Set(ranyValue);
    }

    inline SParamInfo CSdvParamMap::GetParamInfo(const u8string& rssPath) const
    {
        auto ptrParamInfo = FindParamObject(rssPath);
        if (!ptrParamInfo) return {};
        return ptrParamInfo.get()->InfoStruct();
    }

    inline bool CSdvParamMap::IsParamDirty(const u8string& rssPath) const
    {
        auto ptrParam = FindParamObject(rssPath);
        if (!ptrParam) return false;
        return ptrParam->Dirty();
    }

    inline void CSdvParamMap::SetParamDirtyFlag(const std::string& rssPath)
    {
        auto ptrParam = FindParamObject(rssPath);
        if (!ptrParam) return;
        ptrParam->SetDirty();
    }

    inline void CSdvParamMap::ResetParamDirtyFlag(const u8string& rssPath)
    {
        auto ptrParam = FindParamObject(rssPath);
        if (!ptrParam) return;
        ptrParam->ResetDirty();
    }

    inline bool CSdvParamMap::IsParamMapDirty() const
    {
        std::set<u8string> setOneTime;
        for (const auto& rsRegistration : m_vecParamMapRegistration)
        {
            // Member parameter map entry
            if (rsRegistration.MemberMap() && rsRegistration.MemberMap()->Access())
            {
                if (rsRegistration.MemberMap()->Access()->IsParamMapDirty())
                    return true;
                continue;
            }

            // Parameter entry
            if (rsRegistration.Parameter())
            {
                // Check if already present in the list
                if (setOneTime.find(rsRegistration.Parameter()->Name()) != setOneTime.end())
                    continue;
                setOneTime.insert(rsRegistration.Parameter()->Name());

                // Check for dirtiness
                if (rsRegistration.Parameter()->Dirty())
                    return true;
            }
        }
        return false;
    }

    inline void CSdvParamMap::ResetParamMapDirtyFlags()
    {
        std::set<u8string> setOneTime;
        for (const auto& rsRegistration : m_vecParamMapRegistration)
        {
            // Member parameter map entry
            if (rsRegistration.MemberMap() && rsRegistration.MemberMap()->Access())
            {
                rsRegistration.MemberMap()->Access()->ResetParamMapDirtyFlags();
                continue;
            }

            // Parameter entry
            if (rsRegistration.Parameter())
            {
                // Check if already present in the list
                if (setOneTime.find(rsRegistration.Parameter()->Name()) != setOneTime.end())
                    continue;
                setOneTime.insert(rsRegistration.Parameter()->Name());

                // Reset dirtiness
                rsRegistration.Parameter()->ResetDirty();
            }
        }
    }

    inline void CSdvParamMap::LockParamMap()
    {
        for (const auto& rsRegistration : m_vecParamMapRegistration)
        {
            // Member parameter map entry
            if (rsRegistration.MemberMap() && rsRegistration.MemberMap()->Access())
            {
                rsRegistration.MemberMap()->Access()->LockParamMap();
                continue;
            }

            // Parameter entry
            if (rsRegistration.Parameter())
            {
                // Lock
                rsRegistration.Parameter()->Lock();
            }
        }
    }

    inline void CSdvParamMap::UnlockParamMap()
    {
        for (const auto& rsRegistration : m_vecParamMapRegistration)
        {
            // Member parameter map entry
            if (rsRegistration.MemberMap() && rsRegistration.MemberMap()->Access())
            {
                rsRegistration.MemberMap()->Access()->UnlockParamMap();
                continue;
            }

            // Parameter entry
            if (rsRegistration.Parameter())
            {
                // Lock
                rsRegistration.Parameter()->Unlock();
            }
        }
    }

    template <typename TVar, typename... TConstruct>
    inline std::shared_ptr<CSdvParamInfo> CSdvParamMap::RegisterParameter(TVar& rtVar, bool bLockable, bool bAutoDirty,
        TConstruct... tConstruct)
    {
        auto ptrParam = std::make_shared<internal::CParamValue<TVar>>(rtVar, bLockable, bAutoDirty, tConstruct...);
        m_vecParamMapRegistration.push_back(SParamRegistration(ptrParam));
        return ptrParam;
    }

    inline void CSdvParamMap::RegisterMemberMap(const std::shared_ptr<internal::SMemberMapAccess>& rptrMember)
    {
        m_vecParamMapRegistration.push_back(SParamRegistration(rptrMember));
    }

    inline void CSdvParamMap::InitParamMap()
    {
        // Nothing to do here.
    }

    inline bool CSdvParamMap::BuildNecessary() const
    {
        return m_vecParamMapRegistration.empty();
    }

    template <typename TType>
    constexpr bool CSdvParamInfo::TypeIsString()
    {
        using TType2 = std::remove_cv_t<TType>;
        using TRoot = std::remove_cv_t<std::remove_pointer_t<std::remove_extent_t<TType2>>>;
        constexpr bool bArray   = std::is_array_v<TType2>;
        constexpr bool bPointer = std::is_pointer_v<TType2>;
        constexpr bool bCharacter = std::is_same_v<TRoot, char> || std::is_same_v<TRoot, char16_t> ||
            std::is_same_v<TRoot, char32_t> || std::is_same_v<TRoot, wchar_t>;
        constexpr bool bStdString = std::is_same_v<TType2, std:: string> || std::is_same_v<TType2, std::u16string> ||
            std::is_same_v<TType2, std::u32string> || std::is_same_v<TType2, std::wstring>;
        constexpr bool bSdvString = std::is_same_v<TType2, string> || std::is_same_v<TType2, u8string> ||
            std::is_same_v<TType2, u16string> || std::is_same_v<TType2, u32string> || std::is_same_v<TType2, wstring>;
        constexpr bool bStdPath = std::is_same_v<TType2, std::filesystem::path>;
        return bStdString || bSdvString || (bArray && bCharacter) || (bPointer && bCharacter) || bStdPath;
    }

    template <typename TType>
    constexpr bool CSdvParamInfo::TypeIsReadOnly()
    {
        using TType2 = std::remove_cv_t<TType>;
        constexpr bool bArray = std::is_array_v<TType2>;
        constexpr bool bPointer = std::is_pointer_v<TType2>;
        return bArray || bPointer || std::is_const_v<TType>;
    }
}; // namespace sdv


#endif // !defined PARAM_IMPL_INL