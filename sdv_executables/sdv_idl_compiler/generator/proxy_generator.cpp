#include "proxy_generator.h"
#include "../exception.h"
#include <fstream>

CProxyGenerator::CProxyGenerator(sdv::IInterfaceAccess* pParser) : CPSClassGeneratorBase(pParser)
{}

CProxyGenerator::~CProxyGenerator()
{}

std::string CProxyGenerator::GetNameAppendix() const
{
    return "proxy";
}

std::string CProxyGenerator::GetClassDefFileComments() const
{
    return "This file contains the proxy definition for the interfaces.";
}

std::string CProxyGenerator::GetClassImplFileComments() const
{
    return "This file contains the proxy implementation for the interfaces.";
}

std::string CProxyGenerator::GetClassDefBegin(CKeywordMap& /*rmapKeywords*/) const
{
    return R"code(
/**
 * @brief Proxy class implementation for the %interface_name%.
 */
class %class_name% : public sdv::ps::CProxyHandler<%interface_name%>, public sdv::ps::IProxyControl
{
public:
    /**
     * @brief Constructor
     */
    %class_name%();

    /**
     * @brief Destructor
     */
    virtual ~%class_name%() override = default;

    // Object class name
    DECLARE_OBJECT_CLASS_NAME("Proxy_%interface_id%")
    DECLARE_OBJECT_CLASS_ALIAS("Proxy_%alias_name%")

    // Interface map
    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_CHAIN_BASE(sdv::ps::CProxyHandler<%interface_name%>)
        SDV_INTERFACE_ENTRY(IProxyControl)
    END_SDV_INTERFACE_MAP()

    /**
     * @brief Get the target interface from the proxy object. Overload of sdv::ps::IProxyControl::GetTargetInterface.
     * @return The target interface.
     */
    sdv::interface_t GetTargetInterface() override;

    /**
     * @brief Direct access to the target interface from the proxy object.
     * @return Reference to the target interface.
     */
    %interface_name%& Access();

private:
    /**
     * @brief Interface access implementation
     */
    class CInterfaceAccess : public %interface_name%
    {
    public:
        /**
         * @brief Constructor
         */
        CInterfaceAccess(%class_name%& rHandler);

        /**
         * @brief Destructor
         */
        ~CInterfaceAccess() = default;
)code";
}

std::string CProxyGenerator::GetClassDefEnd(CKeywordMap& /*rmapKeywords*/) const
{
    return R"code(
    private:
        %class_name%&   m_rHandler; ///< Proxy handler class.
    };

    CInterfaceAccess    m_access;   ///< Interface access object.
};
DEFINE_SDV_OBJECT(%class_name%)
)code";
}

std::string CProxyGenerator::GetConstructImplBegin(CKeywordMap& /*rmapKeywords*/) const
{
    return R"code(
%class_name%::%class_name%() : m_access(*this)
{}

sdv::interface_t %class_name%::GetTargetInterface()
{
    return &Access();
}

%interface_name%& %class_name%::Access()
{
    return m_access;
}

%class_name%::CInterfaceAccess::CInterfaceAccess(%class_name%& rHandler) : m_rHandler(rHandler)
{
)code";
}

std::string CProxyGenerator::GetConstructImplEnd(CKeywordMap& /*rmapKeywords*/) const
{
    return R"code(}
)code";
}

std::string CProxyGenerator::GetConstructFuncImpl(const SFuncInfo& /*rsFunc*/, CKeywordMap& /*rmapKeywords*/) const
{
    // The proxy doesn't implement construction code.
    return std::string();
}

std::string CProxyGenerator::GetFuncDef(const SFuncInfo& rsFunc, CKeywordMap& rmapKeywords) const
{
    rmapKeywords.insert(std::make_pair("func_const", rsFunc.bIsConst ? " const" : ""));
    return R"code(
        /** Implementation of %func_name%. */
        virtual %func_decl_type% %func_name%(%param_pack_def%)%func_const% override;
)code";
}

