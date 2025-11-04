#ifndef SDV_PS_SUPPORT_INL
#define SDV_PS_SUPPORT_INL

#ifndef SDV_PS_SUPPORT_H
#error Do not include "pssup.inl" directly. Include "pssup.h" instead!
#endif //!defined SDV_PS_SUPPORT_H

#include "../interfaces/core_ps.h"
#include "../interfaces/serdes/core_ps_serdes.h"
#include "../interfaces/serdes/core_types_serdes.h"
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

namespace serdes
{
    /**
    * @brief Specialization of serializer/deserializer class for sdv::interface_t.
    */
    template <>
    class CSerdes<sdv::interface_t>
    {
    public:
        /**
         * @brief Calculate the size of the value in serialized form.
         * @remarks Dependable on the size value, padding is added to align data.
         * @param[in] rifc Reference to the interface variable.
         * @param[in, out] rnSize Reference to the variable containing the current size and increased by the size of the value.
         */
        static void CalcSize(const sdv::interface_t& rifc, size_t& rnSize)
        {
            sdv::ser_size(rifc.id(), rnSize);
            sdv::ser_size(sdv::ps::TMarshallID(), rnSize);
        }

        /**
         * @brief Stream the variable into the serializer.
         * @tparam eTargetEndianess The target endianess detemines whether to swap the bytes before storing them into the buffer.
         * @param[in] rSerializer Reference to the serializer.
         * @param[in] rifc Reference to the variable.
         * @return Reference to the serializer.
         */
        template <sdv::EEndian eTargetEndianess>
        static sdv::serializer<eTargetEndianess>& Serialize(sdv::serializer<eTargetEndianess>& rSerializer,
            const sdv::interface_t& rifc)
        {
            // Get interface to the component isolation service.
            auto ptrComControl = sdv::core::GetObject("CommunicationControl");
            if (!ptrComControl) throw sdv::ps::XMarshallNotInitialized{};
            sdv::ps::IMarshallAccess* pMarshallAccess = ptrComControl.GetInterface<sdv::ps::IMarshallAccess>();
            if (!pMarshallAccess) throw sdv::ps::XMarshallNotInitialized{};

            // Serialize the interface ID first
            rSerializer << rifc.id();

            // Create and serialize a stub object for the interface.
            sdv::ps::TMarshallID tStubID = pMarshallAccess->GetStub(rifc);
            rSerializer << tStubID;

            return rSerializer;
        }

        /**
         * @brief Stream a variable from the deserializer.
         * @tparam eSourceEndianess The target endianess detemines whether to swap the bytes before storing them into the buffer.
         * @param[in] rDeserializer Reference to the deserializer.
         * @param[out] rifc Reference to the variable to be filled.
         * @return Reference to the deserializer.
         */
        template <sdv::EEndian eSourceEndianess>
        static sdv::deserializer<eSourceEndianess>& Deserialize(sdv::deserializer<eSourceEndianess>& rDeserializer,
            sdv::interface_t& rifc)
        {
            // Get interface to the component isolation service.
            auto ptrComControl = sdv::core::GetObject("CommunicationControl");
            if (!ptrComControl) throw sdv::ps::XMarshallNotInitialized{};
            sdv::ps::IMarshallAccess* pMarshallAccess = ptrComControl.GetInterface<sdv::ps::IMarshallAccess>();
            if (!pMarshallAccess) throw sdv::ps::XMarshallNotInitialized{};

            // Get the interface ID
            sdv::interface_id id = 0;
            rDeserializer >> id;

            // Get the stub ID
            sdv::ps::TMarshallID tStubID{};
            rDeserializer >> tStubID;

            // Create the proxy
            if (!tStubID && !id) // In case the ID is zero, an NULL interface was sent.
                rifc = {};
            else
                rifc = pMarshallAccess->GetProxy(tStubID, id);

            return rDeserializer;
        }
    };

