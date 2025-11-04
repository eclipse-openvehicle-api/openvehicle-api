#include "../../include/gtest_custom.h"
#include "../../../sdv_services/ipc_com/com_ctrl.h"
#include "../../../sdv_services/ipc_com/com_channel.h"
#include "../../../sdv_services/ipc_com/marshall_object.h"
#include <support/sdv_core.h>
#include <support/app_control.h>
#include <support/pssup.h>
#include "generated/test_ifc.h"

EHello& operator++(EHello& reHello, int)
{
    switch (reHello)
    {
    case EHello::hi:            reHello = EHello::huhu;             break;
    case EHello::huhu:          reHello = EHello::hello;            break;
    case EHello::hello:         reHello = EHello::hallo;            break;
    case EHello::hallo:         reHello = EHello::servus;           break;
    case EHello::servus:        reHello = EHello::gruessgott;       break;
    case EHello::gruessgott:    reHello = EHello::hi;               break;
    default:                                                        break;
    }
    return reHello;
}

EHello& operator--(EHello& reHello, int)
{
    switch (reHello)
    {
    case EHello::hi:            reHello = EHello::gruessgott;       break;
    case EHello::huhu:          reHello = EHello::hi;               break;
    case EHello::hello:         reHello = EHello::huhu;             break;
    case EHello::hallo:         reHello = EHello::hello;            break;
    case EHello::servus:        reHello = EHello::hallo;            break;
    case EHello::gruessgott:    reHello = EHello::servus;           break;
    default:                                                        break;
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

class CInterfaceTest : public ISayHello, public IRequestHello, public IRegisterHelloCallback, public IMegaTest, public sdv::IInterfaceAccess
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
        //rsStruct.sUnnamedVal.i += 20;  // 12.04.2024 EVE Unnamed structures are not supported by IDL compiler.
        // Unnamed unions (inline definition) are not supported yet: PBI #398209
        //union switch(int32) { case 0: int32 i; default: int32 j; } uUnnamedVal;
        // Anonymous structs and unions (unnamed and no declaration) are not supported yet: PBI #397894
        //struct { int32 iAnonymous1; int32 iAnonymous2; };
        //union switch(int32) { case 0: int32 iAnonymous1; default: int32 iAnonymous2; };

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
        //if (rsStruct.rgifcVal[0].get<IMultiplyValue>() && rsStruct.rgifcVal[0].id() == sdv::GetInterfaceId<IMultiplyValue>())
        //{
        //    m_summer.set(rsStruct.rgifcVal[0].get<IMultiplyValue>());
        //    rsStruct.rgifcVal[0] = static_cast<IAddValue*>(&m_summer);
        //}
        //else
        //    rsStruct.rgifcVal[0] = nullptr;
        //if (rsStruct.rgifcVal[1].get<IMultiplyValue>() && rsStruct.rgifcVal[1].id() == sdv::GetInterfaceId<IMultiplyValue>())
        //{
        //    m_summer.set(rsStruct.rgifcVal[1].get<IMultiplyValue>());
        //    rsStruct.rgifcVal[1] = static_cast<IAddValue*>(&m_summer);
        //}
        //else
        //    rsStruct.rgifcVal[1] = nullptr;
        //if (rsStruct.rgpMultiplyValue[0])
        //{
        //    m_multiplier.set(rsStruct.rgpMultiplyValue[0]);
        //    rsStruct.rgpMultiplyValue[0] = static_cast<IMultiplyValue*>(&m_multiplier);
        //}
        //else
        //    rsStruct.rgpMultiplyValue[0] = nullptr;
        //if (rsStruct.rgpMultiplyValue[1])
        //{
        //    m_multiplier.set(rsStruct.rgpMultiplyValue[1]);
        //    rsStruct.rgpMultiplyValue[1] = static_cast<IMultiplyValue*>(&m_multiplier);
        //}
        //else
        //    rsStruct.rgpMultiplyValue[1] = nullptr;
        rsStruct.rgsSubVal[0].i += 1;
        rsStruct.rgsSubVal[1].i += 2;
        // Incompatible serdes generation: BUG #398509
        //rsStruct.rgsIndVal[0].i += 10;
        //rsStruct.rgsIndVal[1].i += 20;
        // Unnamed struct with array is not yet supported due to incorret serdes code generation: BUG #398246
        //rsStruct.rgsUnnamedVal[0].i += 20;
        //rsStruct.rgsUnnamedVal[1].i += 40;

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
        //rsStruct.tptr8Val.resize(rsStruct.tptr8Val.size() * 2);
        //for (size_t nIndex = 0; nIndex < rsStruct.tptr8Val.size() / 2; nIndex++)
        //    rsStruct.tptr8Val[nIndex + rsStruct.tptr8Val.size() / 2] = rsStruct.tptr8Val[nIndex];
        //rsStruct.ptrssVal.resize(rsStruct.tptrssVal.size() * 2);
        //for (size_t nIndex = 0; nIndex < rsStruct.tptrssVal.size() / 2; nIndex++)
        //    rsStruct.tptrssVal[nIndex + rsStruct.tptrssVal.size() / 2] = rsStruct.tptrssVal[nIndex];
        //for (size_t nIndex = 0; nIndex < rsStruct.tptr8FixVal.size(); nIndex++)
        //    rsStruct.tptr8FixVal[nIndex] = static_cast<uint8_t>(rsStruct.tptr8FixVal[nIndex] << 1);
        //for (size_t nIndex = 0; nIndex < rsStruct.tptrssFixVal.size(); nIndex++)
        //    rsStruct.ptrssFixVal[nIndex] = rsStruct.tptrssFixVal[nIndex] + rsStruct.tptrssFixVal[nIndex];
        //rsStruct.tseq8Val.resize(rsStruct.tseq8Val.size() * 2);
        //for (size_t nIndex = 0; nIndex < rsStruct.tseq8Val.size() / 2; nIndex++)
        //    rsStruct.tseq8Val[nIndex + rsStruct.tseq8Val.size() / 2] = rsStruct.tseq8Val[nIndex];
        //rsStruct.tseqssVal.resize(rsStruct.tseqssVal.size() * 2);
        //for (size_t nIndex = 0; nIndex < rsStruct.tseqssVal.size() / 2; nIndex++)
        //    rsStruct.seqssVal[nIndex + rsStruct.tseqssVal.size() / 2] = rsStruct.tseqssVal[nIndex];
        //for (size_t nIndex = 0; nIndex < rsStruct.tseq8FixVal.size(); nIndex++)
        //    rsStruct.tseq8FixVal[nIndex] = static_cast<uint8_t>(rsStruct.tseq8FixVal[nIndex] << 1);
        //for (size_t nIndex = 0; nIndex < rsStruct.tseqssFixVal.size(); nIndex++)
        //    rsStruct.tseqssFixVal[nIndex] = rsStruct.tseqssFixVal[nIndex] + rsStruct.tseqssFixVal[nIndex];
        if (rsStruct.tifcVal.get<IMultiplyValue>() && rsStruct.tifcVal.id() == sdv::GetInterfaceId<IMultiplyValue>())
        {
            m_summerTypedef.set(rsStruct.tifcVal.get<IMultiplyValue>());
            rsStruct.tifcVal = static_cast<IAddValue*>(&m_summerTypedef);
        }
        else
            rsStruct.tifcVal = nullptr;
        // Typedef of interfaces current not possible. BUG #399464
        //if (rsStruct.tpMultiplyValue)
        //{
        //    m_multiplierTypedef.set(rsStruct.tpMultiplyValue);
        //    rsStruct.tpMultiplyValue = static_cast<IMultiplyValue*>(&m_multiplierTypedef);
        //}
        //else
        //    rsStruct.tpMultiplyValue = nullptr;
        rsStruct.tsSubVal.i += 1;
        rsStruct.tsIndVal.i += 10;
        // Incompatible serdes generation: BUG #398509
        //rsStruct.tsUnnamedVal.i += 20;
        // Unnamed unions (inline definition) are not supported yet: PBI #398209
        //union switch(int32) { case 0: int32 i; default: int32 j; } tuUnnamedVal;
        // Anonymous structs and unions (unnamed and no declaration) are not supported yet: PBI #397894
        //struct { int32 iAnonymous1; int32 iAnonymous2; };
        //union switch(int32) { case 0: int32 iAnonymous1; default: int32 iAnonymous2; };
    }

    /**
    * @brief Update the mega union. This will test marshalling and unmarshalling of the mega union. Overload of
    * IMegaTest::ProcessMegaTypeBasedUnion.
    */
    virtual void ProcessMegaTypeBasedUnion(/*inout*/ UMegaTypeBasedUnion& ruUnion) override
    {
        switch (ruUnion.get_switch())
        {
        case 1: ruUnion.bVal = !ruUnion.bVal;   break;
        case 2: ruUnion.sVal--;                 break;
        case 3: ruUnion.usVal++;                break;
        case 4: ruUnion.lVal--;                 break;
        case 5: ruUnion.ulVal++;                break;
        case 6: ruUnion.llVal--;                break;
        case 7: ruUnion.ullVal++;               break;
        case 8: ruUnion.i8Val--;                break;
        case 9: ruUnion.ui8Val++;               break;
        case 10: ruUnion.i16Val--;              break;
        case 11: ruUnion.ui16Val++;             break;
        case 12: ruUnion.i32Val--;              break;
        case 13: ruUnion.ui32Val++;             break;
        case 14: ruUnion.i64Val--;              break;
        case 15: ruUnion.ui64Val++;             break;
        case 16: ruUnion.cVal++;                break;
        case 17: ruUnion.c16Val++;              break;
        case 18: ruUnion.c32Val++;              break;
        case 19: ruUnion.wcVal++;               break;
        case 20: ruUnion.fVal -= 1000.000f;     break;
        case 21: ruUnion.dVal += 1000.000;      break;
            // GCC issue with generated initialization in the consructor for the "long double" type: BUG #3982727
            //case 22: ruUnion.ldVal -= 1000.000;     break;
        case 23: ruUnion.nVal++;                break;
        case 24: ruUnion.eHelloVal = ruUnion.eHelloVal++; break;
        case 25:
            if (ruUnion.idVal == sdv::GetInterfaceId<IMultiplyValue>())
                ruUnion.idVal = sdv::GetInterfaceId<IAddValue>();
            else
                ruUnion.idVal = 0;
            break;
        case 26: ruUnion.ssVal += ruUnion.ssVal.empty() ? 'x' : ruUnion.ssVal.back() + 1;                   break;
        case 27: ruUnion.ss8Val += ruUnion.ss8Val.empty() ? 'x' : ruUnion.ss8Val.back() + 1;                break;
        case 28: ruUnion.ss16Val += ruUnion.ss16Val.empty() ? u'x' : ruUnion.ss16Val.back() + 1;            break;
        case 29: ruUnion.ss32Val += ruUnion.ss32Val.empty() ? U'x' : ruUnion.ss32Val.back() + 1;            break;
        case 30: ruUnion.wssVal += ruUnion.wssVal.empty() ? L'x' : ruUnion.wssVal.back() + 1;               break;
        case 31: ruUnion.ssFixVal += ruUnion.ssFixVal.empty() ? 'x' : ruUnion.ssFixVal.back() + 1;          break;
        case 32: ruUnion.ss8FixVal += ruUnion.ss8FixVal.empty() ? 'x' : ruUnion.ss8FixVal.back() + 1;       break;
        case 33: ruUnion.ss16FixVal += ruUnion.ss16FixVal.empty() ? u'x' : ruUnion.ss16FixVal.back() + 1;   break;
        case 34: ruUnion.ss32FixVal += ruUnion.ss32FixVal.empty() ? U'x' : ruUnion.ss32FixVal.back() + 1;   break;
        case 35: ruUnion.wssFixVal += ruUnion.wssFixVal.empty() ? L'x' : ruUnion.wssFixVal.back() + 1;      break;
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
        case 46: ruUnion.sSubVal.i += 1;                                                                    break;
            // Incompatible serdes generation: BUG #398509
            //case 47: ruUnion.sIndVal.i += 10;                                                                   break;
            // Unnamed struct inside unions are not supported yet: PBI #398230
            //case 48: ruUnion.sUnnamedVal.i += 20;                                                               break;
            // Unnamed unions are not supported yet: PBI #398209
            //case 49: union switch(int32) { case 0: int32 i; default: int32 j; } uUnnamedVal;
            // Anonymous structs and unions (unnamed and no declaration) are not supported yet: PBI #397894
            //case 50: struct { int32 iAnonymous1; int32 iAnonymous2; };
            //case 51: union switch(int32) { case 0: int32 iAnonymous1; default: int32 iAnonymous2; };

        case 201: ruUnion.tbVal = !ruUnion.tbVal;   break;
        case 202: ruUnion.tsVal--;                 break;
        case 203: ruUnion.tusVal++;                break;
        case 204: ruUnion.tlVal--;                 break;
        case 205: ruUnion.tulVal++;                break;
        case 206: ruUnion.tllVal--;                break;
        case 207: ruUnion.tullVal++;               break;
        case 208: ruUnion.ti8Val--;                break;
        case 209: ruUnion.tui8Val++;               break;
        case 210: ruUnion.ti16Val--;              break;
        case 211: ruUnion.tui16Val++;             break;
        case 212: ruUnion.ti32Val--;              break;
        case 213: ruUnion.tui32Val++;             break;
        case 214: ruUnion.ti64Val--;              break;
        case 215: ruUnion.tui64Val++;             break;
        case 216: ruUnion.tcVal++;                break;
        case 217: ruUnion.tc16Val++;              break;
        case 218: ruUnion.tc32Val++;              break;
        case 219: ruUnion.twcVal++;               break;
        case 220: ruUnion.tfVal -= 1000.000f;     break;
        case 221: ruUnion.tdVal += 1000.000;      break;
            // GCC issue with generated initialization in the consructor for the "long double" type: BUG #3982727
            //case 22: ruUnion.tldVal -= 1000.000;     break;
        case 223: ruUnion.tnVal++;                break;
        case 224: ruUnion.teHelloVal = ruUnion.teHelloVal++; break;
        case 225:
            if (ruUnion.tidVal == sdv::GetInterfaceId<IMultiplyValue>())
                ruUnion.tidVal = sdv::GetInterfaceId<IAddValue>();
            else
                ruUnion.tidVal = 0;
            break;
        case 226: ruUnion.tssVal += ruUnion.tssVal.empty() ? 'x' : ruUnion.tssVal.back() + 1;                   break;
        case 227: ruUnion.tss8Val += ruUnion.tss8Val.empty() ? 'x' : ruUnion.tss8Val.back() + 1;                break;
        case 228: ruUnion.tss16Val += ruUnion.tss16Val.empty() ? u'x' : ruUnion.tss16Val.back() + 1;            break;
        case 229: ruUnion.tss32Val += ruUnion.tss32Val.empty() ? U'x' : ruUnion.tss32Val.back() + 1;            break;
        case 230: ruUnion.twssVal += ruUnion.twssVal.empty() ? L'x' : ruUnion.twssVal.back() + 1;               break;
        case 231: ruUnion.tssFixVal += ruUnion.tssFixVal.empty() ? 'x' : ruUnion.tssFixVal.back() + 1;          break;
        case 232: ruUnion.tss8FixVal += ruUnion.tss8FixVal.empty() ? 'x' : ruUnion.tss8FixVal.back() + 1;       break;
        case 233: ruUnion.tss16FixVal += ruUnion.tss16FixVal.empty() ? u'x' : ruUnion.tss16FixVal.back() + 1;   break;
        case 234: ruUnion.tss32FixVal += ruUnion.tss32FixVal.empty() ? U'x' : ruUnion.tss32FixVal.back() + 1;   break;
        case 235: ruUnion.twssFixVal += ruUnion.twssFixVal.empty() ? L'x' : ruUnion.twssFixVal.back() + 1;      break;
            // Using typedefs of template based types produced an error: BUG #398266
            //case 236:
            //    ruUnion.tptr8Val.resize(ruUnion.tptr8Val.size() * 2);
            //    for (size_t nIndex = 0; nIndex < ruUnion.tptr8Val.size() / 2; nIndex++)
            //        ruUnion.tptr8Val[nIndex + ruUnion.tptr8Val.size() / 2] = ruUnion.tptr8Val[nIndex];
            //    break;
            //case 237:
            //    ruUnion.tptrssVal.resize(ruUnion.tptrssVal.size() * 2);
            //    for (size_t nIndex = 0; nIndex < ruUnion.tptrssVal.size() / 2; nIndex++)
            //        ruUnion.tptrssVal[nIndex + ruUnion.tptrssVal.size() / 2] = ruUnion.tptrssVal[nIndex];
            //    break;
            //case 238:
            //    for (size_t nIndex = 0; nIndex < ruUnion.tptr8FixVal.size(); nIndex++)
            //        ruUnion.tptr8FixVal[nIndex] = static_cast<uint8_t>(ruUnion.tptr8FixVal[nIndex] << 1);
            //    break;
            //case 239:
            //    for (size_t nIndex = 0; nIndex < ruUnion.tptrssFixVal.size(); nIndex++)
            //        ruUnion.tptrssFixVal[nIndex] = ruUnion.tptrssFixVal[nIndex] + ruUnion.tptrssFixVal[nIndex];
            //    break;
            //case 240:
            //    ruUnion.tseq8Val.resize(ruUnion.tseq8Val.size() * 2);
            //    for (size_t nIndex = 0; nIndex < ruUnion.tseq8Val.size() / 2; nIndex++)
            //        ruUnion.tseq8Val[nIndex + ruUnion.tseq8Val.size() / 2] = ruUnion.tseq8Val[nIndex];
            //    break;
            //case 241:
            //    ruUnion.tseqssVal.resize(ruUnion.tseqssVal.size() * 2);
            //    for (size_t nIndex = 0; nIndex < ruUnion.tseqssVal.size() / 2; nIndex++)
            //        ruUnion.tseqssVal[nIndex + ruUnion.tseqssVal.size() / 2] = ruUnion.tseqssVal[nIndex];
            //    break;
            //case 242:
            //    for (size_t nIndex = 0; nIndex < ruUnion.tseq8FixVal.size(); nIndex++)
            //        ruUnion.tseq8FixVal[nIndex] = static_cast<uint8_t>(ruUnion.tseq8FixVal[nIndex] << 1);
            //    break;
            //case 243:
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
            //case 245:
            //    if (ruUnion.tpMultiplyValue)
            //    {
            //        m_multiplierTypedef.set(ruUnion.tpMultiplyValue);
            //        ruUnion.tpMultiplyValue = static_cast<IMultiplyValue*>(&m_multiplierTypedef);
            //    }
            //    else
            //        ruUnion.tpMultiplyValue = nullptr;
            //    break;
        case 246: ruUnion.tsSubVal.i += 1;                                                                    break;
            // Incompatible serdes generation: BUG #398509
        case 247: ruUnion.tsIndVal.i += 10;                                                                   break;
            // Unnamed struct inside unions are not supported yet: PBI #398230
            //case 248: ruUnion.tsUnnamedVal.i += 20;                                                               break;
            // Unnamed unions are not supported yet: PBI #398209
            //case 249: union switch(int32) { case 0: int32 i; default: int32 j; } uUnnamedVal;
            // Anonymous structs and unions (unnamed and no declaration) are not supported yet: PBI #397894
            //case 250: struct { int32 iAnonymous1; int32 iAnonymous2; };
            //case 251: union switch(int32) { case 0: int32 iAnonymous1; default: int32 iAnonymous2; };
        default:                                break;
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
        case 1: rsUnion.uVal.bVal = !rsUnion.uVal.bVal;   break;
        case 2: rsUnion.uVal.sVal--;                 break;
        case 3: rsUnion.uVal.usVal++;                break;
        case 4: rsUnion.uVal.lVal--;                 break;
        case 5: rsUnion.uVal.ulVal++;                break;
        case 6: rsUnion.uVal.llVal--;                break;
        case 7: rsUnion.uVal.ullVal++;               break;
        case 8: rsUnion.uVal.i8Val--;                break;
        case 9: rsUnion.uVal.ui8Val++;               break;
        case 10: rsUnion.uVal.i16Val--;              break;
        case 11: rsUnion.uVal.ui16Val++;             break;
        case 12: rsUnion.uVal.i32Val--;              break;
        case 13: rsUnion.uVal.ui32Val++;             break;
        case 14: rsUnion.uVal.i64Val--;              break;
        case 15: rsUnion.uVal.ui64Val++;             break;
        case 16: rsUnion.uVal.cVal++;                break;
        case 17: rsUnion.uVal.c16Val++;              break;
        case 18: rsUnion.uVal.c32Val++;              break;
        case 19: rsUnion.uVal.wcVal++;               break;
        case 20: rsUnion.uVal.fVal -= 1000.000f;     break;
        case 21: rsUnion.uVal.dVal += 1000.000;      break;
            // GCC issue with generated initialization in the consructor for the "long double" type: BUG #3982727
            //case 22: rsUnion.uVal.ldVal -= 1000.000;     break;
        case 23: rsUnion.uVal.nVal++;                break;
        case 24: rsUnion.uVal.eHelloVal = rsUnion.uVal.eHelloVal++; break;
        case 25:
            if (rsUnion.uVal.idVal == sdv::GetInterfaceId<IMultiplyValue>())
                rsUnion.uVal.idVal = sdv::GetInterfaceId<IAddValue>();
            else
                rsUnion.uVal.idVal = 0;
            break;
        case 26: rsUnion.uVal.ssVal += rsUnion.uVal.ssVal.empty() ? 'x' : rsUnion.uVal.ssVal.back() + 1;                   break;
        case 27: rsUnion.uVal.ss8Val += rsUnion.uVal.ss8Val.empty() ? 'x' : rsUnion.uVal.ss8Val.back() + 1;                break;
        case 28: rsUnion.uVal.ss16Val += rsUnion.uVal.ss16Val.empty() ? u'x' : rsUnion.uVal.ss16Val.back() + 1;            break;
        case 29: rsUnion.uVal.ss32Val += rsUnion.uVal.ss32Val.empty() ? U'x' : rsUnion.uVal.ss32Val.back() + 1;            break;
        case 30: rsUnion.uVal.wssVal += rsUnion.uVal.wssVal.empty() ? L'x' : rsUnion.uVal.wssVal.back() + 1;               break;
        case 31: rsUnion.uVal.ssFixVal += rsUnion.uVal.ssFixVal.empty() ? 'x' : rsUnion.uVal.ssFixVal.back() + 1;          break;
        case 32: rsUnion.uVal.ss8FixVal += rsUnion.uVal.ss8FixVal.empty() ? 'x' : rsUnion.uVal.ss8FixVal.back() + 1;       break;
        case 33: rsUnion.uVal.ss16FixVal += rsUnion.uVal.ss16FixVal.empty() ? u'x' : rsUnion.uVal.ss16FixVal.back() + 1;   break;
        case 34: rsUnion.uVal.ss32FixVal += rsUnion.uVal.ss32FixVal.empty() ? U'x' : rsUnion.uVal.ss32FixVal.back() + 1;   break;
        case 35: rsUnion.uVal.wssFixVal += rsUnion.uVal.wssFixVal.empty() ? L'x' : rsUnion.uVal.wssFixVal.back() + 1;      break;
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
        case 46: rsUnion.uVal.sSubVal.i += 1;                                                                    break;
            // Incompatible serdes generation: BUG #398509
            //case 47: rsUnion.uVal.sIndVal.i += 10;                                                                   break;
            // Unnamed struct inside unions are not supported yet: PBI #398230
            //case 48: rsUnion.uVal.sUnnamedVal.i += 20;                                                               break;
            // Unnamed unions are not supported yet: PBI #398209
            //case 49: union switch(int32) { case 0: int32 i; default: int32 j; } uUnnamedVal;
            // Anonymous structs and unions (unnamed and no declaration) are not supported yet: PBI #397894
            //case 50: struct { int32 iAnonymous1; int32 iAnonymous2; };
            //case 51: union switch(int32) { case 0: int32 iAnonymous1; default: int32 iAnonymous2; };

        case 201: rsUnion.uVal.tbVal = !rsUnion.uVal.tbVal;   break;
        case 202: rsUnion.uVal.tsVal--;                 break;
        case 203: rsUnion.uVal.tusVal++;                break;
        case 204: rsUnion.uVal.tlVal--;                 break;
        case 205: rsUnion.uVal.tulVal++;                break;
        case 206: rsUnion.uVal.tllVal--;                break;
        case 207: rsUnion.uVal.tullVal++;               break;
        case 208: rsUnion.uVal.ti8Val--;                break;
        case 209: rsUnion.uVal.tui8Val++;               break;
        case 210: rsUnion.uVal.ti16Val--;              break;
        case 211: rsUnion.uVal.tui16Val++;             break;
        case 212: rsUnion.uVal.ti32Val--;              break;
        case 213: rsUnion.uVal.tui32Val++;             break;
        case 214: rsUnion.uVal.ti64Val--;              break;
        case 215: rsUnion.uVal.tui64Val++;             break;
        case 216: rsUnion.uVal.tcVal++;                break;
        case 217: rsUnion.uVal.tc16Val++;              break;
        case 218: rsUnion.uVal.tc32Val++;              break;
        case 219: rsUnion.uVal.twcVal++;               break;
        case 220: rsUnion.uVal.tfVal -= 1000.000f;     break;
        case 221: rsUnion.uVal.tdVal += 1000.000;      break;
            // GCC issue with generated initialization in the consructor for the "long double" type: BUG #3982727
            //case 22: rsUnion.uVal.tldVal -= 1000.000;     break;
        case 223: rsUnion.uVal.tnVal++;                break;
        case 224: rsUnion.uVal.teHelloVal = rsUnion.uVal.teHelloVal++; break;
        case 225:
            if (rsUnion.uVal.tidVal == sdv::GetInterfaceId<IMultiplyValue>())
                rsUnion.uVal.tidVal = sdv::GetInterfaceId<IAddValue>();
            else
                rsUnion.uVal.tidVal = 0;
            break;
        case 226: rsUnion.uVal.tssVal += rsUnion.uVal.tssVal.empty() ? 'x' : rsUnion.uVal.tssVal.back() + 1;                   break;
        case 227: rsUnion.uVal.tss8Val += rsUnion.uVal.tss8Val.empty() ? 'x' : rsUnion.uVal.tss8Val.back() + 1;                break;
        case 228: rsUnion.uVal.tss16Val += rsUnion.uVal.tss16Val.empty() ? u'x' : rsUnion.uVal.tss16Val.back() + 1;            break;
        case 229: rsUnion.uVal.tss32Val += rsUnion.uVal.tss32Val.empty() ? U'x' : rsUnion.uVal.tss32Val.back() + 1;            break;
        case 230: rsUnion.uVal.twssVal += rsUnion.uVal.twssVal.empty() ? L'x' : rsUnion.uVal.twssVal.back() + 1;               break;
        case 231: rsUnion.uVal.tssFixVal += rsUnion.uVal.tssFixVal.empty() ? 'x' : rsUnion.uVal.tssFixVal.back() + 1;          break;
        case 232: rsUnion.uVal.tss8FixVal += rsUnion.uVal.tss8FixVal.empty() ? 'x' : rsUnion.uVal.tss8FixVal.back() + 1;       break;
        case 233: rsUnion.uVal.tss16FixVal += rsUnion.uVal.tss16FixVal.empty() ? u'x' : rsUnion.uVal.tss16FixVal.back() + 1;   break;
        case 234: rsUnion.uVal.tss32FixVal += rsUnion.uVal.tss32FixVal.empty() ? U'x' : rsUnion.uVal.tss32FixVal.back() + 1;   break;
        case 235: rsUnion.uVal.twssFixVal += rsUnion.uVal.twssFixVal.empty() ? L'x' : rsUnion.uVal.twssFixVal.back() + 1;      break;
            // Using typedefs of template based types produced an error: BUG #398266
            //case 236:
            //    rsUnion.uVal.tptr8Val.resize(rsUnion.uVal.tptr8Val.size() * 2);
            //    for (size_t nIndex = 0; nIndex < rsUnion.uVal.tptr8Val.size() / 2; nIndex++)
            //        rsUnion.uVal.tptr8Val[nIndex + rsUnion.uVal.tptr8Val.size() / 2] = rsUnion.uVal.tptr8Val[nIndex];
            //    break;
            //case 237:
            //    rsUnion.uVal.tptrssVal.resize(rsUnion.uVal.tptrssVal.size() * 2);
            //    for (size_t nIndex = 0; nIndex < rsUnion.uVal.tptrssVal.size() / 2; nIndex++)
            //        rsUnion.uVal.tptrssVal[nIndex + rsUnion.uVal.tptrssVal.size() / 2] = rsUnion.uVal.tptrssVal[nIndex];
            //    break;
            //case 238:
            //    for (size_t nIndex = 0; nIndex < rsUnion.uVal.tptr8FixVal.size(); nIndex++)
            //        rsUnion.uVal.tptr8FixVal[nIndex] = static_cast<uint8_t>(rsUnion.uVal.tptr8FixVal[nIndex] << 1);
            //    break;
            //case 239:
            //    for (size_t nIndex = 0; nIndex < rsUnion.uVal.tptrssFixVal.size(); nIndex++)
            //        rsUnion.uVal.tptrssFixVal[nIndex] = rsUnion.uVal.tptrssFixVal[nIndex] + rsUnion.uVal.tptrssFixVal[nIndex];
            //    break;
            //case 240:
            //    rsUnion.uVal.tseq8Val.resize(rsUnion.uVal.tseq8Val.size() * 2);
            //    for (size_t nIndex = 0; nIndex < rsUnion.uVal.tseq8Val.size() / 2; nIndex++)
            //        rsUnion.uVal.tseq8Val[nIndex + rsUnion.uVal.tseq8Val.size() / 2] = rsUnion.uVal.tseq8Val[nIndex];
            //    break;
            //case 241:
            //    rsUnion.uVal.tseqssVal.resize(rsUnion.uVal.tseqssVal.size() * 2);
            //    for (size_t nIndex = 0; nIndex < rsUnion.uVal.tseqssVal.size() / 2; nIndex++)
            //        rsUnion.uVal.tseqssVal[nIndex + rsUnion.uVal.tseqssVal.size() / 2] = rsUnion.uVal.tseqssVal[nIndex];
            //    break;
            //case 242:
            //    for (size_t nIndex = 0; nIndex < rsUnion.uVal.tseq8FixVal.size(); nIndex++)
            //        rsUnion.uVal.tseq8FixVal[nIndex] = static_cast<uint8_t>(rsUnion.uVal.tseq8FixVal[nIndex] << 1);
            //    break;
            //case 243:
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
            //case 245:
            //    if (rsUnion.uVal.tpMultiplyValue)
            //    {
            //        m_multiplierTypedef.set(rsUnion.uVal.tpMultiplyValue);
            //        rsUnion.uVal.tpMultiplyValue = static_cast<IMultiplyValue*>(&m_multiplierTypedef);
            //    }
            //    else
            //        rsUnion.uVal.tpMultiplyValue = nullptr;
            //    break;
        case 246: rsUnion.uVal.tsSubVal.i += 1;                                                                    break;
            // Incompatible serdes generation: BUG #398509
        case 247: rsUnion.uVal.tsIndVal.i += 10;                                                                   break;
            // Unnamed struct inside unions are not supported yet: PBI #398230
            //case 248: rsUnion.uVal.tsUnnamedVal.i += 20;                                                               break;
            // Unnamed unions are not supported yet: PBI #398209
            //case 249: union switch(int32) { case 0: int32 i; default: int32 j; } uUnnamedVal;
            // Anonymous structs and unions (unnamed and no declaration) are not supported yet: PBI #397894
            //case 250: struct { int32 iAnonymous1; int32 iAnonymous2; };
            //case 251: union switch(int32) { case 0: int32 iAnonymous1; default: int32 iAnonymous2; };
        default:                                break;
        }
    }

    ISayHello* m_pHello = nullptr;

    struct SInternalMultiply : IMultiplyValue
    {
        void set(IMultiplyValue* p) { pMultiplyValue = p; }
        size_t Multiply(size_t n)
        {
            if (!pMultiplyValue) return 0;
            return pMultiplyValue->Multiply(n) * pMultiplyValue->Multiply(n);
        }
        IMultiplyValue* pMultiplyValue = nullptr;
    };
    SInternalMultiply   m_multiplier;
    SInternalMultiply   m_multiplierTypedef;

    struct SInternalAdd : IAddValue
    {
        void set(IMultiplyValue* p) { pMultiplyValue = p; }
        size_t Add(size_t n)
        {
            if (!pMultiplyValue) return 0;
            return pMultiplyValue->Multiply(n) + pMultiplyValue->Multiply(n);
        }
        IMultiplyValue* pMultiplyValue = nullptr;
    };
    SInternalAdd        m_summer;
    SInternalAdd        m_summerTypedef;
};

