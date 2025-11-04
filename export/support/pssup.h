#ifndef SDV_PS_SUPPORT_H
#define SDV_PS_SUPPORT_H

#include "../interfaces/core_ps.h"
#include "../interfaces/serdes/core_ps_serdes.h"
#include "../interfaces/serdes/core_types_serdes.h"

/**
 * @brief Compare two marshall IDs.
 * @param[in] rtID1 Reference to the first ID.
 * @param[in] rtID2 Reference to the second ID.
 * @return Returns whether the first ID is identical to the second ID.
 */
bool operator==(const sdv::ps::TMarshallID& rtID1, const sdv::ps::TMarshallID& rtID2);

/**
 * @brief Compare the marshall ID with zero.
 * @param[in] rtID1 Reference to the first ID.
 * @param[in] nVal The value to use for comparison (only 0 is allowed).
 * @return Returns whether the ID is zero.
 */
bool operator==(const sdv::ps::TMarshallID& rtID1, size_t nVal);

/**
 * @brief Compare the marshall ID with zero.
 * @param[in] nVal The value to use for comparison (only 0 is allowed).
 * @param[in] rtID2 Reference to the second ID.
 * @return Returns whether the ID is zero.
 */
bool operator==(size_t nVal, const sdv::ps::TMarshallID& rtID2);

/**
 * @brief Compare two marshall IDs.
 * @param[in] rtID1 Reference to the first ID.
 * @param[in] rtID2 Reference to the second ID.
 * @return Returns whether the first ID is different from the second ID.
 */
bool operator!=(const sdv::ps::TMarshallID& rtID1, const sdv::ps::TMarshallID& rtID2);

/**
 * @brief Compare the marshall ID with zero.
 * @param[in] rtID1 Reference to the first ID.
 * @param[in] nVal The value to use for comparison (only 0 is allowed).
 * @return Returns whether the ID is not zero.
 */
bool operator!=(const sdv::ps::TMarshallID& rtID1, size_t nVal);

/**
 * @brief Compare the marshall ID with zero.
 * @param[in] nVal The value to use for comparison (only 0 is allowed).
 * @param[in] rtID2 Reference to the second ID.
 * @return Returns whether the ID is not zero.
 */
bool operator!=(size_t nVal, const sdv::ps::TMarshallID& rtID2);

/**
 * @brief Compare two marshall IDs.
 * @param[in] rtID1 Reference to the first ID.
 * @param[in] rtID2 Reference to the second ID.
 * @return Returns whether the first ID is smaller than the second ID.
 */
bool operator<(const sdv::ps::TMarshallID& rtID1, const sdv::ps::TMarshallID& rtID2);

/**
 * @brief Compare two marshall IDs.
 * @param[in] rtID1 Reference to the first ID.
 * @param[in] rtID2 Reference to the second ID.
 * @return Returns whether the first ID is smaller or equal the than second ID.
 */
bool operator<=(const sdv::ps::TMarshallID& rtID1, const sdv::ps::TMarshallID& rtID2);

/**
 * @brief Compare two marshall IDs.
 * @param[in] rtID1 Reference to the first ID.
 * @param[in] rtID2 Reference to the second ID.
 * @return Returns whether the first ID is larger than the second ID.
 */
bool operator>(const sdv::ps::TMarshallID& rtID1, const sdv::ps::TMarshallID& rtID2);

/**
 * @brief Compare two marshall IDs.
 * @param[in] rtID1 Reference to the first ID.
 * @param[in] rtID2 Reference to the second ID.
 * @return Returns whether the first ID is larger or equal than the second ID.
 */
bool operator>=(const sdv::ps::TMarshallID& rtID1, const sdv::ps::TMarshallID& rtID2);

/**
 * @brief Check for validity of the marshall ID.
 * @param[in] rtID Reference to the ID.
 * @return Returns whether the ID is valid.
 */
bool operator!(const sdv::ps::TMarshallID& rtID);

#ifndef DOXYGEN_IGNORE
/**
 * @brief Specialization of std::less for sdv::ps::TMarshallID.
 */
template <>
struct std::less<sdv::ps::TMarshallID>
{
    /// The result type
    using result_type = bool;

    /// The first argument type.
    using first_argument_type = sdv::ps::TMarshallID;

