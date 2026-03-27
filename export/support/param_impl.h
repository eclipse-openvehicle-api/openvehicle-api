/********************************************************************************
 * Copyright (c) 2025-2026 Contributors to the Eclipse Foundation
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

#ifndef PARAM_IMPL_H
#define PARAM_IMPL_H

#include "../interfaces/param.h"
#include "interface_ptr.h"
#include <cstdint>
#include <map>
#include <string>
#include <type_traits>
#include <algorithm>
#include <memory>

/**
 * @brief Software Defined Vehicle framework.
 */
namespace sdv
{
    // Forward declaration
    class CSdvParamMap;

    /**
     * @brief Parameter information helper class; used for filling the parameter definition.
     */
    class CSdvParamInfo : private SParamInfo
    {
    public:
        /**
         * @brief Constructor for automatic parameter type detection.
         * @tparam TVar Type of the variable determining the type of information.
         * @tparam TDefaultType Type of the variable of the default value.
         * @param[in] rtVar Reference to the variable to extract the type from.
         * @param[in] rssName Reference to the string containing the name of the parameter.
         * @param[in] tDefaultVal The default value assigned to the parameter.
         * @param[in] rssUnit Reference to the string containing the unit description.
         * @param[in] rssCategory Reference to the string containing the name of the group. Can be empty when no group
         * is provided.
         * @param[in] rssDescription Reference to the string containing the description of the parameter.
         * @param[in] uiFlags The flags to use with this parameter (extended with read-only for 'const' types).
         */
        template <typename TVar, typename TDefaultType>
        CSdvParamInfo(TVar& rtVar, const std::string& rssName, TDefaultType tDefaultVal, const std::string& rssUnit,
            const std::string& rssCategory, const std::string& rssDescription, uint32_t uiFlags);

        /**
         * @brief Constructor for numeric values. The parameter type is extracted from the variable type.
         * @tparam TVar Type of the variable determining the type of information.
         * @param[in] rtVar Reference to the variable to extract the type from.
         * @param[in] rssName Reference to the string containing the name of the parameter.
         * @param[in] tDefaultVal The default value assigned to the parameter.
         * @param[in] ranyLimitLow Reference to the lower limit value or no limit if set to empty.
         * @param[in] bIncludeLow If there is a lower limit set, the boolean defines how to interpret the limit: when set
         * limit &lt;= value, when not set limit &lt; value.
         * @param[in] ranyLimitHigh Reference to the higher limit value or no limit if set to empty.
         * @param[in] bIncludeHigh If there is a higher limit set, the boolean defines how to interpret the limit: when set
         * limit &gt;= value, when not set limit &gt; value.
         * @param[in] rssUnit Reference to the string containing the unit description.
         * @param[in] rssCategory Reference to the string containing the name of the group. Can be empty when no group
         * is provided.
         * @param[in] rssDescription Reference to the string containing the description of the parameter.
         * @param[in] uiFlags The flags to use with this parameter (extended with read-only for 'const' types).
         */
        template <typename TVar, typename TDefaultType>
        CSdvParamInfo(TVar& rtVar, const std::string& rssName, TDefaultType tDefaultVal, const any_t& ranyLimitLow,
            bool bIncludeLow, const any_t& ranyLimitHigh, bool bIncludeHigh, const std::string& rssUnit,
            const std::string& rssCategory, const std::string& rssDescription, uint32_t uiFlags);

        /**
         * @brief Construct function for string values. The numeric type is extracted from the variable type.
         * @tparam TVar Type of the variable determining the type of information.
         * @tparam TDefaultType Type of the variable of the default value.
         * @param[in] rtVar Reference to the variable to extract the type from.
         * @param[in] rssName Reference to the string containing the name of the parameter.
         * @param[in] tDefaultVal The default value assigned to the parameter.
         * @param[in] rssPattern Reference to the string containing the regular expression describing the pattern the value is
         * allowed to have.
         * @param[in] rssUnit Reference to the string containing the unit description.
         * @param[in] rssCategory Reference to the string containing the name of the group. Can be empty when no group
         * is provided.
         * @param[in] rssDescription Reference to the string containing the description of the parameter.
         * @param[in] uiFlags The flags to use with this parameter (extended with read-only for 'const' types).
         */
        template <typename TVar, typename TDefaultType>
        CSdvParamInfo(TVar& rtVar, const std::string& rssName, TDefaultType tDefaultVal, const std::string& rssPattern,
            const std::string& rssUnit, const std::string& rssCategory, const std::string& rssDescription, uint32_t uiFlags);

        /**
         * @brief Constructor function for enumerator and bitmask parameters. The parameter type is extracted from the variable
         * type.
         * @tparam TVar Type of the variable determining the type of information.
         * @tparam TDefaultType Type of the variable of the default value.
         * @param[in] rtVar Reference to the variable to extract the type from.
         * @param[in] rssName Reference to the string containing the name of the parameter.
         * @param[in] tDefaultVal The default value assigned to the parameter.
         * @param[in] rseqLabels Reference to the sequence of labels available for the enumerator or bitmask.
         * @param[in] rssCategory Reference to the string containing the name of the group. Can be empty when no group
         * is provided.
         * @param[in] rssDescription Reference to the string containing the description of the parameter.
         * @param[in] uiFlags The flags to use with this parameter (extended with read-only for 'const' types).
         */
        template <typename TVar, typename TDefaultType>
        CSdvParamInfo(TVar& rtVar, const std::string& rssName, TDefaultType tDefaultVal,
            const sequence<SLabelInfo::SLabel>& rseqLabels, const std::string& rssCategory, const std::string& rssDescription,
            uint32_t uiFlags);

        /**
         * @brief Assignment constructor for parameter information structure.
         * @param rInfo Reference to the parameter information structure.
         */
        CSdvParamInfo(const SParamInfo& rInfo);

        /**
         * @brief Move constructor for the parameter information structure.
         * @param rInfo Reference to the parameter information structure.
         */
        CSdvParamInfo(SParamInfo&& rInfo);

        /**
         * @brief Default destructor.
         */
        virtual ~CSdvParamInfo() = default;

        /**
         * @brief Assignment operator for parameter information.
         * @param rsInfo Reference to the information structure.
         * @return Reference to this class.
         */
        CSdvParamInfo& operator=(const SParamInfo& rsInfo);

        /**
         * @brief Move operator for parameter information.
         * @param rsInfo Reference to the information structure.
         * @return Reference to this class.
         */
        CSdvParamInfo& operator=(SParamInfo&& rsInfo);

        /**
         * @brief Get the parameter information structure.
         * @return Reference to the parameter information structure.
         */
        virtual SParamInfo InfoStruct() const;

        /**
         * @brief Get the name of the parameter.
         * @return Reference to the parameter name.
         */
        const u8string& Name() const;

        /**
         * @brief Get the path of the parameter (name and grops together).
         * @return Path to the parameter.
         */
        u8string Path() const;

        /**
         * @brief Get the default value of the parameter.
         * @return Reference to the default value.
         */
        const any_t& DefaultVal() const;

        /**
         * @brief Get the group of the parameter.
         * @return Reference to the parameter group.
         */
        const u8string& Group() const;

        /**
         * @brief Get the description of the parameter.
         * @return Reference to the parameter description.
         */
        const u8string& Description() const;

        /**
         * @brief Get the unit for the parameter.
         * @return Reference to the parameter unit.
         */
        const u8string& Unit() const;

