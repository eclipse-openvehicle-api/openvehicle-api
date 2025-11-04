#include "vss_bs_codingtx.h"

void CVSSBSCodingTX::GetKeyWordMap(const SSignalBSDefinition& signal, const SSignalVDDefinition& signalVD, CKeywordMap& mapKeywords) const
{
    std::string vd_vssWithColons = ReplaceCharacters(signal.vssVDDefinition, ".", "::");
    std::string vssWithColons = ReplaceCharacters(signal.vssDefinition, ".", "::");

    //------------------------------------------------------- basic service header
    std::stringstream sstreamIncludesBS;
    std::string vd_vssNoDot = ReplaceCharacters(signal.vssVDDefinition, ".", "");        
    std::string vssNoDot = ReplaceCharacters(signal.vssDefinition, ".", "");   
    std::transform(vd_vssNoDot.begin(), vd_vssNoDot.end(), vd_vssNoDot.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    std::transform(vssNoDot.begin(), vssNoDot.end(), vssNoDot.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    sstreamIncludesBS << Code_BS_TXIncludes(vd_vssNoDot, vssNoDot);
    mapKeywords["tx_bs_includes_list"] = std::move(sstreamIncludesBS.str());

    std::stringstream sstreamInterfacesBS;
    for (const auto& func : signal.vecFunctions)
    {
        sstreamInterfacesBS << Code_BS_TXInterface(func.functionName, vssWithColons);
    }
    mapKeywords["tx_bs_interface_list"] = std::move(sstreamInterfacesBS.str());

    std::stringstream sstreamInterfaceEntriesBS;
    for (const auto& func : signal.vecFunctions)
    {
        sstreamInterfaceEntriesBS << Code_BS_TXInterfaceEntry(func.functionName, vssWithColons);
    }
    mapKeywords["tx_bs_interface_entry_list"] = std::move(sstreamInterfaceEntriesBS.str());

    std::stringstream sstreamFunctionsBS;
    for (const auto& func : signal.vecFunctions)
    {
        sstreamFunctionsBS << Code_BS_TXFunction(func);
    }
    mapKeywords["tx_bs_function_list"] = std::move(sstreamFunctionsBS.str());

    std::stringstream sstreamVariablePointers;
    for (const auto& func : signalVD.vecFunctions)
    {
        sstreamVariablePointers << Code_BS_TXVariablePointerFromVD(func, vd_vssWithColons);
    }
    mapKeywords["tx_bs_variable_pointer_list"] = std::move(sstreamVariablePointers.str());

    //------------------------------------------------------- basic service cpp
    std::stringstream sstreamVariablePointerInitializations;
    for (const auto& func : signalVD.vecFunctions)
    {
        sstreamVariablePointerInitializations << Code_BS_TXVariablePointerInitializationFromVD(func,
            signal.className, signal.vssDefinition, vd_vssWithColons);
    }
    mapKeywords["tx_bs_variable_pointer_init_list"] = std::move(sstreamVariablePointerInitializations.str());

    std::stringstream sstreamPointerFunctionImplementation;
    for (uint32_t index = 0; index < signal.vecFunctions.size(); index++)
    {
        if (index >= signalVD.vecFunctions.size())
            break;
        auto func = signal.vecFunctions[index];
        auto funcVD = signalVD.vecFunctions[index];
        sstreamPointerFunctionImplementation << Code_BS_TXFunctionPointerImplementationFromVD(func, funcVD, signal.className);
    }
    mapKeywords["vd_tx_pointer_function_implementations"] = std::move(sstreamPointerFunctionImplementation.str());
}

std::string CVSSBSCodingTX::Code_BS_TXIDLList(const std::vector<std::string>& vssParts,
    const std::vector <SFunctionBSDefinition>& vecFunctions) const
{
    int8_t loop = 1;
    std::string nameSpace = "module vss\n{\n";
    for (auto  it = vssParts.begin(); it != vssParts.end(); ++it)
    {
        nameSpace.append(static_cast <int8_t>(4) * loop, ' ');
        nameSpace.append("module ");
        nameSpace.append(*it);
        if (it == vssParts.end() - 1)
        {
            nameSpace.append("Service");
        }
        nameSpace.append("\n");
        nameSpace.append(static_cast <int8_t>(4) * loop++, ' ');
        nameSpace.append("{\n");
    }

    nameSpace.append("%interfaces%");
    loop--;
    for (size_t i = 0; i < vssParts.size() + 1; i++)
    {
        nameSpace.append(static_cast <int8_t>(4) * loop--, ' ');
        nameSpace.append("};\n");
    }

    bool newLine = false;
    std::string spaces = "";
    spaces.append(static_cast <int8_t>(4) * (vssParts.size() + 1), ' ');
    std::stringstream sstreamInterfaces;
    for (const auto&  func : vecFunctions)
    {
        if (newLine)
            sstreamInterfaces << "\n\n";
        else
            newLine = true;
        sstreamInterfaces << Code_BS_TXIDLInterface(spaces, func);
    }
    CKeywordMap mapKeywords;
    mapKeywords["interfaces"] = std::move(sstreamInterfaces.str());

    return ReplaceKeywords(nameSpace, mapKeywords);
}

std::string CVSSBSCodingTX::Code_BS_TXIncludes(const std::string& vd_vssOriginalNoDot, const std::string& vssOriginalNoDot) const
{
    CKeywordMap mapKeywords;
    mapKeywords["vd_vss_original_no_dot"] = vd_vssOriginalNoDot;
    mapKeywords["vss_original_no_dot"] = vssOriginalNoDot;    

    return ReplaceKeywords(R"code(#include "../vss_%vd_vss_original_no_dot%_vd_tx.h"
#include "../vss_%vss_original_no_dot%_bs_tx.h"
)code", mapKeywords);
}

std::string CVSSBSCodingTX::Code_BS_TXInterface(const std::string& functionName, const std::string& vssWithColons) const
{
    CKeywordMap mapKeywords;
    mapKeywords["function_name"] = functionName;
    mapKeywords["vss_with_colons"] = vssWithColons;

    return ReplaceKeywords(R"code(	, public vss::%vss_with_colons%Service::IVSS_Set%function_name%
)code", mapKeywords);
}

std::string CVSSBSCodingTX::Code_BS_TXInterfaceEntry(const std::string& functionName, const std::string& vssWithColons) const
{
    CKeywordMap mapKeywords;
    mapKeywords["function_name"] = functionName;
    mapKeywords["vss_with_colons"] = vssWithColons;

    return ReplaceKeywords(R"code(		SDV_INTERFACE_ENTRY(vss::%vss_with_colons%Service::IVSS_Set%function_name%)
)code", mapKeywords);
}

std::string CVSSBSCodingTX::Code_BS_TXIDLInterface(const std::string& spaces, const SFunctionBSDefinition& function) const
{
    CKeywordMap mapKeywords;
    mapKeywords["function_name"] = function.functionName;
    mapKeywords["signal_name"] = function.signalName;
    mapKeywords["value_idltype"] = function.idlType;
    mapKeywords["value_ctype"] = GetCTypeFromIDLType(function.idlType);
    mapKeywords["multiple_spaces"] = spaces;

    return ReplaceKeywords(R"code(%multiple_spaces%/**
%multiple_spaces%* @brief IVSS_Set%function_name% Service interface
%multiple_spaces%*/
%multiple_spaces%interface IVSS_Set%function_name%
%multiple_spaces%{
%multiple_spaces%    /**
%multiple_spaces%    * @brief Set %signal_name% signal
%multiple_spaces%    * @param[in] value
%multiple_spaces%    * @return true on success otherwise false
%multiple_spaces%    */
%multiple_spaces%    boolean Set%function_name%(in %value_idltype% value);
%multiple_spaces%};
)code", mapKeywords);
}

std::string CVSSBSCodingTX::Code_BS_TXFunction(const SFunctionBSDefinition& function) const
{
    CKeywordMap mapKeywords;
    mapKeywords["function_name"] = function.functionName;
    mapKeywords["signal_name"] = function.signalName;
    mapKeywords["value_idltype"] = function.idlType;
    auto signalType = GetCTypeFromIDLType(function.idlType);
    mapKeywords["value_ctype"] = CastValueType(signalType);

    return ReplaceKeywords(R"code(
	/**
	 * @brief Set %signal_name% signal
	 * @param[in] value 
     * @return true on success otherwise false
	 */
	bool Set%function_name%(%value_ctype% value) override;
)code", mapKeywords);
}

std::string CVSSBSCodingTX::Code_BS_TXVariablePointerFromVD(const SFunctionVDDefinition& functionVD, 
    const std::string& vd_vssWithColons) const
{
    CKeywordMap mapKeywords;
    mapKeywords["vd_function_name"] = functionVD.functionName;
    mapKeywords["vd_vss_with_colons"] = vd_vssWithColons;

    return ReplaceKeywords(R"code(
	vss::%vd_vss_with_colons%Device::IVSS_Write%vd_function_name%* m_ptr%vd_function_name% = nullptr;)code", mapKeywords);
}

std::string CVSSBSCodingTX::Code_BS_TXFunctionPointerImplementationFromVD(const SFunctionBSDefinition& function, 
    const SFunctionVDDefinition& functionVD, const std::string& className) const
{
    std::string class_name_lowercase = className;
    std::transform(class_name_lowercase.begin(), class_name_lowercase.end(), class_name_lowercase.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

    CKeywordMap mapKeywords;
    mapKeywords["function_name"] = function.functionName;
    mapKeywords["vd_function_name"] = functionVD.functionName;
    mapKeywords["class_name"] = className;
    mapKeywords["value_idltype"] = function.idlType;
    auto signalType = GetCTypeFromIDLType(function.idlType);
    mapKeywords["value_ctype"] = CastValueType(signalType);
    mapKeywords["class_name_lowercase"] = class_name_lowercase;

    return ReplaceKeywords(R"code(
/**
 * @brief %function_name%
 * @param[in] value
 * @return true on success otherwise false 
 */
bool CBasicService%class_name%::Set%function_name%(%value_ctype% value)
{
	return m_ptr%vd_function_name%->Write%vd_function_name%(value);
}
)code", mapKeywords);
}

std::string CVSSBSCodingTX::Code_BS_TXVariablePointerInitializationFromVD(const SFunctionVDDefinition& functionVD, 
    const std::string& className, const std::string& vssOriginal, const std::string& vd_vssWithColons) const
{  
    std::string class_name_lowercase = className;
    std::transform(class_name_lowercase.begin(), class_name_lowercase.end(), class_name_lowercase.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

    CKeywordMap mapKeywords;
    mapKeywords["vd_function_name"] = functionVD.functionName;
    mapKeywords["class_name"] = className;
    mapKeywords["vd_vss_original"] = vssOriginal;
    mapKeywords["vd_vss_with_colons"] = vd_vssWithColons;
    mapKeywords["class_name_lowercase"] = class_name_lowercase;

    return ReplaceKeywords(R"code(
	m_ptr%vd_function_name% = sdv::core::GetObject("%vd_vss_original%_Device").GetInterface<vss::%vd_vss_with_colons%Device::IVSS_Write%vd_function_name%>();
	if (!m_ptr%vd_function_name%)
	{
		SDV_LOG_ERROR("Could not get interface 'IVSS_Write%vd_function_name%': [CBasicService%class_name%]");
		throw std::runtime_error("%vd_function_name% device not found");
	}
)code", mapKeywords);
}