void LoadIPCModules(CCommunicationControl& rControl)
{
    // Load the IPC modules
    sdv::core::IModuleControl* pModuleControl = sdv::core::GetObject<sdv::core::IModuleControl>("ModuleControlService");
    ASSERT_NE(pModuleControl, nullptr);
    EXPECT_NE(pModuleControl->Load("ipc_shared_mem.sdv"), 0u);
    EXPECT_NE(pModuleControl->Load("core_ps.sdv"), 0u);
    EXPECT_NE(pModuleControl->Load("UnitTest_IPC_Communication_ps.sdv"), 0u);

    // Create the services
    sdv::core::IRepositoryControl* pRepositoryControl = sdv::core::GetObject<sdv::core::IRepositoryControl>("RepositoryService");
    ASSERT_NE(pRepositoryControl, nullptr);
    EXPECT_NE(pRepositoryControl->CreateObject("DefaultSharedMemoryChannelControl", {}, {}), 0u);

    // Register the communication control service
    sdv::core::IRegisterForeignObject* pRegisterObject = sdv::core::GetObject<sdv::core::IRegisterForeignObject>("RepositoryService");
    ASSERT_NE(pRegisterObject, nullptr);
    EXPECT_NE(pRegisterObject->RegisterObject(&rControl, "CommunicationControl"), 0u);
}

TEST(IPC_Communication_Test, Instantiate)
{
    CCommunicationControl control;
    EXPECT_NO_THROW(control.Initialize(""));
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::initialized);
    EXPECT_NO_THROW(control.Shutdown());
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::destruction_pending);
}

TEST(IPC_Communication_Test, AssignServerEndpoint)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize("");
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::configuring);

    // Load the shared memory components
    LoadIPCModules(control);

    // Create a channel endpoint
    sdv::ipc::ICreateEndpoint* ptrCreateEndpoint = sdv::core::GetObject<sdv::ipc::ICreateEndpoint>("LocalChannelControl");
    ASSERT_NE(ptrCreateEndpoint, nullptr);
    sdv::ipc::SChannelEndpoint sChannelEndpoint = ptrCreateEndpoint->CreateEndpoint("");
    EXPECT_NE(sChannelEndpoint.pConnection, nullptr);
    EXPECT_FALSE(sChannelEndpoint.ssConnectString.empty());
    sdv::TObjectPtr ptrEndpoint = sChannelEndpoint.pConnection;     // Automatic lifetime management
    EXPECT_TRUE(ptrEndpoint);

    // Assign the server endpoint
    CInterfaceTest test;
    EXPECT_TRUE(control.AssignServerEndpoint(ptrEndpoint, &test, 100, false) != 0u);
    ptrEndpoint.Clear();    // Lifetime taken over by communication control.

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test, AssignClientEndpoint)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize("");
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::configuring);

    // Load the shared memory components
    LoadIPCModules(control);

    // Create a server endpoint
    sdv::ipc::ICreateEndpoint* ptrCreateEndpoint = sdv::core::GetObject<sdv::ipc::ICreateEndpoint>("LocalChannelControl");
    ASSERT_NE(ptrCreateEndpoint, nullptr);
    sdv::ipc::SChannelEndpoint sChannelEndpoint = ptrCreateEndpoint->CreateEndpoint("");
    EXPECT_NE(sChannelEndpoint.pConnection, nullptr);
    EXPECT_FALSE(sChannelEndpoint.ssConnectString.empty());
    sdv::TObjectPtr ptrServerEndpoint = sChannelEndpoint.pConnection;     // Automatic lifetime management
    EXPECT_TRUE(ptrServerEndpoint);

    // Create a client endpoint
    sdv::ipc::IChannelAccess* pChannelAccess = sdv::core::GetObject<sdv::ipc::IChannelAccess>("LocalChannelControl");
    ASSERT_NE(pChannelAccess, nullptr);
    sdv::TObjectPtr ptrClientEndpoint = pChannelAccess->Access(sChannelEndpoint.ssConnectString);
    EXPECT_TRUE(ptrClientEndpoint);

    // Assign the server and client endpoints
    CInterfaceTest test;
    EXPECT_TRUE(control.AssignServerEndpoint(ptrServerEndpoint, &test, 100, false) != 0u);
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.AssignClientEndpoint(ptrClientEndpoint, 1000, pObjectProxy) != 0u);
    EXPECT_NE(pObjectProxy, nullptr);
    ptrServerEndpoint.Clear();    // Lifetime taken over by communication control.
    ptrClientEndpoint.Clear();    // Lifetime taken over by communication control.

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