    /**
    * @brief Specialization of serializer/deserializer class for sdv::any_t.
    */
    template <>
    class CSerdes<sdv::any_t>
    {
    public:
        /**
         * @brief Calculate the size of the value in serialized form.
         * @remarks Dependable on the size value, padding is added to align data.
         * @param[in] rany Reference to the variable.
         * @param[in, out] rnSize Reference to the variable containing the current size and increased by the size of the value.
         */
        static void CalcSize(const sdv::any_t& rany, size_t& rnSize)
        {
            // The size of the type
            sdv::ser_size(rany.eValType, rnSize);

            // Added with the size of the variable.
            switch (rany.eValType)
            {
            case sdv::any_t::EValType::val_type_bool:           sdv::ser_size(rany.bVal, rnSize);            break;
            case sdv::any_t::EValType::val_type_int8:           sdv::ser_size(rany.i8Val, rnSize);           break;
            case sdv::any_t::EValType::val_type_uint8:          sdv::ser_size(rany.ui8Val, rnSize);          break;
            case sdv::any_t::EValType::val_type_int16:          sdv::ser_size(rany.i16Val, rnSize);          break;
            case sdv::any_t::EValType::val_type_uint16:         sdv::ser_size(rany.ui16Val, rnSize);         break;
            case sdv::any_t::EValType::val_type_int32:          sdv::ser_size(rany.i32Val, rnSize);          break;
            case sdv::any_t::EValType::val_type_uint32:         sdv::ser_size(rany.ui32Val, rnSize);         break;
            case sdv::any_t::EValType::val_type_int64:          sdv::ser_size(rany.i64Val, rnSize);          break;
            case sdv::any_t::EValType::val_type_uint64:         sdv::ser_size(rany.ui64Val, rnSize);         break;
            case sdv::any_t::EValType::val_type_char:           sdv::ser_size(rany.cVal, rnSize);            break;
            case sdv::any_t::EValType::val_type_char16:         sdv::ser_size(rany.c16Val, rnSize);          break;
            case sdv::any_t::EValType::val_type_char32:         sdv::ser_size(rany.c32Val, rnSize);          break;
            case sdv::any_t::EValType::val_type_wchar:          sdv::ser_size(rany.cwVal, rnSize);           break;
            case sdv::any_t::EValType::val_type_float:          sdv::ser_size(rany.fVal, rnSize);            break;
            case sdv::any_t::EValType::val_type_double:         sdv::ser_size(rany.dVal, rnSize);            break;
            case sdv::any_t::EValType::val_type_long_double:    sdv::ser_size(rany.ldVal, rnSize);           break;
            //case sdv::any_t::EValType::val_type_fixed:          sdv::ser_size(rany.fixVal, rnSize);          break;
            case sdv::any_t::EValType::val_type_string:         sdv::ser_size(rany.ssVal, rnSize);           break;
            case sdv::any_t::EValType::val_type_u8string:       sdv::ser_size(rany.ss8Val, rnSize);          break;
            case sdv::any_t::EValType::val_type_u16string:      sdv::ser_size(rany.ss16Val, rnSize);         break;
            case sdv::any_t::EValType::val_type_u32string:      sdv::ser_size(rany.ss32Val, rnSize);         break;
            case sdv::any_t::EValType::val_type_wstring:        sdv::ser_size(rany.sswVal, rnSize);          break;
            case sdv::any_t::EValType::val_type_interface:      sdv::ser_size(rany.ifcVal, rnSize);          break;
            case sdv::any_t::EValType::val_type_interface_id:   sdv::ser_size(rany.idIfcVal, rnSize);        break;
            case sdv::any_t::EValType::val_type_exception_id:   sdv::ser_size(rany.idExceptVal, rnSize);     break;
            default:                                                                                         break;
            }
        }

        /**
         * @brief Stream the variable into the serializer.
         * @tparam eTargetEndianess The target endianess detemines whether to swap the bytes before storing them into the buffer.
         * @param[in] rSerializer Reference to the serializer.
         * @param[in] rany Reference to the variable.
         * @return Reference to the serializer.
         */
        template <sdv::EEndian eTargetEndianess>
        static sdv::serializer<eTargetEndianess>& Serialize(sdv::serializer<eTargetEndianess>& rSerializer, const sdv::any_t& rany)
        {
            // Serialize the type first
            rSerializer << rany.eValType;

            // Serialize the data
            switch (rany.eValType)
            {
            case sdv::any_t::EValType::val_type_bool:           rSerializer << rany.bVal;            break;
            case sdv::any_t::EValType::val_type_int8:           rSerializer << rany.i8Val;           break;
            case sdv::any_t::EValType::val_type_uint8:          rSerializer << rany.ui8Val;          break;
            case sdv::any_t::EValType::val_type_int16:          rSerializer << rany.i16Val;          break;
            case sdv::any_t::EValType::val_type_uint16:         rSerializer << rany.ui16Val;         break;
            case sdv::any_t::EValType::val_type_int32:          rSerializer << rany.i32Val;          break;
            case sdv::any_t::EValType::val_type_uint32:         rSerializer << rany.ui32Val;         break;
            case sdv::any_t::EValType::val_type_int64:          rSerializer << rany.i64Val;          break;
            case sdv::any_t::EValType::val_type_uint64:         rSerializer << rany.ui64Val;         break;
            case sdv::any_t::EValType::val_type_char:           rSerializer << rany.cVal;            break;
            case sdv::any_t::EValType::val_type_char16:         rSerializer << rany.c16Val;          break;
            case sdv::any_t::EValType::val_type_char32:         rSerializer << rany.c32Val;          break;
            case sdv::any_t::EValType::val_type_wchar:          rSerializer << rany.cwVal;           break;
            case sdv::any_t::EValType::val_type_float:          rSerializer << rany.fVal;            break;
            case sdv::any_t::EValType::val_type_double:         rSerializer << rany.dVal;            break;
            case sdv::any_t::EValType::val_type_long_double:    rSerializer << rany.ldVal;           break;
                //case sdv::any_t::EValType::val_type_fixed:          rSerializer << rany.fixVal;          break;
            case sdv::any_t::EValType::val_type_string:         rSerializer << rany.ssVal;           break;
            case sdv::any_t::EValType::val_type_u8string:       rSerializer << rany.ss8Val;          break;
            case sdv::any_t::EValType::val_type_u16string:      rSerializer << rany.ss16Val;         break;
            case sdv::any_t::EValType::val_type_u32string:      rSerializer << rany.ss32Val;         break;
            case sdv::any_t::EValType::val_type_wstring:        rSerializer << rany.sswVal;          break;
            case sdv::any_t::EValType::val_type_interface:      rSerializer << rany.ifcVal;          break;
            case sdv::any_t::EValType::val_type_interface_id:   rSerializer << rany.idIfcVal;        break;
            case sdv::any_t::EValType::val_type_exception_id:   rSerializer << rany.idExceptVal;     break;
            default:                                                                     break;
            }

            return rSerializer;
        }

