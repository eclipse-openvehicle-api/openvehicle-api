#include "stub_generator.h"
#include "../exception.h"
#include <fstream>

CStubGenerator::CStubGenerator(sdv::IInterfaceAccess* pParser) : CPSClassGeneratorBase(pParser)
{}

CStubGenerator::~CStubGenerator()
{}

std::string CStubGenerator::GetNameAppendix() const
{
    return "stub";
}

std::string CStubGenerator::GetClassDefFileComments() const
{
    return "This file contains the stub definition for the interfaces.";
}

std::string CStubGenerator::GetClassImplFileComments() const
{
    return "This file contains the stub implementation for the interfaces.";
}

std::string CStubGenerator::GetClassDefBegin(CKeywordMap& /*rmapKeywords*/) const
{
    return R"code(
/**
 * @brief Proxy class implementation for the %interface_name%.
 */
class %class_name% : public sdv::ps::CStubHandler<%interface_name%>, public sdv::ps::IStubLink
{
public:
    /**
     * @brief Constructor
     * @param[in] pInterface Pointer to the target interface this proxy has to operate. Must not be NULL.
     */
    %class_name%();

    /**
     * @brief Destructor
     */
    virtual ~%class_name%() override = default;

    // Object class name
    DECLARE_OBJECT_CLASS_NAME("Stub_%interface_id%")
    DECLARE_OBJECT_CLASS_ALIAS("Stub_%alias_name%")

    // Interface map
    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_CHAIN_BASE(sdv::ps::CStubHandler<%interface_name%>)
        SDV_INTERFACE_ENTRY(IStubLink)
    END_SDV_INTERFACE_MAP()

    /**
     * @brief Link the object target interface to the stub-object. Overload of IStubLink::Link.
     * @remarks Only one link can exists at the time.
     * @param[in] pInterface Interface to be linked.
     */
    void Link(/*in*/ sdv::interface_t ifc) override;

    /**
     * @brief Unlink the linked interface. Overload of IStubLink::Unlink.
     */
    void Unlink() override;
)code";
}

std::string CStubGenerator::GetClassDefEnd(CKeywordMap& /*rmapKeywords*/) const
{
    return R"code(
private:
    %interface_name%* m_ifc = nullptr; ///< Pointer to the marshalled interface.
};
DEFINE_SDV_OBJECT(%class_name%)
)code";
}

std::string CStubGenerator::GetConstructImplBegin(CKeywordMap& /*rmapKeywords*/) const
{
    return R"code(
%class_name%::%class_name%()
{
)code";
}

std::string CStubGenerator::GetConstructImplEnd(CKeywordMap& /*rmapKeywords*/) const
{
    return R"code(
}

void %class_name%::Link(/*in*/ sdv::interface_t ifc)
{
    m_ifc = ifc.template get<%interface_name%>();
    assert(m_ifc);
}

void %class_name%::Unlink()
{
    m_ifc = nullptr;
}
)code";
}

std::string CStubGenerator::GetConstructFuncImpl(const SFuncInfo& /*rsFunc*/, CKeywordMap& /*rmapKeywords*/) const
{
    return R"code(
    RegisterDispatchFunc([this](sdv::EEndian eEndian, const sdv::pointer<uint8_t>& rptrInputParams, sdv::pointer<uint8_t>& rptrOutputParams) -> sdv::ps::ECallResult
    {
        return stub_%func_name%(eEndian, rptrInputParams, rptrOutputParams);
    });
)code";
}

std::string CStubGenerator::GetFuncDef(const SFuncInfo& /*rsFunc*/, CKeywordMap& /*rmapKeywords*/) const
{
    return R"code(
    /** Implementation of stub_%func_name%. */
    sdv::ps::ECallResult stub_%func_name%(sdv::EEndian eEndian, const sdv::pointer<uint8_t>& rptrInputParams, sdv::pointer<uint8_t>&rptrOutputParams);
)code";
}