// Disabled the following test due to an unidentified crash/heap corruption occurring with MINGW on the build-server.
// Bug-report #610009 describes this issue: https://dev.azure.com/SW4ZF/AZP-074_DivDI_SofDCarResearch/_workitems/edit/610009
//#ifdef _WIN32
TEST(IPC_Communication_Test, DISABLED_AssignClientEndpoint_Reconnect)
//#else
//TEST(IPC_Communication_Test, AssignClientEndpoint_Reconnect)
//#endif
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize("");
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::configuring);

    // Load the shared memory components
    LoadIPCModules(control);

    // Create a server endpoint
    sdv::ipc::ICreateEndpoint* ptrCreateEndpoint = sdv::core::GetObject<sdv::ipc::ICreateEndpoint>("LocalChannelControl");
    ASSERT_NE(ptrCreateEndpoint, nullptr);
    sdv::ipc::SChannelEndpoint sChannelEndpoint = ptrCreateEndpoint->CreateEndpoint("");
    EXPECT_NE(sChannelEndpoint.pConnection, nullptr);
    EXPECT_FALSE(sChannelEndpoint.ssConnectString.empty());
    sdv::TObjectPtr ptrServerEndpoint = sChannelEndpoint.pConnection;     // Automatic lifetime management
    EXPECT_TRUE(ptrServerEndpoint);

    // Create a client endpoint
    sdv::ipc::IChannelAccess* pChannelAccess = sdv::core::GetObject<sdv::ipc::IChannelAccess>("LocalChannelControl");
    ASSERT_NE(pChannelAccess, nullptr);
    sdv::TObjectPtr ptrClientEndpoint = pChannelAccess->Access(sChannelEndpoint.ssConnectString);
    EXPECT_TRUE(ptrClientEndpoint);

    // Assign the server and client endpoints
    CInterfaceTest test;
    EXPECT_TRUE(control.AssignServerEndpoint(ptrServerEndpoint, &test, 100, true) != 0u);
    ptrServerEndpoint.Clear();    // Lifetime taken over by communication control.
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    sdv::com::TConnectionID tConnectionID = control.AssignClientEndpoint(ptrClientEndpoint, 1000, pObjectProxy);
    EXPECT_TRUE(tConnectionID != 0u);
    EXPECT_NE(pObjectProxy, nullptr);

    // Disconnect from the client
    sdv::ipc::IConnect* pClientConnect = ptrClientEndpoint.GetInterface<sdv::ipc::IConnect>();
    ASSERT_NE(pClientConnect, nullptr);
    EXPECT_EQ(pClientConnect->GetStatus(), sdv::ipc::EConnectStatus::connected);
    control.RemoveConnection(tConnectionID);
    EXPECT_EQ(pClientConnect->GetStatus(), sdv::ipc::EConnectStatus::disconnected);
    ptrClientEndpoint.Clear();    // Lifetime taken over by communication control.

    // Create another client endpoint
    ptrClientEndpoint = pChannelAccess->Access(sChannelEndpoint.ssConnectString);
    EXPECT_TRUE(ptrClientEndpoint);

    // Assign the new client endpoint
    pObjectProxy = nullptr;
    tConnectionID = control.AssignClientEndpoint(ptrClientEndpoint, 1000, pObjectProxy);
    EXPECT_TRUE(tConnectionID != 0u);
    EXPECT_NE(pObjectProxy, nullptr);
    ptrClientEndpoint.Clear();    // Lifetime taken over by communication control.

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test, AssignClientEndpoint_FailReconnect)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize("");
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::configuring);

    // Load the shared memory components
    LoadIPCModules(control);

    // Create a server endpoint
    sdv::ipc::ICreateEndpoint* ptrCreateEndpoint = sdv::core::GetObject<sdv::ipc::ICreateEndpoint>("LocalChannelControl");
    ASSERT_NE(ptrCreateEndpoint, nullptr);
    sdv::ipc::SChannelEndpoint sChannelEndpoint = ptrCreateEndpoint->CreateEndpoint("");
    EXPECT_NE(sChannelEndpoint.pConnection, nullptr);
    EXPECT_FALSE(sChannelEndpoint.ssConnectString.empty());
    sdv::TObjectPtr ptrServerEndpoint = sChannelEndpoint.pConnection;     // Automatic lifetime management
    EXPECT_TRUE(ptrServerEndpoint);

    // Create a client endpoint
    sdv::ipc::IChannelAccess* pChannelAccess = sdv::core::GetObject<sdv::ipc::IChannelAccess>("LocalChannelControl");
    ASSERT_NE(pChannelAccess, nullptr);
    sdv::TObjectPtr ptrClientEndpoint = pChannelAccess->Access(sChannelEndpoint.ssConnectString);
    EXPECT_TRUE(ptrClientEndpoint);

    // Assign the server and client endpoints
    CInterfaceTest test;
    EXPECT_TRUE(control.AssignServerEndpoint(ptrServerEndpoint, &test, 100, false) != 0u);
    ptrServerEndpoint.Clear();    // Lifetime taken over by communication control.
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    sdv::com::TConnectionID tConnectionID = control.AssignClientEndpoint(ptrClientEndpoint, 1000, pObjectProxy);
    EXPECT_TRUE(tConnectionID != 0u);
    EXPECT_NE(pObjectProxy, nullptr);

    // Disconnect from the client
    sdv::ipc::IConnect* pClientConnect = ptrClientEndpoint.GetInterface<sdv::ipc::IConnect>();
    ASSERT_NE(pClientConnect, nullptr);
    EXPECT_EQ(pClientConnect->GetStatus(), sdv::ipc::EConnectStatus::connected);
    control.RemoveConnection(tConnectionID);
    EXPECT_EQ(pClientConnect->GetStatus(), sdv::ipc::EConnectStatus::disconnected);
    ptrClientEndpoint.Clear();    // Lifetime taken over by communication control.

    // Create another client endpoint
    ptrClientEndpoint = pChannelAccess->Access(sChannelEndpoint.ssConnectString);
    EXPECT_TRUE(ptrClientEndpoint);

    // Assign the new client endpoint - this should fail!
    pObjectProxy = nullptr;
    tConnectionID = control.AssignClientEndpoint(ptrClientEndpoint, 1000, pObjectProxy);
    EXPECT_TRUE(tConnectionID == 0u);
    EXPECT_EQ(pObjectProxy, nullptr);
    ptrClientEndpoint.Clear();    // Lifetime taken over by communication control.

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test, CreateServerConnection)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize("");
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::configuring);

    // Load the shared memory components
    LoadIPCModules(control);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test, CreateClientConnection)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize("");
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::configuring);

    // Load the shared memory components
    LoadIPCModules(control);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    EXPECT_NE(pObjectProxy, nullptr);

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test, CreateClientConnection_FailReconnect)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize("");
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::configuring);

    // Load the shared memory components
    LoadIPCModules(control);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    sdv::com::TConnectionID tConnectionID = control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy);
    EXPECT_TRUE(tConnectionID != 0u);
    EXPECT_NE(pObjectProxy, nullptr);

    // Disconnect from the client
    control.RemoveConnection(tConnectionID);

    // Create a new client connection
    pObjectProxy = nullptr;
    tConnectionID = control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy);
    EXPECT_TRUE(tConnectionID == 0u);
    EXPECT_EQ(pObjectProxy, nullptr);

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test, MarshallClientToServer)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize("");
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::configuring);

    // Load the shared memory components
    LoadIPCModules(control);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Request the ISayHello interface
    ISayHello* pSayHello = nullptr;
    EXPECT_NO_THROW(pSayHello = pObjectProxy->GetInterface<ISayHello>());
    ASSERT_NE(pSayHello, nullptr);

    EXPECT_EQ(pSayHello->Hello(), "Hello");

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test, MarshallInterfaceFromServer)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize("");
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::configuring);

    // Load the shared memory components
    LoadIPCModules(control);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Request the IRequestHello interface
    IRequestHello* pRequestHello = nullptr;
    EXPECT_NO_THROW(pRequestHello = pObjectProxy->GetInterface<IRequestHello>());
    ASSERT_NE(pRequestHello, nullptr);

    // Request the ISayHello interface
    ISayHello* pSayHello = pRequestHello->Request();

    // Call the interface as if talking from the client
    ASSERT_NE(pSayHello, nullptr);
    EXPECT_EQ(pSayHello->Hello(), "Hello");

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test, MarshallInterfaceFromClient)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize("");
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::configuring);

    // Load the shared memory components
    LoadIPCModules(control);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Request the IRegisterHelloCallback interface
    IRegisterHelloCallback* pRegisterHelloCallback = nullptr;
    EXPECT_NO_THROW(pRegisterHelloCallback = pObjectProxy->GetInterface<IRegisterHelloCallback>());
    ASSERT_NE(pRegisterHelloCallback, nullptr);

    // Register the callback (stored in the object)
    pRegisterHelloCallback->Register(&test);

    // Call the interface as if talking from the server
    ASSERT_NE(test.m_pHello, nullptr);
    EXPECT_EQ(test.m_pHello->Hello(), "Hello");

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test, MarshallGenericInterfaceFromServer)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize("");
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::configuring);

    // Load the shared memory components
    LoadIPCModules(control);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Request the IRegisterHelloCallback interface
    sdv::IInterfaceAccess* pRegisterHelloCallback = nullptr;
    EXPECT_NO_THROW(pRegisterHelloCallback = pObjectProxy->GetInterface<sdv::IInterfaceAccess>());
    ASSERT_NE(pRegisterHelloCallback, nullptr);

    // Call GetInterface to get the IInterfaceAccess pointer once more (to be able to test marshalling).
    sdv::IInterfaceAccess* pInterfaceAccess = pObjectProxy->GetInterface<sdv::IInterfaceAccess>();
    ASSERT_NE(pInterfaceAccess, nullptr);

    // Request the pSayHello interface (this will be marshalled as well)
    ISayHello* pSayHello = pInterfaceAccess->GetInterface<ISayHello>();

    // Call the interface as if talking from the client
    ASSERT_NE(pSayHello, nullptr);
    EXPECT_EQ(pSayHello->Hello(), "Hello");

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test, MarshallMegaStructBoolean)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize("");
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::configuring);

    // Load the shared memory components
    LoadIPCModules(control);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaStruct sStruct{};
    sStruct.bVal = false;
    EXPECT_FALSE(sStruct.bVal);
    EXPECT_NO_THROW(pMegaTest->ProcessMegaStruct(sStruct));
    EXPECT_TRUE(sStruct.bVal);

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test, MarshallMegaStructIntegral)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize("");
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::configuring);

    // Load the shared memory components
    LoadIPCModules(control);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaStruct sStruct{};
    sStruct.sVal = -10;
    sStruct.usVal = 10;
    sStruct.lVal = -20;
    sStruct.ulVal = 20;
    sStruct.llVal = -30;
    sStruct.ullVal = 30;
    sStruct.i8Val = -40;
    sStruct.ui8Val = 40;
    sStruct.i16Val = -50;
    sStruct.ui16Val = 50;
    sStruct.i32Val = -60;
    sStruct.ui32Val = 60;
    sStruct.i64Val = -70;
    sStruct.ui64Val = 70;
    sStruct.nVal = 80;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaStruct(sStruct));
    EXPECT_EQ(sStruct.sVal, -11);
    EXPECT_EQ(sStruct.usVal, 11u);
    EXPECT_EQ(sStruct.lVal, -21);
    EXPECT_EQ(sStruct.ulVal, 21u);
    EXPECT_EQ(sStruct.llVal, -31);
    EXPECT_EQ(sStruct.ullVal, 31u);
    EXPECT_EQ(static_cast<uint8_t>(sStruct.i8Val), static_cast<uint8_t>(-41));    // Cast needed for ARM compatibility
    EXPECT_EQ(sStruct.ui8Val, 41u);
    EXPECT_EQ(sStruct.i16Val, -51);
    EXPECT_EQ(sStruct.ui16Val, 51u);
    EXPECT_EQ(sStruct.i32Val, -61);
    EXPECT_EQ(sStruct.ui32Val, 61u);
    EXPECT_EQ(sStruct.i64Val, -71);
    EXPECT_EQ(sStruct.ui64Val, 71u);
    EXPECT_EQ(sStruct.nVal, 81);

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test, MarshallMegaStructCharacter)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize("");
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::configuring);

    // Load the shared memory components
    LoadIPCModules(control);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaStruct sStruct{};
    sStruct.cVal = 'A';
    sStruct.c16Val = u'D';
    sStruct.c32Val = U'G';
    sStruct.wcVal = L'J';
    EXPECT_NO_THROW(pMegaTest->ProcessMegaStruct(sStruct));
    EXPECT_EQ(sStruct.cVal, 'B');
    EXPECT_EQ(sStruct.c16Val, u'E');
    EXPECT_EQ(sStruct.c32Val, U'H');
    EXPECT_EQ(sStruct.wcVal, L'K');

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test, MarshallMegaStructFloatingPoint)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize("");
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::configuring);

    // Load the shared memory components
    LoadIPCModules(control);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaStruct sStruct{};
    float fVal = -1234.5678f;
    sStruct.fVal = fVal;
    double dVal = 8765.4321;
    sStruct.dVal = dVal;
    long double ldVal = -1234.4321;
    sStruct.ldVal = ldVal;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaStruct(sStruct));
    EXPECT_EQ(sStruct.fVal, fVal - 1000.000f);
    EXPECT_EQ(sStruct.dVal, dVal + 1000.000);
    EXPECT_EQ(sStruct.ldVal, ldVal - 1000.000);

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test, MarshallMegaStructEnum)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize("");
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::configuring);

    // Load the shared memory components
    LoadIPCModules(control);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaStruct sStruct{};
    sStruct.eHelloVal = EHello::hello;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaStruct(sStruct));
    EXPECT_EQ(sStruct.eHelloVal, EHello::hallo);

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test, MarshallMegaStructString)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize("");
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::configuring);

    // Load the shared memory components
    LoadIPCModules(control);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaStruct sStruct{};
    sStruct.ssVal = "hi";
    sStruct.ss8Val = "huhu";
    sStruct.ss16Val = u"hey";
    sStruct.ss32Val = U"hello";
    sStruct.wssVal = L"servus";
    sStruct.ssFixVal = "hi";
    sStruct.ss8FixVal = "huhu";
    sStruct.ss16FixVal = u"hey";
    sStruct.ss32FixVal = U"hello";
    sStruct.wssFixVal = L"servus";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaStruct(sStruct));
    EXPECT_EQ(sStruct.ssVal, "hij");
    EXPECT_EQ(sStruct.ss8Val, "huhuv");
    EXPECT_EQ(sStruct.ss16Val, u"heyz");
    EXPECT_EQ(sStruct.ss32Val, U"hellop");
    EXPECT_EQ(sStruct.wssVal, L"servust");
    EXPECT_EQ(sStruct.ssFixVal, "hij");
    EXPECT_EQ(sStruct.ss8FixVal, "huhuv");
    EXPECT_EQ(sStruct.ss16FixVal, u"heyz");
    EXPECT_EQ(sStruct.ss32FixVal, U"hellop");
    EXPECT_EQ(sStruct.wssFixVal, L"servust");

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test, MarshallMegaStructPointer)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize("");
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::configuring);

    // Load the shared memory components
    LoadIPCModules(control);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaStruct sStruct{};
    sStruct.ptr8Val.resize(100);
    for (size_t nIndex = 0; nIndex < sStruct.ptr8Val.size(); nIndex++)
        sStruct.ptr8Val[nIndex] = static_cast<uint8_t>(nIndex);
    sStruct.ptrssVal.resize(3);
    sStruct.ptrssVal[0] = "hello";
    sStruct.ptrssVal[1] = "huhu";
    sStruct.ptrssVal[2] = "hi";
    sStruct.ptr8FixVal.resize(sStruct.ptr8FixVal.capacity());
    for (size_t nIndex = 0; nIndex < sStruct.ptr8FixVal.size(); nIndex++)
        sStruct.ptr8FixVal[nIndex] = static_cast<uint8_t>(nIndex);
    sStruct.ptrssFixVal.resize(sStruct.ptrssFixVal.capacity());
    sStruct.ptrssFixVal[0] = "hi";
    sStruct.ptrssFixVal[1] = "hoi";
    sStruct.ptrssFixVal[2] = "hello";
    sStruct.ptrssFixVal[3] = "hallo";
    sStruct.ptrssFixVal[4] = "servus";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaStruct(sStruct));
    EXPECT_EQ(sStruct.ptr8Val.size(), 200);
    for (size_t nIndex = 0; nIndex < sStruct.ptr8Val.size() / 2; nIndex++)
        EXPECT_EQ(sStruct.ptr8Val[nIndex], sStruct.ptr8Val[nIndex + sStruct.ptr8Val.size() / 2]);
    EXPECT_EQ(sStruct.ptrssVal.size(), 6);
    for (size_t nIndex = 0; nIndex < sStruct.ptrssVal.size() / 2; nIndex++)
        EXPECT_EQ(sStruct.ptrssVal[nIndex], sStruct.ptrssVal[nIndex + sStruct.ptrssVal.size() / 2]);
    EXPECT_EQ(sStruct.ptr8FixVal.size(), 20);
    for (size_t nIndex = 0; nIndex < sStruct.ptr8FixVal.size(); nIndex++)
        EXPECT_EQ(sStruct.ptr8FixVal[nIndex], static_cast<uint8_t>(nIndex << 1));
    EXPECT_EQ(sStruct.ptrssFixVal.size(), 5);
    EXPECT_EQ(sStruct.ptrssFixVal[0], "hihi");
    EXPECT_EQ(sStruct.ptrssFixVal[1], "hoihoi");
    EXPECT_EQ(sStruct.ptrssFixVal[2], "hellohello");
    EXPECT_EQ(sStruct.ptrssFixVal[3], "hallohallo");
    EXPECT_EQ(sStruct.ptrssFixVal[4], "servusservus");

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test, MarshallMegaStructSequence)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize("");
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::configuring);

    // Load the shared memory components
    LoadIPCModules(control);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaStruct sStruct{};
    sStruct.seq8Val.resize(100);
    for (size_t nIndex = 0; nIndex < sStruct.seq8Val.size(); nIndex++)
        sStruct.seq8Val[nIndex] = static_cast<uint8_t>(nIndex);
    sStruct.seqssVal.resize(3);
    sStruct.seqssVal[0] = "hello";
    sStruct.seqssVal[1] = "huhu";
    sStruct.seqssVal[2] = "hi";
    sStruct.seq8FixVal.resize(sStruct.seq8FixVal.capacity());
    for (size_t nIndex = 0; nIndex < sStruct.seq8FixVal.size(); nIndex++)
        sStruct.seq8FixVal[nIndex] = static_cast<uint8_t>(nIndex);
    sStruct.seqssFixVal.resize(sStruct.seqssFixVal.capacity());
    sStruct.seqssFixVal[0] = "hi";
    sStruct.seqssFixVal[1] = "hoi";
    sStruct.seqssFixVal[2] = "hello";
    sStruct.seqssFixVal[3] = "hallo";
    sStruct.seqssFixVal[4] = "servus";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaStruct(sStruct));
    EXPECT_EQ(sStruct.seq8Val.size(), 200);
    for (size_t nIndex = 0; nIndex < sStruct.seq8Val.size() / 2; nIndex++)
        EXPECT_EQ(sStruct.seq8Val[nIndex], sStruct.seq8Val[nIndex + sStruct.seq8Val.size() / 2]);
    EXPECT_EQ(sStruct.seqssVal.size(), 6);
    for (size_t nIndex = 0; nIndex < sStruct.seqssVal.size() / 2; nIndex++)
        EXPECT_EQ(sStruct.seqssVal[nIndex], sStruct.seqssVal[nIndex + sStruct.seqssVal.size() / 2]);
    EXPECT_EQ(sStruct.seq8FixVal.size(), 20);
    for (size_t nIndex = 0; nIndex < sStruct.seq8FixVal.size(); nIndex++)
        EXPECT_EQ(sStruct.seq8FixVal[nIndex], static_cast<uint8_t>(nIndex << 1));
    EXPECT_EQ(sStruct.seqssFixVal.size(), 5);
    EXPECT_EQ(sStruct.seqssFixVal[0], "hihi");
    EXPECT_EQ(sStruct.seqssFixVal[1], "hoihoi");
    EXPECT_EQ(sStruct.seqssFixVal[2], "hellohello");
    EXPECT_EQ(sStruct.seqssFixVal[3], "hallohallo");
    EXPECT_EQ(sStruct.seqssFixVal[4], "servusservus");

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test, MarshallMegaStructInterface)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize("");
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::configuring);

    // Load the shared memory components
    LoadIPCModules(control);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    struct SMultiply : public IMultiplyValue
    {
        size_t Multiply(size_t n)
        {
            return n * n;
        }
    } sMultiply;

    ASSERT_NE(pMegaTest, nullptr);
    SMegaStruct sStruct{};

    sStruct.idVal = sdv::GetInterfaceId<IMultiplyValue>();
    sStruct.ifcVal = static_cast<IMultiplyValue*>(&sMultiply);
    sStruct.pMultiplyValue = &sMultiply;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaStruct(sStruct));
    EXPECT_EQ(sStruct.idVal, sdv::GetInterfaceId<IAddValue>());
    EXPECT_EQ(sStruct.ifcVal.id(), sdv::GetInterfaceId<IAddValue>());
    ASSERT_NE(sStruct.ifcVal, nullptr);
    ASSERT_NE(sStruct.ifcVal.get<IAddValue>(), nullptr);
    EXPECT_EQ(sStruct.ifcVal.get<IAddValue>()->Add(99), 19602); // 99 * 99 + 99 * 99
    ASSERT_NE(sStruct.pMultiplyValue, nullptr);
    EXPECT_EQ(sStruct.pMultiplyValue->Multiply(5), 625); // 5 * 5 * 5 * 5

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test, MarshallMegaStructSubCompound)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize("");
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::configuring);

    // Load the shared memory components
    LoadIPCModules(control);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaStruct sStruct{};
    sStruct.sSubVal.i = 1;
    sStruct.sIndVal.i = 10;
    //sStruct.sUnnamedVal.i = 100;  // 12.04.2024 EVE Unnamed structures are not supported by IDL compiler.
    EXPECT_NO_THROW(pMegaTest->ProcessMegaStruct(sStruct));
    EXPECT_EQ(sStruct.sSubVal.i, 2);
    EXPECT_EQ(sStruct.sIndVal.i, 20);
    //EXPECT_EQ(sStruct.sUnnamedVal.i, 120);  // 12.04.2024 EVE Unnamed structures are not supported by IDL compiler.

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test, MarshallMegaStructBooleanArray)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize("");
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::configuring);

    // Load the shared memory components
    LoadIPCModules(control);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaStruct sStruct{};
    sStruct.rgbVal[0] = false;
    sStruct.rgbVal[1] = true;
    EXPECT_FALSE(sStruct.bVal);
    EXPECT_NO_THROW(pMegaTest->ProcessMegaStruct(sStruct));
    EXPECT_TRUE(sStruct.rgbVal[0]);
    EXPECT_FALSE(sStruct.rgbVal[1]);

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test, MarshallMegaStructIntegralArray)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize("");
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::configuring);

    // Load the shared memory components
    LoadIPCModules(control);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaStruct sStruct{};
    sStruct.rgsVal[0] = -10;
    sStruct.rgsVal[1] = -11;
    sStruct.rgusVal[0] = 10;
    sStruct.rgusVal[1] = 11;
    sStruct.rglVal[0] = -20;
    sStruct.rglVal[1] = -21;
    sStruct.rgulVal[0] = 20;
    sStruct.rgulVal[1] = 21;
    sStruct.rgllVal[0] = -30;
    sStruct.rgllVal[1] = -31;
    sStruct.rgullVal[0] = 30;
    sStruct.rgullVal[1] = 31;
    sStruct.rgi8Val[0] = -40;
    sStruct.rgi8Val[1] = -41;
    sStruct.rgui8Val[0] = 40;
    sStruct.rgui8Val[1] = 41;
    sStruct.rgi16Val[0] = -50;
    sStruct.rgi16Val[1] = -51;
    sStruct.rgui16Val[0] = 50;
    sStruct.rgui16Val[1] = 51;
    sStruct.rgi32Val[0] = -60;
    sStruct.rgi32Val[1] = -61;
    sStruct.rgui32Val[0] = 60;
    sStruct.rgui32Val[1] = 61;
    sStruct.rgi64Val[0] = -70;
    sStruct.rgi64Val[1] = -71;
    sStruct.rgui64Val[0] = 70;
    sStruct.rgui64Val[1] = 71;
    sStruct.rgnVal[0] = 80;
    sStruct.rgnVal[1] = 81;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaStruct(sStruct));
    EXPECT_EQ(sStruct.rgsVal[0], -11);
    EXPECT_EQ(sStruct.rgsVal[1], -10);
    EXPECT_EQ(sStruct.rgusVal[0], 11u);
    EXPECT_EQ(sStruct.rgusVal[1], 13u);
    EXPECT_EQ(sStruct.rglVal[0], -21);
    EXPECT_EQ(sStruct.rglVal[1], -20);
    EXPECT_EQ(sStruct.rgulVal[0], 21u);
    EXPECT_EQ(sStruct.rgulVal[1], 23u);
    EXPECT_EQ(sStruct.rgllVal[0], -31);
    EXPECT_EQ(sStruct.rgllVal[1], -30);
    EXPECT_EQ(sStruct.rgullVal[0], 31u);
    EXPECT_EQ(sStruct.rgullVal[1], 33u);
    EXPECT_EQ(static_cast<uint8_t>(sStruct.rgi8Val[0]), static_cast<uint8_t>(-41));    // Cast needed for ARM compatibility
    EXPECT_EQ(static_cast<uint8_t>(sStruct.rgi8Val[1]), static_cast<uint8_t>(-40));    // Cast needed for ARM compatibility
    EXPECT_EQ(sStruct.rgui8Val[0], 41u);
    EXPECT_EQ(sStruct.rgui8Val[1], 43u);
    EXPECT_EQ(sStruct.rgi16Val[0], -51);
    EXPECT_EQ(sStruct.rgi16Val[1], -50);
    EXPECT_EQ(sStruct.rgui16Val[0], 51u);
    EXPECT_EQ(sStruct.rgui16Val[1], 53u);
    EXPECT_EQ(sStruct.rgi32Val[0], -61);
    EXPECT_EQ(sStruct.rgi32Val[1], -60);
    EXPECT_EQ(sStruct.rgui32Val[0], 61u);
    EXPECT_EQ(sStruct.rgui32Val[1], 63u);
    EXPECT_EQ(sStruct.rgi64Val[0], -71);
    EXPECT_EQ(sStruct.rgi64Val[1], -70);
    EXPECT_EQ(sStruct.rgui64Val[0], 71u);
    EXPECT_EQ(sStruct.rgui64Val[1], 73u);
    EXPECT_EQ(sStruct.rgnVal[0], 81);
    EXPECT_EQ(sStruct.rgnVal[1], 83);

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test, MarshallMegaStructCharacterArray)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize("");
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::configuring);

    // Load the shared memory components
    LoadIPCModules(control);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaStruct sStruct{};
    sStruct.rgcVal[0] = 'A';
    sStruct.rgcVal[1] = 'B';
    sStruct.rgc16Val[0] = u'D';
    sStruct.rgc16Val[1] = u'E';
    sStruct.rgc32Val[0] = U'G';
    sStruct.rgc32Val[1] = U'H';
    sStruct.rgwcVal[0] = L'J';
    sStruct.rgwcVal[1] = L'K';
    EXPECT_NO_THROW(pMegaTest->ProcessMegaStruct(sStruct));
    EXPECT_EQ(sStruct.rgcVal[0], 'B');
    EXPECT_EQ(sStruct.rgcVal[1], 'D');
    EXPECT_EQ(sStruct.rgc16Val[0], u'E');
    EXPECT_EQ(sStruct.rgc16Val[1], u'G');
    EXPECT_EQ(sStruct.rgc32Val[0], U'H');
    EXPECT_EQ(sStruct.rgc32Val[1], U'J');
    EXPECT_EQ(sStruct.rgwcVal[0], L'K');
    EXPECT_EQ(sStruct.rgwcVal[1], L'M');

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test, MarshallMegaStructFloatingPointArray)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize("");
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::configuring);

    // Load the shared memory components
    LoadIPCModules(control);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaStruct sStruct{};
    float fVal = -1234.5678f;
    sStruct.rgfVal[0] = fVal;
    sStruct.rgfVal[1] = fVal;
    double dVal = 8765.4321;
    sStruct.rgdVal[0] = dVal;
    sStruct.rgdVal[1] = dVal;
    long double ldVal = -1234.4321;
    sStruct.rgldVal[0] = ldVal;
    sStruct.rgldVal[1] = ldVal;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaStruct(sStruct));
    EXPECT_EQ(sStruct.rgfVal[0], fVal - 1000.000f);
    EXPECT_EQ(sStruct.rgfVal[1], fVal + 2000.000f);
    EXPECT_EQ(sStruct.rgdVal[0], dVal + 1000.000);
    EXPECT_EQ(sStruct.rgdVal[1], dVal - 2000.000);
    EXPECT_EQ(sStruct.rgldVal[0], ldVal - 1000.000);
    EXPECT_EQ(sStruct.rgldVal[1], ldVal + 2000.000);

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test, MarshallMegaStructEnumArray)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize("");
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::configuring);

    // Load the shared memory components
    LoadIPCModules(control);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaStruct sStruct{};
    sStruct.rgeHelloVal[0] = EHello::hello;
    sStruct.rgeHelloVal[1] = EHello::hello;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaStruct(sStruct));
    EXPECT_EQ(sStruct.rgeHelloVal[0], EHello::hallo);
    EXPECT_EQ(sStruct.rgeHelloVal[1], EHello::huhu);

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test, MarshallMegaStructStringArray)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize("");
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::configuring);

    // Load the shared memory components
    LoadIPCModules(control);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaStruct sStruct{};
    sStruct.rgssVal[0] = "hi";
    sStruct.rgssVal[1] = "hi";
    sStruct.rgss8Val[0] = "huhu";
    sStruct.rgss8Val[1] = "huhu";
    sStruct.rgss16Val[0] = u"hey";
    sStruct.rgss16Val[1] = u"hey";
    sStruct.rgss32Val[0] = U"hello";
    sStruct.rgss32Val[1] = U"hello";
    sStruct.rgwssVal[0] = L"servus";
    sStruct.rgwssVal[1] = L"servus";
    sStruct.rgssFixVal[0] = "hi";
    sStruct.rgssFixVal[1] = "hi";
    sStruct.rgss8FixVal[0] = "huhu";
    sStruct.rgss8FixVal[1] = "huhu";
    sStruct.rgss16FixVal[0] = u"hey";
    sStruct.rgss16FixVal[1] = u"hey";
    sStruct.rgss32FixVal[0] = U"hello";
    sStruct.rgss32FixVal[1] = U"hello";
    sStruct.rgwssFixVal[0] = L"servus";
    sStruct.rgwssFixVal[1] = L"servus";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaStruct(sStruct));
    EXPECT_EQ(sStruct.rgssVal[0], "hij");
    EXPECT_EQ(sStruct.rgssVal[1], "hii");
    EXPECT_EQ(sStruct.rgss8Val[0], "huhuv");
    EXPECT_EQ(sStruct.rgss8Val[1], "huhui");
    EXPECT_EQ(sStruct.rgss16Val[0], u"heyz");
    EXPECT_EQ(sStruct.rgss16Val[1], u"heyi");
    EXPECT_EQ(sStruct.rgss32Val[0], U"hellop");
    EXPECT_EQ(sStruct.rgss32Val[1], U"helloi");
    EXPECT_EQ(sStruct.rgwssVal[0], L"servust");
    EXPECT_EQ(sStruct.rgwssVal[1], L"servust");
    EXPECT_EQ(sStruct.rgssFixVal[0], "hij");
    EXPECT_EQ(sStruct.rgssFixVal[1], "hii");
    EXPECT_EQ(sStruct.rgss8FixVal[0], "huhuv");
    EXPECT_EQ(sStruct.rgss8FixVal[1], "huhui");
    EXPECT_EQ(sStruct.rgss16FixVal[0], u"heyz");
    EXPECT_EQ(sStruct.rgss16FixVal[1], u"heyi");
    EXPECT_EQ(sStruct.rgss32FixVal[0], U"hellop");
    EXPECT_EQ(sStruct.rgss32FixVal[1], U"helloi");
    EXPECT_EQ(sStruct.rgwssFixVal[0], L"servust");
    EXPECT_EQ(sStruct.rgwssFixVal[1], L"servust");

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test, MarshallMegaStructPointerArray)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize("");
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::configuring);

    // Load the shared memory components
    LoadIPCModules(control);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaStruct sStruct{};
    sStruct.rgptr8Val[0].resize(100);
    sStruct.rgptr8Val[1].resize(100);
    for (size_t nIndex = 0; nIndex < sStruct.rgptr8Val[0].size(); nIndex++)
        sStruct.rgptr8Val[0][nIndex] = static_cast<uint8_t>(nIndex);
    for (size_t nIndex = 0; nIndex < sStruct.rgptr8Val[1].size(); nIndex++)
        sStruct.rgptr8Val[1][nIndex] = static_cast<uint8_t>(nIndex);
    sStruct.rgptrssVal[0].resize(3);
    sStruct.rgptrssVal[1].resize(3);
    sStruct.rgptrssVal[0][0] = "hello";
    sStruct.rgptrssVal[1][0] = "ohhello";
    sStruct.rgptrssVal[0][1] = "huhu";
    sStruct.rgptrssVal[1][1] = "ohhuhu";
    sStruct.rgptrssVal[0][2] = "hi";
    sStruct.rgptrssVal[1][2] = "ohhi";
    sStruct.rgptr8FixVal[0].resize(sStruct.rgptr8FixVal[0].capacity());
    sStruct.rgptr8FixVal[1].resize(sStruct.rgptr8FixVal[1].capacity());
    for (size_t nIndex = 0; nIndex < sStruct.rgptr8FixVal[0].size(); nIndex++)
        sStruct.rgptr8FixVal[0][nIndex] = static_cast<uint8_t>(nIndex);
    for (size_t nIndex = 0; nIndex < sStruct.rgptr8FixVal[1].size(); nIndex++)
        sStruct.rgptr8FixVal[1][nIndex] = static_cast<uint8_t>(nIndex);
    sStruct.rgptrssFixVal[0].resize(sStruct.rgptrssFixVal[0].capacity());
    sStruct.rgptrssFixVal[1].resize(sStruct.rgptrssFixVal[1].capacity());
    sStruct.rgptrssFixVal[0][0] = "hi";
    sStruct.rgptrssFixVal[1][0] = "ohhi";
    sStruct.rgptrssFixVal[0][1] = "hoi";
    sStruct.rgptrssFixVal[1][1] = "ohhoi";
    sStruct.rgptrssFixVal[0][2] = "hello";
    sStruct.rgptrssFixVal[1][2] = "ohhello";
    sStruct.rgptrssFixVal[0][3] = "hallo";
    sStruct.rgptrssFixVal[1][3] = "ohhallo";
    sStruct.rgptrssFixVal[0][4] = "servus";
    sStruct.rgptrssFixVal[1][4] = "ohservus";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaStruct(sStruct));
    EXPECT_EQ(sStruct.rgptr8Val[0].size(), 200);
    EXPECT_EQ(sStruct.rgptr8Val[1].size(), 300);
    for (size_t nIndex = 0; nIndex < sStruct.rgptr8Val[0].size() / 2; nIndex++)
        EXPECT_EQ(sStruct.rgptr8Val[0][nIndex], sStruct.rgptr8Val[0][nIndex + sStruct.rgptr8Val[0].size() / 2]);
    for (size_t nIndex = 0; nIndex < sStruct.rgptr8Val[1].size() / 3; nIndex++)
        EXPECT_EQ(sStruct.rgptr8Val[1][nIndex], sStruct.rgptr8Val[1][nIndex + sStruct.rgptr8Val[1].size() / 3]);
    EXPECT_EQ(sStruct.rgptrssVal[0].size(), 6);
    EXPECT_EQ(sStruct.rgptrssVal[1].size(), 9);
    for (size_t nIndex = 0; nIndex < sStruct.rgptrssVal[0].size() / 2; nIndex++)
        EXPECT_EQ(sStruct.rgptrssVal[0][nIndex], sStruct.rgptrssVal[0][nIndex + sStruct.rgptrssVal[0].size() / 2]);
    for (size_t nIndex = 0; nIndex < sStruct.rgptrssVal[1].size() / 3; nIndex++)
        EXPECT_EQ(sStruct.rgptrssVal[1][nIndex], sStruct.rgptrssVal[1][nIndex + sStruct.rgptrssVal[1].size() / 3]);
    EXPECT_EQ(sStruct.rgptr8FixVal[0].size(), 20);
    EXPECT_EQ(sStruct.rgptr8FixVal[1].size(), 20);
    for (size_t nIndex = 0; nIndex < sStruct.rgptr8FixVal[0].size(); nIndex++)
        EXPECT_EQ(sStruct.rgptr8FixVal[0][nIndex], static_cast<uint8_t>(nIndex << 1));
    for (size_t nIndex = 0; nIndex < sStruct.rgptr8FixVal[1].size(); nIndex++)
        EXPECT_EQ(sStruct.rgptr8FixVal[1][nIndex], static_cast<uint8_t>(nIndex << 1));
    EXPECT_EQ(sStruct.rgptrssFixVal[0].size(), 5);
    EXPECT_EQ(sStruct.rgptrssFixVal[1].size(), 5);
    EXPECT_EQ(sStruct.rgptrssFixVal[0][0], "hihi");
    EXPECT_EQ(sStruct.rgptrssFixVal[1][0], "ohhiohhi");
    EXPECT_EQ(sStruct.rgptrssFixVal[0][1], "hoihoi");
    EXPECT_EQ(sStruct.rgptrssFixVal[1][1], "ohhoiohhoi");
    EXPECT_EQ(sStruct.rgptrssFixVal[0][2], "hellohello");
    EXPECT_EQ(sStruct.rgptrssFixVal[1][2], "ohhelloohhello");
    EXPECT_EQ(sStruct.rgptrssFixVal[0][3], "hallohallo");
    EXPECT_EQ(sStruct.rgptrssFixVal[1][3], "ohhalloohhallo");
    EXPECT_EQ(sStruct.rgptrssFixVal[0][4], "servusservus");
    EXPECT_EQ(sStruct.rgptrssFixVal[1][4], "ohservusohservus");

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test, MarshallMegaStructSequenceArray)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize("");
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::configuring);

    // Load the shared memory components
    LoadIPCModules(control);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaStruct sStruct{};
    sStruct.rgseq8Val[0].resize(100);
    sStruct.rgseq8Val[1].resize(100);
    for (size_t nIndex = 0; nIndex < sStruct.rgseq8Val[0].size(); nIndex++)
        sStruct.rgseq8Val[0][nIndex] = static_cast<uint8_t>(nIndex);
    for (size_t nIndex = 0; nIndex < sStruct.rgseq8Val[1].size(); nIndex++)
        sStruct.rgseq8Val[1][nIndex] = static_cast<uint8_t>(nIndex);
    sStruct.rgseqssVal[0].resize(3);
    sStruct.rgseqssVal[1].resize(3);
    sStruct.rgseqssVal[0][0] = "hello";
    sStruct.rgseqssVal[1][0] = "ohhello";
    sStruct.rgseqssVal[0][1] = "huhu";
    sStruct.rgseqssVal[1][1] = "ohhuhu";
    sStruct.rgseqssVal[0][2] = "hi";
    sStruct.rgseqssVal[1][2] = "ohhi";
    sStruct.rgseq8FixVal[0].resize(sStruct.rgseq8FixVal[0].capacity());
    sStruct.rgseq8FixVal[1].resize(sStruct.rgseq8FixVal[1].capacity());
    for (size_t nIndex = 0; nIndex < sStruct.rgseq8FixVal[0].size(); nIndex++)
        sStruct.rgseq8FixVal[0][nIndex] = static_cast<uint8_t>(nIndex);
    for (size_t nIndex = 0; nIndex < sStruct.rgseq8FixVal[1].size(); nIndex++)
        sStruct.rgseq8FixVal[1][nIndex] = static_cast<uint8_t>(nIndex);
    sStruct.rgseqssFixVal[0].resize(sStruct.seqssFixVal.capacity());
    sStruct.rgseqssFixVal[1].resize(sStruct.seqssFixVal.capacity());
    sStruct.rgseqssFixVal[0][0] = "hi";
    sStruct.rgseqssFixVal[1][0] = "ohhi";
    sStruct.rgseqssFixVal[0][1] = "hoi";
    sStruct.rgseqssFixVal[1][1] = "ohhoi";
    sStruct.rgseqssFixVal[0][2] = "hello";
    sStruct.rgseqssFixVal[1][2] = "ohhello";
    sStruct.rgseqssFixVal[0][3] = "hallo";
    sStruct.rgseqssFixVal[1][3] = "ohhallo";
    sStruct.rgseqssFixVal[0][4] = "servus";
    sStruct.rgseqssFixVal[1][4] = "ohservus";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaStruct(sStruct));
    EXPECT_EQ(sStruct.rgseq8Val[0].size(), 200);
    EXPECT_EQ(sStruct.rgseq8Val[1].size(), 300);
    for (size_t nIndex = 0; nIndex < sStruct.rgseq8Val[0].size() / 2; nIndex++)
        EXPECT_EQ(sStruct.rgseq8Val[0][nIndex], sStruct.rgseq8Val[0][nIndex + sStruct.rgseq8Val[0].size() / 2]);
    for (size_t nIndex = 0; nIndex < sStruct.rgseq8Val[1].size() / 3; nIndex++)
        EXPECT_EQ(sStruct.rgseq8Val[1][nIndex], sStruct.rgseq8Val[1][nIndex + sStruct.rgseq8Val[1].size() / 3]);
    EXPECT_EQ(sStruct.rgseqssVal[0].size(), 6);
    EXPECT_EQ(sStruct.rgseqssVal[1].size(), 9);
    for (size_t nIndex = 0; nIndex < sStruct.rgseqssVal[0].size() / 2; nIndex++)
        EXPECT_EQ(sStruct.rgseqssVal[0][nIndex], sStruct.rgseqssVal[0][nIndex + sStruct.rgseqssVal[0].size() / 2]);
    for (size_t nIndex = 0; nIndex < sStruct.rgseqssVal[1].size() / 3; nIndex++)
        EXPECT_EQ(sStruct.rgseqssVal[1][nIndex], sStruct.rgseqssVal[1][nIndex + sStruct.rgseqssVal[1].size() / 3]);
    EXPECT_EQ(sStruct.rgseq8FixVal[0].size(), 20);
    EXPECT_EQ(sStruct.rgseq8FixVal[1].size(), 20);
    for (size_t nIndex = 0; nIndex < sStruct.rgseq8FixVal[0].size(); nIndex++)
        EXPECT_EQ(sStruct.rgseq8FixVal[0][nIndex], static_cast<uint8_t>(nIndex << 1));
    for (size_t nIndex = 0; nIndex < sStruct.rgseq8FixVal[1].size(); nIndex++)
        EXPECT_EQ(sStruct.rgseq8FixVal[1][nIndex], static_cast<uint8_t>(nIndex << 1));
    EXPECT_EQ(sStruct.rgseqssFixVal[0].size(), 5);
    EXPECT_EQ(sStruct.rgseqssFixVal[1].size(), 5);
    EXPECT_EQ(sStruct.rgseqssFixVal[0][0], "hihi");
    EXPECT_EQ(sStruct.rgseqssFixVal[1][0], "ohhiohhi");
    EXPECT_EQ(sStruct.rgseqssFixVal[0][1], "hoihoi");
    EXPECT_EQ(sStruct.rgseqssFixVal[1][1], "ohhoiohhoi");
    EXPECT_EQ(sStruct.rgseqssFixVal[0][2], "hellohello");
    EXPECT_EQ(sStruct.rgseqssFixVal[1][2], "ohhelloohhello");
    EXPECT_EQ(sStruct.rgseqssFixVal[0][3], "hallohallo");
    EXPECT_EQ(sStruct.rgseqssFixVal[1][3], "ohhalloohhallo");
    EXPECT_EQ(sStruct.rgseqssFixVal[0][4], "servusservus");
    EXPECT_EQ(sStruct.rgseqssFixVal[1][4], "ohservusohservus");

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test, MarshallMegaStructInterfaceArray)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize("");
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::configuring);

    // Load the shared memory components
    LoadIPCModules(control);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    struct SMultiply : public IMultiplyValue
    {
        size_t Multiply(size_t n)
        {
            return n * n;
        }
    } sMultiply;

    ASSERT_NE(pMegaTest, nullptr);
    SMegaStruct sStruct{};

    //sStruct.rgidVal[0] = sdv::GetInterfaceId<IMultiplyValue>();
    //sStruct.rgidVal[1] = sdv::GetInterfaceId<IMultiplyValue>();
    //sStruct.rgifcVal[0] = static_cast<IMultiplyValue*>(&sMultiply);
    //sStruct.rgifcVal[1] = static_cast<IMultiplyValue*>(&sMultiply);
    //sStruct.rgpMultiplyValue[0] = &sMultiply;
    //sStruct.rgpMultiplyValue[1] = &sMultiply;
    //EXPECT_NO_THROW(pMegaTest->ProcessMegaStruct(sStruct));
    //EXPECT_EQ(sStruct.rgidVal[0], sdv::GetInterfaceId<IAddValue>());
    //EXPECT_EQ(sStruct.rgidVal[1], sdv::GetInterfaceId<IAddValue>());
    //EXPECT_EQ(sStruct.rgifcVal[0].id(), sdv::GetInterfaceId<IAddValue>());
    //EXPECT_EQ(sStruct.rgifcVal[1].id(), sdv::GetInterfaceId<IAddValue>());
    //ASSERT_NE(sStruct.rgifcVal[0], nullptr);
    //ASSERT_NE(sStruct.rgifcVal[1], nullptr);
    //ASSERT_NE(sStruct.rgifcVal[0].get<IAddValue>(), nullptr);
    //ASSERT_NE(sStruct.rgifcVal[1].get<IAddValue>(), nullptr);
    //EXPECT_EQ(sStruct.rgifcVal[0].get<IAddValue>()->Add(99), 19602); // 99 * 99 + 99 * 99
    //EXPECT_EQ(sStruct.rgifcVal[1].get<IAddValue>()->Add(99), 19602); // 99 * 99 + 99 * 99
    //ASSERT_NE(sStruct.rgpMultiplyValue[0], nullptr);
    //ASSERT_NE(sStruct.rgpMultiplyValue[1], nullptr);
    //EXPECT_EQ(sStruct.rgpMultiplyValue[0]->Multiply(5), 625); // 5 * 5 * 5 * 5
    //EXPECT_EQ(sStruct.rgpMultiplyValue[1]->Multiply(5), 625); // 5 * 5 * 5 * 5

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test, MarshallMegaStructSubCompoundArray)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize("");
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::configuring);

    // Load the shared memory components
    LoadIPCModules(control);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaStruct sStruct{};
    sStruct.rgsSubVal[0].i = 1;
    sStruct.rgsSubVal[1].i = 2;
    // Incompatible serdes generation: BUG #398509
    //sStruct.rgsIndVal[0].i = 10;
    //sStruct.rgsIndVal[1].i = 20;
    // Unnamed struct with array is not yet supported due to incorret serdes code generation: BUG #398246
    //sStruct.rgsUnnamedVal[0].i = 100;
    //sStruct.rgsUnnamedVal[1].i = 200;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaStruct(sStruct));
    EXPECT_EQ(sStruct.rgsSubVal[0].i, 2);
    EXPECT_EQ(sStruct.rgsSubVal[1].i, 4);
    //EXPECT_EQ(sStruct.rgsIndVal[0].i, 20);
    //EXPECT_EQ(sStruct.rgsIndVal[1].i, 40);
    //EXPECT_EQ(sStruct.rgsUnnamedVal[0].i, 120);
    //EXPECT_EQ(sStruct.rgsUnnamedVal[1].i, 240);

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test, MarshallMegaStructBooleanTypedef)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize("");
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::configuring);

    // Load the shared memory components
    LoadIPCModules(control);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaStruct sStruct{};
    sStruct.tbVal = false;
    EXPECT_FALSE(sStruct.tbVal);
    EXPECT_NO_THROW(pMegaTest->ProcessMegaStruct(sStruct));
    EXPECT_TRUE(sStruct.tbVal);

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test, MarshallMegaStructIntegralTypedef)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize("");
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::configuring);

    // Load the shared memory components
    LoadIPCModules(control);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaStruct sStruct{};
    sStruct.tsVal = -10;
    sStruct.tusVal = 10;
    sStruct.tlVal = -20;
    sStruct.tulVal = 20;
    sStruct.tllVal = -30;
    sStruct.tullVal = 30;
    sStruct.ti8Val = -40;
    sStruct.tui8Val = 40;
    sStruct.ti16Val = -50;
    sStruct.tui16Val = 50;
    sStruct.ti32Val = -60;
    sStruct.tui32Val = 60;
    sStruct.ti64Val = -70;
    sStruct.tui64Val = 70;
    sStruct.tnVal = 80;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaStruct(sStruct));
    EXPECT_EQ(sStruct.tsVal, -11);
    EXPECT_EQ(sStruct.tusVal, 11u);
    EXPECT_EQ(sStruct.tlVal, -21);
    EXPECT_EQ(sStruct.tulVal, 21u);
    EXPECT_EQ(sStruct.tllVal, -31);
    EXPECT_EQ(sStruct.tullVal, 31u);
    EXPECT_EQ(static_cast<uint8_t>(sStruct.ti8Val), static_cast<uint8_t>(-41));    // Cast needed for ARM compatibility
    EXPECT_EQ(sStruct.tui8Val, 41u);
    EXPECT_EQ(sStruct.ti16Val, -51);
    EXPECT_EQ(sStruct.tui16Val, 51u);
    EXPECT_EQ(sStruct.ti32Val, -61);
    EXPECT_EQ(sStruct.tui32Val, 61u);
    EXPECT_EQ(sStruct.ti64Val, -71);
    EXPECT_EQ(sStruct.tui64Val, 71u);
    EXPECT_EQ(sStruct.tnVal, 81);

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test, MarshallMegaStructCharacterTypedef)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize("");
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::configuring);

    // Load the shared memory components
    LoadIPCModules(control);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaStruct sStruct{};
    sStruct.tcVal = 'A';
    sStruct.tc16Val = u'D';
    sStruct.tc32Val = U'G';
    sStruct.twcVal = L'J';
    EXPECT_NO_THROW(pMegaTest->ProcessMegaStruct(sStruct));
    EXPECT_EQ(sStruct.tcVal, 'B');
    EXPECT_EQ(sStruct.tc16Val, u'E');
    EXPECT_EQ(sStruct.tc32Val, U'H');
    EXPECT_EQ(sStruct.twcVal, L'K');

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test, MarshallMegaStructFloatingPointTypedef)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize("");
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::configuring);

    // Load the shared memory components
    LoadIPCModules(control);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaStruct sStruct{};
    float fVal = -1234.5678f;
    sStruct.tfVal = fVal;
    double dVal = 8765.4321;
    sStruct.tdVal = dVal;
    long double ldVal = -1234.4321;
    sStruct.tldVal = ldVal;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaStruct(sStruct));
    EXPECT_EQ(sStruct.tfVal, fVal - 1000.000f);
    EXPECT_EQ(sStruct.tdVal, dVal + 1000.000);
    EXPECT_EQ(sStruct.tldVal, ldVal - 1000.000);

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test, MarshallMegaStructEnumTypedef)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize("");
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::configuring);

    // Load the shared memory components
    LoadIPCModules(control);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaStruct sStruct{};
    sStruct.teHelloVal = EHello::hello;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaStruct(sStruct));
    EXPECT_EQ(sStruct.teHelloVal, EHello::hallo);

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test, MarshallMegaStructStringTypedef)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize("");
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::configuring);

    // Load the shared memory components
    LoadIPCModules(control);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaStruct sStruct{};
    sStruct.tssVal = "hi";
    sStruct.tss8Val = "huhu";
    sStruct.tss16Val = u"hey";
    sStruct.tss32Val = U"hello";
    sStruct.twssVal = L"servus";
    sStruct.tssFixVal = "hi";
    sStruct.tss8FixVal = "huhu";
    sStruct.tss16FixVal = u"hey";
    sStruct.tss32FixVal = U"hello";
    sStruct.twssFixVal = L"servus";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaStruct(sStruct));
    EXPECT_EQ(sStruct.tssVal, "hij");
    EXPECT_EQ(sStruct.tss8Val, "huhuv");
    EXPECT_EQ(sStruct.tss16Val, u"heyz");
    EXPECT_EQ(sStruct.tss32Val, U"hellop");
    EXPECT_EQ(sStruct.twssVal, L"servust");
    EXPECT_EQ(sStruct.tssFixVal, "hij");
    EXPECT_EQ(sStruct.tss8FixVal, "huhuv");
    EXPECT_EQ(sStruct.tss16FixVal, u"heyz");
    EXPECT_EQ(sStruct.tss32FixVal, U"hellop");
    EXPECT_EQ(sStruct.twssFixVal, L"servust");

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

