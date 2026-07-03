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

#include <support/app_control.h>
#include <support/pssup.h>
#include <support/sdv_core.h>

#include "../../../sdv_services/ipc_com/com_channel.h"
#include "../../../sdv_services/ipc_com/com_ctrl.h"
#include "../../../sdv_services/ipc_com/marshall_object.h"
#include "../../include/gtest_custom.h"
#include "generated/test_ifc.h"

inline EHello& operator++(EHello& reHello, int)
{
    switch (reHello)
    {
    case EHello::hi:
        reHello = EHello::huhu;
        break;
    case EHello::huhu:
        reHello = EHello::hello;
        break;
    case EHello::hello:
        reHello = EHello::hallo;
        break;
    case EHello::hallo:
        reHello = EHello::servus;
        break;
    case EHello::servus:
        reHello = EHello::gruessgott;
        break;
    case EHello::gruessgott:
        reHello = EHello::hi;
        break;
    default:
        break;
    }
    return reHello;
}

inline EHello& operator--(EHello& reHello, int)
{
    switch (reHello)
    {
    case EHello::hi:
        reHello = EHello::gruessgott;
        break;
    case EHello::huhu:
        reHello = EHello::hi;
        break;
    case EHello::hello:
        reHello = EHello::huhu;
        break;
    case EHello::hallo:
        reHello = EHello::hello;
        break;
    case EHello::servus:
        reHello = EHello::hallo;
        break;
    case EHello::gruessgott:
        reHello = EHello::servus;
        break;
    default:
        break;
    }
    return reHello;
}

/**
 * - Instantiate service
 * - Assign server endpoint
 * - Assign server endpoint + assign client endpoint
 * - Create channel endpoint
 * - Create channel endpoint + connect channel endpoint
 * - Create proxy/stub connection
 * - Reconnect
 * - Multiple connections
 * - Get other interface
 * - Multiple access interface
 * - Random multiple access interface
 * - Random multiple access interface with reconnect
 */

