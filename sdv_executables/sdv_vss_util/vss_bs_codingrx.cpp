#include "vss_bs_codingrx.h"

void CVSSBSCodingRX::GetKeyWordMap(const SSignalBSDefinition& signal, const SSignalVDDefinition& signalVD, CKeywordMap& mapKeywords) const
{
    std::string vd_vssWithColons = ReplaceCharacters(signal.vssVDDefinition, ".", "::");
    std::string vssWithColons = ReplaceCharacters(signal.vssDefinition, ".", "::");

    //------------------------------------------------------- basic service header
    std::stringstream sstreamIncludesVD;
    std::string vssNoDot = ReplaceCharacters(signal.vssDefinition, ".", "");
    std::transform(vssNoDot.begin(), vssNoDot.end(), vssNoDot.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    sstreamIncludesVD << Code_BS_RXIncludes(vssNoDot);
    mapKeywords["rx_bs_includes_list"] = std::move(sstreamIncludesVD.str());

    std::stringstream sstreamBSInterfaces;
    for (uint32_t index = 0; index < signal.vecFunctions.size(); index++)
    {
        if (index >= signalVD.vecFunctions.size())
            break;
        auto func = signal.vecFunctions[index];
        auto funcVD = signalVD.vecFunctions[index];
        sstreamBSInterfaces << Code_BS_RXInterface(func.functionName, vssWithColons,
            funcVD.functionName, vd_vssWithColons);
    }
    mapKeywords["rx_bs_interface_list"] = std::move(sstreamBSInterfaces.str());

    std::stringstream sstreamBSInterfaceEntries;
    for (const auto& func : signal.vecFunctions)
    {
        sstreamBSInterfaceEntries << Code_BS_RXInterfaceEntry(func.functionName, vssWithColons);
    }
    mapKeywords["rx_bs_interface_entry_list"] = std::move(sstreamBSInterfaceEntries.str());

    std::stringstream sstreamBSRegisterEvents;
    for (uint32_t index = 0; index < signal.vecFunctions.size(); index++)
    {
        if (index >= signalVD.vecFunctions.size())
            break;
        auto func = signal.vecFunctions[index];
        auto funcVD = signalVD.vecFunctions[index];
        sstreamBSRegisterEvents << Code_BS_RXReAndUnregisterEvent(vssWithColons, func, funcVD);
    }
    mapKeywords["rx_bs_register_events_list"] = std::move(sstreamBSRegisterEvents.str());

    std::stringstream sstreamPrivateBSHeaderPart;
    for (const auto& func : signal.vecFunctions)
    {
        sstreamPrivateBSHeaderPart << Code_BS_RXPrivateHeaderPart(func, vssWithColons);
    }
    mapKeywords["rx_bs_private_vd_header_part"] = std::move(sstreamPrivateBSHeaderPart.str());

    ////------------------------------------------------------- basic service cpp
    std::stringstream sstreamBSConstructor;
    for (uint32_t index = 0; index < signalVD.vecFunctions.size(); index++)
    {
        if (index >= signal.vecFunctions.size())
            break;
        auto func = signal.vecFunctions[index];
        auto funcVD = signalVD.vecFunctions[index];
        sstreamBSConstructor << Code_BS_RXConstructor(signalVD, vd_vssWithColons, funcVD, func);
    }
    mapKeywords["rx_bs_constructor"] = std::move(sstreamBSConstructor.str());

    std::stringstream sstreamBSDeConstructor;
    for (const auto& func : signalVD.vecFunctions)
    {
        sstreamBSDeConstructor << Code_BS_RXDestructor(signalVD, vd_vssWithColons, func);
    }
    mapKeywords["rx_bs_deconstructor"] = std::move(sstreamBSDeConstructor.str());
  
    std::stringstream sstreamBSGetAndSetFunctions;
    for (uint32_t index = 0; index < signal.vecFunctions.size(); index++)
    {
        if (index >= signalVD.vecFunctions.size())
            break;
        auto func = signal.vecFunctions[index];
        auto funcVD = signalVD.vecFunctions[index];
        sstreamBSGetAndSetFunctions << Code_BS_RXGetAndSetFunctions(signal.className, func, funcVD);
    }
    mapKeywords["rx_bs_getandsetfunctions"] = std::move(sstreamBSGetAndSetFunctions.str());

    std::stringstream sstreamBSRegister;
    for (const auto& func : signal.vecFunctions)
    {
        sstreamBSRegister << Code_BS_RXRegister(signal.className, func, vssWithColons);
    }
    mapKeywords["rx_bs_register"] = std::move(sstreamBSRegister.str());    
}

std::string CVSSBSCodingRX::Code_RXIDLServiceList(const std::vector<std::string>& vssParts, 
    const std::vector <SFunctionBSDefinition>& vecFunctions) const
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
    for (auto func : vecFunctions)
    {
        sstreamInterfaces << Code_RXIDLServiceInterface(spaces, vssWithColons, func);
    }
    CKeywordMap mapKeywords;
    mapKeywords["interfaces"] = std::move(sstreamInterfaces.str());

    return ReplaceKeywords(nameSpace, mapKeywords);
}