// Using typedefs of template based types produced an error: BUG #398266
TEST(IPC_Communication_Test, DISABLED_MarshallMegaStructPointerTypedef)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize("");
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::configuring);

    // Load the shared memory components
    LoadIPCModules(control);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaStruct sStruct{};
    //sStruct.tptr8Val.resize(100);
    //for (size_t nIndex = 0; nIndex < sStruct.tptr8Val.size(); nIndex++)
    //    sStruct.tptr8Val[nIndex] = static_cast<uint8_t>(nIndex);
    //sStruct.tptrssVal.resize(3);
    //sStruct.tptrssVal[0] = "hello";
    //sStruct.tptrssVal[1] = "huhu";
    //sStruct.tptrssVal[2] = "hi";
    //sStruct.tptr8FixVal.resize(sStruct.tptr8FixVal.capacity());
    //for (size_t nIndex = 0; nIndex < sStruct.tptr8FixVal.size(); nIndex++)
    //    sStruct.tptr8FixVal[nIndex] = static_cast<uint8_t>(nIndex);
    //sStruct.tptrssFixVal.resize(sStruct.tptrssFixVal.capacity());
    //sStruct.tptrssFixVal[0] = "hi";
    //sStruct.tptrssFixVal[1] = "hoi";
    //sStruct.tptrssFixVal[2] = "hello";
    //sStruct.tptrssFixVal[3] = "hallo";
    //sStruct.tptrssFixVal[4] = "servus";
    //EXPECT_NO_THROW(pMegaTest->ProcessMegaStruct(sStruct));
    //EXPECT_EQ(sStruct.tptr8Val.size(), 200);
    //for (size_t nIndex = 0; nIndex < sStruct.tptr8Val.size() / 2; nIndex++)
    //    EXPECT_EQ(sStruct.tptr8Val[nIndex], sStruct.tptr8Val[nIndex + sStruct.tptr8Val.size() / 2]);
    //EXPECT_EQ(sStruct.tptrssVal.size(), 6);
    //for (size_t nIndex = 0; nIndex < sStruct.tptrssVal.size() / 2; nIndex++)
    //    EXPECT_EQ(sStruct.tptrssVal[nIndex], sStruct.tptrssVal[nIndex + sStruct.tptrssVal.size() / 2]);
    //EXPECT_EQ(sStruct.tptr8FixVal.size(), 20);
    //for (size_t nIndex = 0; nIndex < sStruct.tptr8FixVal.size(); nIndex++)
    //    EXPECT_EQ(sStruct.tptr8FixVal[nIndex], static_cast<uint8_t>(nIndex << 1));
    //EXPECT_EQ(sStruct.tptrssFixVal.size(), 5);
    //EXPECT_EQ(sStruct.tptrssFixVal[0], "hihi");
    //EXPECT_EQ(sStruct.tptrssFixVal[1], "hoihoi");
    //EXPECT_EQ(sStruct.tptrssFixVal[2], "hellohello");
    //EXPECT_EQ(sStruct.tptrssFixVal[3], "hallohallo");
    //EXPECT_EQ(sStruct.tptrssFixVal[4], "servusservus");

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

