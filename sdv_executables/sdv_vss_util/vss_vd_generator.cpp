#include "vss_vd_generator.h"
#include "vss_coding.h"
#include "vss_vd_codingrx.h"
#include "vss_vd_codingtx.h"


bool CVSSVDGenerator::GeneratedCode()
{
    // Create project directory
    if (!m_pathProject.empty())
    {
        m_pathProject /= "vss_files";
        if (CreateFolder(m_pathProject, ""))
        {
            std::filesystem::path pathSummary = m_pathProject / "summary.txt";
            std::filesystem::path pathSignalIds = m_pathProject / "signal_identifier.h";
            
            CVSSCodingCommon coding(m_ssPrefix);
            CreateFiles(m_ssVersion);
            
            std::ofstream fstreamSummary;   ///< Build description summery.txt file
            fstreamSummary.open(pathSummary, std::ios::out | std::ios::trunc);
            fstreamSummary << SummaryTextFile();
            fstreamSummary.close();

            std::ofstream fstreamSignalIds; ///< Build description SignalIdetifier.h file
            fstreamSignalIds.open(pathSignalIds, std::ios::out | std::ios::trunc);
            fstreamSignalIds << coding.Code_SignalIdentifier(m_signals, pathSignalIds.filename().generic_u8string(), m_ssVersion);
            fstreamSignalIds.close();
            return true;
        }
    }
    return false;
}

std::string CVSSVDGenerator::SummaryTextFile() const
{
    //std::string titlePrefix ="Object prefix";
    std::string titleClassName = "Class name";
    std::string titleSignalName = "Signal name";
    std::string titleCANSignalName = "CAN Signal name";
    std::string titleFunctionname = "Function name";
    std::string titleIDLType = "IDL-Type";
    std::string titleCType = "C-Type";
    std::string titleVSS = "vss Definition";
    bool addTitle = true;
    std::stringstream sstream;
    for (const auto& signal : m_signals)
    {
        if (addTitle)
        {
            addTitle = false;
            sstream << "\n";
            sstream << Align(titleClassName, GetMaxClassName(m_signals, titleClassName)) << "\t";
            sstream << Align(titleSignalName, GetMaxSignalName(m_signals, titleSignalName)) << "\t";
            sstream << Align(titleCANSignalName, GetMaxCANSignalName(m_signals, titleCANSignalName)) << "\t";
            sstream << Align(titleFunctionname, GetMaxFunctionName(m_signals, titleFunctionname)) << "\t";
            sstream << Align("RX/TX ", 6) << "\t";
            sstream << Align(titleIDLType, GetMaxIDLType(m_signals, titleIDLType)) << "\t";
            sstream << Align(titleCType, GetMaxCTypeFromIDLType(m_signals, titleCType)) << "\t";
            sstream << Align(titleVSS, GetMaxVSSDefinition(m_signals, titleVSS)) << "\t";
            sstream << "\n\n";
        }
        for (const auto& func : signal.vecFunctions)
        {
            sstream << Align(signal.className, GetMaxClassName(m_signals, titleClassName)) << "\t";
            sstream << Align(func.signalName, GetMaxSignalName(m_signals, titleSignalName)) << "\t";
            sstream << Align(func.canSignalName, GetMaxCANSignalName(m_signals, titleCANSignalName)) << "\t";
            sstream << Align(func.functionName, GetMaxFunctionName(m_signals, titleFunctionname)) << "\t";
            if (signal.signalDirection == sdv::core::ESignalDirection::sigdir_rx)
                sstream << "'RX' " << "\t";
            else
                sstream << "'TX' " << "\t";
            sstream << Align(func.idlType, GetMaxIDLType(m_signals, titleIDLType)) << "\t";
            sstream << Align(GetCTypeFromIDLType(func.idlType), GetMaxCTypeFromIDLType(m_signals, titleCType)) << "\t";
            sstream << Align(signal.vssDefinition, GetMaxVSSDefinition(m_signals, titleVSS)) << "\t\n";
        }
    }

    return sstream.str();
}

std::string CVSSVDGenerator::AppendExtension(const std::string& prefix, const std::string& filename, 
    const std::string& extension) const
{
    return prefix + filename + extension;
}

void CVSSVDGenerator::CreateFiles(const std::string& ssVersion)
{
    for (const SSignalVDDefinition& signal : m_signals)
    {
        if (signal.signalDirection == sdv::core::ESignalDirection::sigdir_rx)
            m_RXsignals.push_back(signal);
        else
            m_TXsignals.push_back(signal);
    }
    CreateRXFiles(ssVersion);
    CreateTXFiles(ssVersion);
}