        /**
         * @brief Get the parameter flags.
         * @return The parameter flags.
         */
        virtual uint32_t Flags() const;

        /**
         * @brief Is this a read-only parameter?
         * @return Returns whether the parameter has a read-only flag.
         */
        bool ReadOnly() const;

        /**
         * @brief Is this a Temporary parameter?
         * @return Returns whether the parameter has a temporary flag.
         */
        bool Temporary() const;

        /**
         * @brief Is the parameter dirty (state value, not available in static param info)?
         * @return Returns whether the parameter has a dirty flag set.
         */
        virtual bool Dirty() const;

        /**
         * @brief Is this parameter locked (state value, not available in static param info)?
         * @return Returns whether the parameter has a locked flag set.
         */
        virtual bool Locked() const;

        /**
         * @brief Is the paratemer a boolean parameter.
         * @return Returns whether the parameter is boolean.
         */
        bool Boolean() const;

        /**
         * @brief Is the paratemer a numeric parameter.
         * @return Returns whether the parameter is numeric.
         */
        bool Numeric() const;

        /**
         * @brief Is the parameter a string parameter.
         * @return Returns whether the parameter is a string.
         */
        bool String() const;

        /**
         * @brief Is the parameter an enum parameter.
         * @return Returns whether the parameter is an enum.
         */
        bool Enum() const;

        /**
         * @brief Is the parameter a bitmask parameter.
         * @return Returns whether the parameter is a bitmask.
         */
        bool Bitmask() const;

        /**
         * @brief Get the lower limit for a numeric parameter.
         * @return Return a pair with the lower limit and a boolean whether the lower limit can be used as a value. The lower limit
         * will be set to empty if the parameter is not numeric or if there is no lower limit set for the parameter.
         */
        std::pair<any_t, bool> NumericLimitLow() const;

        /**
         * @brief Get the higher limit for a numeric parameter.
         * @return Return a pair with the upper limit and a boolean whether the upper limit can be used as a value. The upper limit
         * will be set to empty if the parameter is not numeric or if there is no upper limit set for the parameter.
         */
        std::pair<any_t, bool> NumericLimitHigh() const;

        /**
         * @brief Get the string pattern for a string parameter.
         * @return Return the string pattern or an empty string.
         */
        std::string StringPattern() const;

        /**
         * @brief Get the labels for an enumerator or bitmask parameter.
         * @return Returns a sequence with the labels or an empty sequence when the parameter is not an enumerator or a bitmask or
         * there are no labels defined.
         */
        sequence<SLabelInfo::SLabel> EnumBitmaskLabels() const;

    protected:
        /**
         * @brief Check the type for being a string (standard C++ string, SDV string, character array, character pointer).
         * @tparam TType Type to check for.
         * @return Returns whether the type is a string.
         */
        template <typename TType>
        static constexpr bool TypeIsString();

        /**
         * @brief Check the type for being read only.
         * @remarks Character arrays and character pointers are also identified as read only.
         * @tparam TType Type to check for.
         * @return Returns whether the type is read only.
         */
        template <typename TType>
        static constexpr bool TypeIsReadOnly();

    private:
        /**
         * @brief Initialization function called by constructors.
         * @param[in] eType The type of the parameter.
         * @param[in] rssName Reference to the string containing the name of the parameter.
         * @param[in] ranyDefaultVal Reference to the default value assigned to the parameter. Could be empty if no value was
         * assigned.
         * @param[in] rssUnit Reference to the string containing the unit description.
         * @param[in] rssCategory Reference to the string containing the name of the group. Can be empty when no group
         * is provided.
         * @param[in] rssDescription Reference to the string containing the description of the parameter.
         * @param[in] uiFlags The flags to use with this parameter (extended with read-only for 'const' types).
         */
        void Init(EParamType eType, const std::string& rssName, const any_t& ranyDefaultVal, const std::string& rssUnit,
            const std::string& rssCategory, const std::string& rssDescription, uint32_t uiFlags);
    };

    /** Internal namespace */
    namespace internal
    {
        /**
         * @brief The parameter guardian class limits manages the parameter states and allows access to the parameter infos and
         * values.
         */
        class CParamGuardian : public CSdvParamInfo
        {
        public:
            /**
             * @brief Constructor.
             * @tparam TInfoConstruct The type of the variables that are provided to the constructor function of the parameter
             * information.
             * @param[in] bLockable When set, the parameter is lockable. Only use with writable parameters.
             * @param[in] bAutoDirty When set, the parameter dirty flag is detected automatically. Only use with writable parameters.
             * @param[in] tConstruct The construct function arguments.
             */
            template <typename... TInfoConstruct>
            CParamGuardian(bool bLockable, bool bAutoDirty, TInfoConstruct... tConstruct);

            /**
             * @brief Get the parameter information structure. Overload of sdv::CSdvParamInfo::InfoStruct.
             * @return Reference to the parameter information structure.
             */
            virtual sdv::SParamInfo InfoStruct() const override;

            /**
             * @brief Get the parameter flags. Overload of sdv::CSdvParamInfo::Flags.
             * @return The parameter flags.
             */
            virtual uint32_t Flags() const override;

            /**
             * @brief Mark the parameter as dirty if the value has changed. This implies the automatic dirty detection to be
             * enabled.
             * @param ranyValue Reference to the value to be used to to check.
             * @return Returns 'true' if the dirty flag has changed.
             */
            bool UpdateDirty(const sdv::any_t& ranyValue);

            /**
             * @brief Explicitly set the parameter to dirty.
             * @return Returns 'true' if the dirty flag has changed.
             */
            bool SetDirty();

            /**
             * @brief Reset the parameter dirty flag.
             * @return Returns 'true' if the dirty flag has changed.
             */
            bool ResetDirty();

            /**
             * @brief Is the parameter marked dirty? Overload of sdv::CSdvParamInfo::Dirty.
             * @return Returns the dirty flag of the parameter.
             */
            virtual bool Dirty() const override;

            /**
             * @brief Is the parameter lockable?
             * @return Returns whether the parameter can be locked.
             */
            bool Lockable() const;

            /**
             * @brief Lock the parameter.
             * @return Returns 'true' if the lock flag has changed.
             */
            bool Lock();

            /**
             * @brief Unlock the parameter.
             * @return Returns 'true' if the lock flag has changed.
             */
            bool Unlock();

            /**
             * @brief Is the parameter currently locked? Overload of sdv::CSdvParamInfo::Locked.
             * @return Returns the lock flag for the parameter.
             */
            virtual bool Locked() const override;

            /**
             * @brief Set a value.
             * @param[in] ranyValue Reference to the value.
             * @return Returns whether the parameter is writable and can be set.
             */
            virtual bool Set(const any_t& ranyValue) = 0;

            /**
             * @brief Get a value.
             * @return Returns the value.
             */
            virtual any_t Get() const = 0;

        protected:
            /**
             * @brief Check the variable for compatibility the restrictions.
             * @param[in] ranyValue Reference to the value.
             * @return Returns whether the variable passes the restrictions.
            */
            bool CheckRestrictions(const any_t& ranyValue);

        private:
            bool    m_bLockable = false;    ///< When set, the parameter is lockable (only for writable parameters).
            bool    m_bLocked = false;      ///< When set, the parameter is locked against writing (only for lockable parameters).
            bool    m_bAutoDirty = false;   ///< When set, automatic dirty flag detection is enabled (only for writable parameters).
            bool    m_bDirty = false;       ///< When set, the parameter is marked as dirty.
            any_t   m_anyStored;            ///< Value of the parameter to detect for dirtiness.
        };

