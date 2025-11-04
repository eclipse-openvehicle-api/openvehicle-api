#include "vss_vd_codingtx.h"

void CVSSVDCodingTX::GetKeyWordMap(const SSignalVDDefinition& signal, CKeywordMap& mapKeywords) const
{
    std::string vssWithColons = ReplaceCharacters(signal.vssDefinition, ".", "::");

    //------------------------------------------------------- vehicle device header
    std::stringstream sstreamIncludesVD;
    std::string vssNoDot = ReplaceCharacters(signal.vssDefinition, ".", "");
    std::transform(vssNoDot.begin(), vssNoDot.end(), vssNoDot.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    sstreamIncludesVD << Code_VD_TXIncludes(vssNoDot);
    mapKeywords["tx_vd_includes_list"] = std::move(sstreamIncludesVD.str());

    std::stringstream sstreamInterfacesVD;
    for (const auto& func : signal.vecFunctions)
    {
        sstreamInterfacesVD << Code_VD_TXInterface(func.functionName, vssWithColons);
    }
    mapKeywords["tx_vd_interface_list"] = std::move(sstreamInterfacesVD.str());

    std::stringstream sstreamInterfaceEntriesVD;
    for (const auto& func : signal.vecFunctions)
    {
        sstreamInterfaceEntriesVD << Code_VD_TXInterfaceEntry(func.functionName, vssWithColons);
    }
    mapKeywords["tx_vd_interface_entry_list"] = std::move(sstreamInterfaceEntriesVD.str());

    std::stringstream sstreamFunctionsVD;
    for (const auto& func : signal.vecFunctions)
    {
        sstreamFunctionsVD << Code_VD_TXFunction(func);
    }
    mapKeywords["tx_vd_function_list"] = std::move(sstreamFunctionsVD.str());

    std::stringstream sstreamVariables;
    for (const auto& func : signal.vecFunctions)
    {
        sstreamVariables << Code_VD_TXVariable(func.signalName);
    }
    mapKeywords["tx_variable_list"] = std::move(sstreamVariables.str());

    //------------------------------------------------------- vehicle device cpp
    std::stringstream sstreamVariableInitializations;
    for (const auto& func : signal.vecFunctions)
    {
        sstreamVariableInitializations << Code_VD_TXVariableInitialization(func);
    }
    mapKeywords["tx_variable_init_list"] = std::move(sstreamVariableInitializations.str());

    if (signal.vecFunctions.size() > 0)
    {
        mapKeywords["tx_variable_check_list"] = Code_VD_TXVariableCheckInitialization(signal.vecFunctions);
    }

    std::stringstream sstreamResetSignal;
    for (const auto& func : signal.vecFunctions)
    {
        sstreamResetSignal << Code_VD_TXResetSignal(func);
    }
    mapKeywords["tx_reset_signals"] = std::move(sstreamResetSignal.str());

    std::stringstream sstreamFunctionImplementation;
    for (const auto& func : signal.vecFunctions)
    {
        sstreamFunctionImplementation << Code_VD_TXFunctionImplementation(func, signal.className);
    }
    mapKeywords["tx_function_implementations"] = std::move(sstreamFunctionImplementation.str());
}

std::string CVSSVDCodingTX::Code_VD_TXIDLList(const std::vector<std::string>& vssParts,
    const std::vector <SFunctionVDDefinition>& vecFunctions) const
{
    int8_t loop = 1;
    std::string nameSpace = "module vss\n{\n";
    for (auto it = vssParts.begin(); it != vssParts.end(); ++it)
    {
        nameSpace.append(static_cast <int8_t>(4) * loop, ' ');
        nameSpace.append("module ");
        nameSpace.append(*it);
        if (it == vssParts.end() - 1)
        {
            nameSpace.append("Device");
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
    for (auto func : vecFunctions)
    {
        if (newLine)
            sstreamInterfaces << "\n\n";
        else
            newLine = true;
        sstreamInterfaces << Code_VD_TXIDLInterface(spaces, func);
    }
    CKeywordMap mapKeywords;
    mapKeywords["interfaces"] = std::move(sstreamInterfaces.str());

    return ReplaceKeywords(nameSpace, mapKeywords);
}

std::string CVSSVDCodingTX::Code_VD_TXIncludes(const std::string& vssOriginalNoDot) const
{
    CKeywordMap mapKeywords;
    mapKeywords["vss_original_no_dot"] = vssOriginalNoDot;

    return ReplaceKeywords(R"code(#include "../vss_%vss_original_no_dot%_vd_tx.h")code", mapKeywords);
}

std::string CVSSVDCodingTX::Code_VD_TXInterface(const std::string& functionName, const std::string& vssWithColons) const
{
    CKeywordMap mapKeywords;
    mapKeywords["function_name"] = functionName;
    mapKeywords["vss_with_colons"] = vssWithColons;

    return ReplaceKeywords(R"code(	, public vss::%vss_with_colons%Device::IVSS_Write%function_name%
)code", mapKeywords);
}

std::string CVSSVDCodingTX::Code_VD_TXInterfaceEntry(const std::string& functionName, const std::string& vssWithColons) const
{
    CKeywordMap mapKeywords;
    mapKeywords["function_name"] = functionName;
    mapKeywords["vss_with_colons"] = vssWithColons;

    return ReplaceKeywords(R"code(		SDV_INTERFACE_ENTRY(vss::%vss_with_colons%Device::IVSS_Write%function_name%)
)code", mapKeywords);
}

std::string CVSSVDCodingTX::Code_VD_TXIDLInterface(const std::string& spaces, const SFunctionVDDefinition& function) const
{
    CKeywordMap mapKeywords;
    mapKeywords["function_name"] = function.functionName;
    mapKeywords["signal_name"] = function.signalName;
    mapKeywords["value_idltype"] = function.idlType;
    mapKeywords["value_ctype"] = GetCTypeFromIDLType(function.idlType);
    mapKeywords["multiple_spaces"] = spaces;

    return ReplaceKeywords(R"code(%multiple_spaces%/**
%multiple_spaces%* @brief IVSS_Write%function_name% Device interface
%multiple_spaces%*/
%multiple_spaces%interface IVSS_Write%function_name%
%multiple_spaces%{
%multiple_spaces%    /**
%multiple_spaces%    * @brief Write %signal_name% signal
%multiple_spaces%    * @param[in] value
%multiple_spaces%    * @return true on success otherwise false
%multiple_spaces%    */
%multiple_spaces%    boolean Write%function_name%(in %value_idltype% value);
%multiple_spaces%};
)code", mapKeywords);
}

std::string CVSSVDCodingTX::Code_VD_TXFunction(const SFunctionVDDefinition& function) const
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
	bool Write%function_name%(%value_ctype% value) override;
)code", mapKeywords);
}

std::string CVSSVDCodingTX::Code_VD_TXResetSignal(const SFunctionVDDefinition& function) const
{
    CKeywordMap mapKeywords;
    mapKeywords["signal_name"] = function.signalName;

    return ReplaceKeywords(R"code(    m_%signal_name%.Reset();
)code", mapKeywords);
}

std::string CVSSVDCodingTX::Code_VD_TXFunctionImplementation(const SFunctionVDDefinition& function, const std::string& className) const
{
    std::string class_name_lowercase = className;
    std::transform(class_name_lowercase.begin(), class_name_lowercase.end(), class_name_lowercase.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

    CKeywordMap mapKeywords;
    mapKeywords["function_name"] = function.functionName;
    mapKeywords["signal_name"] = function.signalName;
    mapKeywords["class_name"] = className;
    auto signalType = GetCTypeFromIDLType(function.idlType);
    mapKeywords["value_ctype"] = CastValueType(signalType);
    mapKeywords["class_name_lowercase"] = class_name_lowercase;

    return ReplaceKeywords(R"code(
/**
 * @brief %function_name%
 * @param[in] value
 * @return true on success otherwise false
 */
bool CVehicleDevice%class_name%::Write%function_name%(%value_ctype% value)
{
	if (m_%signal_name%)
	{
        //
        // TODO:
        // Convert abstract value to vehicle specific unit/range
        //

		m_%signal_name%.Write(value);
		return true;
	}

	return false;
}
)code", mapKeywords);
}

std::string CVSSVDCodingTX::Code_VD_TXVariable(const std::string& signalName) const
{
    CKeywordMap mapKeywords;
    mapKeywords["signal_name"] = signalName;

    return ReplaceKeywords(R"code(
	sdv::core::CSignal m_%signal_name%; ///< %signal_name% signal)code", mapKeywords);
}

std::string CVSSVDCodingTX::Code_VD_TXVariableInitialization(const SFunctionVDDefinition& function) const
{
    CKeywordMap mapKeywords;
    auto startWithUppercase = function.signalName;
    if (!startWithUppercase.empty())
    {
        startWithUppercase[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(startWithUppercase[0])));
    }
    mapKeywords["start_with_uppercase"] = startWithUppercase;
    mapKeywords["signal_name"] = function.signalName;
    mapKeywords["object_prefix"] = m_ssPrefix;

    return ReplaceKeywords(R"code(
	m_%signal_name% = dispatch.AddPublisher(%object_prefix%::ds%start_with_uppercase%);
	if (!m_%signal_name%)
	{
		SDV_LOG_ERROR("Could not get signal: ", %object_prefix%::ds%start_with_uppercase%, " [CVehicleDevice%class_name%]");
	}
)code", mapKeywords);
}

std::string CVSSVDCodingTX::Code_VD_TXVariableCheckInitialization(const std::vector <SFunctionVDDefinition>& vecFunctions) const
{
    uint32_t count { 0 };
    std::stringstream sstreamFunctions;
    sstreamFunctions << "    if (!";
    for (auto func : vecFunctions)
    {
        count++;
        sstreamFunctions << "m_" << func.signalName;
        if (count != vecFunctions.size())
        {
            sstreamFunctions << " || !";
        }
    }
    sstreamFunctions << ")" << std::endl;
    sstreamFunctions << "    {" << std::endl;
    sstreamFunctions << "        m_status = sdv::EObjectStatus::initialization_failure;" << std::endl;
    sstreamFunctions << "        return;" << std::endl;
    sstreamFunctions << "    }" << std::endl;
    sstreamFunctions << "    m_status = sdv::EObjectStatus::initialized;";

    return sstreamFunctions.str();
}