        /**
         * @brief Stream a variable from the deserializer.
         * @tparam eSourceEndianess The target endianess detemines whether to swap the bytes before storing them into the buffer.
         * @param[in] rDeserializer Reference to the deserializer.
         * @param[out] rany Reference to the variable to be filled.
         * @return Reference to the deserializer.
         */
        template <sdv::EEndian eSourceEndianess>
        static sdv::deserializer<eSourceEndianess>& Deserialize(sdv::deserializer<eSourceEndianess>& rDeserializer, sdv::any_t& rany)
        {
            rany.clear();

            // Deserialize the type first
            rDeserializer >> rany.eValType;

            // Serialize the data
            switch (rany.eValType)
            {
            case sdv::any_t::EValType::val_type_bool:           rDeserializer >> rany.bVal;         break;
            case sdv::any_t::EValType::val_type_int8:           rDeserializer >> rany.i8Val;        break;
            case sdv::any_t::EValType::val_type_uint8:          rDeserializer >> rany.ui8Val;       break;
            case sdv::any_t::EValType::val_type_int16:          rDeserializer >> rany.i16Val;       break;
            case sdv::any_t::EValType::val_type_uint16:         rDeserializer >> rany.ui16Val;      break;
            case sdv::any_t::EValType::val_type_int32:          rDeserializer >> rany.i32Val;       break;
            case sdv::any_t::EValType::val_type_uint32:         rDeserializer >> rany.ui32Val;      break;
            case sdv::any_t::EValType::val_type_int64:          rDeserializer >> rany.i64Val;       break;
            case sdv::any_t::EValType::val_type_uint64:         rDeserializer >> rany.ui64Val;      break;
            case sdv::any_t::EValType::val_type_char:           rDeserializer >> rany.cVal;         break;
            case sdv::any_t::EValType::val_type_char16:         rDeserializer >> rany.c16Val;       break;
            case sdv::any_t::EValType::val_type_char32:         rDeserializer >> rany.c32Val;       break;
            case sdv::any_t::EValType::val_type_wchar:          rDeserializer >> rany.cwVal;        break;
            case sdv::any_t::EValType::val_type_float:          rDeserializer >> rany.fVal;         break;
            case sdv::any_t::EValType::val_type_double:         rDeserializer >> rany.dVal;         break;
            case sdv::any_t::EValType::val_type_long_double:    rDeserializer >> rany.ldVal;        break;
                //case sdv::any_t::EValType::val_type_fixed:        new (&rany.fixVal) sdv::fixed; rDeserializer >> rany.fixVal;       break;
            case sdv::any_t::EValType::val_type_string:         new (&rany.ssVal) sdv::string; rDeserializer >> rany.ssVal;        break;
            case sdv::any_t::EValType::val_type_u8string:       new (&rany.ss8Val) sdv::u8string; rDeserializer >> rany.ss8Val;    break;
            case sdv::any_t::EValType::val_type_u16string:      new (&rany.ss16Val) sdv::u16string; rDeserializer >> rany.ss16Val; break;
            case sdv::any_t::EValType::val_type_u32string:      new (&rany.ss32Val) sdv::u32string; rDeserializer >> rany.ss32Val; break;
            case sdv::any_t::EValType::val_type_wstring:        new (&rany.sswVal) sdv::wstring; rDeserializer >> rany.sswVal;     break;
            case sdv::any_t::EValType::val_type_interface:      new (&rany.ifcVal) sdv::interface_t; rDeserializer >> rany.ifcVal; break;
            case sdv::any_t::EValType::val_type_interface_id:   rDeserializer >> rany.idIfcVal;     break;
            case sdv::any_t::EValType::val_type_exception_id:   rDeserializer >> rany.idExceptVal;  break;
            default: rany.eValType = sdv::any_t::EValType::val_type_empty;                          break;
            }

            return rDeserializer;
        }
    };

    /**
    * @brief Specialization of serializer/deserializer class for sdv::ps::SMarshallLocal.
    */
    template <>
    class CSerdes<sdv::ps::SMarshallLocal>
    {
    public:
        /**
         * @brief Calculate the size of the value in serialized form.
         * @remarks Dependable on the size value, padding is added to align data.
         * @param[in] rsValue Reference to the variable.
         * @param[in, out] rnSize Reference to the variable containing the current size and increased by the size of the value.
         */
        static void CalcSize(const sdv::ps::SMarshallLocal& rsValue, size_t& rnSize)
        {
            sdv::ser_size(rsValue.eEndian, rnSize);
            sdv::ser_size(rsValue.uiVersion, rnSize);
            sdv::ser_size(rsValue.tIfcId, rnSize);
            sdv::ser_size(rsValue.uiFuncIndex, rnSize);
            sdv::ser_size(rsValue.uiFlags, rnSize);
            sdv::ser_size(rsValue.seqChecksums, rnSize);
            sdv::ser_size(rsValue.uiHdrChecksum, rnSize);
        }

        /**
         * @brief Stream the variable into the serializer.
         * @tparam eTargetEndianess The target endianess detemines whether to swap the bytes before storing them into the buffer.
         * @param[in] rSerializer Reference to the serializer.
         * @param[in] rsValue Reference to the variable.
         * @return Reference to the serializer.
         */
        template <sdv::EEndian eTargetEndianess>
        static sdv::serializer<eTargetEndianess>& Serialize(sdv::serializer<eTargetEndianess>& rSerializer,
            sdv::ps::SMarshallLocal& rsValue)
        {
            // Stream the initial parameters.
            rSerializer << rsValue.eEndian;
            rSerializer << rsValue.uiVersion;
            rSerializer << rsValue.tIfcId;
            rSerializer << rsValue.uiFuncIndex;
            rSerializer << rsValue.uiFlags;
            rSerializer << rsValue.seqChecksums;

            // Get the currently calculated checksum and store the checksum.
            rsValue.uiHdrChecksum = rSerializer.checksum();

            // Stream the checksum.
            rSerializer << rsValue.uiHdrChecksum;

            return rSerializer;
        }