        /**
         * @brief Parameter value class
         * @tparam TVar Type of the parameter variable.
         */
        template <typename TVar>
        class CParamValue : public CParamGuardian
        {
        public:
            /**
             * @brief Constructor.
             * @tparam TInfoConstruct The type of the variables that are provided to the constructor function of the parameter
             * information.
             * @param[in] rtVar Reference to the parameter variable.
             * @param[in] bLockable When set, the parameter is lockable. Only use with writable parameters.
             * @param[in] bAutoDirty When set, the parameter dirty flag is detected automatically. Only use with writable parameters.
             * @param[in] tConstruct The construct function arguments.
             */
            template <typename... TInfoConstruct>
            CParamValue(TVar& rtVar, bool bLockable, bool bAutoDirty, TInfoConstruct... tConstruct);

            /**
             * @brief Set a value. Overload of CParamGuardian::Set.
             * @param[in] ranyValue Reference to the value.
             * @return Returns whether the parameter is writable and can be set.
             */
            virtual bool Set(const any_t& ranyValue) override;

            /**
             * @brief Get a value. Overload of CParamGuardian::Get.
             * @return Returns the value.
             */
            virtual any_t Get() const override;

        private:
            TVar&   m_rtVar;                ///< Reference to the parameter
        };

        /**
         * @brief Label map helper class. This helper class collects the labels of each defined map.
         */
        class CLabelMapHelper
        {
        public:
            /**
             * @brief Explicit clearing of the label map might be needed when a local memory manager is used.
             * @remarks Call this function before the destruction of the memory manager.
             */
            void Clear()
            {
                m_mapLabelCollections.clear();
            }

            /**
             * @brief Local version of SLabel.
             */
            struct SLabelLocal
            {
                sdv::any_t  anyValue;       ///< Label value (must be an integral number)
                std::string ssLabel;        ///< Label text
            };

            /**
             * @brief Store a collection of labels for an enum type.
             * @attention Since this function is called before the execution of main(), there is no SDV memory manager available
             * yet. Use the C++ allocation instead (this means conversion needs to be done at a later stage).
             * @tparam TEnum The enum type this label collection is describing.
             * @param rseqLabels Reference to the sequence containing the label-value-pairs.
             * @return Return 'true' to allow an assignment to a static variable during startup.
             */
            template <typename TEnum>
            bool StoreLabelMap(const std::vector<SLabelLocal>& rseqLabels)
            {
                m_mapLabelCollections[typeid(TEnum).name()] = rseqLabels;
                return true;
            }
    
            /**
             * @brief Get a previously stored label collection for an enum type.
             * @tparam TEnum The enum type to get the labels for.
             * @return Reference to a sequence containing the label-value-pairs or an empty sequence when no labels were stored for
             * this enum type.
             */
            template <typename TEnum>
            sequence<SLabelInfo::SLabel> GetLabelMap() const
            {
                using TEnumLocal = std::remove_reference_t<TEnum>;
                static const sequence<SLabelInfo::SLabel> seqEmpty;
                auto itEnum = m_mapLabelCollections.find(typeid(TEnumLocal).name());
                if (itEnum == m_mapLabelCollections.end())
                    return seqEmpty;

                // Create a copy pf the data using the SDV memory manager.
                sequence<SLabelInfo::SLabel> seqLabels;
                for (const SLabelLocal& rsLabel : itEnum->second)
                    seqLabels.push_back(SLabelInfo::SLabel{rsLabel.anyValue, rsLabel.ssLabel});

                return seqLabels;
            }
    
        private:
            std::map<std::string, std::vector<SLabelLocal>>    m_mapLabelCollections;    ///< Map with label collections.
        };

        /**
         * @brief Get access to the one instance of the label map helper class.
         * @return Reference to the label map helper class instance.
         */
        inline CLabelMapHelper& GetLabelMapHelper()
        {
            static CLabelMapHelper helper;
            return helper;
        }

        /** Limit type to use */
        enum class ELimitType
        {
            up_to_limit,
            up_to_and_include_limit,
            no_limit
        };

        /**
         * @brief Determine the lower limit of the numeric value.
         * @details The lower limit is defined by the operators > and >=. When no limit is required, the operator != is used
         * (defined by the macro NO_LIMIT).
         */
        struct SLowerLimit
        {
            /**
             * @brief Operator larger than.
             * @tparam TType Type to use for the comparison.
             * @param[in] tLimit The limit value.
             * @return A pair defining the limit. First the type, second the limit type.
             */
            template <typename TType>
            std::pair<TType, ELimitType> operator>(TType tLimit) const
            {
                return std::make_pair(tLimit, ELimitType::up_to_limit);
            }

            /**
             * @brief Operator larger than or equal.
             * @tparam TType Type to use for the comparison.
             * @param[in] tLimit The limit value.
             * @return A pair defining the limit. First the type, second the limit type.
             */
            template <typename TType>
            std::pair<TType, ELimitType> operator>=(TType tLimit) const
            {
                return std::make_pair(tLimit, ELimitType::up_to_and_include_limit);
            }

            /**
             * @brief No limit operator.
             * @param[in] n The value that was supplied. This is typically 0.
             * @return A pair containing the value and the limit type, being no limit.
             */
            std::pair<size_t, ELimitType> operator!=(size_t n) const
            {
                return std::make_pair(n, ELimitType::no_limit);
            }
        };

        /**
         * @brief Determine the upper limit of the numeric value.
         * @details The upper limit is defined by the operators < and <=. When no limit is required, the operator != is used
         * (defined by the macro NO_LIMIT).
         */
        struct SUpperLimit
        {
            /**
             * @brief Operator smaller than.
             * @tparam TType Type to use for the comparison.
             * @param[in] tLimit The limit value.
             * @return A pair defining the limit. First the type, second the limit type.
             */
            template <typename TType>
            std::pair<TType, ELimitType> operator<(TType tLimit) const
            {
                return std::make_pair(tLimit, ELimitType::up_to_limit);
            }

            /**
             * @brief Operator smaller than or equal.
             * @tparam TType Type to use for the comparison.
             * @param[in] tLimit The limit value.
             * @return A pair defining the limit. First the type, second the limit type.
             */
            template <typename TType>
            std::pair<TType, ELimitType> operator<=(TType tLimit) const
            {
                return std::make_pair(tLimit, ELimitType::up_to_and_include_limit);
            }

            /**
             * @brief No limit operator.
             * @param[in] n The value that was supplied. This is typically 0.
             * @return A pair containing the value and the limit type, being no limit.
             */
            std::pair<size_t, ELimitType> operator!=(size_t n) const
            {
                return std::make_pair(n, ELimitType::no_limit);
            }
        };

        /**
         * @brief Helper structure to determine if the type is a shared pointer.
         * @tparam T Type to test for.
         */
        template <typename T>
        struct is_shared_ptr : std::false_type
        {};

        /**
         * @brief Specialization of std::shared_ptr.
         */
        template <typename T>
        struct is_shared_ptr<std::shared_ptr<T>> : std::true_type
        {};

        /**
         * @brief Helper variable for is_shared_ptr.
         */
        template <typename T>
        inline constexpr bool is_shared_ptr_v = is_shared_ptr<T>::value;

