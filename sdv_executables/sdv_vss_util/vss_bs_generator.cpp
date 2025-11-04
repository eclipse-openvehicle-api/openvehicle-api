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

void CVSSBSGenerator::CreateRXFiles(const std::string& ssVersion) const
{
    for (const auto& rxSignal : m_RXsignals)
    {
        if (m_enableComponentCreation)
        {
            CreateBasicServiceFilesForRXSignal(rxSignal, ssVersion);
        }
        CreateIDLBasicServiceFileForRXSignal(rxSignal, ssVersion);
    }
}

void CVSSBSGenerator::CreateBasicServiceFilesForRXSignal(const SSignalBSDefinition& signal, const std::string& ssVersion) const
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

    CKeywordMap mapKeywords;
    CVSSCodingCommon coding(m_ssPrefix);
    coding.GetCommonKeyWordMap(signal, mapKeywords, ssVersion);
    mapKeywords["bs_safeguard"] = coding.Code_SafeGuard(pathBSHeader.filename().generic_u8string());

    mapKeywords["basic_service_h"] = pathLowerCaseHeader.filename().generic_u8string();
    mapKeywords["basic_service_cpp"] = pathLowerCaseClass.filename().generic_u8string();

    CVSSBSCodingRX codingRX;
    auto signalVD = GetVDSignal(signal.vssVDDefinition);
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

    auto signalVD = GetVDSignal(signal.vssVDDefinition);
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
    fstreamBSHeader.open(pathLowerCaseHeader, std::ios::out | std::ios::trunc);
    fstreamBSClass.open(pathLowerCaseClass, std::ios::out | std::ios::trunc);

    CKeywordMap mapKeywords;
    CVSSCodingCommon coding(m_ssPrefix);
    coding.GetCommonKeyWordMap(signal, mapKeywords, ssVersion);
    mapKeywords["bs_safeguard"] = coding.Code_SafeGuard(pathBSHeader.filename().generic_u8string());

    mapKeywords["basic_service_h"] = pathLowerCaseHeader.filename().generic_u8string();
    mapKeywords["basic_service_cpp"] = pathLowerCaseClass.filename().generic_u8string();

    CVSSBSCodingTX codingTX;
    auto signalVD = GetVDSignal(signal.vssVDDefinition);
    if (!signalVD.vecFunctions.size())
    {
        mapKeywords["vss_from_vd_not_found"] = "// corresponding vehicle device interface not found";
    }
    codingTX.GetKeyWordMap(signal, signalVD, mapKeywords);

    mapKeywords["basic_service_h"] = pathBSHeader.filename().generic_u8string();
    mapKeywords["basic_service_cpp"] = pathBSClass.filename().generic_u8string();

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