class CInterfaceTest :
    public ISayHello,
    public IRequestHello,
    public IRegisterHelloCallback,
    public IMegaTest,
    public sdv::IInterfaceAccess
{
public:
    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(ISayHello)
        SDV_INTERFACE_ENTRY(IRequestHello)
        SDV_INTERFACE_ENTRY(IRegisterHelloCallback)
        SDV_INTERFACE_ENTRY(IMegaTest)
        SDV_INTERFACE_ENTRY(sdv::IInterfaceAccess)
    END_SDV_INTERFACE_MAP()

    /**
     * @brief Say hello!
     * @return The greeting text.
     */
    virtual sdv::string Hello() override
    {
        return "Hello";
    }

    /**
     * @brief Request the hello interface.
     * @return pHello The hello interface.
     */
    virtual ISayHello* Request()
    {
        return this;
    }

    /**
     * @brief Register the callback with the hello interface.
     * @param[in] pHello The hello interface.
     */
    virtual void Register(/*in*/ ISayHello* pHello)
    {
        m_pHello = pHello;
    }

    /**
     * @brief Update the mega struct. This will test marshalling and unmarshalling of the mega struct. Overload of
     * IMegaTest::ProcessMegaStruct.
     * @param[in] rsStruct Reference to the mega struct.
     */
    virtual void ProcessMegaStruct(/*inout*/ SMegaStruct& rsStruct) override
    {
        rsStruct.bVal = !rsStruct.bVal;
        rsStruct.sVal--;
        rsStruct.usVal++;
        rsStruct.lVal--;
        rsStruct.ulVal++;
        rsStruct.llVal--;
        rsStruct.ullVal++;
        rsStruct.i8Val--;
        rsStruct.ui8Val++;
        rsStruct.i16Val--;
        rsStruct.ui16Val++;
        rsStruct.i32Val--;
        rsStruct.ui32Val++;
        rsStruct.i64Val--;
        rsStruct.ui64Val++;
        rsStruct.cVal++;
        rsStruct.c16Val++;
        rsStruct.c32Val++;
        rsStruct.wcVal++;
        rsStruct.fVal -= 1000.000f;
        rsStruct.dVal += 1000.000;
        rsStruct.ldVal -= 1000.000;
        rsStruct.nVal++;
        rsStruct.eHelloVal = rsStruct.eHelloVal++;
        if (rsStruct.idVal == sdv::GetInterfaceId<IMultiplyValue>())
            rsStruct.idVal = sdv::GetInterfaceId<IAddValue>();
        else
            rsStruct.idVal = 0;
        rsStruct.ssVal += rsStruct.ssVal.empty() ? 'x' : rsStruct.ssVal.back() + 1;
        rsStruct.ss8Val += rsStruct.ss8Val.empty() ? 'x' : rsStruct.ss8Val.back() + 1;
        rsStruct.ss16Val += rsStruct.ss16Val.empty() ? u'x' : rsStruct.ss16Val.back() + 1;
        rsStruct.ss32Val += rsStruct.ss32Val.empty() ? U'x' : rsStruct.ss32Val.back() + 1;
        rsStruct.wssVal += rsStruct.wssVal.empty() ? L'x' : rsStruct.wssVal.back() + 1;
        rsStruct.ssFixVal += rsStruct.ssFixVal.empty() ? 'x' : rsStruct.ssFixVal.back() + 1;
        rsStruct.ss8FixVal += rsStruct.ss8FixVal.empty() ? 'x' : rsStruct.ss8FixVal.back() + 1;
        rsStruct.ss16FixVal += rsStruct.ss16FixVal.empty() ? u'x' : rsStruct.ss16FixVal.back() + 1;
        rsStruct.ss32FixVal += rsStruct.ss32FixVal.empty() ? U'x' : rsStruct.ss32FixVal.back() + 1;
        rsStruct.wssFixVal += rsStruct.wssFixVal.empty() ? L'x' : rsStruct.wssFixVal.back() + 1;
        rsStruct.ptr8Val.resize(rsStruct.ptr8Val.size() * 2);
        for (size_t nIndex = 0; nIndex < rsStruct.ptr8Val.size() / 2; nIndex++)
            rsStruct.ptr8Val[nIndex + rsStruct.ptr8Val.size() / 2] = rsStruct.ptr8Val[nIndex];
        rsStruct.ptrssVal.resize(rsStruct.ptrssVal.size() * 2);
        for (size_t nIndex = 0; nIndex < rsStruct.ptrssVal.size() / 2; nIndex++)
            rsStruct.ptrssVal[nIndex + rsStruct.ptrssVal.size() / 2] = rsStruct.ptrssVal[nIndex];
        for (size_t nIndex = 0; nIndex < rsStruct.ptr8FixVal.size(); nIndex++)
            rsStruct.ptr8FixVal[nIndex] = static_cast<uint8_t>(rsStruct.ptr8FixVal[nIndex] << 1);
        for (size_t nIndex = 0; nIndex < rsStruct.ptrssFixVal.size(); nIndex++)
            rsStruct.ptrssFixVal[nIndex] = rsStruct.ptrssFixVal[nIndex] + rsStruct.ptrssFixVal[nIndex];
        rsStruct.seq8Val.resize(rsStruct.seq8Val.size() * 2);
        for (size_t nIndex = 0; nIndex < rsStruct.seq8Val.size() / 2; nIndex++)
            rsStruct.seq8Val[nIndex + rsStruct.seq8Val.size() / 2] = rsStruct.seq8Val[nIndex];
        rsStruct.seqssVal.resize(rsStruct.seqssVal.size() * 2);
        for (size_t nIndex = 0; nIndex < rsStruct.seqssVal.size() / 2; nIndex++)
            rsStruct.seqssVal[nIndex + rsStruct.seqssVal.size() / 2] = rsStruct.seqssVal[nIndex];
        for (size_t nIndex = 0; nIndex < rsStruct.seq8FixVal.size(); nIndex++)
            rsStruct.seq8FixVal[nIndex] = static_cast<uint8_t>(rsStruct.seq8FixVal[nIndex] << 1);
        for (size_t nIndex = 0; nIndex < rsStruct.seqssFixVal.size(); nIndex++)
            rsStruct.seqssFixVal[nIndex] = rsStruct.seqssFixVal[nIndex] + rsStruct.seqssFixVal[nIndex];
        if (rsStruct.ifcVal.get<IMultiplyValue>() && rsStruct.ifcVal.id() == sdv::GetInterfaceId<IMultiplyValue>())
        {
            m_summer.set(rsStruct.ifcVal.get<IMultiplyValue>());
            rsStruct.ifcVal = static_cast<IAddValue*>(&m_summer);
        }
        else
            rsStruct.ifcVal = nullptr;
        if (rsStruct.pMultiplyValue)
        {
            m_multiplier.set(rsStruct.pMultiplyValue);
            rsStruct.pMultiplyValue = static_cast<IMultiplyValue*>(&m_multiplier);
        }
        else
            rsStruct.pMultiplyValue = nullptr;
        rsStruct.sSubVal.i += 1;
        rsStruct.sIndVal.i += 10;
        // rsStruct.sUnnamedVal.i += 20;  // 12.04.2024 EVE Unnamed structures are not supported by IDL compiler.
        // Unnamed unions (inline definition) are not supported yet: PBI #398209
        // union switch(int32) { case 0: int32 i; default: int32 j; } uUnnamedVal;
        // Anonymous structs and unions (unnamed and no declaration) are not supported yet: PBI #397894
        // struct { int32 iAnonymous1; int32 iAnonymous2; };
        // union switch(int32) { case 0: int32 iAnonymous1; default: int32 iAnonymous2; };

        rsStruct.rgbVal[0] = !rsStruct.rgbVal[0];
        rsStruct.rgbVal[1] = !rsStruct.rgbVal[1];
        rsStruct.rgsVal[0]--;
        rsStruct.rgsVal[1]++;
        rsStruct.rgusVal[0]++;
        rsStruct.rgusVal[1] += 2;
        rsStruct.rglVal[0]--;
        rsStruct.rglVal[1]++;
        rsStruct.rgulVal[0]++;
        rsStruct.rgulVal[1] += 2;
        rsStruct.rgllVal[0]--;
        rsStruct.rgllVal[1]++;
        rsStruct.rgullVal[0]++;
        rsStruct.rgullVal[1] += 2;
        rsStruct.rgi8Val[0]--;
        rsStruct.rgi8Val[1]++;
        rsStruct.rgui8Val[0]++;
        rsStruct.rgui8Val[1] += 2;
        rsStruct.rgi16Val[0]--;
        rsStruct.rgi16Val[1]++;
        rsStruct.rgui16Val[0]++;
        rsStruct.rgui16Val[1] += 2;
        rsStruct.rgi32Val[0]--;
        rsStruct.rgi32Val[1]++;
        rsStruct.rgui32Val[0]++;
        rsStruct.rgui32Val[1] += 2;
        rsStruct.rgi64Val[0]--;
        rsStruct.rgi64Val[1]++;
        rsStruct.rgui64Val[0]++;
        rsStruct.rgui64Val[1] += 2;
        rsStruct.rgcVal[0]++;
        rsStruct.rgcVal[1] += 2;
        rsStruct.rgc16Val[0]++;
        rsStruct.rgc16Val[1] += 2;
        rsStruct.rgc32Val[0]++;
        rsStruct.rgc32Val[1] += 2;
        rsStruct.rgwcVal[0]++;
        rsStruct.rgwcVal[1] += 2;
        rsStruct.rgfVal[0] -= 1000.000f;
        rsStruct.rgfVal[1] += 2000.000f;
        rsStruct.rgdVal[0] += 1000.000;
        rsStruct.rgdVal[1] -= 2000.000;
        rsStruct.rgldVal[0] -= 1000.000;
        rsStruct.rgldVal[1] += 2000.000;
        rsStruct.rgnVal[0]++;
        rsStruct.rgnVal[1] += 2;
        rsStruct.rgeHelloVal[0] = rsStruct.rgeHelloVal[0]++;
        rsStruct.rgeHelloVal[1] = rsStruct.rgeHelloVal[1]--;
        if (rsStruct.rgidVal[0] == sdv::GetInterfaceId<IMultiplyValue>())
            rsStruct.rgidVal[0] = sdv::GetInterfaceId<IAddValue>();
        else
            rsStruct.rgidVal[0] = 0;
        if (rsStruct.rgidVal[1] == sdv::GetInterfaceId<IAddValue>())
            rsStruct.rgidVal[1] = sdv::GetInterfaceId<IMultiplyValue>();
        else
            rsStruct.rgidVal[1] = 0;
        rsStruct.rgssVal[0] += rsStruct.rgssVal[0].empty() ? 'x' : rsStruct.rgssVal[0].back() + 1;
        rsStruct.rgssVal[1] += rsStruct.rgssVal[1].empty() ? 'x' : rsStruct.rgssVal[1].front() + 1;
        rsStruct.rgss8Val[0] += rsStruct.rgss8Val[0].empty() ? 'x' : rsStruct.rgss8Val[0].back() + 1;
        rsStruct.rgss8Val[1] += rsStruct.rgss8Val[1].empty() ? 'x' : rsStruct.rgss8Val[1].front() + 1;
        rsStruct.rgss16Val[0] += rsStruct.rgss16Val[0].empty() ? u'x' : rsStruct.rgss16Val[0].back() + 1;
        rsStruct.rgss16Val[1] += rsStruct.rgss16Val[1].empty() ? u'x' : rsStruct.rgss16Val[1].front() + 1;
        rsStruct.rgss32Val[0] += rsStruct.rgss32Val[0].empty() ? U'x' : rsStruct.rgss32Val[0].back() + 1;
        rsStruct.rgss32Val[1] += rsStruct.rgss32Val[1].empty() ? U'x' : rsStruct.rgss32Val[1].front() + 1;
        rsStruct.rgwssVal[0] += rsStruct.rgwssVal[0].empty() ? L'x' : rsStruct.rgwssVal[0].back() + 1;
        rsStruct.rgwssVal[1] += rsStruct.rgwssVal[1].empty() ? L'x' : rsStruct.rgwssVal[1].front() + 1;
        rsStruct.rgssFixVal[0] += rsStruct.rgssFixVal[0].empty() ? 'x' : rsStruct.rgssFixVal[0].back() + 1;
        rsStruct.rgssFixVal[1] += rsStruct.rgssFixVal[1].empty() ? 'x' : rsStruct.rgssFixVal[1].front() + 1;
        rsStruct.rgss8FixVal[0] += rsStruct.rgss8FixVal[0].empty() ? 'x' : rsStruct.rgss8FixVal[0].back() + 1;
        rsStruct.rgss8FixVal[1] += rsStruct.rgss8FixVal[1].empty() ? 'x' : rsStruct.rgss8FixVal[1].front() + 1;
        rsStruct.rgss16FixVal[0] += rsStruct.rgss16FixVal[0].empty() ? u'x' : rsStruct.rgss16FixVal[0].back() + 1;
        rsStruct.rgss16FixVal[1] += rsStruct.rgss16FixVal[1].empty() ? u'x' : rsStruct.rgss16FixVal[1].front() + 1;
        rsStruct.rgss32FixVal[0] += rsStruct.rgss32FixVal[0].empty() ? U'x' : rsStruct.rgss32FixVal[0].back() + 1;
        rsStruct.rgss32FixVal[1] += rsStruct.rgss32FixVal[1].empty() ? U'x' : rsStruct.rgss32FixVal[1].front() + 1;
        rsStruct.rgwssFixVal[0] += rsStruct.rgwssFixVal[0].empty() ? L'x' : rsStruct.rgwssFixVal[0].back() + 1;
        rsStruct.rgwssFixVal[1] += rsStruct.rgwssFixVal[1].empty() ? L'x' : rsStruct.rgwssFixVal[1].front() + 1;
        rsStruct.rgptr8Val[0].resize(rsStruct.rgptr8Val[0].size() * 2);
        rsStruct.rgptr8Val[1].resize(rsStruct.rgptr8Val[1].size() * 3);
        for (size_t nIndex = 0; nIndex < rsStruct.rgptr8Val[0].size() / 2; nIndex++)
            rsStruct.rgptr8Val[0][nIndex + rsStruct.rgptr8Val[0].size() / 2] = rsStruct.rgptr8Val[0][nIndex];
        for (size_t nIndex = 0; nIndex < rsStruct.rgptr8Val[1].size() / 3; nIndex++)
            rsStruct.rgptr8Val[1][nIndex + rsStruct.rgptr8Val[1].size() / 3] = rsStruct.rgptr8Val[1][nIndex];
        rsStruct.rgptrssVal[0].resize(rsStruct.rgptrssVal[0].size() * 2);
        rsStruct.rgptrssVal[1].resize(rsStruct.rgptrssVal[1].size() * 3);
        for (size_t nIndex = 0; nIndex < rsStruct.rgptrssVal[0].size() / 2; nIndex++)
            rsStruct.rgptrssVal[0][nIndex + rsStruct.rgptrssVal[0].size() / 2] = rsStruct.rgptrssVal[0][nIndex];
        for (size_t nIndex = 0; nIndex < rsStruct.rgptrssVal[1].size() / 3; nIndex++)
            rsStruct.rgptrssVal[1][nIndex + rsStruct.rgptrssVal[1].size() / 3] = rsStruct.rgptrssVal[1][nIndex];
        for (size_t nIndex = 0; nIndex < rsStruct.rgptr8FixVal[0].size(); nIndex++)
            rsStruct.rgptr8FixVal[0][nIndex] = static_cast<uint8_t>(rsStruct.rgptr8FixVal[0][nIndex] << 1);
        for (size_t nIndex = 0; nIndex < rsStruct.rgptr8FixVal[1].size(); nIndex++)
            rsStruct.rgptr8FixVal[1][nIndex] = static_cast<uint8_t>(rsStruct.rgptr8FixVal[1][nIndex] << 1);
        for (size_t nIndex = 0; nIndex < rsStruct.rgptrssFixVal[0].size(); nIndex++)
            rsStruct.rgptrssFixVal[0][nIndex] = rsStruct.rgptrssFixVal[0][nIndex] + rsStruct.rgptrssFixVal[0][nIndex];
        for (size_t nIndex = 0; nIndex < rsStruct.rgptrssFixVal[1].size(); nIndex++)
            rsStruct.rgptrssFixVal[1][nIndex] = rsStruct.rgptrssFixVal[1][nIndex] + rsStruct.rgptrssFixVal[1][nIndex];
        rsStruct.rgseq8Val[0].resize(rsStruct.rgseq8Val[0].size() * 2);
        rsStruct.rgseq8Val[1].resize(rsStruct.rgseq8Val[1].size() * 3);
        for (size_t nIndex = 0; nIndex < rsStruct.rgseq8Val[0].size() / 2; nIndex++)
            rsStruct.rgseq8Val[0][nIndex + rsStruct.rgseq8Val[0].size() / 2] = rsStruct.rgseq8Val[0][nIndex];
        for (size_t nIndex = 0; nIndex < rsStruct.rgseq8Val[1].size() / 3; nIndex++)
            rsStruct.rgseq8Val[1][nIndex + rsStruct.rgseq8Val[1].size() / 3] = rsStruct.rgseq8Val[1][nIndex];
        rsStruct.rgseqssVal[0].resize(rsStruct.rgseqssVal[0].size() * 2);
        rsStruct.rgseqssVal[1].resize(rsStruct.rgseqssVal[1].size() * 3);
        for (size_t nIndex = 0; nIndex < rsStruct.rgseqssVal[0].size() / 2; nIndex++)
            rsStruct.rgseqssVal[0][nIndex + rsStruct.rgseqssVal[0].size() / 2] = rsStruct.rgseqssVal[0][nIndex];
        for (size_t nIndex = 0; nIndex < rsStruct.rgseqssVal[1].size() / 3; nIndex++)
            rsStruct.rgseqssVal[1][nIndex + rsStruct.rgseqssVal[1].size() / 3] = rsStruct.rgseqssVal[1][nIndex];
        for (size_t nIndex = 0; nIndex < rsStruct.rgseq8FixVal[0].size(); nIndex++)
            rsStruct.rgseq8FixVal[0][nIndex] = static_cast<uint8_t>(rsStruct.rgseq8FixVal[0][nIndex] << 1);
        for (size_t nIndex = 0; nIndex < rsStruct.rgseq8FixVal[1].size(); nIndex++)
            rsStruct.rgseq8FixVal[1][nIndex] = static_cast<uint8_t>(rsStruct.rgseq8FixVal[1][nIndex] << 1);
        for (size_t nIndex = 0; nIndex < rsStruct.rgseqssFixVal[0].size(); nIndex++)
            rsStruct.rgseqssFixVal[0][nIndex] = rsStruct.rgseqssFixVal[0][nIndex] + rsStruct.rgseqssFixVal[0][nIndex];
        for (size_t nIndex = 0; nIndex < rsStruct.rgseqssFixVal[1].size(); nIndex++)
            rsStruct.rgseqssFixVal[1][nIndex] = rsStruct.rgseqssFixVal[1][nIndex] + rsStruct.rgseqssFixVal[1][nIndex];
        // Incompatible serdes generation: BUG #398509
        // if (rsStruct.rgifcVal[0].get<IMultiplyValue>() && rsStruct.rgifcVal[0].id() == sdv::GetInterfaceId<IMultiplyValue>())
        //{
        //    m_summer.set(rsStruct.rgifcVal[0].get<IMultiplyValue>());
        //    rsStruct.rgifcVal[0] = static_cast<IAddValue*>(&m_summer);
        //}
        // else
        //    rsStruct.rgifcVal[0] = nullptr;
        // if (rsStruct.rgifcVal[1].get<IMultiplyValue>() && rsStruct.rgifcVal[1].id() == sdv::GetInterfaceId<IMultiplyValue>())
        //{
        //    m_summer.set(rsStruct.rgifcVal[1].get<IMultiplyValue>());
        //    rsStruct.rgifcVal[1] = static_cast<IAddValue*>(&m_summer);
        //}
        // else
        //    rsStruct.rgifcVal[1] = nullptr;
        // if (rsStruct.rgpMultiplyValue[0])
        //{
        //    m_multiplier.set(rsStruct.rgpMultiplyValue[0]);
        //    rsStruct.rgpMultiplyValue[0] = static_cast<IMultiplyValue*>(&m_multiplier);
        //}
        // else
        //    rsStruct.rgpMultiplyValue[0] = nullptr;
        // if (rsStruct.rgpMultiplyValue[1])
        //{
        //    m_multiplier.set(rsStruct.rgpMultiplyValue[1]);
        //    rsStruct.rgpMultiplyValue[1] = static_cast<IMultiplyValue*>(&m_multiplier);
        //}
        // else
        //    rsStruct.rgpMultiplyValue[1] = nullptr;
        rsStruct.rgsSubVal[0].i += 1;
        rsStruct.rgsSubVal[1].i += 2;
        // Incompatible serdes generation: BUG #398509
        // rsStruct.rgsIndVal[0].i += 10;
        // rsStruct.rgsIndVal[1].i += 20;
        // Unnamed struct with array is not yet supported due to incorret serdes code generation: BUG #398246
        // rsStruct.rgsUnnamedVal[0].i += 20;
        // rsStruct.rgsUnnamedVal[1].i += 40;

        rsStruct.tbVal = !rsStruct.tbVal;
        rsStruct.tsVal--;
        rsStruct.tusVal++;
        rsStruct.tlVal--;
        rsStruct.tulVal++;
        rsStruct.tllVal--;
        rsStruct.tullVal++;
        rsStruct.ti8Val--;
        rsStruct.tui8Val++;
        rsStruct.ti16Val--;
        rsStruct.tui16Val++;
        rsStruct.ti32Val--;
        rsStruct.tui32Val++;
        rsStruct.ti64Val--;
        rsStruct.tui64Val++;
        rsStruct.tcVal++;
        rsStruct.tc16Val++;
        rsStruct.tc32Val++;
        rsStruct.twcVal++;
        rsStruct.tfVal -= 1000.000f;
        rsStruct.tdVal += 1000.000;
        rsStruct.tldVal -= 1000.000;
        rsStruct.tnVal++;
        rsStruct.teHelloVal = rsStruct.teHelloVal++;
        if (rsStruct.tidVal == sdv::GetInterfaceId<IMultiplyValue>())
            rsStruct.tidVal = sdv::GetInterfaceId<IAddValue>();
        else
            rsStruct.tidVal = 0;
        rsStruct.tssVal += rsStruct.tssVal.empty() ? 'x' : rsStruct.tssVal.back() + 1;
        rsStruct.tss8Val += rsStruct.tss8Val.empty() ? 'x' : rsStruct.tss8Val.back() + 1;
        rsStruct.tss16Val += rsStruct.tss16Val.empty() ? u'x' : rsStruct.tss16Val.back() + 1;
        rsStruct.tss32Val += rsStruct.tss32Val.empty() ? U'x' : rsStruct.tss32Val.back() + 1;
        rsStruct.twssVal += rsStruct.twssVal.empty() ? L'x' : rsStruct.twssVal.back() + 1;
        rsStruct.tssFixVal += rsStruct.tssFixVal.empty() ? 'x' : rsStruct.tssFixVal.back() + 1;
        rsStruct.tss8FixVal += rsStruct.tss8FixVal.empty() ? 'x' : rsStruct.tss8FixVal.back() + 1;
        rsStruct.tss16FixVal += rsStruct.tss16FixVal.empty() ? u'x' : rsStruct.tss16FixVal.back() + 1;
        rsStruct.tss32FixVal += rsStruct.tss32FixVal.empty() ? U'x' : rsStruct.tss32FixVal.back() + 1;
        rsStruct.twssFixVal += rsStruct.twssFixVal.empty() ? L'x' : rsStruct.twssFixVal.back() + 1;
        // Using typedefs of template based types produced an error: BUG #398266
        // rsStruct.tptr8Val.resize(rsStruct.tptr8Val.size() * 2);
        // for (size_t nIndex = 0; nIndex < rsStruct.tptr8Val.size() / 2; nIndex++)
        //    rsStruct.tptr8Val[nIndex + rsStruct.tptr8Val.size() / 2] = rsStruct.tptr8Val[nIndex];
        // rsStruct.ptrssVal.resize(rsStruct.tptrssVal.size() * 2);
        // for (size_t nIndex = 0; nIndex < rsStruct.tptrssVal.size() / 2; nIndex++)
        //    rsStruct.tptrssVal[nIndex + rsStruct.tptrssVal.size() / 2] = rsStruct.tptrssVal[nIndex];
        // for (size_t nIndex = 0; nIndex < rsStruct.tptr8FixVal.size(); nIndex++)
        //    rsStruct.tptr8FixVal[nIndex] = static_cast<uint8_t>(rsStruct.tptr8FixVal[nIndex] << 1);
        // for (size_t nIndex = 0; nIndex < rsStruct.tptrssFixVal.size(); nIndex++)
        //    rsStruct.ptrssFixVal[nIndex] = rsStruct.tptrssFixVal[nIndex] + rsStruct.tptrssFixVal[nIndex];
        // rsStruct.tseq8Val.resize(rsStruct.tseq8Val.size() * 2);
        // for (size_t nIndex = 0; nIndex < rsStruct.tseq8Val.size() / 2; nIndex++)
        //    rsStruct.tseq8Val[nIndex + rsStruct.tseq8Val.size() / 2] = rsStruct.tseq8Val[nIndex];
        // rsStruct.tseqssVal.resize(rsStruct.tseqssVal.size() * 2);
        // for (size_t nIndex = 0; nIndex < rsStruct.tseqssVal.size() / 2; nIndex++)
        //    rsStruct.seqssVal[nIndex + rsStruct.tseqssVal.size() / 2] = rsStruct.tseqssVal[nIndex];
        // for (size_t nIndex = 0; nIndex < rsStruct.tseq8FixVal.size(); nIndex++)
        //    rsStruct.tseq8FixVal[nIndex] = static_cast<uint8_t>(rsStruct.tseq8FixVal[nIndex] << 1);
        // for (size_t nIndex = 0; nIndex < rsStruct.tseqssFixVal.size(); nIndex++)
        //    rsStruct.tseqssFixVal[nIndex] = rsStruct.tseqssFixVal[nIndex] + rsStruct.tseqssFixVal[nIndex];
        if (rsStruct.tifcVal.get<IMultiplyValue>() && rsStruct.tifcVal.id() == sdv::GetInterfaceId<IMultiplyValue>())
        {
            m_summerTypedef.set(rsStruct.tifcVal.get<IMultiplyValue>());
            rsStruct.tifcVal = static_cast<IAddValue*>(&m_summerTypedef);
        }
        else
            rsStruct.tifcVal = nullptr;
        // Typedef of interfaces current not possible. BUG #399464
        // if (rsStruct.tpMultiplyValue)
        //{
        //    m_multiplierTypedef.set(rsStruct.tpMultiplyValue);
        //    rsStruct.tpMultiplyValue = static_cast<IMultiplyValue*>(&m_multiplierTypedef);
        //}
        // else
        //    rsStruct.tpMultiplyValue = nullptr;
        rsStruct.tsSubVal.i += 1;
        rsStruct.tsIndVal.i += 10;
        // Incompatible serdes generation: BUG #398509
        // rsStruct.tsUnnamedVal.i += 20;
        // Unnamed unions (inline definition) are not supported yet: PBI #398209
        // union switch(int32) { case 0: int32 i; default: int32 j; } tuUnnamedVal;
        // Anonymous structs and unions (unnamed and no declaration) are not supported yet: PBI #397894
        // struct { int32 iAnonymous1; int32 iAnonymous2; };
        // union switch(int32) { case 0: int32 iAnonymous1; default: int32 iAnonymous2; };
    }

    /**
     * @brief Update the mega union. This will test marshalling and unmarshalling of the mega union. Overload of
     * IMegaTest::ProcessMegaTypeBasedUnion.
     */
    virtual void ProcessMegaTypeBasedUnion(/*inout*/ UMegaTypeBasedUnion& ruUnion) override
    {
        switch (ruUnion.get_switch())
        {
        case 1:
            ruUnion.bVal = !ruUnion.bVal;
            break;
        case 2:
            ruUnion.sVal--;
            break;
        case 3:
            ruUnion.usVal++;
            break;
        case 4:
            ruUnion.lVal--;
            break;
        case 5:
            ruUnion.ulVal++;
            break;
        case 6:
            ruUnion.llVal--;
            break;
        case 7:
            ruUnion.ullVal++;
            break;
        case 8:
            ruUnion.i8Val--;
            break;
        case 9:
            ruUnion.ui8Val++;
            break;
        case 10:
            ruUnion.i16Val--;
            break;
        case 11:
            ruUnion.ui16Val++;
            break;
        case 12:
            ruUnion.i32Val--;
            break;
        case 13:
            ruUnion.ui32Val++;
            break;
        case 14:
            ruUnion.i64Val--;
            break;
        case 15:
            ruUnion.ui64Val++;
            break;
        case 16:
            ruUnion.cVal++;
            break;
        case 17:
            ruUnion.c16Val++;
            break;
        case 18:
            ruUnion.c32Val++;
            break;
        case 19:
            ruUnion.wcVal++;
            break;
        case 20:
            ruUnion.fVal -= 1000.000f;
            break;
        case 21:
            ruUnion.dVal += 1000.000;
            break;
            // GCC issue with generated initialization in the consructor for the "long double" type: BUG #3982727
            // case 22: ruUnion.ldVal -= 1000.000;     break;
        case 23:
            ruUnion.nVal++;
            break;
        case 24:
            ruUnion.eHelloVal = ruUnion.eHelloVal++;
            break;
        case 25:
            if (ruUnion.idVal == sdv::GetInterfaceId<IMultiplyValue>())
                ruUnion.idVal = sdv::GetInterfaceId<IAddValue>();
            else
                ruUnion.idVal = 0;
            break;
        case 26:
            ruUnion.ssVal += ruUnion.ssVal.empty() ? 'x' : ruUnion.ssVal.back() + 1;
            break;
        case 27:
            ruUnion.ss8Val += ruUnion.ss8Val.empty() ? 'x' : ruUnion.ss8Val.back() + 1;
            break;
        case 28:
            ruUnion.ss16Val += ruUnion.ss16Val.empty() ? u'x' : ruUnion.ss16Val.back() + 1;
            break;
        case 29:
            ruUnion.ss32Val += ruUnion.ss32Val.empty() ? U'x' : ruUnion.ss32Val.back() + 1;
            break;
        case 30:
            ruUnion.wssVal += ruUnion.wssVal.empty() ? L'x' : ruUnion.wssVal.back() + 1;
            break;
        case 31:
            ruUnion.ssFixVal += ruUnion.ssFixVal.empty() ? 'x' : ruUnion.ssFixVal.back() + 1;
            break;
        case 32:
            ruUnion.ss8FixVal += ruUnion.ss8FixVal.empty() ? 'x' : ruUnion.ss8FixVal.back() + 1;
            break;
        case 33:
            ruUnion.ss16FixVal += ruUnion.ss16FixVal.empty() ? u'x' : ruUnion.ss16FixVal.back() + 1;
            break;
        case 34:
            ruUnion.ss32FixVal += ruUnion.ss32FixVal.empty() ? U'x' : ruUnion.ss32FixVal.back() + 1;
            break;
        case 35:
            ruUnion.wssFixVal += ruUnion.wssFixVal.empty() ? L'x' : ruUnion.wssFixVal.back() + 1;
            break;
        case 36:
            ruUnion.ptr8Val.resize(ruUnion.ptr8Val.size() * 2);
            for (size_t nIndex = 0; nIndex < ruUnion.ptr8Val.size() / 2; nIndex++)
                ruUnion.ptr8Val[nIndex + ruUnion.ptr8Val.size() / 2] = ruUnion.ptr8Val[nIndex];
            break;
        case 37:
            ruUnion.ptrssVal.resize(ruUnion.ptrssVal.size() * 2);
            for (size_t nIndex = 0; nIndex < ruUnion.ptrssVal.size() / 2; nIndex++)
                ruUnion.ptrssVal[nIndex + ruUnion.ptrssVal.size() / 2] = ruUnion.ptrssVal[nIndex];
            break;
        case 38:
            for (size_t nIndex = 0; nIndex < ruUnion.ptr8FixVal.size(); nIndex++)
                ruUnion.ptr8FixVal[nIndex] = static_cast<uint8_t>(ruUnion.ptr8FixVal[nIndex] << 1);
            break;
        case 39:
            for (size_t nIndex = 0; nIndex < ruUnion.ptrssFixVal.size(); nIndex++)
                ruUnion.ptrssFixVal[nIndex] = ruUnion.ptrssFixVal[nIndex] + ruUnion.ptrssFixVal[nIndex];
            break;
        case 40:
            ruUnion.seq8Val.resize(ruUnion.seq8Val.size() * 2);
            for (size_t nIndex = 0; nIndex < ruUnion.seq8Val.size() / 2; nIndex++)
                ruUnion.seq8Val[nIndex + ruUnion.seq8Val.size() / 2] = ruUnion.seq8Val[nIndex];
            break;
        case 41:
            ruUnion.seqssVal.resize(ruUnion.seqssVal.size() * 2);
            for (size_t nIndex = 0; nIndex < ruUnion.seqssVal.size() / 2; nIndex++)
                ruUnion.seqssVal[nIndex + ruUnion.seqssVal.size() / 2] = ruUnion.seqssVal[nIndex];
            break;
        case 42:
            for (size_t nIndex = 0; nIndex < ruUnion.seq8FixVal.size(); nIndex++)
                ruUnion.seq8FixVal[nIndex] = static_cast<uint8_t>(ruUnion.seq8FixVal[nIndex] << 1);
            break;
        case 43:
            for (size_t nIndex = 0; nIndex < ruUnion.seqssFixVal.size(); nIndex++)
                ruUnion.seqssFixVal[nIndex] = ruUnion.seqssFixVal[nIndex] + ruUnion.seqssFixVal[nIndex];
            break;
        case 44:
            if (ruUnion.ifcVal.get<IMultiplyValue>() && ruUnion.ifcVal.id() == sdv::GetInterfaceId<IMultiplyValue>())
            {
                m_summer.set(ruUnion.ifcVal.get<IMultiplyValue>());
                ruUnion.ifcVal = static_cast<IAddValue*>(&m_summer);
            }
            else
                ruUnion.ifcVal = nullptr;
            break;
        case 45:
            if (ruUnion.pMultiplyValue)
            {
                m_multiplier.set(ruUnion.pMultiplyValue);
                ruUnion.pMultiplyValue = static_cast<IMultiplyValue*>(&m_multiplier);
            }
            else
                ruUnion.pMultiplyValue = nullptr;
            break;
        case 46:
            ruUnion.sSubVal.i += 1;
            break;
            // Incompatible serdes generation: BUG #398509
            // case 47: ruUnion.sIndVal.i += 10;                                                                   break;
            // Unnamed struct inside unions are not supported yet: PBI #398230
            // case 48: ruUnion.sUnnamedVal.i += 20;                                                               break;
            // Unnamed unions are not supported yet: PBI #398209
            // case 49: union switch(int32) { case 0: int32 i; default: int32 j; } uUnnamedVal;
            // Anonymous structs and unions (unnamed and no declaration) are not supported yet: PBI #397894
            // case 50: struct { int32 iAnonymous1; int32 iAnonymous2; };
            // case 51: union switch(int32) { case 0: int32 iAnonymous1; default: int32 iAnonymous2; };

        case 201:
            ruUnion.tbVal = !ruUnion.tbVal;
            break;
        case 202:
            ruUnion.tsVal--;
            break;
        case 203:
            ruUnion.tusVal++;
            break;
        case 204:
            ruUnion.tlVal--;
            break;
        case 205:
            ruUnion.tulVal++;
            break;
        case 206:
            ruUnion.tllVal--;
            break;
        case 207:
            ruUnion.tullVal++;
            break;
        case 208:
            ruUnion.ti8Val--;
            break;
        case 209:
            ruUnion.tui8Val++;
            break;
        case 210:
            ruUnion.ti16Val--;
            break;
        case 211:
            ruUnion.tui16Val++;
            break;
        case 212:
            ruUnion.ti32Val--;
            break;
        case 213:
            ruUnion.tui32Val++;
            break;
        case 214:
            ruUnion.ti64Val--;
            break;
        case 215:
            ruUnion.tui64Val++;
            break;
        case 216:
            ruUnion.tcVal++;
            break;
        case 217:
            ruUnion.tc16Val++;
            break;
        case 218:
            ruUnion.tc32Val++;
            break;
        case 219:
            ruUnion.twcVal++;
            break;
        case 220:
            ruUnion.tfVal -= 1000.000f;
            break;
        case 221:
            ruUnion.tdVal += 1000.000;
            break;
            // GCC issue with generated initialization in the consructor for the "long double" type: BUG #3982727
            // case 22: ruUnion.tldVal -= 1000.000;     break;
        case 223:
            ruUnion.tnVal++;
            break;
        case 224:
            ruUnion.teHelloVal = ruUnion.teHelloVal++;
            break;
        case 225:
            if (ruUnion.tidVal == sdv::GetInterfaceId<IMultiplyValue>())
                ruUnion.tidVal = sdv::GetInterfaceId<IAddValue>();
            else
                ruUnion.tidVal = 0;
            break;
        case 226:
            ruUnion.tssVal += ruUnion.tssVal.empty() ? 'x' : ruUnion.tssVal.back() + 1;
            break;
        case 227:
            ruUnion.tss8Val += ruUnion.tss8Val.empty() ? 'x' : ruUnion.tss8Val.back() + 1;
            break;
        case 228:
            ruUnion.tss16Val += ruUnion.tss16Val.empty() ? u'x' : ruUnion.tss16Val.back() + 1;
            break;
        case 229:
            ruUnion.tss32Val += ruUnion.tss32Val.empty() ? U'x' : ruUnion.tss32Val.back() + 1;
            break;
        case 230:
            ruUnion.twssVal += ruUnion.twssVal.empty() ? L'x' : ruUnion.twssVal.back() + 1;
            break;
        case 231:
            ruUnion.tssFixVal += ruUnion.tssFixVal.empty() ? 'x' : ruUnion.tssFixVal.back() + 1;
            break;
        case 232:
            ruUnion.tss8FixVal += ruUnion.tss8FixVal.empty() ? 'x' : ruUnion.tss8FixVal.back() + 1;
            break;
        case 233:
            ruUnion.tss16FixVal += ruUnion.tss16FixVal.empty() ? u'x' : ruUnion.tss16FixVal.back() + 1;
            break;
        case 234:
            ruUnion.tss32FixVal += ruUnion.tss32FixVal.empty() ? U'x' : ruUnion.tss32FixVal.back() + 1;
            break;
        case 235:
            ruUnion.twssFixVal += ruUnion.twssFixVal.empty() ? L'x' : ruUnion.twssFixVal.back() + 1;
            break;
            // Using typedefs of template based types produced an error: BUG #398266
            // case 236:
            //    ruUnion.tptr8Val.resize(ruUnion.tptr8Val.size() * 2);
            //    for (size_t nIndex = 0; nIndex < ruUnion.tptr8Val.size() / 2; nIndex++)
            //        ruUnion.tptr8Val[nIndex + ruUnion.tptr8Val.size() / 2] = ruUnion.tptr8Val[nIndex];
            //    break;
            // case 237:
            //    ruUnion.tptrssVal.resize(ruUnion.tptrssVal.size() * 2);
            //    for (size_t nIndex = 0; nIndex < ruUnion.tptrssVal.size() / 2; nIndex++)
            //        ruUnion.tptrssVal[nIndex + ruUnion.tptrssVal.size() / 2] = ruUnion.tptrssVal[nIndex];
            //    break;
            // case 238:
            //    for (size_t nIndex = 0; nIndex < ruUnion.tptr8FixVal.size(); nIndex++)
            //        ruUnion.tptr8FixVal[nIndex] = static_cast<uint8_t>(ruUnion.tptr8FixVal[nIndex] << 1);
            //    break;
            // case 239:
            //    for (size_t nIndex = 0; nIndex < ruUnion.tptrssFixVal.size(); nIndex++)
            //        ruUnion.tptrssFixVal[nIndex] = ruUnion.tptrssFixVal[nIndex] + ruUnion.tptrssFixVal[nIndex];
            //    break;
            // case 240:
            //    ruUnion.tseq8Val.resize(ruUnion.tseq8Val.size() * 2);
            //    for (size_t nIndex = 0; nIndex < ruUnion.tseq8Val.size() / 2; nIndex++)
            //        ruUnion.tseq8Val[nIndex + ruUnion.tseq8Val.size() / 2] = ruUnion.tseq8Val[nIndex];
            //    break;
            // case 241:
            //    ruUnion.tseqssVal.resize(ruUnion.tseqssVal.size() * 2);
            //    for (size_t nIndex = 0; nIndex < ruUnion.tseqssVal.size() / 2; nIndex++)
            //        ruUnion.tseqssVal[nIndex + ruUnion.tseqssVal.size() / 2] = ruUnion.tseqssVal[nIndex];
            //    break;
            // case 242:
            //    for (size_t nIndex = 0; nIndex < ruUnion.tseq8FixVal.size(); nIndex++)
            //        ruUnion.tseq8FixVal[nIndex] = static_cast<uint8_t>(ruUnion.tseq8FixVal[nIndex] << 1);
            //    break;
            // case 243:
            //    for (size_t nIndex = 0; nIndex < ruUnion.tseqssFixVal.size(); nIndex++)
            //        ruUnion.tseqssFixVal[nIndex] = ruUnion.tseqssFixVal[nIndex] + ruUnion.tseqssFixVal[nIndex];
            //    break;
        case 244:
            if (ruUnion.tifcVal.get<IMultiplyValue>() && ruUnion.tifcVal.id() == sdv::GetInterfaceId<IMultiplyValue>())
            {
                m_summerTypedef.set(ruUnion.tifcVal.get<IMultiplyValue>());
                ruUnion.tifcVal = static_cast<IAddValue*>(&m_summerTypedef);
            }
            else
                ruUnion.tifcVal = nullptr;
            break;
            // Typedef of interfaces current not possible. BUG #399464
            // case 245:
            //    if (ruUnion.tpMultiplyValue)
            //    {
            //        m_multiplierTypedef.set(ruUnion.tpMultiplyValue);
            //        ruUnion.tpMultiplyValue = static_cast<IMultiplyValue*>(&m_multiplierTypedef);
            //    }
            //    else
            //        ruUnion.tpMultiplyValue = nullptr;
            //    break;
        case 246:
            ruUnion.tsSubVal.i += 1;
            break;
            // Incompatible serdes generation: BUG #398509
        case 247:
            ruUnion.tsIndVal.i += 10;
            break;
            // Unnamed struct inside unions are not supported yet: PBI #398230
            // case 248: ruUnion.tsUnnamedVal.i += 20;                                                               break;
            // Unnamed unions are not supported yet: PBI #398209
            // case 249: union switch(int32) { case 0: int32 i; default: int32 j; } uUnnamedVal;
            // Anonymous structs and unions (unnamed and no declaration) are not supported yet: PBI #397894
            // case 250: struct { int32 iAnonymous1; int32 iAnonymous2; };
            // case 251: union switch(int32) { case 0: int32 iAnonymous1; default: int32 iAnonymous2; };
        default:
            break;
        }
    }

    /**
     * @brief Update the mega union. This will test marshalling and unmarshalling of the mega union. Overload of
     * IMegaTest::ProcessMegaTypeBasedUnion.
     */
    virtual void ProcessMegaVarBasedUnion(/*inout*/ SMegaVarBasedUnion& rsUnion) override
    {
        switch (rsUnion.get_switch())
        {
        case 1:
            rsUnion.uVal.bVal = !rsUnion.uVal.bVal;
            break;
        case 2:
            rsUnion.uVal.sVal--;
            break;
        case 3:
            rsUnion.uVal.usVal++;
            break;
        case 4:
            rsUnion.uVal.lVal--;
            break;
        case 5:
            rsUnion.uVal.ulVal++;
            break;
        case 6:
            rsUnion.uVal.llVal--;
            break;
        case 7:
            rsUnion.uVal.ullVal++;
            break;
        case 8:
            rsUnion.uVal.i8Val--;
            break;
        case 9:
            rsUnion.uVal.ui8Val++;
            break;
        case 10:
            rsUnion.uVal.i16Val--;
            break;
        case 11:
            rsUnion.uVal.ui16Val++;
            break;
        case 12:
            rsUnion.uVal.i32Val--;
            break;
        case 13:
            rsUnion.uVal.ui32Val++;
            break;
        case 14:
            rsUnion.uVal.i64Val--;
            break;
        case 15:
            rsUnion.uVal.ui64Val++;
            break;
        case 16:
            rsUnion.uVal.cVal++;
            break;
        case 17:
            rsUnion.uVal.c16Val++;
            break;
        case 18:
            rsUnion.uVal.c32Val++;
            break;
        case 19:
            rsUnion.uVal.wcVal++;
            break;
        case 20:
            rsUnion.uVal.fVal -= 1000.000f;
            break;
        case 21:
            rsUnion.uVal.dVal += 1000.000;
            break;
            // GCC issue with generated initialization in the consructor for the "long double" type: BUG #3982727
            // case 22: rsUnion.uVal.ldVal -= 1000.000;     break;
        case 23:
            rsUnion.uVal.nVal++;
            break;
        case 24:
            rsUnion.uVal.eHelloVal = rsUnion.uVal.eHelloVal++;
            break;
        case 25:
            if (rsUnion.uVal.idVal == sdv::GetInterfaceId<IMultiplyValue>())
                rsUnion.uVal.idVal = sdv::GetInterfaceId<IAddValue>();
            else
                rsUnion.uVal.idVal = 0;
            break;
        case 26:
            rsUnion.uVal.ssVal += rsUnion.uVal.ssVal.empty() ? 'x' : rsUnion.uVal.ssVal.back() + 1;
            break;
        case 27:
            rsUnion.uVal.ss8Val += rsUnion.uVal.ss8Val.empty() ? 'x' : rsUnion.uVal.ss8Val.back() + 1;
            break;
        case 28:
            rsUnion.uVal.ss16Val += rsUnion.uVal.ss16Val.empty() ? u'x' : rsUnion.uVal.ss16Val.back() + 1;
            break;
        case 29:
            rsUnion.uVal.ss32Val += rsUnion.uVal.ss32Val.empty() ? U'x' : rsUnion.uVal.ss32Val.back() + 1;
            break;
        case 30:
            rsUnion.uVal.wssVal += rsUnion.uVal.wssVal.empty() ? L'x' : rsUnion.uVal.wssVal.back() + 1;
            break;
        case 31:
            rsUnion.uVal.ssFixVal += rsUnion.uVal.ssFixVal.empty() ? 'x' : rsUnion.uVal.ssFixVal.back() + 1;
            break;
        case 32:
            rsUnion.uVal.ss8FixVal += rsUnion.uVal.ss8FixVal.empty() ? 'x' : rsUnion.uVal.ss8FixVal.back() + 1;
            break;
        case 33:
            rsUnion.uVal.ss16FixVal += rsUnion.uVal.ss16FixVal.empty() ? u'x' : rsUnion.uVal.ss16FixVal.back() + 1;
            break;
        case 34:
            rsUnion.uVal.ss32FixVal += rsUnion.uVal.ss32FixVal.empty() ? U'x' : rsUnion.uVal.ss32FixVal.back() + 1;
            break;
        case 35:
            rsUnion.uVal.wssFixVal += rsUnion.uVal.wssFixVal.empty() ? L'x' : rsUnion.uVal.wssFixVal.back() + 1;
            break;
        case 36:
            rsUnion.uVal.ptr8Val.resize(rsUnion.uVal.ptr8Val.size() * 2);
            for (size_t nIndex = 0; nIndex < rsUnion.uVal.ptr8Val.size() / 2; nIndex++)
                rsUnion.uVal.ptr8Val[nIndex + rsUnion.uVal.ptr8Val.size() / 2] = rsUnion.uVal.ptr8Val[nIndex];
            break;
        case 37:
            rsUnion.uVal.ptrssVal.resize(rsUnion.uVal.ptrssVal.size() * 2);
            for (size_t nIndex = 0; nIndex < rsUnion.uVal.ptrssVal.size() / 2; nIndex++)
                rsUnion.uVal.ptrssVal[nIndex + rsUnion.uVal.ptrssVal.size() / 2] = rsUnion.uVal.ptrssVal[nIndex];
            break;
        case 38:
            for (size_t nIndex = 0; nIndex < rsUnion.uVal.ptr8FixVal.size(); nIndex++)
                rsUnion.uVal.ptr8FixVal[nIndex] = static_cast<uint8_t>(rsUnion.uVal.ptr8FixVal[nIndex] << 1);
            break;
        case 39:
            for (size_t nIndex = 0; nIndex < rsUnion.uVal.ptrssFixVal.size(); nIndex++)
                rsUnion.uVal.ptrssFixVal[nIndex] = rsUnion.uVal.ptrssFixVal[nIndex] + rsUnion.uVal.ptrssFixVal[nIndex];
            break;
        case 40:
            rsUnion.uVal.seq8Val.resize(rsUnion.uVal.seq8Val.size() * 2);
            for (size_t nIndex = 0; nIndex < rsUnion.uVal.seq8Val.size() / 2; nIndex++)
                rsUnion.uVal.seq8Val[nIndex + rsUnion.uVal.seq8Val.size() / 2] = rsUnion.uVal.seq8Val[nIndex];
            break;
        case 41:
            rsUnion.uVal.seqssVal.resize(rsUnion.uVal.seqssVal.size() * 2);
            for (size_t nIndex = 0; nIndex < rsUnion.uVal.seqssVal.size() / 2; nIndex++)
                rsUnion.uVal.seqssVal[nIndex + rsUnion.uVal.seqssVal.size() / 2] = rsUnion.uVal.seqssVal[nIndex];
            break;
        case 42:
            for (size_t nIndex = 0; nIndex < rsUnion.uVal.seq8FixVal.size(); nIndex++)
                rsUnion.uVal.seq8FixVal[nIndex] = static_cast<uint8_t>(rsUnion.uVal.seq8FixVal[nIndex] << 1);
            break;
        case 43:
            for (size_t nIndex = 0; nIndex < rsUnion.uVal.seqssFixVal.size(); nIndex++)
                rsUnion.uVal.seqssFixVal[nIndex] = rsUnion.uVal.seqssFixVal[nIndex] + rsUnion.uVal.seqssFixVal[nIndex];
            break;
        case 44:
            if (rsUnion.uVal.ifcVal.get<IMultiplyValue>() && rsUnion.uVal.ifcVal.id() == sdv::GetInterfaceId<IMultiplyValue>())
            {
                m_summer.set(rsUnion.uVal.ifcVal.get<IMultiplyValue>());
                rsUnion.uVal.ifcVal = static_cast<IAddValue*>(&m_summer);
            }
            else
                rsUnion.uVal.ifcVal = nullptr;
            break;
        case 45:
            if (rsUnion.uVal.pMultiplyValue)
            {
                m_multiplier.set(rsUnion.uVal.pMultiplyValue);
                rsUnion.uVal.pMultiplyValue = static_cast<IMultiplyValue*>(&m_multiplier);
            }
            else
                rsUnion.uVal.pMultiplyValue = nullptr;
            break;
        case 46:
            rsUnion.uVal.sSubVal.i += 1;
            break;
            // Incompatible serdes generation: BUG #398509
            // case 47: rsUnion.uVal.sIndVal.i += 10;                                                                   break;
            // Unnamed struct inside unions are not supported yet: PBI #398230
            // case 48: rsUnion.uVal.sUnnamedVal.i += 20;                                                               break;
            // Unnamed unions are not supported yet: PBI #398209
            // case 49: union switch(int32) { case 0: int32 i; default: int32 j; } uUnnamedVal;
            // Anonymous structs and unions (unnamed and no declaration) are not supported yet: PBI #397894
            // case 50: struct { int32 iAnonymous1; int32 iAnonymous2; };
            // case 51: union switch(int32) { case 0: int32 iAnonymous1; default: int32 iAnonymous2; };

        case 201:
            rsUnion.uVal.tbVal = !rsUnion.uVal.tbVal;
            break;
        case 202:
            rsUnion.uVal.tsVal--;
            break;
        case 203:
            rsUnion.uVal.tusVal++;
            break;
        case 204:
            rsUnion.uVal.tlVal--;
            break;
        case 205:
            rsUnion.uVal.tulVal++;
            break;
        case 206:
            rsUnion.uVal.tllVal--;
            break;
        case 207:
            rsUnion.uVal.tullVal++;
            break;
        case 208:
            rsUnion.uVal.ti8Val--;
            break;
        case 209:
            rsUnion.uVal.tui8Val++;
            break;
        case 210:
            rsUnion.uVal.ti16Val--;
            break;
        case 211:
            rsUnion.uVal.tui16Val++;
            break;
        case 212:
            rsUnion.uVal.ti32Val--;
            break;
        case 213:
            rsUnion.uVal.tui32Val++;
            break;
        case 214:
            rsUnion.uVal.ti64Val--;
            break;
        case 215:
            rsUnion.uVal.tui64Val++;
            break;
        case 216:
            rsUnion.uVal.tcVal++;
            break;
        case 217:
            rsUnion.uVal.tc16Val++;
            break;
        case 218:
            rsUnion.uVal.tc32Val++;
            break;
        case 219:
            rsUnion.uVal.twcVal++;
            break;
        case 220:
            rsUnion.uVal.tfVal -= 1000.000f;
            break;
        case 221:
            rsUnion.uVal.tdVal += 1000.000;
            break;
            // GCC issue with generated initialization in the consructor for the "long double" type: BUG #3982727
            // case 22: rsUnion.uVal.tldVal -= 1000.000;     break;
        case 223:
            rsUnion.uVal.tnVal++;
            break;
        case 224:
            rsUnion.uVal.teHelloVal = rsUnion.uVal.teHelloVal++;
            break;
        case 225:
            if (rsUnion.uVal.tidVal == sdv::GetInterfaceId<IMultiplyValue>())
                rsUnion.uVal.tidVal = sdv::GetInterfaceId<IAddValue>();
            else
                rsUnion.uVal.tidVal = 0;
            break;
        case 226:
            rsUnion.uVal.tssVal += rsUnion.uVal.tssVal.empty() ? 'x' : rsUnion.uVal.tssVal.back() + 1;
            break;
        case 227:
            rsUnion.uVal.tss8Val += rsUnion.uVal.tss8Val.empty() ? 'x' : rsUnion.uVal.tss8Val.back() + 1;
            break;
        case 228:
            rsUnion.uVal.tss16Val += rsUnion.uVal.tss16Val.empty() ? u'x' : rsUnion.uVal.tss16Val.back() + 1;
            break;
        case 229:
            rsUnion.uVal.tss32Val += rsUnion.uVal.tss32Val.empty() ? U'x' : rsUnion.uVal.tss32Val.back() + 1;
            break;
        case 230:
            rsUnion.uVal.twssVal += rsUnion.uVal.twssVal.empty() ? L'x' : rsUnion.uVal.twssVal.back() + 1;
            break;
        case 231:
            rsUnion.uVal.tssFixVal += rsUnion.uVal.tssFixVal.empty() ? 'x' : rsUnion.uVal.tssFixVal.back() + 1;
            break;
        case 232:
            rsUnion.uVal.tss8FixVal += rsUnion.uVal.tss8FixVal.empty() ? 'x' : rsUnion.uVal.tss8FixVal.back() + 1;
            break;
        case 233:
            rsUnion.uVal.tss16FixVal += rsUnion.uVal.tss16FixVal.empty() ? u'x' : rsUnion.uVal.tss16FixVal.back() + 1;
            break;
        case 234:
            rsUnion.uVal.tss32FixVal += rsUnion.uVal.tss32FixVal.empty() ? U'x' : rsUnion.uVal.tss32FixVal.back() + 1;
            break;
        case 235:
            rsUnion.uVal.twssFixVal += rsUnion.uVal.twssFixVal.empty() ? L'x' : rsUnion.uVal.twssFixVal.back() + 1;
            break;
            // Using typedefs of template based types produced an error: BUG #398266
            // case 236:
            //    rsUnion.uVal.tptr8Val.resize(rsUnion.uVal.tptr8Val.size() * 2);
            //    for (size_t nIndex = 0; nIndex < rsUnion.uVal.tptr8Val.size() / 2; nIndex++)
            //        rsUnion.uVal.tptr8Val[nIndex + rsUnion.uVal.tptr8Val.size() / 2] = rsUnion.uVal.tptr8Val[nIndex];
            //    break;
            // case 237:
            //    rsUnion.uVal.tptrssVal.resize(rsUnion.uVal.tptrssVal.size() * 2);
            //    for (size_t nIndex = 0; nIndex < rsUnion.uVal.tptrssVal.size() / 2; nIndex++)
            //        rsUnion.uVal.tptrssVal[nIndex + rsUnion.uVal.tptrssVal.size() / 2] = rsUnion.uVal.tptrssVal[nIndex];
            //    break;
            // case 238:
            //    for (size_t nIndex = 0; nIndex < rsUnion.uVal.tptr8FixVal.size(); nIndex++)
            //        rsUnion.uVal.tptr8FixVal[nIndex] = static_cast<uint8_t>(rsUnion.uVal.tptr8FixVal[nIndex] << 1);
            //    break;
            // case 239:
            //    for (size_t nIndex = 0; nIndex < rsUnion.uVal.tptrssFixVal.size(); nIndex++)
            //        rsUnion.uVal.tptrssFixVal[nIndex] = rsUnion.uVal.tptrssFixVal[nIndex] + rsUnion.uVal.tptrssFixVal[nIndex];
            //    break;
            // case 240:
            //    rsUnion.uVal.tseq8Val.resize(rsUnion.uVal.tseq8Val.size() * 2);
            //    for (size_t nIndex = 0; nIndex < rsUnion.uVal.tseq8Val.size() / 2; nIndex++)
            //        rsUnion.uVal.tseq8Val[nIndex + rsUnion.uVal.tseq8Val.size() / 2] = rsUnion.uVal.tseq8Val[nIndex];
            //    break;
            // case 241:
            //    rsUnion.uVal.tseqssVal.resize(rsUnion.uVal.tseqssVal.size() * 2);
            //    for (size_t nIndex = 0; nIndex < rsUnion.uVal.tseqssVal.size() / 2; nIndex++)
            //        rsUnion.uVal.tseqssVal[nIndex + rsUnion.uVal.tseqssVal.size() / 2] = rsUnion.uVal.tseqssVal[nIndex];
            //    break;
            // case 242:
            //    for (size_t nIndex = 0; nIndex < rsUnion.uVal.tseq8FixVal.size(); nIndex++)
            //        rsUnion.uVal.tseq8FixVal[nIndex] = static_cast<uint8_t>(rsUnion.uVal.tseq8FixVal[nIndex] << 1);
            //    break;
            // case 243:
            //    for (size_t nIndex = 0; nIndex < rsUnion.uVal.tseqssFixVal.size(); nIndex++)
            //        rsUnion.uVal.tseqssFixVal[nIndex] = rsUnion.uVal.tseqssFixVal[nIndex] + rsUnion.uVal.tseqssFixVal[nIndex];
            //    break;
        case 244:
            if (rsUnion.uVal.tifcVal.get<IMultiplyValue>() && rsUnion.uVal.tifcVal.id() == sdv::GetInterfaceId<IMultiplyValue>())
            {
                m_summerTypedef.set(rsUnion.uVal.tifcVal.get<IMultiplyValue>());
                rsUnion.uVal.tifcVal = static_cast<IAddValue*>(&m_summerTypedef);
            }
            else
                rsUnion.uVal.tifcVal = nullptr;
            break;
            // Typedef of interfaces current not possible. BUG #399464
            // case 245:
            //    if (rsUnion.uVal.tpMultiplyValue)
            //    {
            //        m_multiplierTypedef.set(rsUnion.uVal.tpMultiplyValue);
            //        rsUnion.uVal.tpMultiplyValue = static_cast<IMultiplyValue*>(&m_multiplierTypedef);
            //    }
            //    else
            //        rsUnion.uVal.tpMultiplyValue = nullptr;
            //    break;
        case 246:
            rsUnion.uVal.tsSubVal.i += 1;
            break;
            // Incompatible serdes generation: BUG #398509
        case 247:
            rsUnion.uVal.tsIndVal.i += 10;
            break;
            // Unnamed struct inside unions are not supported yet: PBI #398230
            // case 248: rsUnion.uVal.tsUnnamedVal.i += 20;                                                               break;
            // Unnamed unions are not supported yet: PBI #398209
            // case 249: union switch(int32) { case 0: int32 i; default: int32 j; } uUnnamedVal;
            // Anonymous structs and unions (unnamed and no declaration) are not supported yet: PBI #397894
            // case 250: struct { int32 iAnonymous1; int32 iAnonymous2; };
            // case 251: union switch(int32) { case 0: int32 iAnonymous1; default: int32 iAnonymous2; };
        default:
            break;
        }
    }

    ISayHello* m_pHello = nullptr;

    struct SInternalMultiply : IMultiplyValue
    {
        void set(IMultiplyValue* p)
        {
            pMultiplyValue = p;
        }
        size_t Multiply(size_t n)
        {
            if (!pMultiplyValue)
                return 0;
            return pMultiplyValue->Multiply(n) * pMultiplyValue->Multiply(n);
        }
        IMultiplyValue* pMultiplyValue = nullptr;
    };
    SInternalMultiply m_multiplier;
    SInternalMultiply m_multiplierTypedef;

    struct SInternalAdd : IAddValue
    {
        void set(IMultiplyValue* p)
        {
            pMultiplyValue = p;
        }
        size_t Add(size_t n)
        {
            if (!pMultiplyValue)
                return 0;
            return pMultiplyValue->Multiply(n) + pMultiplyValue->Multiply(n);
        }
        IMultiplyValue* pMultiplyValue = nullptr;
    };
    SInternalAdd m_summer;
    SInternalAdd m_summerTypedef;
};