        /**
         * @brief Member access class.
         */
        struct SMemberMapAccess
        {
            /**
             * @brief Find the parameter object with the supplied name.
             * @param[in] rssName Reference to the name of the parameter.
             * @return Returns a smart pointer to the parameter object.
             */
            virtual std::shared_ptr<internal::CParamGuardian> FindParamObject(const sdv::u8string& rssName) = 0;

            /**
             * @{
             * @brief Get CSdvParamMap access.
             * @return The parameter map pointer or a NULL pointer if there is no associated parameter map.
             */
            virtual CSdvParamMap* Access() = 0;
            virtual const CSdvParamMap* Access() const = 0;
            /**
             * @}
             */
        };

        /**
         * @brief Struct to give access to a member map.
         * @tparam TMember The member variable type.
         */
        template <typename TMember>
        struct SMemberMap : SMemberMapAccess
        {
            static_assert(std::is_base_of_v<CSdvParamMap, TMember>, "The member type must derive from sdv::CSdvParamMap.");

            /// The type definition.
            using TType = TMember;

            /**
             * @brief Constructor
             * @param[in] rMember Reference to the member.
             * @param[in] pEventObject Pointer to the object receiving the parameter change notifications.
             */
            SMemberMap(TMember& rMember, sdv::CSdvParamMap* pEventObject) : m_pEventObject(pEventObject), refMember(rMember)
            {}

            /**
             * @brief Default destructor.
             */
            virtual ~SMemberMap() = default;

            /**
             * @brief Build the parameter map (static build).
             * @return Vector with parameter info classes.
             */
            static std::vector<std::shared_ptr<sdv::CSdvParamInfo>> BuildStatic()
            {
                return TMember::BuildParamMap(nullptr, nullptr);
            }

            /**
             * @brief Build the parameter map.
             */
            void BuildMap()
            {
                refMember.get().BuildParamMap(&refMember.get(), m_pEventObject);
            }

            /**
             * @brief Find the parameter object with the supplied name. Overload of SMemberMapAccess::FindParamObject.
             * @param[in] rssName Reference to the name of the parameter.
             * @return Returns a smart pointer to the parameter object.
             */
            virtual std::shared_ptr<internal::CParamGuardian> FindParamObject(const sdv::u8string& rssName) override
            {
                return refMember.get().FindParamObject(rssName);
            }

            /**
             * @{
             * @brief Get CSdvParamMap access. Overload of SMemberMapAccess::Interface.
             * @return The parameter map pointer or a NULL pointer if there is no associated parameter map.
             */
            virtual CSdvParamMap* Access() override
            {
                return &refMember.get();
            }
            virtual const CSdvParamMap* Access() const override
            {
                return &refMember.get();
            }
            /**
             * @}
             */

        private:
            sdv::CSdvParamMap*              m_pEventObject;     ///< Pointer to the object to receive notifications.
            std::reference_wrapper<TMember> refMember;          ///< Reference to the member map.
        };

        /**
         * @brief Specialization for a pointer to the member.
         * @tparam TMember The member variable type. Must derive from sdv::CSdvParamMap.
         */
        template <typename TMember>
        struct SMemberMap<TMember*> : SMemberMapAccess
        {
            static_assert(std::is_base_of_v<CSdvParamMap, TMember>, "The member type must derive from sdv::CSdvParamMap.");

            /// The type definition.
            using TType = TMember;

            /**
             * @brief Constructor
             * @param[in] rpMember Reference to the pointer to the member.
             * @param[in] pEventObject Pointer to the object receiving the parameter change notifications.
             */
            SMemberMap(TMember*& rpMember, sdv::CSdvParamMap* pEventObject) : m_pEventObject(pEventObject), refpMember(rpMember)
            {}

            /**
             * @brief Default destructor.
             */
            virtual ~SMemberMap() = default;

            /**
             * @brief Build the parameter map (static build).
             * @return Vector with parameter info classes.
             */
            static std::vector<std::shared_ptr<sdv::CSdvParamInfo>> BuildStatic()
            {
                return TMember::BuildParamMap(nullptr, nullptr);
            }

            /**
             * @brief Build the parameter map.
             */
            void BuildMap()
            {
                if (refpMember.get())
                    refpMember.get()->BuildParamMap(refpMember.get(), m_pEventObject);
            }

            /**
             * @brief Find the parameter object with the supplied name. Overload of SMemberMapAccess::FindParamObject.
             * @param[in] rssName Reference to the name of the parameter.
             * @return Returns a smart pointer to the parameter object.
             */
            virtual std::shared_ptr<internal::CParamGuardian> FindParamObject(const sdv::u8string& rssName) override
            {
                if (!refpMember.get()) return {};
                return refpMember.get()->FindParamObject(rssName);
            }

            /**
             * @{
             * @brief Get the IParameters interface. Overload of SMemberMapAccess::Interface.
             * @return The parameter map pointer or a NULL pointer if there is no associated parameter map.
             */
            virtual CSdvParamMap* Access() override
            {
                // In case a change has happened, or the member was not build previously...
                if (refpMember.get())
                    refpMember.get()->BuildParamMap(refpMember.get(), m_pEventObject);

                return refpMember.get();
            }
            virtual const CSdvParamMap* Access() const override
            {
                if (!refpMember.get()) return nullptr;

                // In case a change has happened, or the member was not build previously...
                if (refpMember.get())
                    refpMember.get()->BuildParamMap(refpMember.get(), m_pEventObject);

                return refpMember;
            }
            /**
             * @}
             */
        private:
            sdv::CSdvParamMap*      m_pEventObject;                 ///< Pointer to the object to receive notifications.
            mutable std::reference_wrapper<TMember*> refpMember;    ///< Reference to the pointer to the member map.
        };

        /**
         * @brief Specialization for a shared pointer to the member.
         * @tparam TMember The member variable type. Must derive from sdv::CSdvParamMap.
         */
        template <typename TMember>
        struct SMemberMap<std::shared_ptr<TMember>> : SMemberMapAccess
        {
            static_assert(std::is_base_of_v<CSdvParamMap, TMember>, "The member type must derive from sdv::CSdvParamMap.");

            /// The type definition.
            using TType = TMember;

            /**
             * @brief Constructor
             * @param[in] rptrMember Reference to the smart pointer holding the member.
             * @param[in] pEventObject Pointer to the object receiving the parameter change notifications.
             */
            // Warning of cppcheck 2.7 for missing m_pEventObject member variable instantiation. Suppress warning. 
            // cppcheck-suppress uninitMemberVar
            SMemberMap(std::shared_ptr<TMember>& rptrMember, sdv::CSdvParamMap* pEventObject) :
                m_pEventObject(pEventObject), refptrMember(rptrMember)
            {}

            /**
             * @brief Default destructor.
             */
            virtual ~SMemberMap() = default;

            /**
             * @brief Build the parameter map (static build).
             * @return Vector with parameter info classes.
             */
            static std::vector<std::shared_ptr<sdv::CSdvParamInfo>> BuildStatic()
            {
                return TMember::BuildParamMap(nullptr, nullptr);
            }

            /**
             * @brief Build the parameter map.
             */
            void BuildMap()
            {
                if (refptrMember.get())
                    refptrMember.get()->BuildParamMap(refptrMember.get().get(), m_pEventObject);
            }