// Using typedefs of template based types produced an error: BUG #398266
TEST(IPC_Communication_Test, DISABLED_MarshallMegaStructSequenceTypedef)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize("");
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::configuring);

    // Load the shared memory components
    LoadIPCModules(control);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaStruct sStruct{};
    //sStruct.tseq8Val.resize(100);
    //for (size_t nIndex = 0; nIndex < sStruct.tseq8Val.size(); nIndex++)
    //    sStruct.tseq8Val[nIndex] = static_cast<uint8_t>(nIndex);
    //sStruct.tseqssVal.resize(3);
    //sStruct.tseqssVal[0] = "hello";
    //sStruct.tseqssVal[1] = "huhu";
    //sStruct.tseqssVal[2] = "hi";
    //sStruct.tseq8FixVal.resize(sStruct.tseq8FixVal.capacity());
    //for (size_t nIndex = 0; nIndex < sStruct.tseq8FixVal.size(); nIndex++)
    //    sStruct.tseq8FixVal[nIndex] = static_cast<uint8_t>(nIndex);
    //sStruct.tseqssFixVal.resize(sStruct.tseqssFixVal.capacity());
    //sStruct.tseqssFixVal[0] = "hi";
    //sStruct.tseqssFixVal[1] = "hoi";
    //sStruct.tseqssFixVal[2] = "hello";
    //sStruct.tseqssFixVal[3] = "hallo";
    //sStruct.tseqssFixVal[4] = "servus";
    //EXPECT_NO_THROW(pMegaTest->ProcessMegaStruct(sStruct));
    //EXPECT_EQ(sStruct.tseq8Val.size(), 200);
    //for (size_t nIndex = 0; nIndex < sStruct.tseq8Val.size() / 2; nIndex++)
    //    EXPECT_EQ(sStruct.tseq8Val[nIndex], sStruct.tseq8Val[nIndex + sStruct.tseq8Val.size() / 2]);
    //EXPECT_EQ(sStruct.tseqssVal.size(), 6);
    //for (size_t nIndex = 0; nIndex < sStruct.tseqssVal.size() / 2; nIndex++)
    //    EXPECT_EQ(sStruct.tseqssVal[nIndex], sStruct.tseqssVal[nIndex + sStruct.tseqssVal.size() / 2]);
    //EXPECT_EQ(sStruct.tseq8FixVal.size(), 20);
    //for (size_t nIndex = 0; nIndex < sStruct.tseq8FixVal.size(); nIndex++)
    //    EXPECT_EQ(sStruct.tseq8FixVal[nIndex], static_cast<uint8_t>(nIndex << 1));
    //EXPECT_EQ(sStruct.tseqssFixVal.size(), 5);
    //EXPECT_EQ(sStruct.tseqssFixVal[0], "hihi");
    //EXPECT_EQ(sStruct.tseqssFixVal[1], "hoihoi");
    //EXPECT_EQ(sStruct.tseqssFixVal[2], "hellohello");
    //EXPECT_EQ(sStruct.tseqssFixVal[3], "hallohallo");
    //EXPECT_EQ(sStruct.tseqssFixVal[4], "servusservus");

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test, MarshallMegaStructInterfaceTypedef)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize("");
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::configuring);

    // Load the shared memory components
    LoadIPCModules(control);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    struct SMultiply : public IMultiplyValue
    {
        size_t Multiply(size_t n)
        {
            return n * n;
        }
    } sMultiply;

    ASSERT_NE(pMegaTest, nullptr);
    SMegaStruct sStruct{};

    sStruct.tidVal = sdv::GetInterfaceId<IMultiplyValue>();
    sStruct.tifcVal = static_cast<IMultiplyValue*>(&sMultiply);
    // Typedef of interfaces current not possible. BUG #399464
    //sStruct.ptMultiplyValue = &sMultiply;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaStruct(sStruct));
    EXPECT_EQ(sStruct.tidVal, sdv::GetInterfaceId<IAddValue>());
    EXPECT_EQ(sStruct.tifcVal.id(), sdv::GetInterfaceId<IAddValue>());
    ASSERT_NE(sStruct.tifcVal, nullptr);
    ASSERT_NE(sStruct.tifcVal.get<IAddValue>(), nullptr);
    EXPECT_EQ(sStruct.tifcVal.get<IAddValue>()->Add(99), 19602); // 99 * 99 + 99 * 99
    // Typedef of interfaces current not possible. BUG #399464
    //ASSERT_NE(sStruct.tpMultiplyValue, nullptr);
    //EXPECT_EQ(sStruct.tpMultiplyValue->Multiply(5), 625); // 5 * 5 * 5 * 5

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test, MarshallMegaStructSubCompoundTypedef)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize("");
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::configuring);

    // Load the shared memory components
    LoadIPCModules(control);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaStruct sStruct{};
    sStruct.tsSubVal.i = 1;
    sStruct.tsIndVal.i = 10;
    // Incompatible serdes generation: BUG #398509
    //sStruct.tsUnnamedVal.i = 100;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaStruct(sStruct));
    EXPECT_EQ(sStruct.tsSubVal.i, 2);
    EXPECT_EQ(sStruct.tsIndVal.i, 20);
    // Incompatible serdes generation: BUG #398509
    //EXPECT_EQ(sStruct.tsUnnamedVal.i, 120);

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test, MarshallMegaTypeBasedUnionBoolean)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize("");
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::configuring);

    // Load the shared memory components
    LoadIPCModules(control);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    UMegaTypeBasedUnion uUnion;
    uUnion.switch_to(1);
    uUnion.bVal = false;
    EXPECT_FALSE(uUnion.bVal);
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_TRUE(uUnion.bVal);

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test, MarshallMegaTypeBasedUnionIntegral)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize("");
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::configuring);

    // Load the shared memory components
    LoadIPCModules(control);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    UMegaTypeBasedUnion uUnion;
    uUnion.switch_to(2);
    uUnion.sVal = -10;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.sVal, -11);
    uUnion.switch_to(3);
    uUnion.usVal = 10;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.usVal, 11u);
    uUnion.switch_to(4);
    uUnion.lVal = -20;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.lVal, -21);
    uUnion.switch_to(5);
    uUnion.ulVal = 20;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.ulVal, 21u);
    uUnion.switch_to(6);
    uUnion.llVal = -30;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.llVal, -31);
    uUnion.switch_to(7);
    uUnion.ullVal = 30;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.ullVal, 31u);
    uUnion.switch_to(8);
    uUnion.i8Val = -40;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(static_cast<uint8_t>(uUnion.i8Val), static_cast<uint8_t>(-41));    // Cast needed for ARM compatibility
    uUnion.switch_to(9);
    uUnion.ui8Val = 40;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.ui8Val, 41u);
    uUnion.switch_to(10);
    uUnion.i16Val = -50;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.i16Val, -51);
    uUnion.switch_to(11);
    uUnion.ui16Val = 50;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.ui16Val, 51u);
    uUnion.switch_to(12);
    uUnion.i32Val = -60;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.i32Val, -61);
    uUnion.switch_to(13);
    uUnion.ui32Val = 60;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.ui32Val, 61u);
    uUnion.switch_to(14);
    uUnion.i64Val = -70;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.i64Val, -71);
    uUnion.switch_to(15);
    uUnion.ui64Val = 70;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.ui64Val, 71u);
    uUnion.switch_to(23);
    uUnion.nVal = 80;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.nVal, 81);

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test, MarshallMegaTypeBasedUnionCharacter)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize("");
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::configuring);

    // Load the shared memory components
    LoadIPCModules(control);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    UMegaTypeBasedUnion uUnion;
    uUnion.switch_to(16);
    uUnion.cVal = 'A';
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.cVal, 'B');
    uUnion.switch_to(17);
    uUnion.c16Val = u'D';
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.c16Val, u'E');
    uUnion.switch_to(18);
    uUnion.c32Val = U'G';
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.c32Val, U'H');
    uUnion.switch_to(19);
    uUnion.wcVal = L'J';
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.wcVal, L'K');

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test, MarshallMegaTypeBasedUnionFloatingPoint)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize("");
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::configuring);

    // Load the shared memory components
    LoadIPCModules(control);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    UMegaTypeBasedUnion uUnion;
    uUnion.switch_to(20);
    float fVal = -1234.5678f;
    uUnion.fVal = fVal;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.fVal, fVal - 1000.000f);
    uUnion.switch_to(21);
    double dVal = 8765.4321;
    uUnion.dVal = dVal;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.dVal, dVal + 1000.000);
    // GCC issue with generated initialization in the consructor for the "long double" type: BUG #3982727
    //uUnion.switch_to(22);
    //long double ldVal = -1234.4321;
    //uUnion.ldVal = ldVal;
    //EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    //EXPECT_EQ(uUnion.ldVal, ldVal - 1000.000);

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test, MarshallMegaTypeBasedUnionEnum)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize("");
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::configuring);

    // Load the shared memory components
    LoadIPCModules(control);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    UMegaTypeBasedUnion uUnion;
    uUnion.switch_to(24);
    uUnion.eHelloVal = EHello::hello;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.eHelloVal, EHello::hallo);

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test, MarshallMegaTypeBasedUnionString)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize("");
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::configuring);

    // Load the shared memory components
    LoadIPCModules(control);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    UMegaTypeBasedUnion uUnion;
    uUnion.switch_to(26);
    uUnion.ssVal = "hi";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.ssVal, "hij");
    uUnion.switch_to(27);
    uUnion.ss8Val = "huhu";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.ss8Val, "huhuv");
    uUnion.switch_to(28);
    uUnion.ss16Val = u"hey";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.ss16Val, u"heyz");
    uUnion.switch_to(29);
    uUnion.ss32Val = U"hello";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.ss32Val, U"hellop");
    uUnion.switch_to(30);
    uUnion.wssVal = L"servus";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.wssVal, L"servust");
    uUnion.switch_to(31);
    uUnion.ssFixVal = "hi";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.ssFixVal, "hij");
    uUnion.switch_to(32);
    uUnion.ss8FixVal = "huhu";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.ss8FixVal, "huhuv");
    uUnion.switch_to(33);
    uUnion.ss16FixVal = u"hey";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.ss16FixVal, u"heyz");
    uUnion.switch_to(34);
    uUnion.ss32FixVal = U"hello";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.ss32FixVal, U"hellop");
    uUnion.switch_to(35);
    uUnion.wssFixVal = L"servus";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.wssFixVal, L"servust");

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test, MarshallMegaTypeBasedUnionPointer)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize("");
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::configuring);

    // Load the shared memory components
    LoadIPCModules(control);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    UMegaTypeBasedUnion uUnion;
    uUnion.switch_to(36);
    uUnion.ptr8Val.resize(100);
    for (size_t nIndex = 0; nIndex < uUnion.ptr8Val.size(); nIndex++)
        uUnion.ptr8Val[nIndex] = static_cast<uint8_t>(nIndex);
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.ptr8Val.size(), 200);
    for (size_t nIndex = 0; nIndex < uUnion.ptr8Val.size() / 2; nIndex++)
        EXPECT_EQ(uUnion.ptr8Val[nIndex], uUnion.ptr8Val[nIndex + uUnion.ptr8Val.size() / 2]);
    uUnion.switch_to(37);
    uUnion.ptrssVal.resize(3);
    uUnion.ptrssVal[0] = "hello";
    uUnion.ptrssVal[1] = "huhu";
    uUnion.ptrssVal[2] = "hi";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.ptrssVal.size(), 6);
    for (size_t nIndex = 0; nIndex < uUnion.ptrssVal.size() / 2; nIndex++)
        EXPECT_EQ(uUnion.ptrssVal[nIndex], uUnion.ptrssVal[nIndex + uUnion.ptrssVal.size() / 2]);
    uUnion.switch_to(38);
    uUnion.ptr8FixVal.resize(uUnion.ptr8FixVal.capacity());
    for (size_t nIndex = 0; nIndex < uUnion.ptr8FixVal.size(); nIndex++)
        uUnion.ptr8FixVal[nIndex] = static_cast<uint8_t>(nIndex);
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.ptr8FixVal.size(), 20);
    for (size_t nIndex = 0; nIndex < uUnion.ptr8FixVal.size(); nIndex++)
        EXPECT_EQ(uUnion.ptr8FixVal[nIndex], static_cast<uint8_t>(nIndex << 1));
    uUnion.switch_to(39);
    uUnion.ptrssFixVal.resize(uUnion.ptrssFixVal.capacity());
    uUnion.ptrssFixVal[0] = "hi";
    uUnion.ptrssFixVal[1] = "hoi";
    uUnion.ptrssFixVal[2] = "hello";
    uUnion.ptrssFixVal[3] = "hallo";
    uUnion.ptrssFixVal[4] = "servus";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.ptrssFixVal.size(), 5);
    EXPECT_EQ(uUnion.ptrssFixVal[0], "hihi");
    EXPECT_EQ(uUnion.ptrssFixVal[1], "hoihoi");
    EXPECT_EQ(uUnion.ptrssFixVal[2], "hellohello");
    EXPECT_EQ(uUnion.ptrssFixVal[3], "hallohallo");
    EXPECT_EQ(uUnion.ptrssFixVal[4], "servusservus");

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test, MarshallMegaTypeBasedUnionSequence)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize("");
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::configuring);

    // Load the shared memory components
    LoadIPCModules(control);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    UMegaTypeBasedUnion uUnion;
    uUnion.switch_to(36);
    uUnion.seq8Val.resize(100);
    for (size_t nIndex = 0; nIndex < uUnion.seq8Val.size(); nIndex++)
        uUnion.seq8Val[nIndex] = static_cast<uint8_t>(nIndex);
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.seq8Val.size(), 200);
    for (size_t nIndex = 0; nIndex < uUnion.seq8Val.size() / 2; nIndex++)
        EXPECT_EQ(uUnion.seq8Val[nIndex], uUnion.seq8Val[nIndex + uUnion.seq8Val.size() / 2]);
    uUnion.switch_to(37);
    uUnion.seqssVal.resize(3);
    uUnion.seqssVal[0] = "hello";
    uUnion.seqssVal[1] = "huhu";
    uUnion.seqssVal[2] = "hi";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.seqssVal.size(), 6);
    for (size_t nIndex = 0; nIndex < uUnion.seqssVal.size() / 2; nIndex++)
        EXPECT_EQ(uUnion.seqssVal[nIndex], uUnion.seqssVal[nIndex + uUnion.seqssVal.size() / 2]);
    uUnion.switch_to(38);
    uUnion.seq8FixVal.resize(uUnion.seq8FixVal.capacity());
    for (size_t nIndex = 0; nIndex < uUnion.seq8FixVal.size(); nIndex++)
        uUnion.seq8FixVal[nIndex] = static_cast<uint8_t>(nIndex);
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.seq8FixVal.size(), 20);
    for (size_t nIndex = 0; nIndex < uUnion.seq8FixVal.size(); nIndex++)
        EXPECT_EQ(uUnion.seq8FixVal[nIndex], static_cast<uint8_t>(nIndex << 1));
    uUnion.switch_to(39);
    uUnion.seqssFixVal.resize(uUnion.seqssFixVal.capacity());
    uUnion.seqssFixVal[0] = "hi";
    uUnion.seqssFixVal[1] = "hoi";
    uUnion.seqssFixVal[2] = "hello";
    uUnion.seqssFixVal[3] = "hallo";
    uUnion.seqssFixVal[4] = "servus";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.seqssFixVal.size(), 5);
    EXPECT_EQ(uUnion.seqssFixVal[0], "hihi");
    EXPECT_EQ(uUnion.seqssFixVal[1], "hoihoi");
    EXPECT_EQ(uUnion.seqssFixVal[2], "hellohello");
    EXPECT_EQ(uUnion.seqssFixVal[3], "hallohallo");
    EXPECT_EQ(uUnion.seqssFixVal[4], "servusservus");

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test, MarshallMegaTypeBasedUnionInterface)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize("");
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::configuring);

    // Load the shared memory components
    LoadIPCModules(control);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    struct SMultiply : public IMultiplyValue
    {
        size_t Multiply(size_t n)
        {
            return n * n;
        }
    } sMultiply;

    ASSERT_NE(pMegaTest, nullptr);
    UMegaTypeBasedUnion uUnion;
    uUnion.switch_to(25);

    uUnion.idVal = sdv::GetInterfaceId<IMultiplyValue>();
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.idVal, sdv::GetInterfaceId<IAddValue>());
    uUnion.switch_to(44);
    uUnion.ifcVal = static_cast<IMultiplyValue*>(&sMultiply);
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.ifcVal.id(), sdv::GetInterfaceId<IAddValue>());
    ASSERT_NE(uUnion.ifcVal, nullptr);
    ASSERT_NE(uUnion.ifcVal.get<IAddValue>(), nullptr);
    EXPECT_EQ(uUnion.ifcVal.get<IAddValue>()->Add(99), 19602); // 99 * 99 + 99 * 99
    uUnion.switch_to(45);
    uUnion.pMultiplyValue = &sMultiply;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    ASSERT_NE(uUnion.pMultiplyValue, nullptr);
    EXPECT_EQ(uUnion.pMultiplyValue->Multiply(5), 625); // 5 * 5 * 5 * 5

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test, MarshallMegaTypeBasedUnionSubCompound)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize("");
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::configuring);

    // Load the shared memory components
    LoadIPCModules(control);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    UMegaTypeBasedUnion uUnion;
    uUnion.switch_to(46);
    uUnion.sSubVal.i = 1;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.sSubVal.i, 2);

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test, MarshallMegaTypeBasedUnionBooleanTypedef)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize("");
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::configuring);

    // Load the shared memory components
    LoadIPCModules(control);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    UMegaTypeBasedUnion uUnion;
    uUnion.switch_to(201);
    uUnion.tbVal = false;
    EXPECT_FALSE(uUnion.tbVal);
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_TRUE(uUnion.tbVal);

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test, MarshallMegaTypeBasedUnionIntegralTypedef)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize("");
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::configuring);

    // Load the shared memory components
    LoadIPCModules(control);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    UMegaTypeBasedUnion uUnion;
    uUnion.switch_to(202);
    uUnion.tsVal = -10;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.tsVal, -11);
    uUnion.switch_to(203);
    uUnion.tusVal = 10;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.tusVal, 11u);
    uUnion.switch_to(204);
    uUnion.tlVal = -20;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.tlVal, -21);
    uUnion.switch_to(205);
    uUnion.tulVal = 20;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.tulVal, 21u);
    uUnion.switch_to(206);
    uUnion.tllVal = -30;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.tllVal, -31);
    uUnion.switch_to(207);
    uUnion.tullVal = 30;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.tullVal, 31u);
    uUnion.switch_to(208);
    uUnion.ti8Val = -40;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(static_cast<uint8_t>(uUnion.ti8Val), static_cast<uint8_t>(-41));    // Cast needed for ARM compatibility
    uUnion.switch_to(209);
    uUnion.tui8Val = 40;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.tui8Val, 41u);
    uUnion.switch_to(210);
    uUnion.ti16Val = -50;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.ti16Val, -51);
    uUnion.switch_to(211);
    uUnion.tui16Val = 50;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.tui16Val, 51u);
    uUnion.switch_to(212);
    uUnion.ti32Val = -60;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.ti32Val, -61);
    uUnion.switch_to(213);
    uUnion.tui32Val = 60;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.tui32Val, 61u);
    uUnion.switch_to(214);
    uUnion.ti64Val = -70;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.ti64Val, -71);
    uUnion.switch_to(215);
    uUnion.tui64Val = 70;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.tui64Val, 71u);
    uUnion.switch_to(223);
    uUnion.tnVal = 80;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.tnVal, 81);

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test, MarshallMegaTypeBasedUnionCharacterTypedef)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize("");
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::configuring);

    // Load the shared memory components
    LoadIPCModules(control);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    UMegaTypeBasedUnion uUnion;
    uUnion.switch_to(216);
    uUnion.tcVal = 'A';
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.tcVal, 'B');
    uUnion.switch_to(217);
    uUnion.tc16Val = u'D';
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.tc16Val, u'E');
    uUnion.switch_to(218);
    uUnion.tc32Val = U'G';
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.tc32Val, U'H');
    uUnion.switch_to(219);
    uUnion.twcVal = L'J';
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.twcVal, L'K');

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test, MarshallMegaTypeBasedUnionFloatingPointTypedef)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize("");
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::configuring);

    // Load the shared memory components
    LoadIPCModules(control);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    UMegaTypeBasedUnion uUnion;
    uUnion.switch_to(220);
    float fVal = -1234.5678f;
    uUnion.tfVal = fVal;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.tfVal, fVal - 1000.000f);
    uUnion.switch_to(221);
    double dVal = 8765.4321;
    uUnion.tdVal = dVal;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.tdVal, dVal + 1000.000);
    // GCC issue with generated initialization in the consructor for the "long double" type: BUG #3982727
    //uUnion.switch_to(222);
    //long double ldVal = -1234.4321;
    //uUnion.tldVal = ldVal;
    //EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    //EXPECT_EQ(uUnion.tldVal, ldVal - 1000.000);

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test, MarshallMegaTypeBasedUnionEnumTypedef)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize("");
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::configuring);

    // Load the shared memory components
    LoadIPCModules(control);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    UMegaTypeBasedUnion uUnion;
    uUnion.switch_to(224);
    uUnion.teHelloVal = EHello::hello;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.teHelloVal, EHello::hallo);

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test, MarshallMegaTypeBasedUnionStringTypedef)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize("");
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::configuring);

    // Load the shared memory components
    LoadIPCModules(control);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    UMegaTypeBasedUnion uUnion;
    uUnion.switch_to(226);
    uUnion.tssVal = "hi";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.tssVal, "hij");
    uUnion.switch_to(227);
    uUnion.tss8Val = "huhu";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.tss8Val, "huhuv");
    uUnion.switch_to(228);
    uUnion.tss16Val = u"hey";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.tss16Val, u"heyz");
    uUnion.switch_to(229);
    uUnion.tss32Val = U"hello";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.tss32Val, U"hellop");
    uUnion.switch_to(230);
    uUnion.twssVal = L"servus";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.twssVal, L"servust");
    uUnion.switch_to(231);
    uUnion.tssFixVal = "hi";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.tssFixVal, "hij");
    uUnion.switch_to(232);
    uUnion.tss8FixVal = "huhu";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.tss8FixVal, "huhuv");
    uUnion.switch_to(233);
    uUnion.tss16FixVal = u"hey";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.tss16FixVal, u"heyz");
    uUnion.switch_to(234);
    uUnion.tss32FixVal = U"hello";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.tss32FixVal, U"hellop");
    uUnion.switch_to(235);
    uUnion.twssFixVal = L"servus";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.twssFixVal, L"servust");

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