    /// The second argument type.
    using second_argument_type = sdv::ps::TMarshallID;

    /**
     * @brief Compare ID1 < ID2.
     * @param[in] rtID1 Reference to the first ID.
     * @param[in] rtID2 Reference to the second ID.
     * @return Result of the comparison.
     */
    bool operator()(const sdv::ps::TMarshallID& rtID1, const sdv::ps::TMarshallID& rtID2) const { return ::operator<(rtID1, rtID2); }
};
#endif // !defined DOXYGEN_IGNORE

/**
 * @brief Compare two connection IDs.
 * @param[in] rtID1 Reference to the first ID.
 * @param[in] rtID2 Reference to the second ID.
 * @return Returns whether the first ID is identical to the second ID.
 */
bool operator==(const sdv::com::TConnectionID& rtID1, const sdv::com::TConnectionID& rtID2);

/**
 * @brief Compare the connection ID with zero.
 * @param[in] rtID1 Reference to the first ID.
 * @param[in] nVal The value to use for comparison (only 0 is allowed).
 * @return Returns whether the ID is zero.
 */
bool operator==(const sdv::com::TConnectionID& rtID1, size_t nVal);

/**
 * @brief Compare the connection ID with zero.
 * @param[in] nVal The value to use for comparison (only 0 is allowed).
 * @param[in] rtID2 Reference to the second ID.
 * @return Returns whether the ID is zero.
 */
bool operator==(size_t nVal, const sdv::com::TConnectionID& rtID2);

/**
* @brief Compare two connection IDs.
* @param[in] rtID1 Reference to the first ID.
* @param[in] rtID2 Reference to the second ID.
* @return Returns whether the first ID is different from the second ID.
*/
bool operator!=(const sdv::com::TConnectionID& rtID1, const sdv::com::TConnectionID& rtID2);

/**
 * @brief Compare the connection ID with zero.
 * @param[in] rtID1 Reference to the first ID.
 * @param[in] nVal The value to use for comparison (only 0 is allowed).
 * @return Returns whether the ID is not zero.
 */
bool operator!=(const sdv::com::TConnectionID& rtID1, size_t nVal);

/**
 * @brief Compare the connection ID with zero.
 * @param[in] nVal The value to use for comparison (only 0 is allowed).
 * @param[in] rtID2 Reference to the second ID.
 * @return Returns whether the ID is not zero.
 */
bool operator!=(size_t nVal, const sdv::com::TConnectionID& rtID2);

/**
 * @brief Compare two connection IDs.
 * @param[in] rtID1 Reference to the first ID.
 * @param[in] rtID2 Reference to the second ID.
 * @return Returns whether the first ID is smaller than the second ID.
 */
bool operator<(const sdv::com::TConnectionID& rtID1, const sdv::com::TConnectionID& rtID2);

/**
 * @brief Compare two connection IDs.
 * @param[in] rtID1 Reference to the first ID.
 * @param[in] rtID2 Reference to the second ID.
 * @return Returns whether the first ID is smaller or equal the than second ID.
 */
bool operator<=(const sdv::com::TConnectionID& rtID1, const sdv::com::TConnectionID& rtID2);

/**
 * @brief Compare two connection IDs.
 * @param[in] rtID1 Reference to the first ID.
 * @param[in] rtID2 Reference to the second ID.
 * @return Returns whether the first ID is larger than the second ID.
 */
bool operator>(const sdv::com::TConnectionID& rtID1, const sdv::com::TConnectionID& rtID2);

/**
 * @brief Compare two connection IDs.
 * @param[in] rtID1 Reference to the first ID.
 * @param[in] rtID2 Reference to the second ID.
 * @return Returns whether the first ID is larger or equal than the second ID.
 */
bool operator>=(const sdv::com::TConnectionID& rtID1, const sdv::com::TConnectionID& rtID2);

/**
 * @brief Check for validity of the connection ID.
 * @param[in] rtID Reference to the ID.
 * @return Returns whether the ID is valid.
 */
bool operator!(const sdv::com::TConnectionID& rtID);

#ifndef DOXYGEN_IGNORE
/**
 * @brief Specialization of std::less for sdv::com::TConnectionID.
 */