std::string CVSSBSCodingRX::Code_BS_RXIncludes(const std::string& vssOriginalNoDot) const
{
    CKeywordMap mapKeywords;
    mapKeywords["vss_original_no_dot"] = vssOriginalNoDot;

    return ReplaceKeywords(R"code(#include "../vss_%vss_original_no_dot%_bs_rx.h"
)code", mapKeywords);
}

std::string CVSSBSCodingRX::Code_RXIDLServiceInterface(const std::string& spaces, const std::string& vssWithColons, 
    const SFunctionBSDefinition& function) const
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

std::string CVSSBSCodingRX::Code_BS_RXInterface(const std::string& functionName, const std::string& vssWithColons,
    const std::string& vd_functionName, const std::string& vd_vssWithColons) const
{
    CKeywordMap mapKeywords;
    mapKeywords["function_name"] = functionName;
    mapKeywords["vssWithColons"] = vssWithColons;
    mapKeywords["vd_function_name"] = vd_functionName;
    mapKeywords["vd_vssWithColons"] = vd_vssWithColons;

    return ReplaceKeywords(R"code(    , public vss::%vssWithColons%Service::IVSS_Get%function_name%
	, public vss::%vssWithColons%Service::IVSS_Set%function_name%_Event
	, public vss::%vd_vssWithColons%Device::IVSS_Write%vd_function_name%_Event
)code", mapKeywords);
}

std::string CVSSBSCodingRX::Code_BS_RXInterfaceEntry(const std::string& functionName, const std::string& vssWithColons) const
{
    CKeywordMap mapKeywords;
    mapKeywords["function_name"] = functionName;
    mapKeywords["vssWithColons"] = vssWithColons;

    return ReplaceKeywords(R"code(		SDV_INTERFACE_ENTRY(vss::%vssWithColons%Service::IVSS_Get%function_name%)
)code", mapKeywords);
}