// Using typedefs of template based types produced an error: BUG #398266
TEST(IPC_Communication_Test, DISABLED_MarshallMegaTypeBasedUnionPointerTypedef)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize("");
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::configuring);

    // Load the shared memory components
    LoadIPCModules(control);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    UMegaTypeBasedUnion uUnion;
    uUnion.switch_to(236);
    //uUnion.tptr8Val.resize(100);
    //for (size_t nIndex = 0; nIndex < uUnion.tptr8Val.size(); nIndex++)
    //    uUnion.tptr8Val[nIndex] = static_cast<uint8_t>(nIndex);
    //EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    //EXPECT_EQ(uUnion.tptr8Val.size(), 200);
    //for (size_t nIndex = 0; nIndex < uUnion.tptr8Val.size() / 2; nIndex++)
    //    EXPECT_EQ(uUnion.tptr8Val[nIndex], uUnion.tptr8Val[nIndex + uUnion.tptr8Val.size() / 2]);
    //uUnion.switch_to(237);
    //uUnion.tptrssVal.resize(3);
    //uUnion.tptrssVal[0] = "hello";
    //uUnion.tptrssVal[1] = "huhu";
    //uUnion.tptrssVal[2] = "hi";
    //EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    //EXPECT_EQ(uUnion.tptrssVal.size(), 6);
    //for (size_t nIndex = 0; nIndex < uUnion.tptrssVal.size() / 2; nIndex++)
    //    EXPECT_EQ(uUnion.tptrssVal[nIndex], uUnion.tptrssVal[nIndex + uUnion.tptrssVal.size() / 2]);
    //uUnion.switch_to(238);
    //uUnion.tptr8FixVal.resize(uUnion.tptr8FixVal.capacity());
    //for (size_t nIndex = 0; nIndex < uUnion.tptr8FixVal.size(); nIndex++)
    //    uUnion.tptr8FixVal[nIndex] = static_cast<uint8_t>(nIndex);
    //EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    //EXPECT_EQ(uUnion.tptr8FixVal.size(), 20);
    //for (size_t nIndex = 0; nIndex < uUnion.tptr8FixVal.size(); nIndex++)
    //    EXPECT_EQ(uUnion.tptr8FixVal[nIndex], static_cast<uint8_t>(nIndex << 1));
    //uUnion.switch_to(239);
    //uUnion.tptrssFixVal.resize(uUnion.tptrssFixVal.capacity());
    //uUnion.tptrssFixVal[0] = "hi";
    //uUnion.tptrssFixVal[1] = "hoi";
    //uUnion.tptrssFixVal[2] = "hello";
    //uUnion.tptrssFixVal[3] = "hallo";
    //uUnion.tptrssFixVal[4] = "servus";
    //EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    //EXPECT_EQ(uUnion.tptrssFixVal.size(), 5);
    //EXPECT_EQ(uUnion.tptrssFixVal[0], "hihi");
    //EXPECT_EQ(uUnion.tptrssFixVal[1], "hoihoi");
    //EXPECT_EQ(uUnion.tptrssFixVal[2], "hellohello");
    //EXPECT_EQ(uUnion.tptrssFixVal[3], "hallohallo");
    //EXPECT_EQ(uUnion.tptrssFixVal[4], "servusservus");

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

