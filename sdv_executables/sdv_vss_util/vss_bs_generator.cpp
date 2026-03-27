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

 #include "vss_bs_generator.h"
#include "vss_coding.h"
#include "vss_bs_codingrx.h"
#include "vss_bs_codingtx.h"


bool CVSSBSGenerator::GeneratedCode()
{
    // Create project directory
    if (!m_pathProject.empty())
    {
        m_pathProject /= "vss_files";
        if (CreateFolder(m_pathProject, ""))
        {
            //CVSSCodingCommon coding(m_ssPrefix);
            CreateFiles(m_ssVersion);
            return true;
        }
    }
    return false;
}

std::string CVSSBSGenerator::AppendExtension(const std::string& prefix, const std::string& filename, 
    const std::string& extension) const
{
    return prefix + filename + extension;
}

void CVSSBSGenerator::CreateFiles(const std::string& ssVersion)
{
    for (const SSignalBSDefinition& signal : m_signals)
    {
        if (signal.signalDirection == sdv::core::ESignalDirection::sigdir_rx)
            m_RXsignals.push_back(signal);
        else
            m_TXsignals.push_back(signal);
    }
    CreateRXFiles(ssVersion);
    CreateTXFiles(ssVersion);
}

void CVSSBSGenerator::CreateRXFiles(const std::string& ssVersion)
{
    m_createdFiles.clear();
    for (const auto& rxSignal : m_RXsignals)
    {
        if (m_enableComponentCreation)
        {
            CreateBasicServiceFilesForRXSignal(rxSignal, ssVersion);
        }
        CreateIDLBasicServiceFileForRXSignal(rxSignal, ssVersion);
    }
}