            /**
             * @brief Find the parameter object with the supplied name. Overload of SMemberMapAccess::FindParamObject.
             * @param[in] rssName Reference to the name of the parameter.
             * @return Returns a smart pointer to the parameter object.
             */
            virtual std::shared_ptr<internal::CParamGuardian> FindParamObject(const sdv::u8string& rssName) override
            {
                if (!refptrMember.get()) return {};
                return refptrMember.get()->FindParamObject(rssName);
            }

            /**
             * @{
             * @brief Get the IParameters interface. Overload of SMemberMapAccess::Interface.
             * @return The parameter map pointer or a NULL pointer if there is no associated parameter map.
             */
            virtual CSdvParamMap* Access() override
            {
                if (!refptrMember.get()) return nullptr;

                // In case a change has happened, or the member was not build previously...
                if (refptrMember.get())
                    refptrMember.get()->BuildParamMap(refptrMember.get().get(), m_pEventObject);

                return refptrMember.get().get();
            }
            virtual const CSdvParamMap* Access() const override
            {
                if (!refptrMember.get()) return nullptr;

                // In case a change has happened, or the member was not build previously...
                if (refptrMember.get())
                    refptrMember.get()->BuildParamMap(refptrMember.get().get(), m_pEventObject);

                return refptrMember.get().get();
            }
            /**
             * @}
             */
        private:
            sdv::CSdvParamMap*      m_pEventObject;                         ///< Pointer to the object to receive notifications.
            mutable std::reference_wrapper<std::shared_ptr<TMember>> refptrMember;  ///< Reference to the smart pointer to the
                                                                            ///< member map.
        };
    } // namespace internal

    /**
     * @brief Concatenate two variables together.
     * @param a First variable
     * @param b Second variable
     */
#define SDV_CONCAT_IMPL(a, b) a##b

    /**
     * @brief Concatenate two variables together with additional indirection.
     * @param a First variable
     * @param b Second variable
     */
#define SDV_CONCAT(a, b) SDV_CONCAT_IMPL(a, b)

    /**
     * @brief Create a unique variable by using the __COUNTER__ macro concatenated to a provided prefix.
     * @param prefix The prefix to use for the unique variable.
     */
#define SDV_UNIQUE_VAR(prefix) SDV_CONCAT(prefix, __COUNTER__)

    /**
    * @brief Begin an label map.
    * @param enum_type The name of the enumeration declaration.
    */
#define BEGIN_SDV_LABEL_MAP(enum_type)                                                                                             \
    /** Global variable initialized at the beginning triggering the label map storage. */                                          \
    [[maybe_unused]] static inline bool SDV_UNIQUE_VAR(_bLabelMap) = sdv::internal::GetLabelMapHelper().StoreLabelMap<enum_type>({

    /**
     * @brief Label element entry. Associates the value to the label.
     * @param enum_element The enumerator element.
     * @param name_string The name of the element as a string.
     */
#define SDV_LABEL_ENTRY(enum_element, name_string)                                                                                 \
            { static_cast<std::underlying_type_t<decltype(enum_element)>>(enum_element), name_string },

    /**
     * @brief End of enumerator description map.
     */
#define END_SDV_LABEL_MAP()                                                                                                        \
        });

    /**
    * @brief Begin a parameter map.
    */
#define BEGIN_SDV_PARAM_MAP()                                                                                                      \
        /**                                                                                                                        \
         * @brief Build the parameter map. If a pointer is supplied, registers the parameters in the map.                          \
         * @tparam TClass Type of the class containing the parameter map. This class must derive from sdv::CSdvParamMap or         \
         * std::nullptr_t.                                                                                                         \
         * @param[in] pObject Pointer to the class instance holding the parameter map. Can be NULL if only initial parameter       \
         * information is requested.                                                                                               \
         * @param[in] pEventObject Pointer to the object receiving the parameter change notifications.                             \
         * @return Returns a vector with the parameter information structures.                                                     \
         */                                                                                                                        \
        template <typename TClassPtr = std::nullptr_t>                                                                             \
        static std::vector<std::shared_ptr<sdv::CSdvParamInfo>> BuildParamMap([[maybe_unused]] TClassPtr pObject,                  \
            [[maybe_unused]] sdv::CSdvParamMap* pEventObject)                                                                      \
        {                                                                                                                          \
            bool constexpr bStatic = std::is_same_v<TClassPtr, std::nullptr_t>;                                                    \
            static_assert(bStatic || (std::is_pointer_v<TClassPtr> &&                                                              \
                    std::is_base_of_v<sdv::CSdvParamMap, std::remove_pointer_t<TClassPtr>>),                                       \
                "The class needs to derive from sdv::CSdvParamMap.");                                                              \
            if constexpr (!bStatic)                                                                                                \
            {                                                                                                                      \
                /* Build only if necessary */                                                                                      \
                if (!pObject->BuildNecessary()) return {};                                                                         \
            }                                                                                                                      \
            std::vector<std::shared_ptr<sdv::CSdvParamInfo>> vecParamInfo;                                                         \
            [[maybe_unused]] uint32_t uiFlags = 0;                                                                                 \
            [[maybe_unused]] std::string ssGroup;                                                                               \
            [[maybe_unused]] bool bLockable = false;

    /**
     * @brief End the parameter map.
     */
#define END_SDV_PARAM_MAP()                                                                                                        \
            return vecParamInfo;                                                                                                   \
        }                                                                                                                          \
                                                                                                                                   \
        /**                                                                                                                        \
         * @brief Return a vector with parameter information structures.                                                           \
         * @attention Only parameters that are statically available are available here. The returned list can differ from the list \
         * after instantiation.                                                                                                    \
         * @return Vector containing parameter information structures.                                                             \
         */                                                                                                                        \
        static std::vector<std::shared_ptr<sdv::CSdvParamInfo>> GetParamMapInfoStatic()                                            \
        {                                                                                                                          \
            std::vector<std::shared_ptr<sdv::CSdvParamInfo>> vecParamInfo = BuildParamMap(nullptr, nullptr);                       \
            return vecParamInfo;                                                                                                   \
        }                                                                                                                          \
                                                                                                                                   \
        /**                                                                                                                        \
         * @brief Initialize the parameter map. This function is called only once for initial map creation. Overload of            \
         * sdv::CSdvParamMap::InitParamMap.                                                                                        \
         */                                                                                                                        \
        virtual void InitParamMap() override                                                                                       \
        {                                                                                                                          \
            BuildParamMap(this, this);                                                                                             \
        }


    /**
     * @brief The following parameters belong to the defined group.
     * @details Parameters cabn be grouped together. Within a group, they need to be unique. A group can have sub-groups, separated
     * with a dot '.'. For example: "Sensor.ADAS.Radar". Each (sub-)group represents its own TOML table in the configuration. For
     * the example this would be:
     * @code
     * [Sensor]
     * [Sensor.ADAS]
     * [Sensor.Radar]
     * @endcode
     * @param group_string Name of the group and sub-groups separated by dots. Group names are case-sensitive.
     */
#define SDV_PARAM_GROUP(group_string)                                                                                              \
            ssGroup = group_string;                                                                                                 \

    /**
     * @brief The following parameters belong to the top level group again (not having any group name assigned to them).
     */
#define SDV_PARAM_NO_GROUP()                                                                                                       \
            ssGroup.clear();