// Using typedefs of template based types produced an error: BUG #398266
TEST(IPC_Communication_Test, DISABLED_MarshallMegaTypeBasedUnionSequenceTypedef)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize("");
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::configuring);

    // Load the shared memory components
    LoadIPCModules(control);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    UMegaTypeBasedUnion uUnion;
    uUnion.switch_to(236);
    //uUnion.tseq8Val.resize(100);
    //for (size_t nIndex = 0; nIndex < uUnion.tseq8Val.size(); nIndex++)
    //    uUnion.tseq8Val[nIndex] = static_cast<uint8_t>(nIndex);
    //EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    //EXPECT_EQ(uUnion.tseq8Val.size(), 200);
    //for (size_t nIndex = 0; nIndex < uUnion.tseq8Val.size() / 2; nIndex++)
    //    EXPECT_EQ(uUnion.tseq8Val[nIndex], uUnion.tseq8Val[nIndex + uUnion.tseq8Val.size() / 2]);
    //uUnion.switch_to(237);
    //uUnion.tseqssVal.resize(3);
    //uUnion.tseqssVal[0] = "hello";
    //uUnion.tseqssVal[1] = "huhu";
    //uUnion.tseqssVal[2] = "hi";
    //EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    //EXPECT_EQ(uUnion.tseqssVal.size(), 6);
    //for (size_t nIndex = 0; nIndex < uUnion.tseqssVal.size() / 2; nIndex++)
    //    EXPECT_EQ(uUnion.tseqssVal[nIndex], uUnion.tseqssVal[nIndex + uUnion.tseqssVal.size() / 2]);
    //uUnion.switch_to(238);
    //uUnion.tseq8FixVal.resize(uUnion.tseq8FixVal.capacity());
    //for (size_t nIndex = 0; nIndex < uUnion.tseq8FixVal.size(); nIndex++)
    //    uUnion.tseq8FixVal[nIndex] = static_cast<uint8_t>(nIndex);
    //EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    //EXPECT_EQ(uUnion.tseq8FixVal.size(), 20);
    //for (size_t nIndex = 0; nIndex < uUnion.tseq8FixVal.size(); nIndex++)
    //    EXPECT_EQ(uUnion.tseq8FixVal[nIndex], static_cast<uint8_t>(nIndex << 1));
    //uUnion.switch_to(239);
    //uUnion.tseqssFixVal.resize(uUnion.tseqssFixVal.capacity());
    //uUnion.tseqssFixVal[0] = "hi";
    //uUnion.tseqssFixVal[1] = "hoi";
    //uUnion.tseqssFixVal[2] = "hello";
    //uUnion.tseqssFixVal[3] = "hallo";
    //uUnion.tseqssFixVal[4] = "servus";
    //EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    //EXPECT_EQ(uUnion.tseqssFixVal.size(), 5);
    //EXPECT_EQ(uUnion.tseqssFixVal[0], "hihi");
    //EXPECT_EQ(uUnion.tseqssFixVal[1], "hoihoi");
    //EXPECT_EQ(uUnion.tseqssFixVal[2], "hellohello");
    //EXPECT_EQ(uUnion.tseqssFixVal[3], "hallohallo");
    //EXPECT_EQ(uUnion.tseqssFixVal[4], "servusservus");

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test, MarshallMegaTypeBasedUnionInterfaceTypedef)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize("");
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::configuring);

    // Load the shared memory components
    LoadIPCModules(control);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    struct SMultiply : public IMultiplyValue
    {
        size_t Multiply(size_t n)
        {
            return n * n;
        }
    } sMultiply;

    ASSERT_NE(pMegaTest, nullptr);
    UMegaTypeBasedUnion uUnion;
    uUnion.switch_to(225);

    uUnion.tidVal = sdv::GetInterfaceId<IMultiplyValue>();
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.tidVal, sdv::GetInterfaceId<IAddValue>());
    uUnion.switch_to(244);
    uUnion.tifcVal = static_cast<IMultiplyValue*>(&sMultiply);
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.tifcVal.id(), sdv::GetInterfaceId<IAddValue>());
    ASSERT_NE(uUnion.tifcVal, nullptr);
    ASSERT_NE(uUnion.tifcVal.get<IAddValue>(), nullptr);
    EXPECT_EQ(uUnion.tifcVal.get<IAddValue>()->Add(99), 19602); // 99 * 99 + 99 * 99
    // Typedef of interfaces current not possible. BUG #399464
    //uUnion.switch_to(245);
    //uUnion.tpMultiplyValue = &sMultiply;
    //EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    //ASSERT_NE(uUnion.tpMultiplyValue, nullptr);
    //EXPECT_EQ(uUnion.tpMultiplyValue->Multiply(5), 625); // 5 * 5 * 5 * 5

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test, MarshallMegaTypeBasedUnionSubCompoundTypedef)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize("");
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::configuring);

    // Load the shared memory components
    LoadIPCModules(control);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    UMegaTypeBasedUnion uUnion;
    uUnion.switch_to(246);
    uUnion.tsSubVal.i = 1;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.tsSubVal.i, 2);
    uUnion.switch_to(247);
    uUnion.tsIndVal.i = 10;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.tsIndVal.i, 20);

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test, MarshallMegaVarBasedUnionBoolean)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize("");
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::configuring);

    // Load the shared memory components
    LoadIPCModules(control);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaVarBasedUnion sUnion;
    sUnion.switch_to(1);
    sUnion.uVal.bVal = false;
    EXPECT_FALSE(sUnion.uVal.bVal);
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_TRUE(sUnion.uVal.bVal);

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test, MarshallMegaVarBasedUnionIntegral)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize("");
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::configuring);

    // Load the shared memory components
    LoadIPCModules(control);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaVarBasedUnion sUnion;
    sUnion.switch_to(2);
    sUnion.uVal.sVal = -10;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.sVal, -11);
    sUnion.switch_to(3);
    sUnion.uVal.usVal = 10;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.usVal, 11u);
    sUnion.switch_to(4);
    sUnion.uVal.lVal = -20;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.lVal, -21);
    sUnion.switch_to(5);
    sUnion.uVal.ulVal = 20;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.ulVal, 21u);
    sUnion.switch_to(6);
    sUnion.uVal.llVal = -30;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.llVal, -31);
    sUnion.switch_to(7);
    sUnion.uVal.ullVal = 30;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.ullVal, 31u);
    sUnion.switch_to(8);
    sUnion.uVal.i8Val = -40;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(static_cast<uint8_t>(sUnion.uVal.i8Val), static_cast<uint8_t>(-41));    // Cast needed for ARM compatibility
    sUnion.switch_to(9);
    sUnion.uVal.ui8Val = 40;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.ui8Val, 41u);
    sUnion.switch_to(10);
    sUnion.uVal.i16Val = -50;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.i16Val, -51);
    sUnion.switch_to(11);
    sUnion.uVal.ui16Val = 50;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.ui16Val, 51u);
    sUnion.switch_to(12);
    sUnion.uVal.i32Val = -60;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.i32Val, -61);
    sUnion.switch_to(13);
    sUnion.uVal.ui32Val = 60;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.ui32Val, 61u);
    sUnion.switch_to(14);
    sUnion.uVal.i64Val = -70;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.i64Val, -71);
    sUnion.switch_to(15);
    sUnion.uVal.ui64Val = 70;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.ui64Val, 71u);
    sUnion.switch_to(23);
    sUnion.uVal.nVal = 80;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.nVal, 81);

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test, MarshallMegaVarBasedUnionCharacter)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize("");
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::configuring);

    // Load the shared memory components
    LoadIPCModules(control);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaVarBasedUnion sUnion;
    sUnion.switch_to(16);
    sUnion.uVal.cVal = 'A';
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.cVal, 'B');
    sUnion.switch_to(17);
    sUnion.uVal.c16Val = u'D';
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.c16Val, u'E');
    sUnion.switch_to(18);
    sUnion.uVal.c32Val = U'G';
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.c32Val, U'H');
    sUnion.switch_to(19);
    sUnion.uVal.wcVal = L'J';
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.wcVal, L'K');

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test, MarshallMegaVarBasedUnionFloatingPoint)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize("");
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::configuring);

    // Load the shared memory components
    LoadIPCModules(control);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaVarBasedUnion sUnion;
    sUnion.switch_to(20);
    float fVal = -1234.5678f;
    sUnion.uVal.fVal = fVal;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.fVal, fVal - 1000.000f);
    sUnion.switch_to(21);
    double dVal = 8765.4321;
    sUnion.uVal.dVal = dVal;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.dVal, dVal + 1000.000);
    // GCC issue with generated initialization in the consructor for the "long double" type: BUG #3982727
    //sUnion.switch_to(22);
    //long double ldVal = -1234.4321;
    //sUnion.uVal.ldVal = ldVal;
    //EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    //EXPECT_EQ(sUnion.uVal.ldVal, ldVal - 1000.000);

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test, MarshallMegaVarBasedUnionEnum)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize("");
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::configuring);

    // Load the shared memory components
    LoadIPCModules(control);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaVarBasedUnion sUnion;
    sUnion.switch_to(24);
    sUnion.uVal.eHelloVal = EHello::hello;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.eHelloVal, EHello::hallo);

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test, MarshallMegaVarBasedUnionString)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize("");
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::configuring);

    // Load the shared memory components
    LoadIPCModules(control);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaVarBasedUnion sUnion;
    sUnion.switch_to(26);
    sUnion.uVal.ssVal = "hi";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.ssVal, "hij");
    sUnion.switch_to(27);
    sUnion.uVal.ss8Val = "huhu";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.ss8Val, "huhuv");
    sUnion.switch_to(28);
    sUnion.uVal.ss16Val = u"hey";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.ss16Val, u"heyz");
    sUnion.switch_to(29);
    sUnion.uVal.ss32Val = U"hello";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.ss32Val, U"hellop");
    sUnion.switch_to(30);
    sUnion.uVal.wssVal = L"servus";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.wssVal, L"servust");
    sUnion.switch_to(31);
    sUnion.uVal.ssFixVal = "hi";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.ssFixVal, "hij");
    sUnion.switch_to(32);
    sUnion.uVal.ss8FixVal = "huhu";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.ss8FixVal, "huhuv");
    sUnion.switch_to(33);
    sUnion.uVal.ss16FixVal = u"hey";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.ss16FixVal, u"heyz");
    sUnion.switch_to(34);
    sUnion.uVal.ss32FixVal = U"hello";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.ss32FixVal, U"hellop");
    sUnion.switch_to(35);
    sUnion.uVal.wssFixVal = L"servus";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.wssFixVal, L"servust");

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test, MarshallMegaVarBasedUnionPointer)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize("");
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::configuring);

    // Load the shared memory components
    LoadIPCModules(control);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaVarBasedUnion sUnion;
    sUnion.switch_to(36);
    sUnion.uVal.ptr8Val.resize(100);
    for (size_t nIndex = 0; nIndex < sUnion.uVal.ptr8Val.size(); nIndex++)
        sUnion.uVal.ptr8Val[nIndex] = static_cast<uint8_t>(nIndex);
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.ptr8Val.size(), 200);
    for (size_t nIndex = 0; nIndex < sUnion.uVal.ptr8Val.size() / 2; nIndex++)
        EXPECT_EQ(sUnion.uVal.ptr8Val[nIndex], sUnion.uVal.ptr8Val[nIndex + sUnion.uVal.ptr8Val.size() / 2]);
    sUnion.switch_to(37);
    sUnion.uVal.ptrssVal.resize(3);
    sUnion.uVal.ptrssVal[0] = "hello";
    sUnion.uVal.ptrssVal[1] = "huhu";
    sUnion.uVal.ptrssVal[2] = "hi";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.ptrssVal.size(), 6);
    for (size_t nIndex = 0; nIndex < sUnion.uVal.ptrssVal.size() / 2; nIndex++)
        EXPECT_EQ(sUnion.uVal.ptrssVal[nIndex], sUnion.uVal.ptrssVal[nIndex + sUnion.uVal.ptrssVal.size() / 2]);
    sUnion.switch_to(38);
    sUnion.uVal.ptr8FixVal.resize(sUnion.uVal.ptr8FixVal.capacity());
    for (size_t nIndex = 0; nIndex < sUnion.uVal.ptr8FixVal.size(); nIndex++)
        sUnion.uVal.ptr8FixVal[nIndex] = static_cast<uint8_t>(nIndex);
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.ptr8FixVal.size(), 20);
    for (size_t nIndex = 0; nIndex < sUnion.uVal.ptr8FixVal.size(); nIndex++)
        EXPECT_EQ(sUnion.uVal.ptr8FixVal[nIndex], static_cast<uint8_t>(nIndex << 1));
    sUnion.switch_to(39);
    sUnion.uVal.ptrssFixVal.resize(sUnion.uVal.ptrssFixVal.capacity());
    sUnion.uVal.ptrssFixVal[0] = "hi";
    sUnion.uVal.ptrssFixVal[1] = "hoi";
    sUnion.uVal.ptrssFixVal[2] = "hello";
    sUnion.uVal.ptrssFixVal[3] = "hallo";
    sUnion.uVal.ptrssFixVal[4] = "servus";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.ptrssFixVal.size(), 5);
    EXPECT_EQ(sUnion.uVal.ptrssFixVal[0], "hihi");
    EXPECT_EQ(sUnion.uVal.ptrssFixVal[1], "hoihoi");
    EXPECT_EQ(sUnion.uVal.ptrssFixVal[2], "hellohello");
    EXPECT_EQ(sUnion.uVal.ptrssFixVal[3], "hallohallo");
    EXPECT_EQ(sUnion.uVal.ptrssFixVal[4], "servusservus");

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test, MarshallMegaVarBasedUnionSequence)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize("");
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::configuring);

    // Load the shared memory components
    LoadIPCModules(control);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaVarBasedUnion sUnion;
    sUnion.switch_to(36);
    sUnion.uVal.seq8Val.resize(100);
    for (size_t nIndex = 0; nIndex < sUnion.uVal.seq8Val.size(); nIndex++)
        sUnion.uVal.seq8Val[nIndex] = static_cast<uint8_t>(nIndex);
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.seq8Val.size(), 200);
    for (size_t nIndex = 0; nIndex < sUnion.uVal.seq8Val.size() / 2; nIndex++)
        EXPECT_EQ(sUnion.uVal.seq8Val[nIndex], sUnion.uVal.seq8Val[nIndex + sUnion.uVal.seq8Val.size() / 2]);
    sUnion.switch_to(37);
    sUnion.uVal.seqssVal.resize(3);
    sUnion.uVal.seqssVal[0] = "hello";
    sUnion.uVal.seqssVal[1] = "huhu";
    sUnion.uVal.seqssVal[2] = "hi";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.seqssVal.size(), 6);
    for (size_t nIndex = 0; nIndex < sUnion.uVal.seqssVal.size() / 2; nIndex++)
        EXPECT_EQ(sUnion.uVal.seqssVal[nIndex], sUnion.uVal.seqssVal[nIndex + sUnion.uVal.seqssVal.size() / 2]);
    sUnion.switch_to(38);
    sUnion.uVal.seq8FixVal.resize(sUnion.uVal.seq8FixVal.capacity());
    for (size_t nIndex = 0; nIndex < sUnion.uVal.seq8FixVal.size(); nIndex++)
        sUnion.uVal.seq8FixVal[nIndex] = static_cast<uint8_t>(nIndex);
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.seq8FixVal.size(), 20);
    for (size_t nIndex = 0; nIndex < sUnion.uVal.seq8FixVal.size(); nIndex++)
        EXPECT_EQ(sUnion.uVal.seq8FixVal[nIndex], static_cast<uint8_t>(nIndex << 1));
    sUnion.switch_to(39);
    sUnion.uVal.seqssFixVal.resize(sUnion.uVal.seqssFixVal.capacity());
    sUnion.uVal.seqssFixVal[0] = "hi";
    sUnion.uVal.seqssFixVal[1] = "hoi";
    sUnion.uVal.seqssFixVal[2] = "hello";
    sUnion.uVal.seqssFixVal[3] = "hallo";
    sUnion.uVal.seqssFixVal[4] = "servus";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.seqssFixVal.size(), 5);
    EXPECT_EQ(sUnion.uVal.seqssFixVal[0], "hihi");
    EXPECT_EQ(sUnion.uVal.seqssFixVal[1], "hoihoi");
    EXPECT_EQ(sUnion.uVal.seqssFixVal[2], "hellohello");
    EXPECT_EQ(sUnion.uVal.seqssFixVal[3], "hallohallo");
    EXPECT_EQ(sUnion.uVal.seqssFixVal[4], "servusservus");

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test, MarshallMegaVarBasedUnionInterface)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize("");
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::configuring);

    // Load the shared memory components
    LoadIPCModules(control);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    struct SMultiply : public IMultiplyValue
    {
        size_t Multiply(size_t n)
        {
            return n * n;
        }
    } sMultiply;

    ASSERT_NE(pMegaTest, nullptr);
    SMegaVarBasedUnion sUnion;
    sUnion.switch_to(25);

    sUnion.uVal.idVal = sdv::GetInterfaceId<IMultiplyValue>();
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.idVal, sdv::GetInterfaceId<IAddValue>());
    sUnion.switch_to(44);
    sUnion.uVal.ifcVal = static_cast<IMultiplyValue*>(&sMultiply);
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.ifcVal.id(), sdv::GetInterfaceId<IAddValue>());
    ASSERT_NE(sUnion.uVal.ifcVal, nullptr);
    ASSERT_NE(sUnion.uVal.ifcVal.get<IAddValue>(), nullptr);
    EXPECT_EQ(sUnion.uVal.ifcVal.get<IAddValue>()->Add(99), 19602); // 99 * 99 + 99 * 99
    sUnion.switch_to(45);
    sUnion.uVal.pMultiplyValue = &sMultiply;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    ASSERT_NE(sUnion.uVal.pMultiplyValue, nullptr);
    EXPECT_EQ(sUnion.uVal.pMultiplyValue->Multiply(5), 625); // 5 * 5 * 5 * 5

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test, MarshallMegaVarBasedUnionSubCompound)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize("");
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::configuring);

    // Load the shared memory components
    LoadIPCModules(control);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaVarBasedUnion sUnion;
    sUnion.switch_to(46);
    sUnion.uVal.sSubVal.i = 1;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.sSubVal.i, 2);

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test, MarshallMegaVarBasedUnionBooleanTypedef)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize("");
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::configuring);

    // Load the shared memory components
    LoadIPCModules(control);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaVarBasedUnion sUnion;
    sUnion.switch_to(201);
    sUnion.uVal.tbVal = false;
    EXPECT_FALSE(sUnion.uVal.tbVal);
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_TRUE(sUnion.uVal.tbVal);

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test, MarshallMegaVarBasedUnionIntegralTypedef)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize("");
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::configuring);

    // Load the shared memory components
    LoadIPCModules(control);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaVarBasedUnion sUnion;
    sUnion.switch_to(202);
    sUnion.uVal.tsVal = -10;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.tsVal, -11);
    sUnion.switch_to(203);
    sUnion.uVal.tusVal = 10;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.tusVal, 11u);
    sUnion.switch_to(204);
    sUnion.uVal.tlVal = -20;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.tlVal, -21);
    sUnion.switch_to(205);
    sUnion.uVal.tulVal = 20;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.tulVal, 21u);
    sUnion.switch_to(206);
    sUnion.uVal.tllVal = -30;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.tllVal, -31);
    sUnion.switch_to(207);
    sUnion.uVal.tullVal = 30;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.tullVal, 31u);
    sUnion.switch_to(208);
    sUnion.uVal.ti8Val = -40;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(static_cast<uint8_t>(sUnion.uVal.ti8Val), static_cast<uint8_t>(-41));    // Cast needed for ARM compatibility
    sUnion.switch_to(209);
    sUnion.uVal.tui8Val = 40;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.tui8Val, 41u);
    sUnion.switch_to(210);
    sUnion.uVal.ti16Val = -50;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.ti16Val, -51);
    sUnion.switch_to(211);
    sUnion.uVal.tui16Val = 50;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.tui16Val, 51u);
    sUnion.switch_to(212);
    sUnion.uVal.ti32Val = -60;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.ti32Val, -61);
    sUnion.switch_to(213);
    sUnion.uVal.tui32Val = 60;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.tui32Val, 61u);
    sUnion.switch_to(214);
    sUnion.uVal.ti64Val = -70;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.ti64Val, -71);
    sUnion.switch_to(215);
    sUnion.uVal.tui64Val = 70;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.tui64Val, 71u);
    sUnion.switch_to(223);
    sUnion.uVal.tnVal = 80;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.tnVal, 81);

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test, MarshallMegaVarBasedUnionCharacterTypedef)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize("");
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::configuring);

    // Load the shared memory components
    LoadIPCModules(control);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaVarBasedUnion sUnion;
    sUnion.switch_to(216);
    sUnion.uVal.tcVal = 'A';
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.tcVal, 'B');
    sUnion.switch_to(217);
    sUnion.uVal.tc16Val = u'D';
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.tc16Val, u'E');
    sUnion.switch_to(218);
    sUnion.uVal.tc32Val = U'G';
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.tc32Val, U'H');
    sUnion.switch_to(219);
    sUnion.uVal.twcVal = L'J';
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.twcVal, L'K');

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test, MarshallMegaVarBasedUnionFloatingPointTypedef)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize("");
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::configuring);

    // Load the shared memory components
    LoadIPCModules(control);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaVarBasedUnion sUnion;
    sUnion.switch_to(220);
    float fVal = -1234.5678f;
    sUnion.uVal.tfVal = fVal;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.tfVal, fVal - 1000.000f);
    sUnion.switch_to(221);
    double dVal = 8765.4321;
    sUnion.uVal.tdVal = dVal;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.tdVal, dVal + 1000.000);
    // GCC issue with generated initialization in the consructor for the "long double" type: BUG #3982727
    //sUnion.switch_to(222);
    //long double ldVal = -1234.4321;
    //sUnion.uVal.tldVal = ldVal;
    //EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    //EXPECT_EQ(sUnion.uVal.tldVal, ldVal - 1000.000);

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test, MarshallMegaVarBasedUnionEnumTypedef)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize("");
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::configuring);

    // Load the shared memory components
    LoadIPCModules(control);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaVarBasedUnion sUnion;
    sUnion.switch_to(224);
    sUnion.uVal.teHelloVal = EHello::hello;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.teHelloVal, EHello::hallo);

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test, MarshallMegaVarBasedUnionStringTypedef)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize("");
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::configuring);

    // Load the shared memory components
    LoadIPCModules(control);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaVarBasedUnion sUnion;
    sUnion.switch_to(226);
    sUnion.uVal.tssVal = "hi";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.tssVal, "hij");
    sUnion.switch_to(227);
    sUnion.uVal.tss8Val = "huhu";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.tss8Val, "huhuv");
    sUnion.switch_to(228);
    sUnion.uVal.tss16Val = u"hey";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.tss16Val, u"heyz");
    sUnion.switch_to(229);
    sUnion.uVal.tss32Val = U"hello";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.tss32Val, U"hellop");
    sUnion.switch_to(230);
    sUnion.uVal.twssVal = L"servus";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.twssVal, L"servust");
    sUnion.switch_to(231);
    sUnion.uVal.tssFixVal = "hi";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.tssFixVal, "hij");
    sUnion.switch_to(232);
    sUnion.uVal.tss8FixVal = "huhu";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.tss8FixVal, "huhuv");
    sUnion.switch_to(233);
    sUnion.uVal.tss16FixVal = u"hey";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.tss16FixVal, u"heyz");
    sUnion.switch_to(234);
    sUnion.uVal.tss32FixVal = U"hello";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.tss32FixVal, U"hellop");
    sUnion.switch_to(235);
    sUnion.uVal.twssFixVal = L"servus";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.twssFixVal, L"servust");

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