template <>
struct std::less<sdv::com::TConnectionID>
{
    /// The result type
    using result_type = bool;

    /// The first argument type.
    using first_argument_type = sdv::com::TConnectionID;

    /// The second argument type.
    using second_argument_type = sdv::com::TConnectionID;

    /**
     * @brief Compare ID1 < ID2.
     * @param[in] rtID1 Reference to the first ID.
     * @param[in] rtID2 Reference to the second ID.
     * @return Result of the comparison.
     */
    bool operator()(const sdv::com::TConnectionID& rtID1, const sdv::com::TConnectionID& rtID2) const { return ::operator<(rtID1, rtID2); }
};
#endif // !defined DOXYGEN_IGNORE

#include "sequence.h"
#include "pointer.h"
#include "crc.h"
#include "component_impl.h"
#include <cassert>
#include <thread>
#include <condition_variable>
#include <map>
#include <functional>
#include <chrono>
#include <queue>

namespace sdv
{
    namespace ps
    {
        /**
        * Local SMarshall structure to handle specific CRC calculations
        */
        struct SMarshallLocal : SMarshall
        {};

        /**
        * @brief Bypass class available during a function call. This class uses thread local storage to allow the storage of
        * raw data bypassing the serialization.
        */
        class CRawDataBypass
        {
        public:
            /**
            * @brief Constructor
            */
            CRawDataBypass();

            /**
            * @brief Add raw data to the raw data queue.
            * @param[in] rptrData Reference to the smart pointer holding the data.
            */
            void push(const pointer<uint8_t>& rptrData);

            /**
             * @brief Prop the data from the raw data queue.
             * @return The data to pop.
             */
            pointer<uint8_t> pop();

            /**
             * @brief Is the raw data queue empty?
             * @return Returns whether the queue is empty.
            */
            bool empty() const;

            /**
            * @brief Clear the raw data queue.
            */
            void clear();

        private:
             std::queue<pointer<uint8_t>> m_queueBypassData;    ///< Additional buffers not being serialized.
        };

        /**
        * @brief Global access function for the raw data bypass.
        * @return Reference to the bypass.
        */
        CRawDataBypass& GetRawDataBypass();

        /**
        * @brief Result enumeration of the call function.
        */
        enum class ECallResult : uint32_t
        {
            result_ok = 0,          ///< Normal processing. The return buffer contains the return values.
            result_exception = 1,   ///< Exception occurred. The return buffer contains the serialized exception.
        };

        /**
         * @brief Proxy class managing the function calls and incoming calls containing the response.
         * @tparam TInterface The interface the deriving class is deriving from as well.
         */
        template <typename TInterface>
        class CProxyHandler : public sdv::CSdvObject, public IMarshallObjectIdent, public IMarshallLink
        {
        public:
            // Ensure that the EEndian structure is 8 bits (to prevent byte swapping during detection).
            static_assert(sizeof(EEndian) == 1);

            /**
             * @brief Constructor
             */
            CProxyHandler();

            /**
             * @brief Constructor
             */
            virtual ~CProxyHandler() override;

            // Interface map
            BEGIN_SDV_INTERFACE_MAP()
                SDV_INTERFACE_CHAIN_BASE(sdv::CSdvObject)
                SDV_INTERFACE_ENTRY(IMarshallObjectIdent)
                SDV_INTERFACE_ENTRY(IMarshallLink)
            END_SDV_INTERFACE_MAP()

            // Object class type
            DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::Proxy)

            /**
             * @brief Set the identification. Overload of IMarshallObjectIdent::SetIdentification.
             * @param[in] tMarshallID Reference to the marshall object ID. For a proxy object, this is the proxy ID. For a stub object
             * this is a stub ID.
             */
            virtual void SetIdentification(/*in*/ const TMarshallID& tMarshallID) override;

            /**
            * @brief Link the communication interface to the object. Overload of IMarshallLink::Link.
            * @remarks Only one link can exists at the time.
            * @param[in] pMarshall Interface to be linked.
            */
            virtual void Link(/*in*/ IMarshall* pMarshall) override;

            /**
            * @brief Unlink the linked interface. Overload of IMarshallLink::Unlink.
            */
            virtual void Unlink() override;