std::string CVSSBSCodingRX::Code_BS_RXReAndUnregisterEvent(const std::string& vssWithColons, 
    const SFunctionBSDefinition& function, const SFunctionVDDefinition& functionVD) const
{
    auto startWithUppercase = function.signalName;
    if (!startWithUppercase.empty())
    {
        startWithUppercase[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(startWithUppercase[0])));
    }

    CKeywordMap mapKeywords;
    mapKeywords["vssWithColons"] = vssWithColons;
    mapKeywords["function_name"] = function.functionName;
    mapKeywords["vd_function_name"] = functionVD.functionName;
    mapKeywords["signal_name"] = function.signalName;
    mapKeywords["start_with_uppercase"] = startWithUppercase;
    auto signalType = GetCTypeFromIDLType(function.idlType);
    mapKeywords["casted_value_ctype"] = CastValueType(signalType);
    mapKeywords["value_ctype"] = GetCTypeFromIDLType(function.idlType);

    return ReplaceKeywords(R"code(
	/**
	 * @brief Set %signal_name% signal
	 * @param[in] value %signal_name%
	 */
	void Set%function_name%(%casted_value_ctype% value) override;

	/**
     * @brief Get %signal_name%
     * @return Returns the %signal_name%
     */
	%value_ctype% Get%function_name%() const override;

	/**
	 * @brief Write %signal_name% signal
	 * @param[in] value %signal_name%
	 */
	void Write%vd_function_name%(%casted_value_ctype% value) override;

	/**
	* @brief Register CallBack function on signal change
	* @param[in] callback function
	*/
	void RegisterOnSignalChangeOf%start_with_uppercase%(vss::%vssWithColons%Service::IVSS_Set%function_name%_Event* callback) override;

	/**
	* @brief Unregister CallBack function on signal change
	* @param[in] callback function
	*/
	void UnregisterOnSignalChangeOf%start_with_uppercase%(vss::%vssWithColons%Service::IVSS_Set%function_name%_Event* callback) override;
)code", mapKeywords);
}

std::string CVSSBSCodingRX::Code_BS_RXPrivateHeaderPart(const SFunctionBSDefinition& function, const std::string& vssWithColons) const
{
    CKeywordMap mapKeywords;
    mapKeywords["vssWithColons"] = vssWithColons;
    mapKeywords["function_name"] = function.functionName;
    mapKeywords["signal_name"] = function.signalName;
    mapKeywords["value_ctype"] = GetCTypeFromIDLType(function.idlType);

    return ReplaceKeywords(R"code(
	%value_ctype% m_%signal_name% { 0 }; ///< %signal_name% signal
	mutable std::mutex m_%signal_name%MutexCallbacks; ///< Mutex protecting m_%signal_name%Callbacks
	std::set<vss::%vssWithColons%Service::IVSS_Set%function_name%_Event*> m_%signal_name%Callbacks; ///< collection of events to be called
)code", mapKeywords);
}

std::string CVSSBSCodingRX::Code_BS_RXConstructor(const SSignalVDDefinition& signalVD, const std::string& vd_vssWithColons,
    const SFunctionVDDefinition& functionVD, const SFunctionBSDefinition& function) const
{
    CKeywordMap mapKeywords;
    mapKeywords["vd_vss_original"] = signalVD.vssDefinition;
    mapKeywords["vd_vssWithColons"] = vd_vssWithColons;
    mapKeywords["vd_function_name"] = functionVD.functionName;
    mapKeywords["vd_class_name"] = signalVD.className;
    mapKeywords["signal_name"] = function.signalName;

    return ReplaceKeywords(R"code(
	auto %signal_name%Device = sdv::core::GetObject("%vd_vss_original%_Device").GetInterface<vss::%vd_vssWithColons%Device::IVSS_%vd_function_name%>();
	if (!%signal_name%Device)
	{
		SDV_LOG_ERROR("Could not get interface 'IVSS_%vd_function_name%': [CBasicService%vd_class_name%]");
		throw std::runtime_error("%vd_vss_original% mode device not found");
	}
	%signal_name%Device->Register%vd_function_name%Event(dynamic_cast<vss::%vd_vssWithColons%Device::IVSS_Write%vd_function_name%_Event*> (this));
	SDV_LOG_TRACE("CBasicService%vd_class_name% created: [%vd_vss_original%_Device]");

	m_%signal_name% = 0;
)code", mapKeywords);
}