        /**
         * @brief Stream a variable from the deserializer.
         * @tparam eSourceEndianess The target endianess detemines whether to swap the bytes before storing them into the buffer.
         * @param[in] rDeserializer Reference to the deserializer.
         * @param[out] rsValue Reference to the variable to be filled.
         * @return Reference to the deserializer.
         */
        template <sdv::EEndian eSourceEndianess>
        static sdv::deserializer<eSourceEndianess>& Deserialize(sdv::deserializer<eSourceEndianess>& rDeserializer,
            sdv::ps::SMarshallLocal& rsValue)
        {
            // Stream the initial parameters.
            rDeserializer >> rsValue.eEndian;
            rDeserializer >> rsValue.uiVersion;

            // Continue only when the version is corresponding with the interface version (otherwise the order might be
            // mixed up).
            if (rsValue.uiVersion == SDVFrameworkInterfaceVersion)
            {
                rDeserializer >> rsValue.tIfcId;
                rDeserializer >> rsValue.uiFuncIndex;
                rDeserializer >> rsValue.uiFlags;
                rDeserializer >> rsValue.seqChecksums;
                rDeserializer >> rsValue.uiHdrChecksum;
            }

            return rDeserializer;
        }
    };
} // namespace serdes

namespace sdv
{
    namespace ps
    {
        inline CRawDataBypass::CRawDataBypass()
        {}

        inline void CRawDataBypass::push(const pointer<uint8_t>& rptrData)
        {
            m_queueBypassData.push(rptrData);
        }

        inline pointer<uint8_t> CRawDataBypass::pop()
        {
            pointer<uint8_t> ptrData = std::move(m_queueBypassData.front());
            m_queueBypassData.pop();
            return ptrData;
        }

        inline bool CRawDataBypass::empty() const
        {
            return m_queueBypassData.empty();
        }

        inline void CRawDataBypass::clear()
        {
            while (!m_queueBypassData.empty())
                m_queueBypassData.pop();
        }

        inline CRawDataBypass& GetRawDataBypass()
        {
            thread_local static CRawDataBypass bypass;
            return bypass;
        }

        template <typename TInterface>
        inline CProxyHandler<TInterface>::CProxyHandler()
        {}

        template <typename TInterface>
        inline CProxyHandler<TInterface>::~CProxyHandler()
        {}

        template <typename TInterface>
        inline void CProxyHandler<TInterface>::SetIdentification(/*in*/ const TMarshallID& tMarshallID)
        {
            m_tProxyID = tMarshallID;
        }

        template <typename TInterface>
        inline void CProxyHandler<TInterface>::Link(/*in*/ IMarshall* pMarshall)
        {
            assert(pMarshall);
            m_pRequest = pMarshall;
        }

        template <typename TInterface>
        inline void CProxyHandler<TInterface>::Unlink()
        {
            m_pRequest = nullptr;
        }