            /**
             * @brief Schedule a call.
             * @param[in] uiFuncIndex Operation/attribute index.
             * @param[in] rserInput Serializer containing the input parameters.
             * @param[in] rdesOutput Deserializer containing the return value and output parameters or the exception.
             * @return The call result value.
             */
            ECallResult DoCall(uint32_t uiFuncIndex, const serializer<GetPlatformEndianess()>& rserInput,
                deserializer<GetPlatformEndianess()>& rdesOutput);

        private:
            IMarshall*      m_pRequest = nullptr;       ///< Marshall interface for call requests.
            TMarshallID     m_tProxyID = {};             ///< Proxy handler ID.
            TMarshallID     m_tStubID = {};              ///< Stub handler ID.
        };

        /**
         * @brief Stub class managing incoming calls containing the request and dispatching them to the functions.
         * @tparam TInterface The interface this stub is dispatching to.
         */
        template <typename TInterface>
        class CStubHandler : public sdv::CSdvObject, public IMarshallObjectIdent, public IMarshall
        {
            // Ensure that the EEndian structure is 8 bits (to prevent byte swapping during detection).
            static_assert(sizeof(EEndian) == 1);

        public:
            /**
             * @brief Constructor
             */
            CStubHandler();

            /**
             * @brief Destructor
             */
            virtual ~CStubHandler() override;

            // Interface map
            BEGIN_SDV_INTERFACE_MAP()
                SDV_INTERFACE_CHAIN_BASE(sdv::CSdvObject)
                SDV_INTERFACE_ENTRY(IMarshallObjectIdent)
                SDV_INTERFACE_ENTRY(IMarshall)
            END_SDV_INTERFACE_MAP()

            // Object class type
            DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::Stub)

        protected:
            /**
             * @brief Dispatch function type. Function arguments are endianness, input parameters and output
             * parameters/return value/exception information. Function returns the dispact result.
             */
            typedef std::function<ECallResult(EEndian, const pointer<uint8_t>&, pointer<uint8_t>&)> FNDispatch;

            /**
             * @brief Register dispatch function.
             * @remarks This dispatch functions must be registered in the correct order.
             * @attention Registering dispatch functions should happen during construction. No protection of the vector is
             * available during data dispatching.
             * @param[in] fnDispatch Dispatch function to call for this interface.
             */
            void RegisterDispatchFunc(FNDispatch fnDispatch);

            /**
             * @brief Serialize exception helper function.
             * @tparam TExcept The exception type to serialize.
             * @param[in] eEndian The endianness of the serialization.
             * @param[in] rexcept Reference to the exception.
             * @return The serialized exception.
            */
            template <typename TExcept>
            pointer<uint8_t> SerializeException(EEndian eEndian, const TExcept& rexcept);

            /**
             * @brief Set the identification. Overload of IMarshallObjectIdent::SetIdentification.
             * @param[in] tMarshallID Reference to The marshall object ID. For a proxy object, this is the proxy ID. For a stub object
             * this is a stub ID.
             */
            virtual void SetIdentification(/*in*/ const TMarshallID& tMarshallID) override;

            /**
             * @brief Marshall a function call. Overload of IMarshall::Call.
             * @remarks This function call is synchronous and does not return until the call has been finalized or a timeout
             * exception has occurred.
             * @remarks The sequence contains all data to make the call. It is important that the data in the sequence is
             * complete and in the correct order.
             * @param[inout] seqInputData Reference to sequence of input data pointers. The first data pointer contains the
             * marshalling header. The second contains the parameters (if available) and the others contain raw data pointers
             * (if available). The call is allowed to change the sequence to be able to add additional information during the
             * communication without having to copy the existing data.
             * @return Sequence of output data pointers. The first data pointer contains the marshalling header. The second
             * contains the return value and parameters (if available) and the others contain raw data pointers (if available).
             */
            virtual sequence<pointer<uint8_t>> Call(/*inout*/ sequence<pointer<uint8_t>>& seqInputData) override;

        private:
            std::vector<FNDispatch>     m_vecDispatch;  ///< Vector containing the dispatch functions.
            TMarshallID                 m_tStubID{};    ///< Stub handler ID.
        };
    } // namespace ps
} // namespace sdv

#include "pssup.inl"

#endif // !defined(SDV_PS_SUPPORT_H)