    /**
     * @brief Reset all attributes for all following parameters.
     */
#define SDV_PARAM_RESET_ATTRIBUTES()                                                                                               \
            uiFlags = 0;                                                                                                           \
            bLockable = false;

    /**
     * @brief Following parameters are not writable.
     */
#define SDV_PARAM_SET_READONLY()                                                                                                   \
            uiFlags |= static_cast<uint32_t>(::sdv::EParamFlags::read_only);

    /**
     * @brief Following parameters can be written (if not defined as const).
     */
#define SDV_PARAM_RESET_READONLY()                                                                                                 \
            uiFlags &= ~static_cast<uint32_t>(::sdv::EParamFlags::read_only);

    /**
     * @brief Following parameters will be protected against writing when locked (e.g. after initialization).
     */
#define SDV_PARAM_ENABLE_LOCKING()                                                                                                 \
            bLockable = true;

    /**
     * @brief Following parameters will not be protected against writing.
     */
#define SDV_PARAM_DISABLE_LOCKING()                                                                                                \
            bLockable = false;

    /**
     * @brief Following parameters are marked temporary (will not be stored in the configuration).
     */
#define SDV_PARAM_SET_TEMPORARY()                                                                                                  \
            uiFlags |= static_cast<uint32_t>(::sdv::EParamFlags::temporary);

    /**
     * @brief Following parameters are not marked temporary.
     */
#define SDV_PARAM_RESET_TEMPORARY()                                                                                                \
            uiFlags &= ~static_cast<uint32_t>(::sdv::EParamFlags::temporary);

    /**
     * @brief Define a parameter.
     * @param var The member variable of the class containing this parameter map.
     * @param name_string The name of the parameter. The name must be unique within the group.
     * @param default_val The default value for the parameter.
     * @param unit_string The unit of the parameter.
     * @param description_string The description of the parameter.
     */
#define SDV_PARAM_ENTRY(var, name_string, default_val, unit_string, description_string)                                            \
    if constexpr (bStatic)                                                                                                         \
    {                                                                                                                              \
        decltype(var) temp{};                                                                                                      \
        std::shared_ptr<sdv::CSdvParamInfo> ptrParamInfo = std::make_shared<sdv::CSdvParamInfo>(temp, name_string, default_val,    \
            unit_string, ssGroup, description_string, uiFlags);                                                                    \
        if (ptrParamInfo) vecParamInfo.push_back(std::move(ptrParamInfo));                                                         \
    }                                                                                                                              \
    else                                                                                                                           \
    {                                                                                                                              \
        std::shared_ptr<sdv::CSdvParamInfo> ptrParamInfo = pObject->RegisterParameter(pObject->var, bLockable, true, name_string,  \
            default_val, unit_string, ssGroup, description_string, uiFlags);                                                       \
        if (ptrParamInfo) vecParamInfo.push_back(std::move(ptrParamInfo));                                                         \
    }

    /** No limit for the SDV_PARAM_NUMBER_ENTRY limitation definition. */
#define NO_LIMIT != 0

    /**
     * @brief Define a number parameter with restriction.
     * @param var The member variable of the class containing this parameter map.
     * @param name_string The name of the parameter. The name must be unique within the group.
     * @param default_val The default value for the parameter.
     * @param low_limit The minimal value limit. Can be '> val' or '>= val' or NO_LIMIT for no limitation.
     * @param high_limit The maximal value limit. Can be '< val' or '<= val' or NO_LIMIT for no limitation.
     * @param unit_string The unit of the parameter.
     * @param description_string The description of the parameter.
     */
#define SDV_PARAM_NUMBER_ENTRY(var, name_string, default_val, low_limit, high_limit, unit_string, description_string)              \
    {																													           \
        auto prLowLimit = sdv::internal::SLowerLimit() low_limit;                                                                  \
        sdv::any_t anyLower;                                                                                                       \
        if (prLowLimit.second != sdv::internal::ELimitType::no_limit) anyLower = prLowLimit.first;                                 \
        auto prHighLimit = sdv::internal::SUpperLimit() high_limit;                                                                \
        sdv::any_t anyUpper;                                                                                                       \
        if (prHighLimit.second != sdv::internal::ELimitType::no_limit) anyUpper = prHighLimit.first;                               \
        if constexpr (bStatic)                                                                                                     \
        {                                                                                                                          \
            decltype(var) temp{};                                                                                                  \
            std::shared_ptr<sdv::CSdvParamInfo> ptrParamInfo = std::make_shared<sdv::CSdvParamInfo>(temp, name_string,             \
                default_val, anyLower, prLowLimit.second != sdv::internal::ELimitType::up_to_limit, anyUpper,                      \
                prHighLimit.second != sdv::internal::ELimitType::up_to_limit, unit_string, ssGroup, description_string,            \
                uiFlags);                                                                                                          \
            if (ptrParamInfo) vecParamInfo.push_back(std::move(ptrParamInfo));                                                     \
        }                                                                                                                          \
        else                                                                                                                       \
        {                                                                                                                          \
            std::shared_ptr<sdv::CSdvParamInfo> ptrParamInfo = pObject->RegisterParameter(pObject->var, bLockable, true,           \
                name_string, default_val, anyLower, prLowLimit.second != sdv::internal::ELimitType::up_to_limit, anyUpper,         \
                prHighLimit.second != sdv::internal::ELimitType::up_to_limit, unit_string, ssGroup, description_string,            \
                uiFlags);                                                                                                          \
            if (ptrParamInfo)vecParamInfo.push_back(std::move(ptrParamInfo));                                                      \
        }                                                                                                                          \
    }

    /**
     * @brief Define an enum parameter.
     * @param var The member variable of the class containing this parameter map.
     * @param name_string The name of the parameter. The name must be unique within the group.
     * @param default_val The default value for the parameter.
     * @param description_string The description of the parameter.
     */
#define SDV_PARAM_ENUM_ENTRY(var, name_string, default_val, description_string)                                                    \
    SDV_PARAM_ENTRY(var, name_string, default_val, "", description_string)

    /**
     * @brief Define a string parameter.
     * @param var The member variable of the class containing this parameter map.
     * @param name_string The name of the parameter. The name must be unique within the group.
     * @param default_val The default value for the parameter.
     * @param pattern_string The pattern the parameter should fit to. This is a regular expression.
     * @param unit_string The unit of the parameter.
     * @param description_string The description of the parameter.
     */
#define SDV_PARAM_STRING_ENTRY(var, name_string, default_val, pattern_string, unit_string, description_string)                     \
    if constexpr (bStatic)                                                                                                         \
    {                                                                                                                              \
        decltype(var) temp{};                                                                                                      \
        std::shared_ptr<sdv::CSdvParamInfo> ptrParamInfo = std::make_shared<sdv::CSdvParamInfo>(temp, name_string, default_val,    \
            pattern_string, unit_string, ssGroup, description_string, uiFlags);                                                    \
        if (ptrParamInfo) vecParamInfo.push_back(std::move(ptrParamInfo));                                                         \
    }                                                                                                                              \
    else                                                                                                                           \
    {                                                                                                                              \
        std::shared_ptr<sdv::CSdvParamInfo> ptrParamInfo = pObject->RegisterParameter(pObject->var, bLockable, true, name_string,  \
            default_val, pattern_string, unit_string, ssGroup, description_string, uiFlags);                                       \
        if (ptrParamInfo) vecParamInfo.push_back(std::move(ptrParamInfo));                                                         \
    }