std::string CVSSBSCodingRX::Code_BS_RXDestructor(const SSignalVDDefinition& signalVD, const std::string& vd_vssWithColons, 
    const SFunctionVDDefinition& functionVD) const
{
    CKeywordMap mapKeywords;
    mapKeywords["vd_signal_name"] = functionVD.signalName;
    mapKeywords["vd_vss_original"] = signalVD.vssDefinition;
    mapKeywords["vd_vssWithColons"] = vd_vssWithColons;
    mapKeywords["vd_function_name"] = functionVD.functionName;

    return ReplaceKeywords(R"code(
	auto %vd_signal_name%Device = sdv::core::GetObject("%vd_vss_original%_Device").GetInterface<vss::%vd_vssWithColons%Device::IVSS_%vd_function_name%>();
	if (%vd_signal_name%Device)
	{
		%vd_signal_name%Device->Unregister%vd_function_name%Event(dynamic_cast<vss::%vd_vssWithColons%Device::IVSS_Write%vd_function_name%_Event*> (this));
	}
    %vd_signal_name%Device = nullptr;
)code", mapKeywords);
}

std::string CVSSBSCodingRX::Code_BS_RXGetAndSetFunctions(const std::string& class_name, const SFunctionBSDefinition& function, 
    const SFunctionVDDefinition& functionVD) const
{
    std::string class_name_lowercase = class_name;
    std::transform(class_name_lowercase.begin(), class_name_lowercase.end(), class_name_lowercase.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

    CKeywordMap mapKeywords;
    mapKeywords["function_name"] = function.functionName;
    mapKeywords["vd_function_name"] = functionVD.functionName;
    mapKeywords["class_name"] = class_name;
    mapKeywords["signal_name"] = function.signalName;
    auto signalType = GetCTypeFromIDLType(function.idlType);
    mapKeywords["casted_value_ctype"] = CastValueType(signalType);
    mapKeywords["value_ctype"] = GetCTypeFromIDLType(function.idlType);
    mapKeywords["class_name_lowercase"] = class_name_lowercase;

    return ReplaceKeywords(R"code(
/**
 * @brief Set %signal_name%
 * @param[in] value %signal_name%
 */
void CBasicService%class_name%::Set%function_name%(%casted_value_ctype% value)
{
	m_%signal_name% = value;
	std::lock_guard<std::mutex> lock(m_%signal_name%MutexCallbacks);
	for (auto callback : m_%signal_name%Callbacks)
	{
		callback->Set%function_name%(value);
	}
}

/**
 * @brief Write %signal_name%
 * @param[in] value %signal_name%
 */
void CBasicService%class_name%::Write%vd_function_name%(%casted_value_ctype% value)
{
    Set%function_name%(value);
}


/**
 * @brief Get %signal_name%
 * @return Returns the %signal_name%
 */
%value_ctype% CBasicService%class_name%::Get%function_name%() const
{
	return m_%signal_name%;
}
)code", mapKeywords);
}

std::string CVSSBSCodingRX::Code_BS_RXRegister(const std::string& class_name, const SFunctionBSDefinition& function, 
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
    mapKeywords["function_name"] = function.functionName;
    mapKeywords["start_with_uppercase"] = startWithUppercase;
    mapKeywords["class_name"] = class_name;
    mapKeywords["signal_name"] = function.signalName;
    mapKeywords["vssWithColons"] = vssWithColons;
    mapKeywords["class_name_lowercase"] = class_name_lowercase;

    return ReplaceKeywords(R"code(
/**
 * @brief Register Callback on signal change
 * @param[in] callback function
 */
void CBasicService%class_name%::RegisterOnSignalChangeOf%start_with_uppercase%(vss::%vssWithColons%Service::IVSS_Set%function_name%_Event* %signal_name%Callback)
{
	if ( %signal_name%Callback)
	{
		std::lock_guard<std::mutex> lock(m_%signal_name%MutexCallbacks);
		m_%signal_name%Callbacks.insert(%signal_name%Callback);
	}
}

/**
 * @brief Unregister Callback
 * @param[in] callback function
 */
void CBasicService%class_name%::UnregisterOnSignalChangeOf%start_with_uppercase%(vss::%vssWithColons%Service::IVSS_Set%function_name%_Event* %signal_name%Callback)
{
	if (%signal_name%Callback)
	{
		std::lock_guard<std::mutex> lock(m_%signal_name%MutexCallbacks);
		m_%signal_name%Callbacks.erase(%signal_name%Callback);
	}
}
)code", mapKeywords);
}

