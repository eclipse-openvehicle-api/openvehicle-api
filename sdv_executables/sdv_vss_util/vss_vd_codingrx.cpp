#include "vss_vd_codingrx.h"

void CVSSVDCodingRX::GetKeyWordMap(const SSignalVDDefinition& signal, CKeywordMap& mapKeywords) const
{
    std::string vssWithColons = ReplaceCharacters(signal.vssDefinition, ".", "::");

    //------------------------------------------------------- vehicle device header
    std::stringstream sstreamIncludesVD;
    std::string vssNoDot = ReplaceCharacters(signal.vssDefinition, ".", "");
    std::transform(vssNoDot.begin(), vssNoDot.end(), vssNoDot.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    sstreamIncludesVD << Code_VD_RXIncludes(vssNoDot);
    mapKeywords["rx_vd_includes_list"] = std::move(sstreamIncludesVD.str());

    std::stringstream sstreamInterfaces;
    for (const auto& func : signal.vecFunctions)
    {
        sstreamInterfaces << Code_VD_RXInterface(func.functionName, vssWithColons);
    }
    mapKeywords["rx_vd_interface_list"] = std::move(sstreamInterfaces.str());

    std::stringstream sstreamVDInterfaceEntries;
    for (const auto& func : signal.vecFunctions)
    {
        sstreamVDInterfaceEntries << Code_VD_RXInterfaceEntry(func.functionName, vssWithColons);
    }
    mapKeywords["rx_vd_interface_entry_list"] = std::move(sstreamVDInterfaceEntries.str());

    std::stringstream sstreamVDRegisterEvents;
    for (const auto& func : signal.vecFunctions)
    {
        sstreamVDRegisterEvents << Code_VD_RXReAndUnregisterEvent(vssWithColons, func);
    }
    mapKeywords["rx_vd_register_events_list"] = std::move(sstreamVDRegisterEvents.str());

    std::stringstream sstreamPrivateVDHeaderPart;
    for (const auto& func : signal.vecFunctions)
    {
        sstreamPrivateVDHeaderPart << Code_VD_RXPrivateHeaderPart(func, vssWithColons, signal.className);
    }
    mapKeywords["rx_vd_private_vd_header_part"] = std::move(sstreamPrivateVDHeaderPart.str());

    //------------------------------------------------------- vehicle device cpp
    std::stringstream sstreamVDSubscribe;
    for (const auto& func : signal.vecFunctions)
    {
        sstreamVDSubscribe << Code_VD_RXSubscribeSignal(signal.className, func);
    }
    mapKeywords["rx_vd_subscribe_signals"] = std::move(sstreamVDSubscribe.str());

    if (signal.vecFunctions.size() > 0)
    {
        mapKeywords["rx_check_subscriptions"] = Code_VD_RXCheckSignalSubscribtions(signal.vecFunctions);
    }

    std::stringstream sstreamVDResetSignal;
    for (const auto& func : signal.vecFunctions)
    {
        sstreamVDResetSignal << Code_VD_RXResetSignal(func);
    }
    mapKeywords["rx_reset_signals"] = std::move(sstreamVDResetSignal.str());

    std::stringstream sstreamVDRegister;
    for (const auto& func : signal.vecFunctions)
    {
        sstreamVDRegister << Code_VD_RXRegister(signal.className, func, vssWithColons);
    }
    mapKeywords["rx_vd_register"] = std::move(sstreamVDRegister.str());
}

std::string CVSSVDCodingRX::Code_RXIDLDeviceList(const std::vector<std::string>& vssParts, 
    const std::vector <SFunctionVDDefinition>& vecFunctions) const
{
    std::string prefix = "Device";
    int8_t loop = 1;
    std::string nameSpace = "module vss\n{\n";
    for (auto it = vssParts.begin(); it != vssParts.end(); ++it)
    {
        nameSpace.append(static_cast <int8_t>(4) * loop, ' ');
        nameSpace.append("module ");
        nameSpace.append(*it);
        if (it == vssParts.end() - 1)
        {
            nameSpace.append(prefix);
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

    std::string spaces = "";
    spaces.append(static_cast <int8_t>(4) * (vssParts.size() + 1), ' ');
    std::stringstream sstreamInterfaces;
    for (const auto& func : vecFunctions)
    {
        sstreamInterfaces << Code_RXIDLDeviceInterface(spaces, func);
    }
    CKeywordMap mapKeywords;
    mapKeywords["interfaces"] = std::move(sstreamInterfaces.str());

    return ReplaceKeywords(nameSpace, mapKeywords);
}

std::string CVSSVDCodingRX::Code_RXIDLServiceList(const std::vector<std::string>& vssParts, 
    const std::vector <SFunctionVDDefinition>& vecFunctions) const
{
    std::string vssWithColons = "";
    int8_t loop = 1;
    std::string nameSpace = "module vss\n{\n";
    for (auto it = vssParts.begin(); it != vssParts.end(); ++it)
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
        vssWithColons.append("::");
        vssWithColons.append(*it);
    }

    nameSpace.append("%interfaces%");
    loop--;
    for (size_t i = 0; i < vssParts.size() + 1; i++)
    {
        nameSpace.append(static_cast <int8_t>(4) * loop--, ' ');
        nameSpace.append("};\n");
    }

    std::string spaces = "";
    spaces.append(static_cast <int8_t>(4) * (vssParts.size() + 1), ' ');
    std::stringstream sstreamInterfaces;
    for (const auto& func : vecFunctions)
    {
        sstreamInterfaces << Code_RXIDLServiceInterface(spaces, vssWithColons, func);
    }
    CKeywordMap mapKeywords;
    mapKeywords["interfaces"] = std::move(sstreamInterfaces.str());

    return ReplaceKeywords(nameSpace, mapKeywords);
}


std::string CVSSVDCodingRX::Code_VD_RXIncludes(const std::string& vssOriginalNoDot) const
{
    CKeywordMap mapKeywords;
    mapKeywords["vss_original_no_dot"] = vssOriginalNoDot;

    return ReplaceKeywords(R"code(#include "../vss_%vss_original_no_dot%_vd_rx.h")code", mapKeywords);
}


std::string CVSSVDCodingRX::Code_RXIDLDeviceInterface(const std::string& spaces, const SFunctionVDDefinition& function) const
{
    std::string prefix = "Device";
    CKeywordMap mapKeywords;
    mapKeywords["function_name"] = function.functionName;
    mapKeywords["signal_name"] = function.signalName;
    mapKeywords["value_idltype"] = function.idlType;
    mapKeywords["value_ctype"] = GetCTypeFromIDLType(function.idlType);
    mapKeywords["prefix"] = prefix;
    mapKeywords["multiple_spaces"] = spaces;
    return ReplaceKeywords(R"code(%multiple_spaces%/**
%multiple_spaces%* @brief IVSS_Write%function_name% event interface
%multiple_spaces%*/
%multiple_spaces%interface IVSS_Write%function_name%_Event
%multiple_spaces%{
%multiple_spaces%    /**
%multiple_spaces%    * @brief Write %signal_name% signal
%multiple_spaces%    * @param[in] value %signal_name%
%multiple_spaces%    */
%multiple_spaces%    void Write%function_name%(in %value_idltype% value);
%multiple_spaces%};

%multiple_spaces%/**
%multiple_spaces%* @brief IVSS_%function_name% abstract %prefix% interface
%multiple_spaces%*/
%multiple_spaces%interface IVSS_%function_name%
%multiple_spaces%{
%multiple_spaces%    /**
%multiple_spaces%    * @brief Register Write%function_name% event on signal change
%multiple_spaces%    * Register all events and call them on signal change
%multiple_spaces%    * @param[in] event function
%multiple_spaces%    */
%multiple_spaces%    void Register%function_name%Event(in IVSS_Write%function_name%_Event event);

%multiple_spaces%    /**
%multiple_spaces%    * @brief Unregister %function_name% event
%multiple_spaces%    * @param[in] event function
%multiple_spaces%    */
%multiple_spaces%    void Unregister%function_name%Event(in IVSS_Write%function_name%_Event event);
%multiple_spaces%};
)code", mapKeywords);
}

std::string CVSSVDCodingRX::Code_RXIDLServiceInterface(const std::string& spaces, const std::string& vssWithColons, 
    const SFunctionVDDefinition& function) const
{
    auto signalStartWithUppercase = function.signalName;
    if (!signalStartWithUppercase.empty())
    {
        signalStartWithUppercase[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(signalStartWithUppercase[0])));
    }

    std::string prefix = "Service";
    CKeywordMap mapKeywords;
    mapKeywords["function_name"] = function.functionName;
    mapKeywords["signal_start_with_uppercase"] = signalStartWithUppercase;
    mapKeywords["signal_name"] = function.signalName;
    mapKeywords["value_idltype"] = function.idlType;
    mapKeywords["value_ctype"] = GetCTypeFromIDLType(function.idlType);
    mapKeywords["prefix"] = prefix;
    mapKeywords["multiple_spaces"] = spaces;
    mapKeywords["vssWithColons"] = vssWithColons;
    return ReplaceKeywords(R"code(%multiple_spaces%/**
%multiple_spaces%* @brief IVSS_Set%function_name% event interface
%multiple_spaces%*/
%multiple_spaces%interface IVSS_Set%function_name%_Event
%multiple_spaces%{
%multiple_spaces%    /**
%multiple_spaces%    * @brief Set %signal_name% signal
%multiple_spaces%    * @param[in] value %signal_name%
%multiple_spaces%    */
%multiple_spaces%    void Set%function_name%(in %value_idltype% value);
%multiple_spaces%};

%multiple_spaces%/**
%multiple_spaces%* @brief IVSS_Get%function_name% %prefix% interface
%multiple_spaces%*/
%multiple_spaces%interface IVSS_Get%function_name%
%multiple_spaces%{
%multiple_spaces%    /**
%multiple_spaces%    * @brief Get %signal_name% signal
%multiple_spaces%    * @return Returns the %signal_name%
%multiple_spaces%    */
%multiple_spaces%    %value_idltype% Get%function_name%() const;

%multiple_spaces%    /**
%multiple_spaces%    * @brief Register Callback on signal change
%multiple_spaces%    * @param[in] callback function
%multiple_spaces%    */
%multiple_spaces%    void RegisterOnSignalChangeOf%signal_start_with_uppercase%(in vss%vssWithColons%Service::IVSS_Set%function_name%_Event callback);

%multiple_spaces%    /**
%multiple_spaces%    * @brief Unregister Callback
%multiple_spaces%    * @param[in] callback function
%multiple_spaces%    */
%multiple_spaces%    void UnregisterOnSignalChangeOf%signal_start_with_uppercase%(in vss%vssWithColons%Service::IVSS_Set%function_name%_Event callback);
%multiple_spaces%};
)code", mapKeywords);
}

std::string CVSSVDCodingRX::Code_VD_RXInterface(const std::string & functionName, const std::string& vssShortenWithColons) const
{
    CKeywordMap mapKeywords;
    mapKeywords["function_name"] = functionName;
    mapKeywords["vss_shorten_with_colons"] = vssShortenWithColons;

    return ReplaceKeywords(R"code(	, public vss::%vss_shorten_with_colons%Device::IVSS_%function_name%
)code", mapKeywords);
}

std::string CVSSVDCodingRX::Code_VD_RXInterfaceEntry(const std::string& functionName, const std::string& vssWithColons) const
{
    CKeywordMap mapKeywords;
    mapKeywords["function_name"] = functionName;
    mapKeywords["vssWithColons"] = vssWithColons;

    return ReplaceKeywords(R"code(		SDV_INTERFACE_ENTRY(vss::%vssWithColons%Device::IVSS_%function_name%)
)code", mapKeywords);
}

std::string CVSSVDCodingRX::Code_VD_RXReAndUnregisterEvent( const std::string& vssWithColons, const SFunctionVDDefinition& function) const
{
    CKeywordMap mapKeywords;
    mapKeywords["vssWithColons"] = vssWithColons;
    mapKeywords["function_name"] = function.functionName;

    return ReplaceKeywords(R"code(
	/**
	* @brief Register IVSS_Write%function_name%_Event on signal change
	* Collect all events and call them on signal change
	* @param[in] event function
	*/
	void Register%function_name%Event(vss::%vssWithColons%Device::IVSS_Write%function_name%_Event* event) override;

	/**
	* @brief Unregister IVSS_%function_name%_Event
	* @param[in] event function
	*/
	void Unregister%function_name%Event(vss::%vssWithColons%Device::IVSS_Write%function_name%_Event* event) override;
)code", mapKeywords);
}

std::string CVSSVDCodingRX::Code_VD_RXPrivateHeaderPart(const SFunctionVDDefinition& function, 
    const std::string& vssWithColons, const std::string& class_name) const
{
    auto startWithUppercase = function.signalName;
    if (!startWithUppercase.empty())
    {
        startWithUppercase[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(startWithUppercase[0])));
    }

    std::string class_name_lowercase = class_name;
    std::transform(class_name_lowercase.begin(), class_name_lowercase.end(), class_name_lowercase.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

    CKeywordMap mapKeywords;
    mapKeywords["function_name"] = function.functionName;
    mapKeywords["vssWithColons"] = vssWithColons;
    mapKeywords["signal_name"] = function.signalName;
    mapKeywords["start_with_uppercase"] = startWithUppercase;
    mapKeywords["class_name"] = class_name;
    mapKeywords["class_name_lowercase"] = class_name_lowercase;

    return ReplaceKeywords(R"code(
	/**
	* @brief Execute all callbacks
	*/
	void ExecuteAllCallBacksFor%start_with_uppercase%(sdv::any_t value);

	sdv::core::CSignal m_%signal_name%Signal;                                                        ///< Signal of the vehicle device
	mutable std::mutex m_%signal_name%MutexCallbacks;                                                ///< Mutex protecting m_%signal_name%Callbacks
	std::set<vss::%vssWithColons%Device::IVSS_Write%function_name%_Event*> m_%signal_name%Callbacks; ///< collection of events to be called
)code", mapKeywords);
}

std::string CVSSVDCodingRX::Code_VD_RXSubscribeSignal(const std::string& class_name, const SFunctionVDDefinition& function) const
{
    auto startWithUppercase = function.signalName;
    if (!startWithUppercase.empty())
    {
        startWithUppercase[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(startWithUppercase[0])));
    }

    std::string class_name_lowercase = class_name;
    std::transform(class_name_lowercase.begin(), class_name_lowercase.end(), class_name_lowercase.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

    CKeywordMap mapKeywords;
    mapKeywords["object_prefix"] = m_ssPrefix;
    mapKeywords["start_with_uppercase"] = startWithUppercase;
    mapKeywords["class_name"] = class_name;
    mapKeywords["signal_name"] = function.signalName;
    mapKeywords["class_name_lowercase"] = class_name_lowercase;

    return ReplaceKeywords(R"code(
	m_%signal_name%Signal = dispatch.Subscribe(%object_prefix%::ds%start_with_uppercase%, [&](sdv::any_t value) { ExecuteAllCallBacksFor%start_with_uppercase%( value ); });
	if (!m_%signal_name%Signal)
	{
		SDV_LOG_ERROR("Could not get signal: ", %object_prefix%::ds%start_with_uppercase%, " [CVehicleDevice%class_name%]");
	}
)code", mapKeywords);
}

std::string CVSSVDCodingRX::Code_VD_RXCheckSignalSubscribtions(const std::vector <SFunctionVDDefinition>& vecFunctions) const
{
    uint32_t count{ 0 };
    std::stringstream sstreamFunctions;
    sstreamFunctions << "    if (!";
    for (const auto& func : vecFunctions)
    {
        count++;
        sstreamFunctions << "m_" << func.signalName << "Signal";
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

std::string CVSSVDCodingRX::Code_VD_RXResetSignal(const SFunctionVDDefinition& function) const
{
    CKeywordMap mapKeywords;
    mapKeywords["signal_name"] = function.signalName;

    return ReplaceKeywords(R"code(    m_%signal_name%Signal.Reset();
)code", mapKeywords);
}

std::string CVSSVDCodingRX::Code_VD_RXRegister( const std::string& class_name, const SFunctionVDDefinition& function, 
    const std::string& vssWithColons) const
{
    auto startWithUppercase = function.signalName;
    if (!startWithUppercase.empty())
    {
        startWithUppercase[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(startWithUppercase[0])));
    }
    std::string class_name_lowercase = class_name;
    std::transform(class_name_lowercase.begin(), class_name_lowercase.end(), class_name_lowercase.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

    CKeywordMap mapKeywords;
    mapKeywords["vssWithColons"] = vssWithColons;
    mapKeywords["function_name"] = function.functionName;
    mapKeywords["start_with_uppercase"] = startWithUppercase;
    mapKeywords["class_name"] = class_name;
    mapKeywords["signal_name"] = function.signalName;
    mapKeywords["value_ctype"] = GetCTypeFromIDLType(function.idlType);
    mapKeywords["class_name_lowercase"] = class_name_lowercase;
    if (!function.formula.empty())
    {
        auto formula = "    " + function.formula;
        std::string target = ";";
        std::string replacement = ";\n";

        size_t pos = 0;
        while ((pos = formula.find(target, pos)) != std::string::npos) 
        {
            formula.replace(pos, target.length(), replacement);
            pos += replacement.length(); 
        }

        mapKeywords["convertFormula"] = formula;
    }
    else
    {
        std::stringstream formula;
        formula << Code_VD_RXFormular(function);
        mapKeywords["convertFormula"] = formula.str();
    }

    return ReplaceKeywords(R"code(
/**
* @brief Register %function_name% event on signal change
* Collect all events and call them on signal change
* @param[in] event function
*/
void CVehicleDevice%class_name%::Register%function_name%Event(vss::%vssWithColons%Device::IVSS_Write%function_name%_Event* event)
{
	if (event)
	{
		std::lock_guard<std::mutex> lock(m_%signal_name%MutexCallbacks);
		m_%signal_name%Callbacks.insert(event);
	}
}

/**
* @brief Unregister %function_name% event
* @param[in] event function
*/
void CVehicleDevice%class_name%::Unregister%function_name%Event(vss::%vssWithColons%Device::IVSS_Write%function_name%_Event* event)
{
	if (event)
	{
		std::lock_guard<std::mutex> lock(m_%signal_name%MutexCallbacks);
		m_%signal_name%Callbacks.erase(event);
	}
}

/**
* @brief Execute all callbacks
*/
void CVehicleDevice%class_name%::ExecuteAllCallBacksFor%start_with_uppercase%(sdv::any_t value)
{
%convertFormula%
	std::lock_guard<std::mutex> lock(m_%signal_name%MutexCallbacks);
	for (const auto& callback : m_%signal_name%Callbacks)
	{
		callback->Write%function_name%(%signal_name%);
	}
}
)code", mapKeywords);
}

std::string CVSSVDCodingRX::Code_VD_RXFormular(const SFunctionVDDefinition& function) const
{
    CKeywordMap mapKeywords;
    mapKeywords["signal_name"] = function.signalName;
    mapKeywords["value_ctype"] = GetCTypeFromIDLType(function.idlType);

    return ReplaceKeywords(R"code(    %value_ctype% %signal_name% = value.get<%value_ctype%>();

    //
    // TODO:    
    // Convert vehicle specific value to abstract unit/range
    //
)code", mapKeywords);
}