    /**
     * @brief Define a path parameter.
     * @param var The member variable of the class containing this parameter map.
     * @param name_string The name of the parameter. The name must be unique within the group.
     * @param default_val The default value for the parameter.
     * @param description_string The description of the parameter.
     */
#define SDV_PARAM_PATH_ENTRY(var, name_string, default_val, description_string)                                                    \
    SDV_PARAM_ENTRY(var, name_string, default_val, "", description_string)

    /**
     * @brief Define a bitmask parameter.
     * @param TEnum Name of the enum definition defining the bits through BEGIN_SDV_ENUM_DEF_MAP macro.
     * @param var The member variable of the class containing this parameter map.
     * @param name_string The name of the parameter. The name must be unique within the group.
     * @param description_string The description of the parameter.
     */
#define SDV_PARAM_BITMASK_ENTRY(TEnum, var, name_string, default_val, description_string)                                          \
    if constexpr (bStatic)                                                                                                         \
    {                                                                                                                              \
        decltype(var) temp{};                                                                                                      \
        std::shared_ptr<sdv::CSdvParamInfo> ptrParamInfo = std::make_shared<sdv::CSdvParamInfo>(temp, name_string, default_val,    \
            sdv::internal::GetLabelMapHelper().GetLabelMap<TEnum>(), ssGroup, description_string, uiFlags);                        \
        if (ptrParamInfo) vecParamInfo.push_back(std::move(ptrParamInfo));                                                         \
    }                                                                                                                              \
    else                                                                                                                           \
    {                                                                                                                              \
        std::shared_ptr<sdv::CSdvParamInfo> ptrParamInfo = pObject->RegisterParameter(pObject->var, bLockable, true, name_string,  \
            default_val, sdv::internal::GetLabelMapHelper().GetLabelMap<TEnum>(), ssGroup, description_string, uiFlags);           \
        if (ptrParamInfo) vecParamInfo.push_back(std::move(ptrParamInfo));                                                         \
    }

    /**
     * @brief Chain the parameter map of a base class.
     * @param base_class Name of the base class.
     */
#define SDV_PARAM_CHAIN_BASE(base_class)                                                                                           \
    {                                                                                                                              \
        auto vecBaseParamInfo = base_class::BuildParamMap(pObject, pEventObject);                                                  \
        vecParamInfo.insert(vecParamInfo.end(), vecBaseParamInfo.begin(), vecBaseParamInfo.end());                                 \
    }

    /**
     * @brief Call the parameter map a member variable containing a parameter map.
     * @param member Name of the member variable containing a parameter map. This can be a member instance, a pointer or a smart
     * pointer to the instance.
     */
#define SDV_PARAM_CHAIN_MEMBER(member)                                                                                             \
    {                                                                                                                              \
        std::vector<std::shared_ptr<sdv::CSdvParamInfo>> vecParamInfoMember;                                                       \
        if constexpr (bStatic)                                                                                                     \
        {                                                                                                                          \
            vecParamInfoMember = sdv::internal::SMemberMap<decltype(member)>::BuildStatic();                                       \
        }                                                                                                                          \
        else                                                                                                                       \
        {                                                                                                                          \
            auto ptrMemberMap = std::make_shared<sdv::internal::SMemberMap<decltype(member)>>(pObject->member, pObject);           \
            if (ptrMemberMap)                                                                                                      \
            {                                                                                                                      \
                ptrMemberMap->BuildMap();                                                                                          \
                pObject->RegisterMemberMap(ptrMemberMap);                                                                          \
            }                                                                                                                      \
        }                                                                                                                          \
        vecParamInfo.insert(vecParamInfo.end(), vecParamInfoMember.begin(), vecParamInfoMember.end());                             \
    }

    /**
     * @brief Parameter map base class implementing parameter support using the parameter map macros.
     * @details The parameter map base class implements the support for the parameter map macros as well as parameter state
     * management. A class using parameters can derive from this class and overload the following events to control its behavior:
     * - OnParamChanged - parameter has changed
     * - OnParamFlagChanged - parameter flag has changed
     */
    class CSdvParamMap : public IParameters
    {
    public:

        /**
         * @brief Default destructor.
         */
        virtual ~CSdvParamMap() = default;

        /**
         * @{
         * @brief Find a parameter object by name.
         * @param[in] rssName Reference to the name of the parameter.
         * @return Smart pointer to the parameter guardian object or an empty pointer when the parameter was not found.
         */
        std::shared_ptr<internal::CParamGuardian> FindParamObject(const sdv::u8string& rssName);
        std::shared_ptr<const internal::CParamGuardian> FindParamObject(const sdv::u8string& rssName) const;
        /**
         * @}
         */

        /**
         * Return a sequence with parameter paths. Each path is unique and can be used to get and set the parameter value. Overload
         * of sdv::IParameters::GetParamPaths.
         * @return Sequence containing parameter paths. Parameter paths are composed from group/sub-groups and the parameter name,
         * separated by a dot.
         */
        virtual sequence<u8string> GetParamPaths() const override;

        /**
         * @brief Returns the parameter value. Overload of sdv::IParameters::GetParam.
         * @param[in] ssPath Path of the parameter. The parameter path is composed from group/sub-groups and the parameter name,
         * separated by a dot.
         * @return Returns the parameter value. Returns an 'empty' parameter value when not successful or the parameter is not set.
         */
        virtual any_t GetParam(/*in*/ const u8string& ssPath) const override;

        /**
         * @brief Set the parameter value. Overload of sdv::IParameters::SetParam.
         * @param[in] ssPath Path of the parameter. The parameter path is composed from group/sub-groups and the parameter name,
         * separated by a dot.
         * @param[in] anyValue Reference to the parameter value to set.
         * @return Returns 'true' on success or 'false' when parameter could not be set (e.g. the parameter is read-only) or the
         * index is larger than the amount of parameters being available.
         */
        virtual bool SetParam(/*in*/ const u8string& ssPath, /*in*/ any_t anyValue) override;

        /**
         * @brief Get parameter information.Overload of sdv::IParameters::GetParamInfo.
         * @param[in] ssPath Path of the parameter. The parameter path is composed from group/sub-groups and the parameter name,
         * separated by a dot.
         * @return Return the parameter information for the requested parameter or an empty structure when no parameter information
         * is available or the parameter is not available.
         */
        virtual SParamInfo GetParamInfo(/*in*/ const u8string& ssPath) const override;

        /**
         * @brief Is the parameter dirty (was it changed)? Checks the parameter dirty flag. Overload of sdv::IParameters::IsDirty.
         * @param[in] ssPath Path of the parameter. The parameter path is composed from group/sub-groups and the parameter name,
         * separated by a dot.
         * @return Returns whether the parameter was changed either by an explicit call to the SetParam function or internally by
         * the object itself.
         */
        virtual bool IsParamDirty(/*in*/ const u8string& ssPath) const override;

        /**
         * @brief Set dirty flag for one parameter (or all properties for the function without parameters).
         * @param[in] rssPath Reference to the path of the parameter.
         */
        void SetParamDirtyFlag(const std::string& rssPath);

        /**
         * @brief Reset the dirty flag. Overload of sdv::IParameters::ResetParamDirtyFlag.
         * @param[in] ssPath Path of the parameter. The parameter path is composed from group/sub-groups and the parameter name,
         * separated by a dot.
         */
        virtual void ResetParamDirtyFlag(/*in*/ const u8string& ssPath) override;