void CVSSVDGenerator::CreateRXFiles(const std::string& ssVersion) const
{
    for (const auto& rxSignal : m_RXsignals)
    {
        if (m_enableComponentCreation)
        {
            CreateVehicleDeviceFilesForRXSignal(rxSignal, ssVersion);
        }
        CreateIDLVehicleDeviceFileForRXSignal(rxSignal, ssVersion);
    }
}

void CVSSVDGenerator::CreateVehicleDeviceFilesForRXSignal(const SSignalVDDefinition& signal, const std::string& ssVersion) const
{
    std::string folderName = "VD_";
    folderName.append(signal.className);
    std::transform(folderName.begin(), folderName.end(), folderName.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    if (!CreateFolder(m_pathProject, folderName))
    {
        return;
    }
    std::filesystem::path  pathVDHeader = m_pathProject / folderName / AppendExtension("VD_", signal.className, ".h");
    std::filesystem::path  pathVDClass = m_pathProject / folderName / AppendExtension("VD_", signal.className, ".cpp");
    std::filesystem::path  pathCMakeLists = m_pathProject / folderName / "CMakeLists.txt";
    std::ofstream          fstreamVDHeader;
    std::ofstream          fstreamVDClass;
    auto pathLowerCaseHeader = MakeLowercaseFilename(pathVDHeader);        
    auto pathLowerCaseClass = MakeLowercaseFilename(pathVDClass);        
    fstreamVDHeader.open(pathLowerCaseHeader, std::ios::out | std::ios::trunc);
    fstreamVDClass.open(pathLowerCaseClass, std::ios::out | std::ios::trunc);

    CKeywordMap mapKeywords;
    CVSSCodingCommon coding(m_ssPrefix);
    coding.GetCommonKeyWordMap(signal, mapKeywords, ssVersion);
    mapKeywords["vd_safeguard"] = coding.Code_SafeGuard(pathVDHeader.filename().generic_u8string());

    mapKeywords["abstract_device_h"] = pathLowerCaseHeader.filename().generic_u8string();
    mapKeywords["abstract_device_cpp"] = pathLowerCaseClass.filename().generic_u8string();

    CVSSVDCodingRX codingRX(m_ssPrefix);
    codingRX.GetKeyWordMap(signal, mapKeywords);

    fstreamVDHeader << ReplaceKeywords(szRXVehicleDeviceHeaderTemplate, mapKeywords);
    fstreamVDClass << ReplaceKeywords(szRXVehicleDeviceClassTemplate, mapKeywords);
    fstreamVDHeader.close();
    fstreamVDClass.close();

    // create/write CMakeLists.txt only if content has changed
    std::string libaryName = m_ssPrefix;
    libaryName.append("_vd_");

    std::string classNameLowerCase = signal.className;
    std::transform(classNameLowerCase.begin(), classNameLowerCase.end(), classNameLowerCase.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

    libaryName.append(classNameLowerCase);
    libaryName.append("_rx");
    auto cmakeContent = coding.Code_CMakeProject(libaryName, folderName);
    CreateCMakeFile(pathCMakeLists, cmakeContent);
}

void CVSSVDGenerator::CreateIDLVehicleDeviceFileForRXSignal(const SSignalVDDefinition& signal, const std::string& ssVersion) const
{
    std::stringstream ss(signal.vssDefinition);
    std::string item;
    std::string filenameVD = "VSS_";
    std::vector<std::string> parts;
    while (std::getline(ss, item, '.'))
    {
        parts.push_back(item);
        filenameVD.append(item);
    }
    filenameVD.append("_vd_rx");
    DeleteHeaderFile(m_pathProject, filenameVD);
    filenameVD.append(".idl");

    std::filesystem::path  pathVDRXIDL = m_pathProject / filenameVD;
    std::ofstream          fstreamVDRXIDL;
    auto pathLowerCase = MakeLowercaseFilename(pathVDRXIDL);        
    fstreamVDRXIDL.open(pathLowerCase, std::ios::out | std::ios::trunc);

    CKeywordMap mapKeywords;
    CVSSCodingCommon coding(m_ssPrefix);
    coding.GetCommonKeyWordMap(signal, mapKeywords, ssVersion);
    mapKeywords["_vd_idl_h"] = pathLowerCase.filename().generic_u8string();

    CVSSVDCodingRX codingRX(m_ssPrefix);
    mapKeywords["vss_device"] = codingRX.Code_RXIDLDeviceList(parts, signal.vecFunctions);
    mapKeywords["vss_service"] = codingRX.Code_RXIDLServiceList(parts, signal.vecFunctions);

    fstreamVDRXIDL << ReplaceKeywords(szVDRXIDLTemplate, mapKeywords);
    fstreamVDRXIDL.close();
}

void CVSSVDGenerator::CreateTXFiles(const std::string& ssVersion) const
{
    for (const auto& txSignal : m_TXsignals)
    {
        if (m_enableComponentCreation)
        {
            CreateVehicleDeviceFilesForTXSignal(txSignal, ssVersion);
        }
        CreateIDLVehicleDeviceFileForTXSignal(txSignal, ssVersion);
    }
}

void CVSSVDGenerator::CreateVehicleDeviceFilesForTXSignal(const SSignalVDDefinition& signal, const std::string& ssVersion) const
{
    std::string folderName = "VD_";
    folderName.append(signal.className);
    std::transform(folderName.begin(), folderName.end(), folderName.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });    
    if (!CreateFolder(m_pathProject, folderName))
    {
        return;
    }
    std::filesystem::path  pathVDHeader = m_pathProject / folderName / AppendExtension("VD_", signal.className, ".h");
    std::filesystem::path  pathVDClass = m_pathProject / folderName / AppendExtension("VD_", signal.className, ".cpp");
    std::filesystem::path  pathCMakeLists = m_pathProject / folderName / "CMakeLists.txt";
    std::ofstream          fstreamVDHeader;
    std::ofstream          fstreamVDClass;
    auto pathLowerCaseHeader = MakeLowercaseFilename(pathVDHeader);    
    auto pathLowerCaseClass = MakeLowercaseFilename(pathVDClass);            
    fstreamVDHeader.open(pathLowerCaseHeader, std::ios::out | std::ios::trunc);
    fstreamVDClass.open(pathLowerCaseClass, std::ios::out | std::ios::trunc);

    CKeywordMap mapKeywords;
    CVSSCodingCommon coding(m_ssPrefix);
    coding.GetCommonKeyWordMap(signal, mapKeywords, ssVersion);
    mapKeywords["vd_safeguard"] = coding.Code_SafeGuard(pathVDHeader.filename().generic_u8string());

    mapKeywords["abstract_device_h"] = pathLowerCaseHeader.filename().generic_u8string();
    mapKeywords["abstract_device_cpp"] = pathLowerCaseClass.filename().generic_u8string();

    CVSSVDCodingTX codingTX(m_ssPrefix);
    codingTX.GetKeyWordMap(signal, mapKeywords);

    mapKeywords["abstract_device_h"] = pathVDHeader.filename().generic_u8string();
    mapKeywords["abstract_device_cpp"] = pathVDClass.filename().generic_u8string();

    fstreamVDHeader << ReplaceKeywords(szTXVehicleDeviceHeaderTemplate, mapKeywords);
    fstreamVDClass << ReplaceKeywords(szTXVehicleDeviceClassTemplate, mapKeywords);
    fstreamVDHeader.close();
    fstreamVDClass.close();

    std::string classNameLowerCase = signal.className;
    std::transform(classNameLowerCase.begin(), classNameLowerCase.end(), classNameLowerCase.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
		
    std::string libaryName = m_ssPrefix + "_vd_" + classNameLowerCase + "_tx";
    auto cmakeContent = coding.Code_CMakeProject(libaryName, folderName);
    CreateCMakeFile(pathCMakeLists, cmakeContent);
}

void CVSSVDGenerator::CreateIDLVehicleDeviceFileForTXSignal(const SSignalVDDefinition& signal, const std::string& ssVersion) const
{
    std::stringstream ss(signal.vssDefinition);
    std::string item;
    std::string filenameVD = "VSS_";
    std::vector<std::string> parts;
    while (std::getline(ss, item, '.'))
    {
        parts.push_back(item);
        filenameVD.append(item);
    }
    filenameVD.append("_vd_tx");
    DeleteHeaderFile(m_pathProject, filenameVD);
    filenameVD.append(".idl");

    std::filesystem::path  pathVDTXIDL = m_pathProject / filenameVD;
    std::ofstream          fstreamVDTXIDL;
    auto pathLowerCase = MakeLowercaseFilename(pathVDTXIDL);        
    fstreamVDTXIDL.open(pathLowerCase, std::ios::out | std::ios::trunc);

    CKeywordMap mapKeywords;
    CVSSCodingCommon coding(m_ssPrefix);
    coding.GetCommonKeyWordMap(signal, mapKeywords, ssVersion);
    mapKeywords["_vd_idl_h"] = pathLowerCase.filename().generic_u8string();

    CVSSVDCodingTX codingTX(m_ssPrefix);
    mapKeywords["vss_device"] = codingTX.Code_VD_TXIDLList(parts, signal.vecFunctions);

    fstreamVDTXIDL << ReplaceKeywords(szVDTXIDLTemplate, mapKeywords);
    fstreamVDTXIDL.close();
}