        template <typename TInterface>
        inline ECallResult CProxyHandler<TInterface>::DoCall(uint32_t uiFuncIndex,
            const serializer<GetPlatformEndianess()>& rserInput, deserializer<GetPlatformEndianess()>& rdesOutput)
        {
            // Needs a valid request interface
            if (!m_pRequest) throw XMarshallNotInitialized{};

            // The sequence holding the input data pointers
            sequence<pointer<uint8_t>> seqInputData;

            // Fill the marshall packet
            SMarshallLocal sInputPacket{};
            sInputPacket.eEndian = GetPlatformEndianess();
            sInputPacket.uiVersion = SDVFrameworkInterfaceVersion;
            sInputPacket.tIfcId = TInterface::_id;
            sInputPacket.uiFuncIndex = uiFuncIndex;

            // Are there any parameters?
            if (rserInput.buffer())
            {
                // Add the serialized parameters to the marshall sequence
                seqInputData.push_back(rserInput.buffer());
                sInputPacket.seqChecksums.push_back(rserInput.checksum());

                // Add additional raw data
                while (!GetRawDataBypass().empty())
                {
                    // Calculate the checksum
                    crcCCITT_FALSE crc;
                    pointer<uint8_t> ptrRawData = GetRawDataBypass().pop();
                    crcCCITT_FALSE crcBypass;
                    uint16_t uiChecksum = crcBypass.calc_checksum(ptrRawData.get(), ptrRawData.size());

                    // Add the raw data to the marshall sequence
                    seqInputData.push_back(ptrRawData);
                    sInputPacket.seqChecksums.push_back(uiChecksum);
                }
            }

            // Create an additional stream for the marshall struct.
            serializer serHdr;
            serdes::CSerdes<SMarshallLocal>::Serialize(serHdr, sInputPacket);

            // Add the packet to the top of the pointer sequence.
            seqInputData.insert(seqInputData.begin(), serHdr.buffer());

            // Call the request; this will update the sequence with result information
            sequence<pointer<uint8_t>> seqOutputData = m_pRequest->Call(seqInputData);
            if (seqOutputData.empty()) throw XMarshallMissingData{};

            // The first data pointer in the sequence contains the marshall header
            pointer<uint8_t> ptrHeader = seqOutputData[0];
            if (!ptrHeader) throw ps::XMarshallMissingData{};

            // And the first byte in the data pointer determines the endianess
            EEndian eSourceEndianess = static_cast<EEndian>(ptrHeader[0]);

            // Deserialize the packet
            SMarshallLocal sOutputPacket{};
            if (eSourceEndianess == EEndian::big_endian)
            {
                deserializer<EEndian::big_endian> deserializer;
                deserializer.attach(ptrHeader, 0);  // Do not check checksum...
                serdes::CSerdes<SMarshallLocal>::Deserialize(deserializer, sOutputPacket);
            } else
            {
                deserializer<EEndian::little_endian> deserializer;
                deserializer.attach(ptrHeader, 0);  // Do not check checksum...
                serdes::CSerdes<SMarshallLocal>::Deserialize(deserializer, sOutputPacket);
            }

            // Check the packet version... must fit, otherwise data misalignment might occur.
            if (sOutputPacket.uiVersion != SDVFrameworkInterfaceVersion) throw XMarshallVersion{};

            // The checksum can be calculated by serializing the packet once more.
            uint16_t uiHdrChecksum = sOutputPacket.uiHdrChecksum;
            serializer serializer;
            serdes::CSerdes<SMarshallLocal>::Serialize(serializer, sOutputPacket);
            // Ignore cppcheck suppress warning is always false. The checksum is written into the packet again. If previously it
            // was different, this causes an integrity exception.
            // cppcheck-suppress knownConditionTrueFalse
            if (sOutputPacket.uiHdrChecksum != uiHdrChecksum)
                throw XMarshallIntegrity{};

            // Check whether the result contains an exception
            if (sOutputPacket.uiFlags & static_cast<uint32_t>(sdv::ps::EMarshallFlags::exception_triggered))
            {
                // Check the checksum of the data
                if (seqOutputData.size() != 2) throw XMarshallIntegrity{};
                if (sOutputPacket.seqChecksums.size() != 1) throw XMarshallIntegrity{};
                crcCCITT_FALSE crc;
                if (crc.calc_checksum(seqOutputData[1].get(), seqOutputData[1].size()) != sOutputPacket.seqChecksums[0])
                    throw XMarshallIntegrity{};

                // Attach the data to the deserializer.
                rdesOutput.attach(seqOutputData[1]);

                // Check for system and/or proxy-stub exceptions
                sdv::exception_id id = 0;
                rdesOutput.peek_front(id);
                switch(id)
                {
                case GetExceptionId<XNoInterface>():
                {
                    XNoInterface exception;
                    serdes::CSerdes<XNoInterface>::Deserialize(rdesOutput, exception);
                    throw exception;
                }
                case GetExceptionId<XIndexOutOfRange>():
                {
                    XIndexOutOfRange exception;
                    serdes::CSerdes<XIndexOutOfRange>::Deserialize(rdesOutput, exception);
                    throw exception;
                }
                case GetExceptionId<XInvalidIterator>():
                {
                    XInvalidIterator exception;
                    serdes::CSerdes<XInvalidIterator>::Deserialize(rdesOutput, exception);
                    throw exception;
                }
                case GetExceptionId<XNullPointer>():
                {
                    XNullPointer exception;
                    serdes::CSerdes<XNullPointer>::Deserialize(rdesOutput, exception);
                    throw exception;
                }
                case GetExceptionId<XInvalidRefCount>():
                {
                    XInvalidRefCount exception;
                    serdes::CSerdes<XInvalidRefCount>::Deserialize(rdesOutput, exception);
                    throw exception;
                }
                case GetExceptionId<XBufferTooSmall>():
                {
                    XBufferTooSmall exception;
                    serdes::CSerdes<XBufferTooSmall>::Deserialize(rdesOutput, exception);
                    throw exception;
                }
                case GetExceptionId<XHashNotMatching>():
                {
                    XHashNotMatching exception;
                    serdes::CSerdes<XHashNotMatching>::Deserialize(rdesOutput, exception);
                    throw exception;
                }
                case GetExceptionId<XOffsetPastBufferSize>():
                {
                    XOffsetPastBufferSize exception;
                    serdes::CSerdes<XOffsetPastBufferSize>::Deserialize(rdesOutput, exception);
                    throw exception;
                }
                case GetExceptionId<XUnknownException>():
                {
                    XUnknownException exception;
                    serdes::CSerdes<XUnknownException>::Deserialize(rdesOutput, exception);
                    throw exception;
                }
                case GetExceptionId<XUnhandledException>():
                {
                    XUnhandledException exception;
                    serdes::CSerdes<XUnhandledException>::Deserialize(rdesOutput, exception);
                    throw exception;
                }
                case GetExceptionId<core::XNoMemMgr>():
                {
                    core::XNoMemMgr exception;
                    serdes::CSerdes<core::XNoMemMgr>::Deserialize(rdesOutput, exception);
                    throw exception;
                }
                case GetExceptionId<core::XAllocFailed>():
                {
                    core::XAllocFailed exception;
                    serdes::CSerdes<core::XAllocFailed>::Deserialize(rdesOutput, exception);
                    throw exception;
                }
                case GetExceptionId<XMarshallNotInitialized>():
                {
                    XMarshallNotInitialized exception;
                    serdes::CSerdes<XMarshallNotInitialized>::Deserialize(rdesOutput, exception);
                    throw exception;
                }
                case GetExceptionId<XMarshallTimeout>():
                {
                    XMarshallTimeout exception;
                    serdes::CSerdes<XMarshallTimeout>::Deserialize(rdesOutput, exception);
                    throw exception;
                }
                case GetExceptionId<XMarshallMissingData>():
                {
                    XMarshallMissingData exception;
                    serdes::CSerdes<XMarshallMissingData>::Deserialize(rdesOutput, exception);
                    throw exception;
                }
                case GetExceptionId<XMarshallVersion>():
                {
                    XMarshallVersion exception;
                    serdes::CSerdes<XMarshallVersion>::Deserialize(rdesOutput, exception);
                    throw exception;
                }
                case GetExceptionId<XMarshallIntegrity>():
                {
                    XMarshallIntegrity exception;
                    serdes::CSerdes<XMarshallIntegrity>::Deserialize(rdesOutput, exception);
                    throw exception;
                }
                default:
                    break;
                }

                // No known exception, let the caller check whether it can handle the exception.
                return ECallResult::result_exception;
            }

            // Check for packet integrity and add the raw data to the raw data bypass
            if (sOutputPacket.seqChecksums.size() != seqOutputData.size() - 1) throw XMarshallIntegrity{};
            size_t nChecksumIndex = 0;
            pointer<uint8_t> ptrParams;
            GetRawDataBypass().clear();
            for (auto itData = seqOutputData.begin() + 1; itData != seqOutputData.end(); itData++)
            {
                crcCCITT_FALSE crc;
                if (crc.calc_checksum(itData->get(), itData->size()) != sOutputPacket.seqChecksums[nChecksumIndex])
                    throw XMarshallIntegrity{};
                if (nChecksumIndex)
                    GetRawDataBypass().push(*itData);
                else
                    ptrParams = *itData;
                nChecksumIndex++;
            }

            // Return the deserialization object.
            if (ptrParams)
                rdesOutput.attach(ptrParams);
            return ECallResult::result_ok;
        }