std::string CProxyGenerator::GetFuncImpl(const SFuncInfo& rsFunc, CKeywordMap& rmapKeywords, const CExceptionVector& rvecExceptions) const
{
    rmapKeywords.insert(std::make_pair("func_const", rsFunc.bIsConst ? " const" : ""));
    rmapKeywords.insert(std::make_pair("retval_init_comment", rsFunc.ssDecl != "void" ? R"code(

    // Initialize return value)code" : ""));
    rmapKeywords.insert(std::make_pair("call_return", rsFunc.nOutputParamCnt ? " = " : ""));
    rmapKeywords.insert(std::make_pair("deserialize", rsFunc.nOutputParamCnt ? R"code(// Deserialize output parameters)code" : ""));
    rmapKeywords.insert(std::make_pair("return_from_func", rsFunc.ssDecl != "void" ? R"code(
        return return_value;)code" : ""));

    std::stringstream sstreamExceptions;
    sstreamExceptions << R"code(// Fire serialized exceptions caught during the call
        sdv::exception_id except_id = 0;
        desOutput.peek_front(except_id);
        )code";
    if (!rvecExceptions.empty())
        sstreamExceptions << R"code(switch (except_id)
        {
        )code";
    else
        sstreamExceptions << R"code(sdv::XUnknownException exception;
        exception.unknown_id = except_id;
        throw exception;)code";
    for (const std::string& rssException : rvecExceptions)
    {
        sstreamExceptions << "case sdv::GetExceptionId<" << rssException << R"code(>():
        {
            )code" << rssException << R"code( exception;
            desOutput >> exception;
            throw exception;
        }
        )code";
    }
    if (!rvecExceptions.empty())
        sstreamExceptions << R"code(default:
        {
            sdv::XUnknownException exception;
            exception.unknown_id = except_id;
            throw exception;
        }
        })code";
    rmapKeywords.insert(std::make_pair("exception_handling", sstreamExceptions.str()));

    return R"code(
%func_decl_type% %class_name%::CInterfaceAccess::%func_name%(%param_pack_def%)%func_const%
{
    // Clear raw data bypass (needed for streaming large data).
    sdv::ps::GetRawDataBypass().clear();%retval_init_comment%%param_init%

    // Serialize input parameters
    sdv::serializer serInput;%stream_param_input%

    // Execute a call to the interface stub.
    sdv::deserializer desOutput;
    sdv::ps::ECallResult eResult = m_rHandler.DoCall(%func_index%, serInput, desOutput);
    if (eResult == sdv::ps::ECallResult::result_ok)
    {
        %deserialize%%stream_param_output%%return_from_func%
    } else if (eResult == sdv::ps::ECallResult::result_exception)
    {
        %exception_handling%
    } else
        throw sdv::ps::XMarshallIntegrity();
}
)code";
}

std::string CProxyGenerator::GetFuncImplParamInit(const SFuncInfo& /*rsFunc*/, const SParamInfo& rsParam, CKeywordMap& /*rmapKeywords*/) const
{
    return (rsParam.eDirection == SParamInfo::EDirection::ret && rsParam.bValidType) ?
        R"code(
    %param_decl_type% return_value = %param_default_val%;)code" : "";
}

std::string CProxyGenerator::GetFuncImplStreamParamInput(const SFuncInfo& /*rsFunc*/, const SParamInfo& rsParam, CKeywordMap& /*rmapKeywords*/) const
{
    switch (rsParam.eDirection)
    {
    case SParamInfo::EDirection::inout:
    case SParamInfo::EDirection::in:
        return R"code(
    serInput << %param_name%;)code";
        break;
    default:
        return std::string();
        break;
    }
}

std::string CProxyGenerator::GetFuncImplStreamParamOutput(const SFuncInfo& /*rsFunc*/, const SParamInfo& rsParam, CKeywordMap& /*rmapKeywords*/) const
{
	switch (rsParam.eDirection)
	{
    case SParamInfo::EDirection::ret:
        return R"code(
        desOutput >> return_value;)code";
		break;
	case SParamInfo::EDirection::inout:
	case SParamInfo::EDirection::out:
        return R"code(
        desOutput >> %param_name%;)code";
		break;
	default:
		break;
	}
    return std::string();
}

std::string CProxyGenerator::GetFuncImplParamTerm(const SFuncInfo& /*rsFunc*/, const SParamInfo& /*rsParam*/, CKeywordMap& /*rmapKeywords*/) const
{
	return std::string();
}