// Using typedefs of template based types produced an error: BUG #398266
TEST(IPC_Communication_Test, DISABLED_MarshallMegaVarBasedUnionPointerTypedef)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize("");
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::configuring);

    // Load the shared memory components
    LoadIPCModules(control);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaVarBasedUnion sUnion;
    sUnion.switch_to(236);
    //sUnion.uVal.tptr8Val.resize(100);
    //for (size_t nIndex = 0; nIndex < sUnion.uVal.tptr8Val.size(); nIndex++)
    //    sUnion.uVal.tptr8Val[nIndex] = static_cast<uint8_t>(nIndex);
    //EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    //EXPECT_EQ(sUnion.uVal.tptr8Val.size(), 200);
    //for (size_t nIndex = 0; nIndex < sUnion.uVal.tptr8Val.size() / 2; nIndex++)
    //    EXPECT_EQ(sUnion.uVal.tptr8Val[nIndex], sUnion.uVal.tptr8Val[nIndex + sUnion.uVal.tptr8Val.size() / 2]);
    //sUnion.switch_to(237);
    //sUnion.uVal.tptrssVal.resize(3);
    //sUnion.uVal.tptrssVal[0] = "hello";
    //sUnion.uVal.tptrssVal[1] = "huhu";
    //sUnion.uVal.tptrssVal[2] = "hi";
    //EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    //EXPECT_EQ(sUnion.uVal.tptrssVal.size(), 6);
    //for (size_t nIndex = 0; nIndex < sUnion.uVal.tptrssVal.size() / 2; nIndex++)
    //    EXPECT_EQ(sUnion.uVal.tptrssVal[nIndex], sUnion.uVal.tptrssVal[nIndex + sUnion.uVal.tptrssVal.size() / 2]);
    //sUnion.switch_to(238);
    //sUnion.uVal.tptr8FixVal.resize(sUnion.uVal.tptr8FixVal.capacity());
    //for (size_t nIndex = 0; nIndex < sUnion.uVal.tptr8FixVal.size(); nIndex++)
    //    sUnion.uVal.tptr8FixVal[nIndex] = static_cast<uint8_t>(nIndex);
    //EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    //EXPECT_EQ(sUnion.uVal.tptr8FixVal.size(), 20);
    //for (size_t nIndex = 0; nIndex < sUnion.uVal.tptr8FixVal.size(); nIndex++)
    //    EXPECT_EQ(sUnion.uVal.tptr8FixVal[nIndex], static_cast<uint8_t>(nIndex << 1));
    //sUnion.switch_to(239);
    //sUnion.uVal.tptrssFixVal.resize(sUnion.uVal.tptrssFixVal.capacity());
    //sUnion.uVal.tptrssFixVal[0] = "hi";
    //sUnion.uVal.tptrssFixVal[1] = "hoi";
    //sUnion.uVal.tptrssFixVal[2] = "hello";
    //sUnion.uVal.tptrssFixVal[3] = "hallo";
    //sUnion.uVal.tptrssFixVal[4] = "servus";
    //EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    //EXPECT_EQ(sUnion.uVal.tptrssFixVal.size(), 5);
    //EXPECT_EQ(sUnion.uVal.tptrssFixVal[0], "hihi");
    //EXPECT_EQ(sUnion.uVal.tptrssFixVal[1], "hoihoi");
    //EXPECT_EQ(sUnion.uVal.tptrssFixVal[2], "hellohello");
    //EXPECT_EQ(sUnion.uVal.tptrssFixVal[3], "hallohallo");
    //EXPECT_EQ(sUnion.uVal.tptrssFixVal[4], "servusservus");

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

// Using typedefs of template based types produced an error: BUG #398266
TEST(IPC_Communication_Test, DISABLED_MarshallMegaVarBasedUnionSequenceTypedef)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize("");
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::configuring);

    // Load the shared memory components
    LoadIPCModules(control);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaVarBasedUnion sUnion;
    sUnion.switch_to(236);
    //sUnion.uVal.tseq8Val.resize(100);
    //for (size_t nIndex = 0; nIndex < sUnion.uVal.tseq8Val.size(); nIndex++)
    //    sUnion.uVal.tseq8Val[nIndex] = static_cast<uint8_t>(nIndex);
    //EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    //EXPECT_EQ(sUnion.uVal.tseq8Val.size(), 200);
    //for (size_t nIndex = 0; nIndex < sUnion.uVal.tseq8Val.size() / 2; nIndex++)
    //    EXPECT_EQ(sUnion.uVal.tseq8Val[nIndex], sUnion.uVal.tseq8Val[nIndex + sUnion.uVal.tseq8Val.size() / 2]);
    //sUnion.switch_to(237);
    //sUnion.uVal.tseqssVal.resize(3);
    //sUnion.uVal.tseqssVal[0] = "hello";
    //sUnion.uVal.tseqssVal[1] = "huhu";
    //sUnion.uVal.tseqssVal[2] = "hi";
    //EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    //EXPECT_EQ(sUnion.uVal.tseqssVal.size(), 6);
    //for (size_t nIndex = 0; nIndex < sUnion.uVal.tseqssVal.size() / 2; nIndex++)
    //    EXPECT_EQ(sUnion.uVal.tseqssVal[nIndex], sUnion.uVal.tseqssVal[nIndex + sUnion.uVal.tseqssVal.size() / 2]);
    //sUnion.switch_to(238);
    //sUnion.uVal.tseq8FixVal.resize(sUnion.uVal.tseq8FixVal.capacity());
    //for (size_t nIndex = 0; nIndex < sUnion.uVal.tseq8FixVal.size(); nIndex++)
    //    sUnion.uVal.tseq8FixVal[nIndex] = static_cast<uint8_t>(nIndex);
    //EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    //EXPECT_EQ(sUnion.uVal.tseq8FixVal.size(), 20);
    //for (size_t nIndex = 0; nIndex < sUnion.uVal.tseq8FixVal.size(); nIndex++)
    //    EXPECT_EQ(sUnion.uVal.tseq8FixVal[nIndex], static_cast<uint8_t>(nIndex << 1));
    //sUnion.switch_to(239);
    //sUnion.uVal.tseqssFixVal.resize(sUnion.uVal.tseqssFixVal.capacity());
    //sUnion.uVal.tseqssFixVal[0] = "hi";
    //sUnion.uVal.tseqssFixVal[1] = "hoi";
    //sUnion.uVal.tseqssFixVal[2] = "hello";
    //sUnion.uVal.tseqssFixVal[3] = "hallo";
    //sUnion.uVal.tseqssFixVal[4] = "servus";
    //EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    //EXPECT_EQ(sUnion.uVal.tseqssFixVal.size(), 5);
    //EXPECT_EQ(sUnion.uVal.tseqssFixVal[0], "hihi");
    //EXPECT_EQ(sUnion.uVal.tseqssFixVal[1], "hoihoi");
    //EXPECT_EQ(sUnion.uVal.tseqssFixVal[2], "hellohello");
    //EXPECT_EQ(sUnion.uVal.tseqssFixVal[3], "hallohallo");
    //EXPECT_EQ(sUnion.uVal.tseqssFixVal[4], "servusservus");

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test, MarshallMegaVarBasedUnionInterfaceTypedef)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize("");
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::configuring);

    // Load the shared memory components
    LoadIPCModules(control);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    struct SMultiply : public IMultiplyValue
    {
        size_t Multiply(size_t n)
        {
            return n * n;
        }
    } sMultiply;

    ASSERT_NE(pMegaTest, nullptr);
    SMegaVarBasedUnion sUnion;
    sUnion.switch_to(225);

    sUnion.uVal.tidVal = sdv::GetInterfaceId<IMultiplyValue>();
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.tidVal, sdv::GetInterfaceId<IAddValue>());
    sUnion.switch_to(244);
    sUnion.uVal.tifcVal = static_cast<IMultiplyValue*>(&sMultiply);
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.tifcVal.id(), sdv::GetInterfaceId<IAddValue>());
    ASSERT_NE(sUnion.uVal.tifcVal, nullptr);
    ASSERT_NE(sUnion.uVal.tifcVal.get<IAddValue>(), nullptr);
    EXPECT_EQ(sUnion.uVal.tifcVal.get<IAddValue>()->Add(99), 19602); // 99 * 99 + 99 * 99
    // Typedef of interfaces current not possible. BUG #399464
    //sUnion.switch_to(245);
    //sUnion.uVal.tpMultiplyValue = &sMultiply;
    //EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    //ASSERT_NE(sUnion.uVal.tpMultiplyValue, nullptr);
    //EXPECT_EQ(sUnion.uVal.tpMultiplyValue->Multiply(5), 625); // 5 * 5 * 5 * 5

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test, MarshallMegaVarBasedUnionSubCompoundTypedef)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize("");
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetStatus(), sdv::EObjectStatus::configuring);

    // Load the shared memory components
    LoadIPCModules(control);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaVarBasedUnion sUnion;
    sUnion.switch_to(246);
    sUnion.uVal.tsSubVal.i = 1;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.tsSubVal.i, 2);
    sUnion.switch_to(247);
    sUnion.uVal.tsIndVal.i = 10;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.tsIndVal.i, 20);

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}