        template <typename TInterface>
        inline CStubHandler<TInterface>::CStubHandler()
        {}

        template <typename TInterface>
        inline CStubHandler<TInterface>::~CStubHandler()
        {}

        template <typename TInterface>
        inline void CStubHandler<TInterface>::RegisterDispatchFunc(FNDispatch fnDispatch)
        {
            m_vecDispatch.push_back(fnDispatch);
        }

        template <typename TInterface>
        template <typename TExcept>
        inline pointer<uint8_t> CStubHandler<TInterface>::SerializeException(EEndian eEndian, const TExcept& rexcept)
        {
            GetRawDataBypass().clear(); // just in case
            if (eEndian == sdv::EEndian::big_endian)
            {
                sdv::serializer<sdv::EEndian::big_endian> serOutput;
                serdes::CSerdes<TExcept>::Serialize(serOutput, rexcept);
                return serOutput.buffer();
            } else
            {
                sdv::serializer<sdv::EEndian::little_endian> serOutput;
                serdes::CSerdes<TExcept>::Serialize(serOutput, rexcept);
                return serOutput.buffer();
            }
        }

        template <typename TInterface>
        inline void CStubHandler<TInterface>::SetIdentification(/*in*/ const TMarshallID& tMarshallID)
        {
            m_tStubID = tMarshallID;
        }