std::string CStubGenerator::GetFuncImpl(const SFuncInfo& rsFunc, CKeywordMap& rmapKeywords, const CExceptionVector& rvecExceptions) const
{
    rmapKeywords.insert(std::make_pair("func_return", rsFunc.ssDeclType != "void" ? "return_value = " : ""));
    rmapKeywords.insert(std::make_pair("argument_endianess", rsFunc.nInputParamCnt || rsFunc.nOutputParamCnt || rvecExceptions.size() ? " eEndian" : ""));
    rmapKeywords.insert(std::make_pair("argument_input_params", rsFunc.nInputParamCnt ? " rptrInputParams" : ""));
    rmapKeywords.insert(std::make_pair("argument_output_params", rsFunc.nOutputParamCnt || rvecExceptions.size() ? " rptrOutputParams" : ""));
    rmapKeywords.insert(std::make_pair("param_init_comments", rsFunc.nInputParamCnt || rsFunc.nOutputParamCnt ? R"code(

    // Initialize parameters
    // CppCheck warns about parameter that could be declared as const. This is not wanted here. Suppress the warning.
    // cppcheck-suppress constVariablePointer)code" : ""));

    std::stringstream sstream;
    sstream << R"code(
sdv::ps::ECallResult %class_name%::stub_%func_name%(sdv::EEndian%argument_endianess%, const sdv::pointer<uint8_t>&%argument_input_params%, sdv::pointer<uint8_t>&%argument_output_params%)
{
    if (!m_ifc) throw sdv::XNoInterface(); // Error, interface must be assigned.%param_init_comments%%param_init%)code";
    if (rsFunc.nInputParamCnt)
        sstream << R"code(

    // Deserialize parameters
    if (eEndian == sdv::EEndian::big_endian)
    {
        sdv::deserializer<sdv::EEndian::big_endian> desInput;
        desInput.attach(rptrInputParams);%stream_param_input%
    } else
    {
        sdv::deserializer<sdv::EEndian::little_endian> desInput;
        desInput.attach(rptrInputParams);%stream_param_input%
    })code";

    // Call function. Add try/catch if exceptions are used.
    if (rvecExceptions.size())
    {
        sstream << R"code(

    try
    {
        // Call the function
        %func_return%m_ifc->%func_name%(%param_pack_use%);
    })code";
        for (const std::string& rssException : rvecExceptions)
        {
            sstream << " catch (const " << rssException << "& rexcept)";
            sstream << R"code(
    {
        if (eEndian == sdv::EEndian::big_endian)
        {
            sdv::serializer<sdv::EEndian::big_endian> serOutput;
            serOutput << rexcept;
            rptrOutputParams = std::move(serOutput.buffer());
            return sdv::ps::ECallResult::result_exception;
        } else
        {
            sdv::serializer<sdv::EEndian::little_endian> serOutput;
            serOutput << rexcept;
            rptrOutputParams = std::move(serOutput.buffer());
            return sdv::ps::ECallResult::result_exception;
        }
    })code";
        }
    } else
    {
        sstream << R"code(

    // Call the function
    %func_return%m_ifc->%func_name%(%param_pack_use%);)code";
    }

    if (rsFunc.nOutputParamCnt) sstream << R"code(

    // Serializer
    if (eEndian == sdv::EEndian::big_endian)
    {
        sdv::serializer<sdv::EEndian::big_endian> serOutput;%stream_param_output%
        rptrOutputParams = std::move(serOutput.buffer());
        return sdv::ps::ECallResult::result_ok;
    } else
    {
        sdv::serializer<sdv::EEndian::little_endian> serOutput;%stream_param_output%
        rptrOutputParams = std::move(serOutput.buffer());
        return sdv::ps::ECallResult::result_ok;
    })code";
    else
        sstream << R"code(

    return sdv::ps::ECallResult::result_ok;)code";
    sstream << R"code(
}
)code";
    return sstream.str();
}

std::string CStubGenerator::GetFuncImplParamInit(const SFuncInfo& /*rsFunc*/, const SParamInfo& rsParam, CKeywordMap& /*rmapKeywords*/) const
{
    if (!rsParam.bValidType) return {};

    // Do not initialize the return value; this will be initialized later.
    if (rsParam.ssName == "return_value")
        return R"code(
    %param_decl_type% %param_name%;)code";
    else
        return R"code(
    %param_decl_type% %param_name% = %param_default_val%;)code";
}

std::string CStubGenerator::GetFuncImplStreamParamInput(const SFuncInfo& /*rsFunc*/, const SParamInfo& rsParam, CKeywordMap& /*rmapKeywords*/) const
{
    if (!rsParam.bValidType) return "";
    switch (rsParam.eDirection)
    {
    case SParamInfo::EDirection::in:
    case SParamInfo::EDirection::inout:
        return R"code(
        desInput >> %param_name%;)code";
    default:
        return "";
    }
}

std::string CStubGenerator::GetFuncImplStreamParamOutput(const SFuncInfo& /*rsFunc*/, const SParamInfo& rsParam, CKeywordMap& /*rmapKeywords*/) const
{
    if (!rsParam.bValidType) return "";
    switch (rsParam.eDirection)
    {
    case SParamInfo::EDirection::ret:
    case SParamInfo::EDirection::out:
    case SParamInfo::EDirection::inout:
        return R"code(
        serOutput << %param_name%;)code";
    default:
        return "";
    }
}

std::string CStubGenerator::GetFuncImplParamTerm(const SFuncInfo& /*rsFunc*/, const SParamInfo& /*rsParam*/, CKeywordMap& /*rmapKeywords*/) const
{
    return std::string();
}