void CVSSBSGenerator::CreateBasicServiceFilesForRXSignal(const SSignalBSDefinition& signal, const std::string& ssVersion)
{
    std::string folderName = "BS_";
    folderName.append(signal.className);
    std::transform(folderName.begin(), folderName.end(), folderName.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    if (!CreateFolder(m_pathProject, folderName))
    {
        return;
    }
    std::filesystem::path  pathBSHeader = m_pathProject / folderName / AppendExtension("BS_", signal.className, ".h");
    std::filesystem::path  pathBSClass = m_pathProject / folderName / AppendExtension("BS_", signal.className, ".cpp");
    std::filesystem::path  pathCMakeLists = m_pathProject / folderName / "CMakeLists.txt";
    std::ofstream          fstreamBSHeader;
    std::ofstream          fstreamBSClass;
    auto pathLowerCaseHeader = MakeLowercaseFilename(pathBSHeader);    
    auto pathLowerCaseClass = MakeLowercaseFilename(pathBSClass);        
    fstreamBSHeader.open(pathLowerCaseHeader, std::ios::out | std::ios::trunc);
    fstreamBSClass.open(pathLowerCaseClass, std::ios::out | std::ios::trunc);

    m_createdFiles.push_back(pathLowerCaseHeader);
    m_createdFiles.push_back(pathLowerCaseClass);

    CKeywordMap mapKeywords;
    CVSSCodingCommon coding(m_ssPrefix);
    coding.GetCommonKeyWordMap(signal, mapKeywords, ssVersion);
    mapKeywords["bs_safeguard"] = coding.Code_SafeGuard(pathBSHeader.filename().generic_u8string());

    mapKeywords["basic_service_h"] = pathLowerCaseHeader.filename().generic_u8string();
    mapKeywords["basic_service_cpp"] = pathLowerCaseClass.filename().generic_u8string();

    CVSSBSCodingRX codingRX;
    auto signalVD = GetVDSignal(signal.vssVDDefinition, signal.signalDirection);
    if (!signalVD.vecFunctions.size())
    {
        mapKeywords["vss_from_vd_not_found"] = "// corresponding vehicle device interface not found";
    }
    codingRX.GetKeyWordMap(signal, signalVD, mapKeywords);

    fstreamBSHeader << ReplaceKeywords(szRXBasicServiceHeaderTemplate, mapKeywords);
    fstreamBSClass << ReplaceKeywords(szRXBasicServiceClassTemplate, mapKeywords);
    fstreamBSHeader.close();
    fstreamBSClass.close();

    std::string classNameLowerCase = signal.className;
    std::transform(classNameLowerCase.begin(), classNameLowerCase.end(), classNameLowerCase.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

    std::string libaryName = m_ssPrefix + "_bs_" + classNameLowerCase + "_rx";
    auto cmakeContent = coding.Code_CMakeProject(libaryName, folderName);
    CreateCMakeFile(pathCMakeLists, cmakeContent);
}

void CVSSBSGenerator::CreateIDLBasicServiceFileForRXSignal(const SSignalBSDefinition& signal, const std::string& ssVersion) const
{
    std::stringstream ss(signal.vssDefinition);
    std::string item;
    std::string filenameBS = "VSS_";
    std::vector<std::string> parts;
    while (std::getline(ss, item, '.'))
    {
        parts.push_back(item);
        filenameBS.append(item);
    }
    filenameBS.append("_bs_rx");
    DeleteHeaderFile(m_pathProject, filenameBS);
    filenameBS.append(".idl");

    std::filesystem::path  pathBSRXIDL = m_pathProject / filenameBS;
    std::ofstream          fstreamBSRXIDL;
    auto pathLowerCase = MakeLowercaseFilename(pathBSRXIDL);
    fstreamBSRXIDL.open(pathLowerCase, std::ios::out | std::ios::trunc);

    CKeywordMap mapKeywords;
    CVSSCodingCommon coding(m_ssPrefix);
    coding.GetCommonKeyWordMap(signal, mapKeywords, ssVersion);
    mapKeywords["_bs_idl_h"] = pathLowerCase.filename().generic_u8string();

    CVSSBSCodingRX codingRX;
    mapKeywords["vss_service"] = codingRX.Code_RXIDLServiceList(parts, signal.vecFunctions);

    std::string vdIDLFileLowerCase = "vss_";
    vdIDLFileLowerCase.append(ReplaceCharacters(signal.vssVDDefinition, ".", ""));
    vdIDLFileLowerCase.append("_vd_rx.idl");
    std::transform(vdIDLFileLowerCase.begin(), vdIDLFileLowerCase.end(), vdIDLFileLowerCase.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    mapKeywords["vd_idl_file"] = vdIDLFileLowerCase;

    auto signalVD = GetVDSignal(signal.vssVDDefinition, signal.signalDirection);
    if (signalVD.vecFunctions.size())
    {
        std::string vssNoDot = ReplaceCharacters(signal.vssVDDefinition, ".", "");
        mapKeywords["vd_vss_original_no_dot"] = vssNoDot;
    }
    else
    {
        mapKeywords["vss_device"] = "// corresponding vehicle device interface not found";
    }

    fstreamBSRXIDL << ReplaceKeywords(szBSRXIDLTemplate, mapKeywords);
    fstreamBSRXIDL.close();
}

void CVSSBSGenerator::CreateTXFiles(const std::string& ssVersion) const
{
    for (const auto& txSignal : m_TXsignals)
    {
        if (m_enableComponentCreation)
        {
            CreateBasicServiceFilesForTXSignal(txSignal, ssVersion);
        }
        CreateIDLBasicServiceFileForTXSignal(txSignal, ssVersion);
    }
}

void CVSSBSGenerator::CreateBasicServiceFilesForTXSignal(const SSignalBSDefinition& signal, const std::string& ssVersion) const
{
    std::string folderName = "BS_";
    folderName.append(signal.className);
    std::transform(folderName.begin(), folderName.end(), folderName.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });      
    if (!CreateFolder(m_pathProject, folderName))
    {
        return;
    }

    std::filesystem::path  pathBSHeader = m_pathProject / folderName / AppendExtension("BS_", signal.className, ".h");
    std::filesystem::path  pathBSClass = m_pathProject / folderName / AppendExtension("BS_", signal.className, ".cpp");
    std::filesystem::path  pathCMakeLists = m_pathProject / folderName / "CMakeLists.txt";
    std::ofstream          fstreamBSHeader;
    std::ofstream          fstreamBSClass;
    auto pathLowerCaseHeader = MakeLowercaseFilename(pathBSHeader);    
    auto pathLowerCaseClass = MakeLowercaseFilename(pathBSClass);

    auto headerExists = std::find(m_createdFiles.begin(), m_createdFiles.end(), pathLowerCaseHeader) != m_createdFiles.end();
    auto classExists = std::find(m_createdFiles.begin(), m_createdFiles.end(), pathLowerCaseClass) != m_createdFiles.end();    
    if (headerExists || classExists)
    {
        UpdateExistingFiles(m_pathProject, folderName, signal);
        return;
    }       
    fstreamBSHeader.open(pathLowerCaseHeader, std::ios::out | std::ios::trunc);
    fstreamBSClass.open(pathLowerCaseClass, std::ios::out | std::ios::trunc);

    CKeywordMap mapKeywords;
    CVSSCodingCommon coding(m_ssPrefix);
    coding.GetCommonKeyWordMap(signal, mapKeywords, ssVersion);
    mapKeywords["bs_safeguard"] = coding.Code_SafeGuard(pathBSHeader.filename().generic_u8string());

    mapKeywords["basic_service_h"] = pathLowerCaseHeader.filename().generic_u8string();
    mapKeywords["basic_service_cpp"] = pathLowerCaseClass.filename().generic_u8string();

    CVSSBSCodingTX codingTX;
    auto signalVD = GetVDSignal(signal.vssVDDefinition, signal.signalDirection);
    if (!signalVD.vecFunctions.size())
    {
        mapKeywords["vss_from_vd_not_found"] = "// corresponding vehicle device interface not found";
    }
    codingTX.GetKeyWordMap(signal, signalVD, mapKeywords);

    fstreamBSHeader << ReplaceKeywords(szTXBasicServiceHeaderTemplate, mapKeywords);
    fstreamBSClass << ReplaceKeywords(szTXBasicServiceClassTemplate, mapKeywords);
    fstreamBSHeader.close();
    fstreamBSClass.close();

    std::string classNameLowerCase = signal.className;
    std::transform(classNameLowerCase.begin(), classNameLowerCase.end(), classNameLowerCase.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
		
    std::string libaryName = m_ssPrefix + "_bs_" + classNameLowerCase + "_tx";
    auto cmakeContent = coding.Code_CMakeProject(libaryName, folderName);
    CreateCMakeFile(pathCMakeLists, cmakeContent);
}


void CVSSBSGenerator::CreateIDLBasicServiceFileForTXSignal(const SSignalBSDefinition& signal, const std::string& ssVersion) const
{
    std::stringstream ss(signal.vssDefinition);
    std::string item;
    std::string filenameBS = "VSS_";
    std::vector<std::string> parts;
    while (std::getline(ss, item, '.'))
    {
        parts.push_back(item);
        filenameBS.append(item);
    }
    filenameBS.append("_bs_tx");
    DeleteHeaderFile(m_pathProject, filenameBS);
    filenameBS.append(".idl");

    std::filesystem::path  pathBSTXIDL = m_pathProject / filenameBS;
    std::ofstream          fstreamBSTXIDL;
    auto pathLowerCase = MakeLowercaseFilename(pathBSTXIDL);    
    fstreamBSTXIDL.open(pathLowerCase, std::ios::out | std::ios::trunc);

    CKeywordMap mapKeywords;
    CVSSCodingCommon coding(m_ssPrefix);
    coding.GetCommonKeyWordMap(signal, mapKeywords, ssVersion);
    mapKeywords["_bs_idl_h"] = pathLowerCase.filename().generic_u8string();

    CVSSBSCodingTX codingTX;
    mapKeywords["vss_service"] = codingTX.Code_BS_TXIDLList(parts, signal.vecFunctions);

    fstreamBSTXIDL << ReplaceKeywords(szBSTXIDLTemplate, mapKeywords);
    fstreamBSTXIDL.close();
}


bool CVSSBSGenerator::UpdateExistingFiles(const std::filesystem::path& rootPath, const std::string& subfolder, const SSignalBSDefinition& signal) const
{
    std::filesystem::path  path = rootPath / subfolder;

    CKeywordMap mapKeywords;
    CVSSBSCodingTX codingTX;
    auto signalVD = GetVDSignal(signal.vssVDDefinition, signal.signalDirection);
    if (!signalVD.vecFunctions.size())
    {
        mapKeywords["vss_from_vd_not_found"] = "// corresponding vehicle device interface not found";
    }
    codingTX.GetKeyWordMap(signal, signalVD, mapKeywords);    

    auto pointerList = mapKeywords["tx_bs_variable_pointer_init_list"]; 
    auto funtionImplementation = mapKeywords["vd_tx_pointer_function_implementations"]; 
    if (UpdateCppFile(path, signal, pointerList, funtionImplementation))
    {
        auto includeList = mapKeywords["tx_bs_includes_list"]; 
        auto interfaceList =  mapKeywords["tx_bs_interface_list"];     
        auto interfaceEntryList = mapKeywords["tx_bs_interface_entry_list"]; 
        auto functionList = mapKeywords["tx_bs_function_list"]; 
        auto variablePointerList = mapKeywords["tx_bs_variable_pointer_list"]; 

        if (UpdateHeaderFile(path, signal, includeList, interfaceList, interfaceEntryList, functionList, variablePointerList))
        {
            return true;
        }
    }

    return false;
}

bool CVSSBSGenerator::UpdateCppFile(const std::filesystem::path& rootFolder, const SSignalBSDefinition& signal, const std::string& initializeList, const std::string& implementation) const
{
    std::filesystem::path  pathBSCpp = rootFolder / AppendExtension("BS_", signal.className, ".cpp");
    auto pathLowerCaseCpp = MakeLowercaseFilename(pathBSCpp);  
    
    std::ifstream in(pathLowerCaseCpp); 
    if (!in) 
    {   
        std::cerr << "Failed to read file '" << pathLowerCaseCpp << ")\n";
        return false;
    }
    
    std::string s;
    std::vector<std::string> lines; // write new file into memory            
    while (std::getline(in, s)) 
    {
        lines.push_back(s);
        if (s.find("::CBasicService") != std::string::npos) // we have to find the constructor
        {
            std::getline(in, s);  // add next line, the bracket
            lines.push_back(s);
            lines.push_back(initializeList);
            break;
        }
    }

    while (std::getline(in, s)) 
    {
        lines.push_back(s);
    }

    lines.push_back(implementation);   // implementation at end of file
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

bool CVSSBSGenerator::UpdateHeaderFile(const std::filesystem::path& rootFolder, const SSignalBSDefinition& signal, const std::string& includeList, 
    const std::string& interfaceList, const std::string& interfaceEntryList, const std::string& functionList, const std::string& variablePointerList) const
{
    std::filesystem::path  pathBSCHeader = rootFolder / AppendExtension("BS_", signal.className, ".h");
    auto pathLowerCaseHeader = MakeLowercaseFilename(pathBSCHeader);  
    
    std::ifstream in(pathLowerCaseHeader);
    if (!in) 
    {   
        std::cerr << "Failed to read file '" << pathLowerCaseHeader << ")\n";
        return false;
    }
    
    std::string s;
    std::vector<std::string> lines;       // write new file into memory
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
            lines.push_back(functionList);  // we need to add entries before and after
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

    for (const auto& line : lines)   // write new file to disc
        out << line << "\n";

    out.close();
    return true;
}