        template <typename TInterface>
        inline sequence<pointer<uint8_t>> CStubHandler<TInterface>::Call(/*inout*/ sequence<pointer<uint8_t>>& seqInputData)
        {
            if (seqInputData.empty()) throw XMarshallMissingData{};

            // The first data pointer in the sequence contains the marshall header
            pointer<uint8_t> ptrHeader = seqInputData[0];
            if (!ptrHeader) throw ps::XMarshallMissingData{};

            // And the first byte in the data pointer determines the endianess
            EEndian eSourceEndianess = static_cast<EEndian>(ptrHeader[0]);

            // Deserialize the packet
            SMarshallLocal sInputPacket{};
            if (eSourceEndianess == EEndian::big_endian)
            {
                deserializer<EEndian::big_endian> desInput;
                desInput.attach(ptrHeader, 0);  // Do not check checksum...
                serdes::CSerdes<SMarshallLocal>::Deserialize(desInput, sInputPacket);
            } else
            {
                deserializer<EEndian::little_endian> desInput;
                desInput.attach(ptrHeader, 0);  // Do not check checksum...
                serdes::CSerdes<SMarshallLocal>::Deserialize(desInput, sInputPacket);
            }

            // Check the packet version... must fit, otherwise data misalignment might occur.
            if (sInputPacket.uiVersion != SDVFrameworkInterfaceVersion) throw XMarshallVersion{};

            // The checksum can be calculated by serializing the packet once more.
            uint16_t uiHdrChecksum = sInputPacket.uiHdrChecksum;
            serializer rserInput;
            serdes::CSerdes<SMarshallLocal>::Serialize(rserInput, sInputPacket);
            // Ignore cppcheck suppress warning is always false. The checksum is written into the packet again. If previously it
            // was different, this causes an integrity exception.
            // cppcheck-suppress knownConditionTrueFalse
            if (sInputPacket.uiHdrChecksum != uiHdrChecksum) throw XMarshallIntegrity{};

            // Check for packet integrity and add the raw data to the raw data bypass
            if (sInputPacket.seqChecksums.size() != seqInputData.size() - 1) throw XMarshallIntegrity{};
            size_t nChecksumIndex = 0;
            pointer<uint8_t> ptrInputParams;
            GetRawDataBypass().clear();
            for (auto itData = seqInputData.begin() + 1; itData != seqInputData.end(); itData++)
            {
                crcCCITT_FALSE crc;
                if (crc.calc_checksum(itData->get(), itData->size()) != sInputPacket.seqChecksums[nChecksumIndex])
                    throw XMarshallIntegrity{};
                if (nChecksumIndex)
                    GetRawDataBypass().push(*itData);
                else
                    ptrInputParams = *itData;
                nChecksumIndex++;
            }

            // If any exception occured during the input processing, return the buffer without processing. This allows the
            // caller to receive the exception.
            if (sInputPacket.uiFlags & static_cast<uint32_t>(EMarshallFlags::exception_triggered))
                return seqInputData;

            // Call dispatch function
            FNDispatch fnDispatch = m_vecDispatch[sInputPacket.uiFuncIndex];
            if (!fnDispatch) throw XMarshallNotInitialized{};
            pointer<uint8_t> ptrOutputParams;
            ECallResult eResult = ECallResult::result_ok;
            try
            {
                eResult = fnDispatch(eSourceEndianess, ptrInputParams, ptrOutputParams);
            } catch (const XNoInterface& rexcept)
            {
                ptrOutputParams = SerializeException(eSourceEndianess, rexcept);
                eResult = ECallResult::result_exception;
            } catch (const XIndexOutOfRange& rexcept)
            {
                ptrOutputParams = SerializeException(eSourceEndianess, rexcept);
                eResult = ECallResult::result_exception;
            } catch (const XInvalidIterator& rexcept)
            {
                ptrOutputParams = SerializeException(eSourceEndianess, rexcept);
                eResult = ECallResult::result_exception;
            } catch (const XNullPointer& rexcept)
            {
                ptrOutputParams = SerializeException(eSourceEndianess, rexcept);
                eResult = ECallResult::result_exception;
            } catch (const XInvalidRefCount& rexcept)
            {
                ptrOutputParams = SerializeException(eSourceEndianess, rexcept);
                eResult = ECallResult::result_exception;
            } catch (const XBufferTooSmall& rexcept)
            {
                ptrOutputParams = SerializeException(eSourceEndianess, rexcept);
                eResult = ECallResult::result_exception;
            } catch (const XHashNotMatching& rexcept)
            {
                ptrOutputParams = SerializeException(eSourceEndianess, rexcept);
                eResult = ECallResult::result_exception;
            } catch (const XOffsetPastBufferSize& rexcept)
            {
                ptrOutputParams = SerializeException(eSourceEndianess, rexcept);
                eResult = ECallResult::result_exception;
            } catch (const XUnknownException& rexcept)
            {
                ptrOutputParams = SerializeException(eSourceEndianess, rexcept);
                eResult = ECallResult::result_exception;
            } catch (const XUnhandledException& rexcept)
            {
                ptrOutputParams = SerializeException(eSourceEndianess, rexcept);
                eResult = ECallResult::result_exception;
            } catch (const core::XNoMemMgr& rexcept)
            {
                ptrOutputParams = SerializeException(eSourceEndianess, rexcept);
                eResult = ECallResult::result_exception;
            } catch (const core::XAllocFailed& rexcept)
            {
                ptrOutputParams = SerializeException(eSourceEndianess, rexcept);
                eResult = ECallResult::result_exception;
            } catch (...)
            {
                XUnhandledException xUnhandled;
                ptrOutputParams = SerializeException(eSourceEndianess, xUnhandled);
                eResult = ECallResult::result_exception;
            }

            // The sequence holding the output data pointers
            sequence<pointer<uint8_t>> seqOutputData;

            // Fill the marshall packet
            SMarshallLocal sOutputPacket{};
            sOutputPacket.eEndian = eSourceEndianess;
            sOutputPacket.uiVersion = SDVFrameworkInterfaceVersion;
            sOutputPacket.tIfcId = TInterface::_id;
            sOutputPacket.uiFuncIndex = sInputPacket.uiFuncIndex;
            sOutputPacket.uiFlags = static_cast<uint32_t>(EMarshallFlags::direction_output);
            if (eResult == ECallResult::result_exception)
                sOutputPacket.uiFlags |= static_cast<uint32_t>(EMarshallFlags::exception_triggered);

            // Are there any parameters?
            if (ptrOutputParams)
            {
                // Calculate the checksum
                crcCCITT_FALSE crc;
                uint16_t uiChecksum = crc.calc_checksum(ptrOutputParams.get(), ptrOutputParams.size());

                // Add the serialized parameters to the marshall sequence
                seqOutputData.push_back(ptrOutputParams);
                sOutputPacket.seqChecksums.push_back(uiChecksum);

                // Add additional raw data
                while (!GetRawDataBypass().empty())
                {
                    // Calculate the checksum
                    pointer<uint8_t> ptrRawData = GetRawDataBypass().pop();
                    crcCCITT_FALSE crcBypass;
                    uiChecksum = crcBypass.calc_checksum(ptrRawData.get(), ptrRawData.size());

                    // Add the raw data to the marshall sequence
                    seqOutputData.push_back(ptrRawData);
                    sOutputPacket.seqChecksums.push_back(uiChecksum);
                }
            }

            // Create an additional stream for the marshall struct.
            serializer serHdr;
            serdes::CSerdes<SMarshallLocal>::Serialize(serHdr, sOutputPacket);

            // Add the packet to the top of the pointer sequence.
            seqOutputData.insert(seqOutputData.begin(), serHdr.buffer());

            return seqOutputData;
        }
    } // namespace ps
} // namespace sdv

inline bool operator==(const sdv::ps::TMarshallID& rtID1, const sdv::ps::TMarshallID& rtID2)
{
    if (rtID1.uiHostID != rtID2.uiHostID) return false;
    if (rtID1.tProcessID != rtID2.tProcessID) return false;
    if (rtID1.uiIdent != rtID2.uiIdent) return false;
    if (rtID1.uiControl != rtID2.uiControl) return false;
    return true;
}

inline bool operator==(const sdv::ps::TMarshallID& rtID1, size_t nVal)
{
    if (nVal) return false;
    return operator==(rtID1, sdv::ps::TMarshallID{});
}

