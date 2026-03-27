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
 *   Thomas Pfleiderer - initial API and implementation
 ********************************************************************************/

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

void CVSSVDGenerator::CreateRXFiles(const std::string& ssVersion)
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

void CVSSVDGenerator::CreateVehicleDeviceFilesForRXSignal(const SSignalVDDefinition& signal, const std::string& ssVersion)
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
    m_createdFiles.push_back(pathLowerCaseHeader);
    m_createdFiles.push_back(pathLowerCaseClass);

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

    auto headerExists = std::find(m_createdFiles.begin(), m_createdFiles.end(), pathLowerCaseHeader) != m_createdFiles.end();
    auto classExists = std::find(m_createdFiles.begin(), m_createdFiles.end(), pathLowerCaseClass) != m_createdFiles.end();    
    if (headerExists || classExists)
    {
        UpdateExistingFiles(m_pathProject, folderName, signal);
        return;
    }               
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


bool CVSSVDGenerator::UpdateExistingFiles(const std::filesystem::path& rootPath,  const std::string& subfolder, const SSignalVDDefinition& signal) const
{
    std::filesystem::path  path = rootPath / subfolder;

    CKeywordMap mapKeywords;
    CVSSVDCodingTX codingTX(m_ssPrefix);
    codingTX.GetKeyWordMap(signal, mapKeywords); 

    auto initializeList = mapKeywords["tx_variable_init_list"]; 
    auto functionImplementation = mapKeywords["tx_function_implementations"]; 
    if (UpdateCppFile(path, signal, initializeList, functionImplementation))
    {

        auto includeList = mapKeywords["tx_vd_includes_list"]; 
        auto interfaceList =  mapKeywords["tx_vd_interface_list"];    
        auto interfaceEntryList = mapKeywords["tx_vd_interface_entry_list"]; 
        auto functionList = mapKeywords["tx_vd_function_list"]; 
        auto variablePointerList = mapKeywords["tx_variable_list"]; 
       
        if (UpdateHeaderFile(path, signal, includeList, interfaceList, interfaceEntryList, functionList, variablePointerList))
        {
            return true;
        }
    }

    return false;
}

bool CVSSVDGenerator::UpdateCppFile(const std::filesystem::path& rootFolder, const SSignalVDDefinition& signal, const std::string& initializeList, const std::string& implementation) const
{
    std::filesystem::path  pathVDCpp = rootFolder / AppendExtension("VD_", signal.className, ".cpp");
    auto pathLowerCaseCpp = MakeLowercaseFilename(pathVDCpp);  

    std::ifstream in(pathLowerCaseCpp);  
    if (!in) 
    {
        std::cerr << "Failed to read file '" << pathLowerCaseCpp << ")\n";
        return false;
    }
    
    std::string s;
    std::vector<std::string> lines;  // write new file into memory    
    while (std::getline(in, s)) 
    {
        lines.push_back(s);
        if (s.find("sdv::core::CDispatchService dispatch;") != std::string::npos) 
        {
            lines.push_back(initializeList);
            break;
        }
    }

    while (std::getline(in, s)) 
    {
        lines.push_back(s);
    }
    
    lines.push_back(implementation);  // implementation at end of file
    in.close();

    std::error_code ec;
    std::filesystem::remove(pathLowerCaseCpp, ec); // delete file
    if (ec)
    {
        std::cerr << "Failed to delete file '" << pathLowerCaseCpp << "': " << ec.message() << " (error code: " << ec.value() << ")\n";
        return false;
    }

    std::ofstream out(pathLowerCaseCpp, std::ios::trunc);
    if (!out)
    {
        std::cerr << "Failed to write file '" << pathLowerCaseCpp << ")\n";
        return false;
    }

    for (const auto& line : lines)
        out << line << "\n";

    out.close();
    return true;
}

bool CVSSVDGenerator::UpdateHeaderFile(const std::filesystem::path& rootFolder, const SSignalVDDefinition& signal, const std::string& includeList, 
    const std::string& interfaceList, const std::string& interfaceEntryList, const std::string& functionList, const std::string& variablePointerList) const
{
    std::filesystem::path  pathVDHeader = rootFolder / AppendExtension("VD_", signal.className, ".h");
    auto pathLowerCaseHeader = MakeLowercaseFilename(pathVDHeader);  
    
    std::ifstream in(pathLowerCaseHeader);
    if (!in) 
    {
        std::cerr << "Failed to read file '" << pathLowerCaseHeader << ")\n";
        return false;
    }
    
    std::string s;
    std::vector<std::string> lines;   // write new file into memory        
    while (std::getline(in, s)) 
    {
        lines.push_back(s);
        if (s.find("<support/signal_support.h>") != std::string::npos) 
        {
            lines.push_back(includeList);
            break;
        }
    }
        
    while (std::getline(in, s)) 
    {
        lines.push_back(s);
        if (s.find(", public") != std::string::npos) 
        {
            lines.push_back(interfaceList);
            break;
        }
    }

    while (std::getline(in, s)) 
    {
        lines.push_back(s);
        if (s.find("BEGIN_SDV_INTERFACE_MAP()") != std::string::npos) 
        {
            lines.push_back(interfaceEntryList);
            break;
        }
    }    

    while (std::getline(in, s)) 
    {
        if (s.find("private:") != std::string::npos) 
        {
            lines.push_back(functionList);   // we need to add entries before and after
            lines.push_back(s); 
            lines.push_back(variablePointerList);                   
            break;
        }
        lines.push_back(s);
    }  

    while (std::getline(in, s)) 
    {
        lines.push_back(s);
    }
    in.close();


    std::error_code ec;
    std::filesystem::remove(pathLowerCaseHeader, ec); // delete file
    if (ec)
    {
        std::cerr << "Failed to delete file '" << pathLowerCaseHeader << "': " << ec.message() << " (error code: " << ec.value() << ")\n";
        return false;
    }

    std::ofstream out(pathLowerCaseHeader, std::ios::trunc);
    if (!out)
    {
        std::cerr << "Failed to write file '" << pathLowerCaseHeader << ")\n";
        return false;
    }

    for (const auto& line : lines) // write new file to disc
        out << line << "\n";
    out.close();
    return true;
}