enum class ELocalIpcBackend
{
    sharedmem,
    sockets,
    tunnel
};

inline void LoadIPCModules(CCommunicationControl& rControl, ELocalIpcBackend eLocalBackend)
{
    sdv::u8string ssModule, ssObject;
    switch (eLocalBackend)
    {
    case ELocalIpcBackend::sockets:
#if defined(_WIN32)
        ssModule = "uds_win_sockets.sdv";
        ssObject = "WinSocketsChannelControl";
#elif defined(__unix__)
        ssModule = "uds_unix_sockets.sdv";
        ssObject = "UnixSocketsChannelControl";
#else
    #error Unsupported platform for IPC communication unit tests
#endif
        break;
    case ELocalIpcBackend::tunnel:
#if defined(_WIN32)
        ssModule = "uds_win_tunnel.sdv";
        ssObject = "WinTunnelChannelControl";
#elif defined(__unix__)
        ssModule = "uds_unix_tunnel.sdv";
        ssObject = "UnixTunnelChannelControl";
#else
    #error Unsupported platform for IPC communication unit tests
#endif
        break;
    case ELocalIpcBackend::sharedmem:
    default:
        ssModule = "ipc_shared_mem.sdv";
        ssObject = "DefaultSharedMemory";
        break;
    }

    // Load the IPC modules
    sdv::core::IModuleControl* pModuleControl = sdv::core::GetObject<sdv::core::IModuleControl>("ModuleControlService");
    ASSERT_NE(pModuleControl, nullptr);
    EXPECT_NE(pModuleControl->Load(ssModule), 0u);
    EXPECT_NE(pModuleControl->Load("core_ps.sdv"), 0u);
    EXPECT_NE(pModuleControl->Load("UnitTest_IPC_Communication_ps.sdv"), 0u);

    // Register communication control
    sdv::core::IRegisterForeignObject* pRegisterObject = sdv::core::GetObject<sdv::core::IRegisterForeignObject>("RepositoryService");

    ASSERT_NE(pRegisterObject, nullptr);

    EXPECT_NE(pRegisterObject->RegisterObject(&rControl, "CommunicationControl"), 0u);

    // Create channel control 
    sdv::core::IRepositoryControl* pRepositoryControl = sdv::core::GetObject<sdv::core::IRepositoryControl>("RepositoryService");

    ASSERT_NE(pRepositoryControl, nullptr);

    if (eLocalBackend == ELocalIpcBackend::tunnel)
    {
        // CRITICAL: channel name MUST match CreateEndpoint("DEFAULT_CHANNEL")
        EXPECT_NE(pRepositoryControl->CreateObject(ssObject, "DEFAULT_CHANNEL", ""), 0u);
    }
    else
    {
        // sockets + sharedmem don't require explicit channel naming
        EXPECT_NE(pRepositoryControl->CreateObject(ssObject, "", ""), 0u);
    }
}