inline bool operator==(size_t nVal, const sdv::ps::TMarshallID& rtID2)
{
    if (nVal) return false;
    return operator==(sdv::ps::TMarshallID{}, rtID2);
}

inline bool operator!=(const sdv::ps::TMarshallID& rtID1, const sdv::ps::TMarshallID& rtID2)
{
    return !operator==(rtID1, rtID2);
}

inline bool operator!=(const sdv::ps::TMarshallID& rtID1, size_t nVal)
{
    if (nVal) return true;
    return !operator==(rtID1, sdv::ps::TMarshallID{});
}

inline bool operator!=(size_t nVal, const sdv::ps::TMarshallID& rtID2)
{
    if (nVal) return true;
    return !operator==(sdv::ps::TMarshallID{}, rtID2);
}

inline bool operator<(const sdv::ps::TMarshallID& rtID1, const sdv::ps::TMarshallID& rtID2)
{
    if (rtID1.uiHostID < rtID2.uiHostID) return true;
    if (rtID1.uiHostID > rtID2.uiHostID) return false;
    if (rtID1.tProcessID < rtID2.tProcessID) return true;
    if (rtID1.tProcessID > rtID2.tProcessID) return false;
    if (rtID1.uiIdent < rtID2.uiIdent) return true;
    return false;
}

inline bool operator<=(const sdv::ps::TMarshallID& rtID1, const sdv::ps::TMarshallID& rtID2)
{
    if (rtID1.uiHostID < rtID2.uiHostID) return true;
    if (rtID1.uiHostID > rtID2.uiHostID) return false;
    if (rtID1.tProcessID < rtID2.tProcessID) return true;
    if (rtID1.tProcessID > rtID2.tProcessID) return false;
    if (rtID1.uiIdent < rtID2.uiIdent) return true;
    if (rtID1.uiControl == rtID2.uiControl) return true;
    return false;
}

inline bool operator>(const sdv::ps::TMarshallID& rtID1, const sdv::ps::TMarshallID& rtID2)
{
    if (rtID1.uiHostID > rtID2.uiHostID) return true;
    if (rtID1.uiHostID < rtID2.uiHostID) return false;
    if (rtID1.tProcessID > rtID2.tProcessID) return true;
    if (rtID1.tProcessID < rtID2.tProcessID) return false;
    if (rtID1.uiIdent > rtID2.uiIdent) return true;
    return false;
}

inline bool operator>=(const sdv::ps::TMarshallID& rtID1, const sdv::ps::TMarshallID& rtID2)
{
    if (rtID1.uiHostID > rtID2.uiHostID) return true;
    if (rtID1.uiHostID < rtID2.uiHostID) return false;
    if (rtID1.tProcessID > rtID2.tProcessID) return true;
    if (rtID1.tProcessID < rtID2.tProcessID) return false;
    if (rtID1.uiIdent > rtID2.uiIdent) return true;
    if (rtID1.uiControl == rtID2.uiControl) return true;
    return false;
}

inline bool operator!(const sdv::ps::TMarshallID& rtID)
{
    // The control ID is the onl ID which is not allowed to be zero.
    return rtID.uiControl ? false : true;
}

inline bool operator==(const sdv::com::TConnectionID& rtID1, const sdv::com::TConnectionID& rtID2)
{
    if (rtID1.uiIdent != rtID2.uiIdent) return false;
    if (rtID1.uiControl != rtID2.uiControl) return false;
    return true;
}

inline bool operator==(const sdv::com::TConnectionID& rtID1, size_t nVal)
{
    if (nVal) return false;
    return operator==(rtID1, sdv::com::TConnectionID{});
}

inline bool operator==(size_t nVal, const sdv::com::TConnectionID& rtID2)
{
    if (nVal) return false;
    return operator==(sdv::com::TConnectionID{}, rtID2);
}

inline bool operator!=(const sdv::com::TConnectionID& rtID1, const sdv::com::TConnectionID& rtID2)
{
    return !operator==(rtID1, rtID2);
}

inline bool operator!=(const sdv::com::TConnectionID& rtID1, size_t nVal)
{
    if (nVal) return true;
    return !operator==(rtID1, sdv::com::TConnectionID{});
}

inline bool operator!=(size_t nVal, const sdv::com::TConnectionID& rtID2)
{
    if (nVal) return true;
    return !operator==(sdv::com::TConnectionID{}, rtID2);
}

inline bool operator<(const sdv::com::TConnectionID& rtID1, const sdv::com::TConnectionID& rtID2)
{
    if (rtID1.uiIdent < rtID2.uiIdent) return true;
    return false;
}

inline bool operator<=(const sdv::com::TConnectionID& rtID1, const sdv::com::TConnectionID& rtID2)
{
    if (rtID1.uiIdent < rtID2.uiIdent) return true;
    if (rtID1.uiControl == rtID2.uiControl) return true;
    return false;
}

inline bool operator>(const sdv::com::TConnectionID& rtID1, const sdv::com::TConnectionID& rtID2)
{
    if (rtID1.uiIdent > rtID2.uiIdent) return true;
    return false;
}

inline bool operator>=(const sdv::com::TConnectionID& rtID1, const sdv::com::TConnectionID& rtID2)
{
    if (rtID1.uiIdent > rtID2.uiIdent) return true;
    if (rtID1.uiControl == rtID2.uiControl) return true;
    return false;
}

inline bool operator!(const sdv::com::TConnectionID& rtID)
{
    // The control ID is the onl ID which is not allowed to be zero.
    return rtID.uiControl ? false : true;
}

#endif // !defined(SDV_PS_SUPPORT_INL)