        /**
        * @brief Is the parameter map dirty? Overload of sdv::IParameters::IsParamMapDirty.
        * @return Returns true when any one of the parameters is flagged as dirty.
        */
        virtual bool IsParamMapDirty() const override;

        /**
         * @brief Reset the dirty flag for all parameters. Overload of sdv::IParameters::ResetParamMapDirtyFlags.
         */
        virtual void ResetParamMapDirtyFlags() override;

        /**
         * @brief Lock the parameter map.
         */
        void LockParamMap();

        /**
         * @brief Unlock the parameter map.
         */
        void UnlockParamMap();

        /**
         * @brief Parameter changed event. Overload this function to implement parameter event handling.
         * @param[in] rssPath Reference to the string containing the path of the parameter that changed.
         * @param[in] rvarOld Reference to the any holding the value of the parameter.
         * @param[in] rvarNew Reference to the any holding the value of the parameter.
         * @param[in] rptrParamInfo Reference to the smart pointer to the parameter information object.
         */
        virtual void OnParamChanged([[maybe_unused]] const std::string& rssPath, [[maybe_unused]] const any_t& rvarOld,
            [[maybe_unused]] const any_t& rvarNew, [[maybe_unused]] const std::shared_ptr<CSdvParamInfo>& rptrParamInfo) {}

        /**
         * @brief Parameter dirty flag changed event. Overload this function to implement parameter dirty flag event
         * handling.
         * @param[in] rssPath Reference to the string containing the path of the parameter that changed.
         * @param[in] uiOldFlags The old value of the flags.
         * @param[in] uiNewFlags The new value of the flags.
         * @param[in] uiMask The mask of the flags that are changed by the new value.
         * @param[in] rvar Reference to the any holding the value of the parameter.
         * @param[in] rptrParamInfo Reference to the smart pointer to the parameter information object.
         */
        virtual void OnParamFlagChanged([[maybe_unused]] const std::string& rssPath, [[maybe_unused]] uint32_t uiOldFlags,
            [[maybe_unused]] uint32_t uiNewFlags, [[maybe_unused]] uint32_t uiMask, [[maybe_unused]] const any_t& rvar, 
            [[maybe_unused]] const std::shared_ptr<CSdvParamInfo>& rptrParamInfo) {}

    protected:
        /**
         * @brief Register the parameter in the parameter map.
         * @tparam TVar Type of the variable.
         * @tparam TConstruct The arguments for the parameter info construct function.
         * @param[in] rtVar Reference to the parameter.
         * @param[in] bLockable When set, the parameter is lockable. Only use with writable parameters.
         * @param[in] bAutoDirty When set, the parameter dirty flag is detected automatically. Only use with writable parameters.
         * @param[in] tConstruct The construct function arguments.
         * @return Smart pointer to the parameter information structure.
         */
        template <typename TVar, typename... TConstruct>
        std::shared_ptr<CSdvParamInfo> RegisterParameter(TVar& rtVar, bool bLockable, bool bAutoDirty, TConstruct... tConstruct);

        /**
         * @brief Register a member parameter map into this parameter map.
         * @param[in] rptrMember Reference to the smart pointer pointing to the member map.
         */
        void RegisterMemberMap(const std::shared_ptr<internal::SMemberMapAccess>& rptrMember);

        /** 
         * @brief Initialize the parameter map. This function is called only once for initial map creation. This implementation
         * doesn't do anything. The parameter map macros overload this function to allow map building.
         */
        virtual void InitParamMap();

        /**
         * @brief Is a parameter map build necessary?
         * @return Returns whether parameter map building is required.
         */
        bool BuildNecessary() const;

    public:
        /**
         * @brief Return a vector with parameter information structures. Empty function called when the derived class doesn't
         * implement its own parameter map.
         * @return Vector containing parameter information structures.
         */
        static std::vector<std::shared_ptr<sdv::CSdvParamInfo>> GetParamMapInfoStatic()
        {
            return {};
        }
            
    private:

        /**
         * @brief Member registration.
         */
        struct SParamRegistration
        {
            /**
             * @brief Constructor for a parameter.
             * @tparam TVar Type of the variable.
             * @param[in] rptrParameter Reference to the smart pointer containing the parameter.
             */
            template <typename TVar>
            SParamRegistration(const std::shared_ptr<internal::CParamValue<TVar>>& rptrParameter) :
                eType(EType::parameter), ptrParameter(rptrParameter)
            {}

            /**
             * @brief Constructor for a member parameter map
             * @param[in] rptrMember Reference to the smart pointer pointing to the member map.
             */
            SParamRegistration(const std::shared_ptr<internal::SMemberMapAccess>& rptrMember) :
                eType(EType::member_map), ptrMemberMap(rptrMember)
            {}

            /**
             * @brief Copy constructor
             */
            SParamRegistration(const SParamRegistration& rRegistration)
            {
                eType = rRegistration.eType;
                switch (rRegistration.eType)
                {
                case EType::parameter:
                    new (&ptrParameter) std::shared_ptr<internal::CParamGuardian>(rRegistration.ptrParameter);
                    break;
                case EType::member_map:
                    new (&ptrMemberMap) std::shared_ptr<internal::SMemberMapAccess>(rRegistration.ptrMemberMap);
                    break;
                }
            }

            /**
             * @brief Move constructor
             */
            SParamRegistration(SParamRegistration&& rRegistration)
            {
                eType = rRegistration.eType;
                switch (rRegistration.eType)
                {
                case EType::parameter:
                    new (&ptrParameter) std::shared_ptr<internal::CParamGuardian>(std::move(rRegistration.ptrParameter));
                    break;
                case EType::member_map:
                    new (&ptrMemberMap) std::shared_ptr<internal::SMemberMapAccess>(std::move(rRegistration.ptrMemberMap));
                    break;
                }
            }

            /**
             * @brief Destructor
             */
            ~SParamRegistration()
            {
                switch (eType)
                {
                case EType::parameter:
                    ptrParameter.~shared_ptr();
                    break;
                case EType::member_map:
                    ptrMemberMap.~shared_ptr();
                    break;
                }
            }

            /**
             * @brief Get the parameter if this structure is defined as parameter.
             * @return Smart pointer to the parameter or empty when there is no parameter.
             */
            std::shared_ptr<internal::CParamGuardian> Parameter() const
            {
                if (eType == EType::parameter)
                    return ptrParameter;
                return {};
            }

            /**
             * @brief Get the member parameter map if this structure is defined as member parameter map.
             * @return Smart pointer to the member map or empty when there is no member map.
            */
            std::shared_ptr<internal::SMemberMapAccess> MemberMap() const
            {
                if (eType == EType::member_map)
                    return ptrMemberMap;
                return {};
            }

        private:
            enum class EType {parameter, member_map}        eType;              ///< Type of registration
            union
            {
                std::shared_ptr<internal::CParamGuardian>   ptrParameter;       ///< Smart pointer to parameter of this map.
                std::shared_ptr<internal::SMemberMapAccess> ptrMemberMap;       ///< Smart pointer to a member containing another
                                                                                ///< parameter map.
            };
        };

        mutable std::vector<SParamRegistration>     m_vecParamMapRegistration;        ///< Vector with parameters.
    };
}; // namespace sdv

#include "param_impl.inl"

#endif // !defined PARAM_IMPL_H