#include "fmu_templates.h"
#include "fmu_fmi_templates.h"
#include "fmu.h"
#include <support/any.h>
#include <cmath>
#include <utility>
#include <thread>


CSoftcarFMUGen::CSoftcarFMUGen(const std::filesystem::path& rpathOutputDir, const dbc::CDbcParser& rparser,
    const std::string& rsModelIdentifier, const std::string& rsVersion, const std::vector<std::string>& rvecNodes) :
    m_rparser(rparser)
{
    auto sModelIdentifier = CodeModelIdentifier(rsModelIdentifier);
    auto fmuSubPath = "fmu_" + sModelIdentifier;

    std::filesystem::path rootPath = (rpathOutputDir / fmuSubPath).lexically_normal();
    CleanUpDirectory(rootPath);

    auto pathCMakeFileListsFile = rootPath / "CMakeLists.txt";
    CKeywordMap mapKeywords;
    mapKeywords["cmakefilelists_path"] = pathCMakeFileListsFile.filename().generic_u8string();
    UpdateKeywordMap(sModelIdentifier, rsVersion, rvecNodes, mapKeywords);    

    // create/write CMakeLists.txt only if content has changed
    std::string cmakeExisitingContent = "";
    auto cmakeNewContent = ReplaceKeywords(szMappingCMakeFileListsTemplate, mapKeywords);
    if (std::filesystem::exists(pathCMakeFileListsFile))
    {
        std::ifstream stream;
        stream.open(pathCMakeFileListsFile);
        if (stream.is_open())
        {
            std::stringstream sstream;
            sstream << stream.rdbuf();
            cmakeExisitingContent = std::move(sstream.str());
        }
        stream.close();
    }

    if (cmakeNewContent.compare(cmakeExisitingContent) != 0)
    {
        std::ofstream fstreamCMakeFile;
        fstreamCMakeFile.open(pathCMakeFileListsFile, std::ios::out | std::ios::trunc);
        fstreamCMakeFile << cmakeNewContent;
        fstreamCMakeFile.close();
    }

    CreateSourceFiles(rootPath);
    CreateIncludeFiles(rootPath);
    CreateFMUFiles(rootPath, sModelIdentifier, mapKeywords);
}

void CSoftcarFMUGen::UpdateKeywordMap(const std::string& rsModelIdentifier, const std::string& rsVersion, 
    const std::vector<std::string>& rvecNodes, CKeywordMap& mapKeywords) const
{
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream sstreamDate;
    sstreamDate << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X");
    mapKeywords["date"] = sstreamDate.str();
    auto vecSources = m_rparser.GetSources();
    std::stringstream sstreamDbcSources;
    for (const dbc::CDbcSource& rsource : vecSources)
    {
        if (!sstreamDbcSources.str().empty())
            sstreamDbcSources << ", ";
        sstreamDbcSources << rsource.Path().filename().generic_u8string();
    }
    mapKeywords["dbc_sources"] = sstreamDbcSources.str();

    mapKeywords["model_Identifier"] = rsModelIdentifier;
    mapKeywords["model_guid"] = newUUID();
    mapKeywords["buildDescription_xml"] = CodeBuildDescriptionXML(rsModelIdentifier);
    mapKeywords["dbc_version"] = CodeDBCFileVersion(rsVersion);

    // Safeguards start with "__IDL_GENERATED__", add the file name, add the date and time and end with "__"
    std::stringstream sstreamSafeguardConfig;
    sstreamSafeguardConfig << "__DBC_GENERATED__CONFIG_H__";
    sstreamSafeguardConfig << std::put_time(std::localtime(&in_time_t), "%Y%m%d_%H%M%S") << "_";
    sstreamSafeguardConfig << std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count() % 1000;
    sstreamSafeguardConfig << "__";
    mapKeywords["safeguardconfig"] = sstreamSafeguardConfig.str();
    // Safeguards start with "__IDL_GENERATED__", add the file name, add the date and time and end with "__"
    std::stringstream sstreamSafeguardIdentifier;
    sstreamSafeguardIdentifier << "__DBC_GENERATED__SIGNALIDENTIFIER_H__";
    sstreamSafeguardIdentifier << std::put_time(std::localtime(&in_time_t), "%Y%m%d_%H%M%S") << "_";
    sstreamSafeguardIdentifier << std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count() % 1000;
    sstreamSafeguardIdentifier << "__";
    mapKeywords["safeguardsignalidentifier"] = sstreamSafeguardIdentifier.str();

    std::stringstream sstreamValueReferenceList;
    std::stringstream sstreamModelDataList;
    std::stringstream sstreamMappingFile;
    std::stringstream sstreamFMIDescriptionFile;
    uint32_t indexFMI = 0;
    uint32_t indexConfigH = 0;
    std::vector<signalDefinition> vecSignalDefinitions;

    auto vecMsgIDs = m_rparser.GetMessageIDs();
    for (uint32_t uiRawMsgID : vecMsgIDs)
    {
        auto prMessage = m_rparser.GetMsgDef(uiRawMsgID);
        if (!prMessage.second) continue;

        // Run through the transmitter nodes and check whether the receiver of the message is defined in our node list.
        bool bPartOfTransmitNode = false;
        for (const std::string& rssTransmitter : prMessage.first.vecTransmitters)
        {
            if (rvecNodes.empty() || std::find(rvecNodes.begin(), rvecNodes.end(), rssTransmitter) != rvecNodes.end())
            {
                bPartOfTransmitNode = true;
                break;
            }
        }

        // Run through the signal definitions and check whether the receiver of the message is defined in our node list.
        bool bPartOfReceiveNode = false;
        for (const dbc::SSignalDef& rSignal : prMessage.first.vecSignals)
        {
            if (std::find_if(rSignal.vecReceivers.begin(), rSignal.vecReceivers.end(), [&](const std::string& rssRcvNode)
                {
                    if (rvecNodes.empty()) return true;
                    return std::find(rvecNodes.begin(), rvecNodes.end(), rssRcvNode) != rvecNodes.end();
                }) != rSignal.vecReceivers.end())
            {
                bPartOfReceiveNode = true;
                break;
            }
        }

        if (!bPartOfReceiveNode && !bPartOfTransmitNode) continue;

        // Create message definitions
        if (bPartOfReceiveNode)
        {
            sstreamValueReferenceList << CodeConfigH_Rx_ValueReference(prMessage.first, indexConfigH, vecSignalDefinitions);
        }
        if (bPartOfTransmitNode)
        {
            sstreamValueReferenceList << CodeConfigH_Tx_ValueReference(prMessage.first, indexConfigH, vecSignalDefinitions);
        }

        mapKeywords["value_reference"] = std::move(sstreamValueReferenceList.str());
    }

    mapKeywords["unknown_index"] = to_string(indexFMI);

    sstreamModelDataList << CodeConfigH_ModelDataList(vecSignalDefinitions);
    mapKeywords["model_data"] = std::move(sstreamModelDataList.str());

    sstreamFMIDescriptionFile << CodeFMIFile_VariableList(vecSignalDefinitions);
    mapKeywords["model_variables"] = std::move(sstreamFMIDescriptionFile.str());

    sstreamMappingFile << CodeVariableList(vecSignalDefinitions);
    mapKeywords["variable_list"] = std::move(sstreamMappingFile.str());

    mapKeywords["global_signals"] = CodeModelCPP_GlobalDefinitionList(vecSignalDefinitions);
    mapKeywords["global_signals_register"] = CodeModelCPP_GlobalRegisterList(vecSignalDefinitions);
    mapKeywords["global_signals_register_check"] = CodeModelCPP_GlobalRegisterCheckList(vecSignalDefinitions);
    mapKeywords["global_signals_reset"] = CodeModelCPP_GlobalResetList(vecSignalDefinitions);
    mapKeywords["vapi_load_config_files"] = CodeModelCPP_OpenAPILoadFunction();
    mapKeywords["getFloat64"] = CodeModelCPP_GetFloat64(vecSignalDefinitions);
    mapKeywords["getInt32"] = CodeModelCPP_GetInt32(vecSignalDefinitions);
    mapKeywords["setFloat64"] = CodeModelCPP_SetFloat64(vecSignalDefinitions);
    mapKeywords["setInt32"] = CodeModelCPP_SetInt32(vecSignalDefinitions);
    mapKeywords["event_update"] = CodeModelCPP_EventUpdateList(vecSignalDefinitions);

    mapKeywords["signals"] = Code_AllSignalsIdentifierList(vecSignalDefinitions);
    mapKeywords["object_prefix"] = rsModelIdentifier;
}

void CSoftcarFMUGen::CreateFMUFiles(const std::filesystem::path& rRootPath, const std::string& rsModelIdentifier, CKeywordMap& rmapKeywords) const
{
    if (!CreateDirectories(rRootPath, rsModelIdentifier))
        return;
    auto fmuPath = rRootPath / rsModelIdentifier;

    std::ofstream fstreamBuidDescription;    ///< Build description XML file
    std::ofstream fstreamConfigH;            ///< Config.h file, used by the FMI code
    std::ofstream fstreamFMI2XML;            ///< fmi2.xml contains 'fmiModelDescription'
    std::ofstream fstreamModelCPP;           ///< model.cpp contains code to load and exchange signals to VAPI components
    std::ofstream fstreamSignalIdentifierFile;

    auto pathBuidDescription = fmuPath / "buildDescription.xml";
    auto pathConfigH = fmuPath / "config.h";
    auto pathFMI2XML = fmuPath / "FMI2.XML";
    auto pathModelCPP = fmuPath / "model.cpp";
    auto pathSignalIdentifierFile = fmuPath / "signal_identifier.h";
    fstreamBuidDescription.open(pathBuidDescription, std::ios::out | std::ios::trunc);
    fstreamConfigH.open(pathConfigH, std::ios::out | std::ios::trunc);
    fstreamFMI2XML.open(pathFMI2XML, std::ios::out | std::ios::trunc);
    fstreamModelCPP.open(pathModelCPP, std::ios::out | std::ios::trunc);
    fstreamSignalIdentifierFile.open(pathSignalIdentifierFile, std::ios::out | std::ios::trunc);

    rmapKeywords["buiddescription_path"] = pathBuidDescription.filename().generic_u8string();
    rmapKeywords["configH_path"] = pathConfigH.filename().generic_u8string();
    rmapKeywords["fmi2xml_path"] = pathFMI2XML.filename().generic_u8string();
    rmapKeywords["modelcpp_path"] = pathModelCPP.filename().generic_u8string();
    rmapKeywords["signalidentifierfile_path"] = pathSignalIdentifierFile.filename().generic_u8string();

    fstreamBuidDescription << ReplaceKeywords(szBuildDescriptionTemplate, rmapKeywords);
    fstreamBuidDescription.close();

    fstreamConfigH << ReplaceKeywords(szConfigHTemplate, rmapKeywords);
    fstreamConfigH.close();
    
    fstreamFMI2XML << ReplaceKeywords(szFMI2XMLTemplate, rmapKeywords);
    fstreamFMI2XML.close();

    fstreamModelCPP << ReplaceKeywords(szModelCPPTemplate, rmapKeywords);
    fstreamModelCPP.close();

    fstreamSignalIdentifierFile << ReplaceKeywords(szMappingSignalIdentifierTemplate, rmapKeywords);
    fstreamSignalIdentifierFile.close();

    CreateResourcesFiles(fmuPath);
}

void CSoftcarFMUGen::CreateResourcesFiles(const std::filesystem::path& rRootPath) const
{  
    if (!CreateDirectories(rRootPath, "resources"))
        return;
    auto resourcesPath = rRootPath / "resources";

    std::ofstream fstreamDataDispatchTomlFile;
    std::ofstream fstreamSimulationTaskTimerTomlFile;
    std::ofstream fstreamTaskTimerTomlFile;

    auto pathDataDispatchTomlFile = resourcesPath / "data_dispatch_config_file.toml";
    fstreamDataDispatchTomlFile.open(pathDataDispatchTomlFile, std::ios::out | std::ios::trunc);

    fstreamDataDispatchTomlFile << szDataDispatchServiceTomlFile;
    fstreamDataDispatchTomlFile.close();

    auto pathSimulationTaskTimerFile = resourcesPath / "simulation_task_timer_config_file.toml";
    fstreamSimulationTaskTimerTomlFile.open(pathSimulationTaskTimerFile, std::ios::out | std::ios::trunc);

    fstreamSimulationTaskTimerTomlFile << szSimulationTaskTimerServiceTomlFile;
    fstreamSimulationTaskTimerTomlFile.close();

    auto pathTaskTimerTomlFile = resourcesPath / "task_timer_config_file.toml";
    fstreamTaskTimerTomlFile.open(pathTaskTimerTomlFile, std::ios::out | std::ios::trunc);

    fstreamTaskTimerTomlFile << szTaskTimerhServiceTomlFile;
    fstreamTaskTimerTomlFile.close();
}

void CSoftcarFMUGen::CreateSourceFiles(const std::filesystem::path& rRootPath) const
{
    if (!CreateDirectories(rRootPath, "src"))
        return;
    auto sourcePath = rRootPath / "src";

    std::ofstream fstreamCosimulationSrcFileTemplate;
    std::ofstream fstreamFmi2FunctionsSrcFileTemplate;

    auto pathCosimulationSrcFileTemplate = sourcePath / "cosimulation.c";
    auto pathFmi2FunctionsSrcFileTemplate = sourcePath / "fmi2Functions.c";
    fstreamCosimulationSrcFileTemplate.open(pathCosimulationSrcFileTemplate, std::ios::out | std::ios::trunc);
    fstreamFmi2FunctionsSrcFileTemplate.open(pathFmi2FunctionsSrcFileTemplate, std::ios::out | std::ios::trunc);

    fstreamCosimulationSrcFileTemplate << szCosimulationSrcFileTemplate;
    fstreamCosimulationSrcFileTemplate.close();
    fstreamFmi2FunctionsSrcFileTemplate << szFmi2FunctionsSrcFileTemplate1;
    fstreamFmi2FunctionsSrcFileTemplate << szFmi2FunctionsSrcFileTemplate2;
    fstreamFmi2FunctionsSrcFileTemplate.close();
}

void CSoftcarFMUGen::CreateIncludeFiles(const std::filesystem::path& rRootPath) const
{
    if(!CreateDirectories(rRootPath, "include"))
        return;
    auto includePath = rRootPath / "include";

    std::ofstream fstreamCosimulationHeaderTemplate;
    std::ofstream fstreamFMI2FunctionsHeaderTemplate;
    std::ofstream fstreamFMI2FunctionTypesHeaderTemplate;
    std::ofstream fstreamFMI2TypesPlatformHeaderTemplate;
    std::ofstream fstreamModelHeaderTemplate;

    auto pathCosimulationHeaderTemplate = includePath / "cosimulation.h";
    auto pathFMI2FunctionsHeaderTemplate = includePath / "fmi2Functions.h";
    auto pathFMI2FunctionTypesHeaderTemplate = includePath / "fmi2FunctionTypes.h";
    auto pathFMI2TypesPlatformHeaderTemplate = includePath / "fmi2TypesPlatform.h";
    auto pathModelHeaderTemplate = includePath / "model.h";
    fstreamCosimulationHeaderTemplate.open(pathCosimulationHeaderTemplate, std::ios::out | std::ios::trunc);
    fstreamFMI2FunctionsHeaderTemplate.open(pathFMI2FunctionsHeaderTemplate, std::ios::out | std::ios::trunc);
    fstreamFMI2FunctionTypesHeaderTemplate.open(pathFMI2FunctionTypesHeaderTemplate, std::ios::out | std::ios::trunc);
    fstreamFMI2TypesPlatformHeaderTemplate.open(pathFMI2TypesPlatformHeaderTemplate, std::ios::out | std::ios::trunc);
    fstreamModelHeaderTemplate.open(pathModelHeaderTemplate, std::ios::out | std::ios::trunc);

    fstreamCosimulationHeaderTemplate << szCosimulationHeaderTemplate;
    fstreamCosimulationHeaderTemplate.close();
    fstreamFMI2FunctionsHeaderTemplate << szFMI2FunctionsHeaderTemplate1;
    fstreamFMI2FunctionsHeaderTemplate << szFMI2FunctionsHeaderTemplate2;
    fstreamFMI2FunctionsHeaderTemplate.close();
    fstreamFMI2FunctionTypesHeaderTemplate << szFMI2FunctionTypesHeaderTemplate;
    fstreamFMI2FunctionTypesHeaderTemplate.close();
    fstreamFMI2TypesPlatformHeaderTemplate << szFMI2TypesPlatformHeaderTemplate;
    fstreamFMI2TypesPlatformHeaderTemplate.close();
    fstreamModelHeaderTemplate << szModelHeaderTemplate;
    fstreamModelHeaderTemplate.close();
}

std::string CSoftcarFMUGen::ReplaceKeywords(const std::string& rsStr, const CKeywordMap& rmapKeywords, char cMarker /*= '%'*/)
{
    std::stringstream sstream;
    size_t nPos = 0;
    while (nPos < rsStr.size())
    {
        // Find the initial separator
        size_t nSeparator = rsStr.find(cMarker, nPos);
        sstream << rsStr.substr(nPos, nSeparator == std::string::npos ? nSeparator : nSeparator - nPos);
        nPos = nSeparator;
        if (nSeparator == std::string::npos) continue;
        nPos++;

        // Find the next separator.
        nSeparator = rsStr.find(cMarker, nPos);
        if (nSeparator == std::string::npos)
        {
            // Internal error: missing second separator during code building.
            continue;
        }

        // Find the keyword in the keyword map (between the separator and the position).
        CKeywordMap::const_iterator itKeyword = rmapKeywords.find(rsStr.substr(nPos, nSeparator - nPos));
        if (itKeyword == rmapKeywords.end())
        {
            // Internal error: invalid keyword during building code.
            nPos = nSeparator + 1;
            continue;
        }
        sstream << itKeyword->second;
        nPos = nSeparator + 1;
    }
    return sstream.str();
}

bool CSoftcarFMUGen::CreateDirectories(const std::filesystem::path& rpathRootDirectory, const std::filesystem::path& rpathSubDirectory) const
{
    std::filesystem::path subFolder = (rpathRootDirectory / rpathSubDirectory).lexically_normal();

    try
    {
        if (!std::filesystem::exists(subFolder))
        {
            std::filesystem::create_directories(subFolder);
        }
    }
    catch (const std::filesystem::filesystem_error& e)
    {
        std::cout << "Filesystem error: " << e.what() << '\n';
    }
    catch (const std::exception& e)
    {
        std::cout << "Error: " << e.what() << '\n';
    }

    return std::filesystem::exists(subFolder);
}

bool CSoftcarFMUGen::CleanUpDirectory(const std::filesystem::path& rpathRootDirectory) const
{
    try
    {
        if (!std::filesystem::exists(rpathRootDirectory))
        {
            std::filesystem::create_directories(rpathRootDirectory);
        }
        else
        {
            for (const auto& entry : std::filesystem::directory_iterator(rpathRootDirectory))
            {
                if (entry.path().filename() != "CMakeLists.txt")
                {
                    if (std::filesystem::is_directory(entry.path()))
                    {
                        std::filesystem::remove_all(entry.path());
                    }
                    else
                    {
                        std::filesystem::remove(entry.path());
                    }
                }
            }
        }
    }
    catch (const std::filesystem::filesystem_error& e)
    {
        std::cout << "Filesystem error: " << e.what() << '\n';
    }
    catch (const std::exception& e) 
    {
        std::cout << "Error: " << e.what() << '\n';
    }

    return std::filesystem::exists(rpathRootDirectory);
}

std::string CSoftcarFMUGen::CodeDBCFileVersion(const std::string& rsVersion) const
{
    if (rsVersion.empty())
    {
        return "";
    }

    CKeywordMap mapKeywords;
    mapKeywords["dbc_version"] = rsVersion;

    return ReplaceKeywords(R"code(DBC file version: %dbc_version%)code", mapKeywords);
}

std::string CSoftcarFMUGen::CodeModelIdentifier(const std::string& rsModelIdentifier) const
{
    if (rsModelIdentifier.empty())
    {
        return "BasicModelIdentifier";
    }
    return rsModelIdentifier;
}

std::string CSoftcarFMUGen::CodeBuildDescriptionXML(const std::string& rsModelIdentifier) const
{
    CKeywordMap mapKeywords;
    mapKeywords["model_Identifier"] = rsModelIdentifier;

    return ReplaceKeywords(R"code(<?xml version="1.0" encoding="UTF-8"?>
<fmiBuildDescription fmiVersion="2.0">
    <BuildConfiguration modelIdentifier="%model_Identifier%">
        <SourceFileSet language="C++17">
            <SourceFile name="fmi2Functions.c"/>
            <SourceFile name="model.c"/>
            <SourceFile name="cosimulation.c"/>
            <PreprocessorDefinition name="FMI_VERSION" value="2"/>
        </SourceFileSet>
    </BuildConfiguration>
</fmiBuildDescription>
)code", mapKeywords);
}

std::string CSoftcarFMUGen::CodeFMIFile_VariableList(const std::vector<signalDefinition>& rvecSignalDefinitions) const
{
    CKeywordMap mapKeywords;
    std::stringstream sstreamSignalDecl;
    for (const signalDefinition& rsSignal : rvecSignalDefinitions)
    {
        sstreamSignalDecl << CodeFMIFile_SignalVariableList(rsSignal);
    }
    mapKeywords["sig_list"] = std::move(sstreamSignalDecl.str());

    return ReplaceKeywords(R"code(%sig_list%)code", mapKeywords);
}

std::string CSoftcarFMUGen::CodeFMIFile_SignalVariableList(const signalDefinition& rSignal) const
{
    CKeywordMap mapKeywords;
    mapKeywords["sig_name"] = rSignal.name;
    if (rSignal.isInput)
    {
        mapKeywords["sign_io"] = "input";
    }
    else
    {
        mapKeywords["sign_io"] = "output";
    }
    mapKeywords["sig_index"] = to_string(rSignal.index + 1);
    mapKeywords["sig_reference"] = to_string(rSignal.index);
    mapKeywords["siggn_attributes"] = rSignal.attributes;
    return ReplaceKeywords(R"code(    <!--Index for next variable = %sig_index% -->
    <ScalarVariable name = "%sig_name%"
        valueReference = "%sig_reference%"
        description = "  "
        causality = "%sign_io%">
        %siggn_attributes%
    </ScalarVariable>
)code", mapKeywords);
}

std::string CSoftcarFMUGen::CodeConfigH_Rx_ValueReference(const dbc::SMessageDef& rsMsg, uint32_t& rIndex,
    std::vector<signalDefinition>& rvecSignalDefinitions) const
{
    CKeywordMap mapKeywords;
    std::stringstream sstreamSignalDecl;
    for (const dbc::SSignalDef& rsSignal : rsMsg.vecSignals)
    {
        sstreamSignalDecl << CodeConfigH_Rx_SignalForValueReference(rsSignal, rIndex, rsMsg.ssName, rvecSignalDefinitions);
        rIndex++;
    }
    mapKeywords["sig_list"] = std::move(sstreamSignalDecl.str());

    return ReplaceKeywords(R"code(%sig_list%)code", mapKeywords);
}

std::string CSoftcarFMUGen::CodeConfigH_Tx_ValueReference(const dbc::SMessageDef& rsMsg, uint32_t& rIndex,
    std::vector<signalDefinition>& rvecSignalDefinitions) const
{
    CKeywordMap mapKeywords;
    std::stringstream sstreamSignalDecl;
    for (const dbc::SSignalDef& rsSignal : rsMsg.vecSignals)
    {
        sstreamSignalDecl << CodeConfigH_Tx_SignalForValueReference(rsSignal, rIndex, rsMsg.ssName, rvecSignalDefinitions);
        rIndex++;
    }
    mapKeywords["sig_list"] = std::move(sstreamSignalDecl.str());

    return ReplaceKeywords(R"code(%sig_list%)code", mapKeywords);
}

std::string CSoftcarFMUGen::CodeConfigH_Rx_SignalForValueReference(const dbc::SSignalDef& rsSig, const uint32_t index,
    const std::string& rsMessageName, std::vector<signalDefinition>& rvecSignalDefinitions) const
{
    auto prSignalTypeDef = m_rparser.GetSignalTypeDef(rsSig.ssSignalTypeDef);
    const dbc::SSignalTypeBase& rsSigType = prSignalTypeDef.second ?
        static_cast<const dbc::SSignalTypeBase&>(prSignalTypeDef.first) :
        static_cast<const dbc::SSignalTypeBase&>(rsSig);

    signalDefinition signal;
    signal.index = index;
    signal.message_name = rsMessageName;
    signal.name = rsSig.ssName;
    signal.isInput = true;
    signal.uiSize = rsSigType.uiSize;
    signal.isDouble = true;
    signal.signalType = rsSigType.eValType;
    if ((rsSigType.eValType == dbc::SSignalTypeBase::EValueType::signed_integer) || (rsSigType.eValType == dbc::SSignalTypeBase::EValueType::unsigned_integer))
    {
        if (rsSigType.dFactor == 1.0)
        {
            signal.isDouble = false;
        }
    }

    signal.attributes = GetAttributes(rsSig, signal.isDouble);
    rvecSignalDefinitions.push_back(signal);

    CKeywordMap mapKeywords;
    mapKeywords["sig_name"] = rsSig.ssName;
    mapKeywords["sig_index"] = to_string(index);

    return ReplaceKeywords(R"code(
    vr_%sig_name% = %sig_index%,)code", mapKeywords);
}

std::string CSoftcarFMUGen::CodeConfigH_Tx_SignalForValueReference(const dbc::SSignalDef& rsSig, const uint32_t index,
    const std::string& rsMessageName, std::vector<signalDefinition>& rvecSignalDefinitions) const
{
    auto prSignalTypeDef = m_rparser.GetSignalTypeDef(rsSig.ssSignalTypeDef);
    const dbc::SSignalTypeBase& rsSigType = prSignalTypeDef.second ?
        static_cast<const dbc::SSignalTypeBase&>(prSignalTypeDef.first) :
        static_cast<const dbc::SSignalTypeBase&>(rsSig);

    signalDefinition signal;
    signal.index = index;
    signal.message_name = rsMessageName;
    signal.name = rsSig.ssName;
    signal.isInput = false;
    signal.uiSize = rsSigType.uiSize;
    signal.signalType = rsSigType.eValType;

    signal.isDouble = true;
    if ((rsSigType.eValType == dbc::SSignalTypeBase::EValueType::signed_integer) || (rsSigType.eValType == dbc::SSignalTypeBase::EValueType::unsigned_integer))
    {
        if (rsSigType.dFactor == 1.0)
        {
            signal.isDouble = false;
        }
    }

    signal.attributes = GetAttributes(rsSig, signal.isDouble);

    rvecSignalDefinitions.push_back(signal);

    CKeywordMap mapKeywords;
    mapKeywords["sig_name"] = rsSig.ssName;
    mapKeywords["sig_index"] = to_string(index);

    return ReplaceKeywords(R"code(
    vr_%sig_name% = %sig_index%,)code", mapKeywords);
}

std::string CSoftcarFMUGen::CodeConfigH_ModelDataList(const std::vector<signalDefinition>& rvecSignalDefinitions) const
{
    CKeywordMap mapKeywords;
    std::stringstream sstreamSignalDecl;
    for (const signalDefinition& rsSignal : rvecSignalDefinitions)
    {
        sstreamSignalDecl << CodeConfigH_SignalForModelData(rsSignal);
    }
    mapKeywords["sig_list"] = std::move(sstreamSignalDecl.str());

    return ReplaceKeywords(R"code(%sig_list%)code", mapKeywords);
}

std::string CSoftcarFMUGen::CodeConfigH_SignalForModelData(const signalDefinition& rSignal) const
{
    CKeywordMap mapKeywords;
    mapKeywords["sig_name"] = rSignal.name;
    if (rSignal.isDouble)
    {
        mapKeywords["sig_type"] = "double";
    }
    else
    {
        mapKeywords["sig_type"] = "int32_t";
    }

    return ReplaceKeywords(R"code(
    %sig_type% %sig_name%;)code", mapKeywords);
}

std::string CSoftcarFMUGen::CodeVariableList(const std::vector<signalDefinition>& rvecSignalDefinitions) const
{
    CKeywordMap mapKeywords;
    std::stringstream sstreamSignalDecl;
    for (const signalDefinition& rsSignal : rvecSignalDefinitions)
    {
        if (rsSignal.isInput)
        {
            sstreamSignalDecl << CodeVariableName(rsSignal);
        }
    }
    sstreamSignalDecl << "\n";
    for (const signalDefinition& rsSignal : rvecSignalDefinitions)
    {
        if (!rsSignal.isInput)
        {
            sstreamSignalDecl << CodeVariableName(rsSignal);
        }
    }
    mapKeywords["sig_list"] = std::move(sstreamSignalDecl.str());

    return ReplaceKeywords(R"code(%sig_list%)code", mapKeywords);
}

std::string CSoftcarFMUGen::CodeVariableName(const signalDefinition& rSignal) const
{
    CKeywordMap mapKeywords;
    mapKeywords["sig_name"] = rSignal.name;

    return ReplaceKeywords(R"code(%sig_name% =
)code", mapKeywords);
}

std::string CSoftcarFMUGen::CodeModelCPP_GlobalDefinitionList(const std::vector<signalDefinition>& rvecSignalDefinitions) const
{
    CKeywordMap mapKeywords;
    std::stringstream sstreamGlobalSignals;
    for (const signalDefinition& rsSignal : rvecSignalDefinitions)
    {
        sstreamGlobalSignals << CodeModelCPP_GlobalSignalDefinition(rsSignal);
    }
    mapKeywords["sig_list"] = std::move(sstreamGlobalSignals.str());

    return ReplaceKeywords(R"code(%sig_list%)code", mapKeywords);
}

std::string CSoftcarFMUGen::CodeModelCPP_GlobalSignalDefinition(const signalDefinition& rSignal) const
{
    CKeywordMap mapKeywords;
    mapKeywords["sig_name"] = rSignal.name.c_str();

    return ReplaceKeywords(R"code(sdv::core::CSignal g_signal%sig_name%;
)code", mapKeywords);
}

std::string CSoftcarFMUGen::CodeModelCPP_GlobalRegisterList(const std::vector<signalDefinition>& rvecSignalDefinitions) const
{
    CKeywordMap mapKeywords;
    std::stringstream sstreamGlobalSignals;
    for (const signalDefinition& rsSignal : rvecSignalDefinitions)
    {
        sstreamGlobalSignals << CodeModelCPP_GlobalRegisterSignal(rsSignal);
    }
    mapKeywords["sig_list"] = std::move(sstreamGlobalSignals.str());

    return ReplaceKeywords(R"code(%sig_list%)code", mapKeywords);
}

std::string CSoftcarFMUGen::CodeModelCPP_GlobalRegisterCheckList(const std::vector<signalDefinition>& rvecSignalDefinitions) const
{
    uint32_t count = 0;
    CKeywordMap mapKeywords;
    std::stringstream sstreamGlobalSignals;

    bool first = true;
    for (const signalDefinition& rsSignal : rvecSignalDefinitions)
    {
        sstreamGlobalSignals << CodeModelCPP_GlobalRegisterSignalCheck(rsSignal, first);
        first = false;
        if(count++ > 4)
        {
            sstreamGlobalSignals << "\n       ";
            count = 0;
        }
    }
    mapKeywords["sig_list"] = std::move(sstreamGlobalSignals.str());

    return ReplaceKeywords(R"code(    if (%sig_list%)
    {
        return sdv::core::EConfigProcessResult::successful;
    })code", mapKeywords);
}

std::string CSoftcarFMUGen::CodeModelCPP_GlobalRegisterSignal(const signalDefinition& rSignal) const
{
    CKeywordMap mapKeywords;
    mapKeywords["sig_name"] = rSignal.name.c_str();
    mapKeywords["msg_name"] = rSignal.message_name.c_str();
    if (rSignal.isInput)
    {
        mapKeywords["sig_type"] = "Rx";
        mapKeywords["sig_default"] = "";
    }
    else
    {
        mapKeywords["sig_type"] = "Tx";
        mapKeywords["sig_default"] = ",0";
    }

    return ReplaceKeywords(R"code(    g_signal%sig_name% = dispatch.Register%sig_type%Signal("%msg_name%.%sig_name%"%sig_default%);
)code", mapKeywords);
}

std::string CSoftcarFMUGen::CodeModelCPP_GlobalRegisterSignalCheck(const signalDefinition& rSignal, bool first) const
{
    CKeywordMap mapKeywords;
    mapKeywords["sig_name"] = rSignal.name.c_str();
    mapKeywords["msg_name"] = rSignal.message_name.c_str();

    if (first)
    {
        return ReplaceKeywords(R"code(g_signal%sig_name%)code", mapKeywords);
    }

    return ReplaceKeywords(R"code( && g_signal%sig_name%)code", mapKeywords);
}

std::string CSoftcarFMUGen::CodeModelCPP_GlobalResetList(const std::vector<signalDefinition>& rvecSignalDefinitions) const
{
    CKeywordMap mapKeywords;
    std::stringstream sstreamGlobalSignals;
    for (const signalDefinition& rsSignal : rvecSignalDefinitions)
    {
        sstreamGlobalSignals << CodeModelCPP_GlobalResetSignal(rsSignal);
    }
    mapKeywords["sig_list"] = std::move(sstreamGlobalSignals.str());

    return ReplaceKeywords(R"code(%sig_list%)code", mapKeywords);
}

std::string CSoftcarFMUGen::CodeModelCPP_GlobalResetSignal(const signalDefinition& rSignal) const
{
    CKeywordMap mapKeywords;
    mapKeywords["sig_name"] = rSignal.name.c_str();

    return ReplaceKeywords(R"code(    if (g_signal%sig_name%)
    {
        g_signal%sig_name%.Reset();
    }
)code", mapKeywords);
}

std::string CSoftcarFMUGen::CodeModelCPP_EventUpdateList(const std::vector<signalDefinition>& rvecSignalDefinitions) const
{
    CKeywordMap mapKeywords;
    std::stringstream sstreamGlobalSignals;
    for (const signalDefinition& rsSignal : rvecSignalDefinitions)
    {
        if (rsSignal.isInput)
        {
            sstreamGlobalSignals << CodeModelCPP_EventUpdateSignalDefinitionWrite(rsSignal);
        }
        else
        {
            sstreamGlobalSignals << CodeModelCPP_EventUpdateSignalDefinitionRead(rsSignal);
        }
    }
    mapKeywords["sig_list"] = std::move(sstreamGlobalSignals.str());

    return ReplaceKeywords(R"code(
        if (g_pTimerSimulationStep) // in case the simulation timer was used, maybe the step size has to be adjusted
        {
            g_pTimerSimulationStep->SimulationStep(1000);
        }

%sig_list%)code", mapKeywords);
}

std::string CSoftcarFMUGen::CodeModelCPP_EventUpdateSignalDefinitionWrite(const signalDefinition& rSignal) const
{
    CKeywordMap mapKeywords;
    mapKeywords["sig_name"] = rSignal.name.c_str();

    return ReplaceKeywords(R"code(        g_signal%sig_name%.Write( M(%sig_name%));
)code", mapKeywords);
}

std::string CSoftcarFMUGen::CodeModelCPP_EventUpdateSignalDefinitionRead(const signalDefinition& rSignal) const
{
    CKeywordMap mapKeywords;
    mapKeywords["sig_name"] = rSignal.name.c_str();
    if (rSignal.isDouble)
    {
        mapKeywords["sig_type"] = "float";
    }
    else
    {
        mapKeywords["sig_type"] = "uint32_t";
    }

    return ReplaceKeywords(R"code(        M(%sig_name%) = g_signal%sig_name%.Read().get<%sig_type%>();
)code", mapKeywords);
}

std::string CSoftcarFMUGen::CodeModelCPP_GetFloat64(const std::vector<signalDefinition>& rvecSignalDefinitions) const
{
    bool empty = true;
    CKeywordMap mapKeywords;
    std::stringstream sstreamGlobalSignals;
    for (const signalDefinition& rsSignal : rvecSignalDefinitions)
    {
        if (rsSignal.isDouble)
        {
            empty = false;
            sstreamGlobalSignals << CodeModelCPP_GetFloat64SignalDefinition(rsSignal);
        }
    }
    mapKeywords["sig_list"] = std::move(sstreamGlobalSignals.str());

    if (empty)
    {
        return "";
    }

    return ReplaceKeywords(R"code(
        switch (vr)
        {
%sig_list%
            default:
                logError(comp, "Get Float64 is not allowed for value reference %u.", vr);
                return Error;
        }
)code", mapKeywords);
}

std::string CSoftcarFMUGen::CodeModelCPP_GetInt32(const std::vector<signalDefinition>& rvecSignalDefinitions) const
{
    bool empty = true;
    CKeywordMap mapKeywords;
    std::stringstream sstreamGlobalSignals;
    for (const signalDefinition& rsSignal : rvecSignalDefinitions)
    {
        if (!rsSignal.isDouble)// && !rsSignal.isInput)
        {
            empty = false;
            sstreamGlobalSignals << CodeModelCPP_GetInt32SignalDefinition(rsSignal);
        }
    }
    mapKeywords["sig_list"] = std::move(sstreamGlobalSignals.str());

    if (empty)
    {
        return "";
    }

    return ReplaceKeywords(R"code(
        switch (vr)
        {
%sig_list%
            default:
                logError(comp, "Get Int32 is not allowed for value reference %u.", vr);
                return Error;
        }
)code", mapKeywords);
}

std::string CSoftcarFMUGen::CodeModelCPP_SetFloat64(const std::vector<signalDefinition>& rvecSignalDefinitions) const
{
    bool empty = true;
    CKeywordMap mapKeywords;
    std::stringstream sstreamGlobalSignals;
    for (const signalDefinition& rsSignal : rvecSignalDefinitions)
    {
        if (rsSignal.isDouble)
        {
            empty = false;
            sstreamGlobalSignals << CodeModelCPP_SetFloat64SignalDefinition(rsSignal);
        }
    }
    mapKeywords["sig_list"] = std::move(sstreamGlobalSignals.str());

    if (empty)
    {
        return "";
    }

    return ReplaceKeywords(R"code(
        switch (vr)
        {
%sig_list%
            default:
                logError(comp, "Set Float64 is not allowed for value reference %u.", vr);
                return Error;
        }
)code", mapKeywords);
}

std::string CSoftcarFMUGen::CodeModelCPP_SetInt32(const std::vector<signalDefinition>& rvecSignalDefinitions) const
{
    bool empty = true;
    CKeywordMap mapKeywords;
    std::stringstream sstreamGlobalSignals;
    for (const signalDefinition& rsSignal : rvecSignalDefinitions)
    {
        if (!rsSignal.isDouble)
        {
            empty = false;
            sstreamGlobalSignals << CodeModelCPP_SetInt32SignalDefinition(rsSignal);
        }
    }
    mapKeywords["sig_list"] = std::move(sstreamGlobalSignals.str());

    if (empty)
    {
        return "";
    }

    return ReplaceKeywords(R"code(
        switch (vr)
        {
%sig_list%
            default:
                logError(comp, "Set Int32 is not allowed for value reference %u.", vr);
                return Error;
        }
)code", mapKeywords);
}

std::string CSoftcarFMUGen::CodeModelCPP_GetFloat64SignalDefinition(const signalDefinition& rSignal) const
{
    CKeywordMap mapKeywords;
    mapKeywords["sig_name"] = rSignal.name.c_str();
    return ReplaceKeywords(R"code(            case vr_%sig_name%:
                values[(*index)++] = M(%sig_name%);
                break;
)code", mapKeywords);
}

std::string CSoftcarFMUGen::CodeModelCPP_GetInt32SignalDefinition(const signalDefinition& rSignal) const
{
    CKeywordMap mapKeywords;
    mapKeywords["sig_name"] = rSignal.name.c_str();
    return ReplaceKeywords(R"code(            case vr_%sig_name%:
                values[(*index)++] = M(%sig_name%);
                break;
)code", mapKeywords);
}

std::string CSoftcarFMUGen::CodeModelCPP_SetFloat64SignalDefinition(const signalDefinition& rSignal) const
{
    CKeywordMap mapKeywords;
    mapKeywords["sig_name"] = rSignal.name.c_str();
    return ReplaceKeywords(R"code(            case vr_%sig_name%:
                M(%sig_name%) = values[(*index)++];
                break;
)code", mapKeywords);
}

std::string CSoftcarFMUGen::CodeModelCPP_SetInt32SignalDefinition(const signalDefinition& rSignal) const
{
    CKeywordMap mapKeywords;
    mapKeywords["sig_name"] = rSignal.name.c_str();
    return ReplaceKeywords(R"code(            case vr_%sig_name%:
                M(%sig_name%) = values[(*index)++];
                break;
)code", mapKeywords);
}

std::string CSoftcarFMUGen::CodeModelCPP_OpenAPILoadFunction() const
{
    CKeywordMap mapKeywords;

    return ReplaceKeywords(R"code(
//
//
// TODO: Load all configurations files
//
//
    // Get the simulation task timer service if the simulation timer should be used
    success &= g_appcontrol->LoadConfig("simulation_task_timer_config_file.toml") == sdv::core::EConfigProcessResult::successful;
    g_pTimerSimulationStep = sdv::core::GetObject<sdv::core::ITimerSimulationStep>("SimulationTaskTimerService");
    if (!g_pTimerSimulationStep)
    {
        SDV_LOG_WARNING("Simulation timer step not available, use normal task timer ");
        success &= g_appcontrol->LoadConfig("task_timer_config_file.toml") == sdv::core::EConfigProcessResult::successful;
    }
)code", mapKeywords);
}

std::string CSoftcarFMUGen::newUUID() const
{
#ifdef WIN32
    UUID uuid{};
    std::string s;
    if (UuidCreate(&uuid) == RPC_S_OK)
    {
        unsigned char* str = nullptr;
        if (UuidToStringA(&uuid, &str) == RPC_S_OK)
            s = reinterpret_cast<char*>(str);
        RpcStringFreeA(&str);
    }
    return s;
#else
    return {};
#endif
}

std::string CSoftcarFMUGen::GetAttributes(const dbc::SSignalDef& rsSig, const bool& isDouble) const
{
    auto prSignalTypeDef = m_rparser.GetSignalTypeDef(rsSig.ssSignalTypeDef);
    const dbc::SSignalTypeBase& rsSigType = prSignalTypeDef.second ?
        static_cast<const dbc::SSignalTypeBase&>(prSignalTypeDef.first) :
        static_cast<const dbc::SSignalTypeBase&>(rsSig);

    CKeywordMap mapKeywords;
    mapKeywords["start_value"] = CodeGetDefaultValueOfTxSignal(rsSig);
    if (isDouble)
    {
        mapKeywords["sig_type"] = "Real";
    }
    else
    {
        mapKeywords["sig_type"] = "Integer";
    }

    switch (rsSigType.eValType)
    {
    case dbc::SSignalDef::EValueType::signed_integer:
        if (std::round(rsSigType.dFactor) != rsSigType.dFactor ||
            std::round(rsSigType.dOffset) != rsSigType.dOffset)
        {
            mapKeywords["sig_factor"] = to_string(rsSigType.dFactor);
            mapKeywords["sig_offset"] = to_string(rsSigType.dOffset);
            mapKeywords["sig_min"] = to_string(rsSigType.dMinimum);
            mapKeywords["sig_max"] = to_string(rsSigType.dMaximum);
            if (rsSigType.uiSize > 32)
                mapKeywords["sig_helper_value"] = "static_cast<double>(uValueHelper.iInt64Value)";
            else
                mapKeywords["sig_helper_value"] = "static_cast<double>(uValueHelper.s32.u32.iValue)";
        }
        else
        {
            mapKeywords["sig_factor"] = to_string(static_cast<int32_t>(rsSigType.dFactor));
            mapKeywords["sig_offset"] = to_string(static_cast<int32_t>(rsSigType.dOffset));
            mapKeywords["sig_min"] = to_string(static_cast<int32_t>(rsSigType.dMinimum));
            mapKeywords["sig_max"] = to_string(static_cast<int32_t>(rsSigType.dMaximum));
            if (rsSigType.uiSize > 32)
                mapKeywords["sig_helper_value"] = "uValueHelper.iInt64Value";
            else
                mapKeywords["sig_helper_value"] = "uValueHelper.s32.u32.iValue";
        }
        break;
    case dbc::SSignalDef::EValueType::unsigned_integer:
        if (std::round(rsSigType.dFactor) != rsSigType.dFactor ||
            std::round(rsSigType.dOffset) != rsSigType.dOffset)
        {
            mapKeywords["sig_factor"] = to_string(rsSigType.dFactor);
            mapKeywords["sig_offset"] = to_string(rsSigType.dOffset);
            mapKeywords["sig_min"] = to_string(rsSigType.dMinimum);
            mapKeywords["sig_max"] = to_string(rsSigType.dMaximum);
            if (rsSigType.uiSize > 32)
                mapKeywords["sig_helper_value"] = "static_cast<double>(uValueHelper.uiUint64Value)";
            else
                mapKeywords["sig_helper_value"] = "static_cast<double>(uValueHelper.s32.u32.uiValue)";
        }
        else
        {
            if (rsSigType.dFactor < 0.0 || rsSigType.dOffset < 0.0)
            {
                mapKeywords["sig_factor"] = to_string(static_cast<int64_t>(rsSigType.dFactor)) + "ll";
                mapKeywords["sig_offset"] = to_string(static_cast<int64_t>(rsSigType.dOffset)) + "ll";
                mapKeywords["sig_min"] = to_string(static_cast<int64_t>(rsSigType.dMinimum)) + "ll";
                mapKeywords["sig_max"] = to_string(static_cast<int64_t>(rsSigType.dMaximum)) + "ll";
                if (rsSigType.uiSize > 32)
                    mapKeywords["sig_helper_value"] = "static_cast<int64_t>(uValueHelper.uiUint64Value)";
                else
                    mapKeywords["sig_helper_value"] = "static_cast<int64_t>(uValueHelper.s32.u32.uiValue)";
            }
            else
            {
                mapKeywords["sig_factor"] = to_string(static_cast<uint32_t>(rsSigType.dFactor)) + "u";
                mapKeywords["sig_offset"] = to_string(static_cast<uint32_t>(rsSigType.dOffset)) + "u";
                mapKeywords["sig_min"] = to_string(static_cast<uint32_t>(std::max(rsSigType.dMinimum, 0.0))) + "u";
                mapKeywords["sig_max"] = to_string(static_cast<uint32_t>(std::max(rsSigType.dMaximum, 0.0))) + "u";
                if (rsSigType.uiSize > 32)
                    mapKeywords["sig_helper_value"] = "uValueHelper.uiUint64Value";
                else
                    mapKeywords["sig_helper_value"] = "uValueHelper.s32.u32.uiValue";
            }
        }
        break;
    case dbc::SSignalDef::EValueType::ieee_float:
        mapKeywords["sig_factor"] = to_string(static_cast<float>(rsSigType.dFactor));
        mapKeywords["sig_offset"] = to_string(static_cast<float>(rsSigType.dOffset));
        mapKeywords["sig_min"] = to_string(static_cast<float>(rsSigType.dMinimum));
        mapKeywords["sig_max"] = to_string(static_cast<float>(rsSigType.dMaximum));
        mapKeywords["sig_helper_value"] = "uValueHelper.s32.u32.fValue";
        break;
    case dbc::SSignalDef::EValueType::ieee_double:
        mapKeywords["sig_factor"] = to_string(rsSigType.dFactor);
        mapKeywords["sig_offset"] = to_string(rsSigType.dOffset);
        mapKeywords["sig_min"] = to_string(rsSigType.dMinimum);
        mapKeywords["sig_max"] = to_string(rsSigType.dMaximum);
        mapKeywords["sig_helper_value"] = "uValueHelper.dValue";
        break;
    default:
    return ReplaceKeywords(R"code(<%sig_type% />)code", mapKeywords);
        break;
    }

    return ReplaceKeywords(R"code(<%sig_type%  start = "%start_value%"
            min = "%sig_min%"
            max = "%sig_max%" />)code", mapKeywords);
}

std::string CSoftcarFMUGen::CodeGetDefaultValueOfTxSignal(const dbc::SSignalDef& rsSig) const
{
    // Check for a start default value (attribute GeSigStartValue).
    std::string ssDefValue;
    std::for_each(rsSig.vecAttributes.begin(), rsSig.vecAttributes.end(), [&](const dbc::SAttributeValue& rsAttrValue)
        {
            if (!rsAttrValue.ptrAttrDef) return;
            if (rsAttrValue.ptrAttrDef->ssName != "GenSigStartValue") return;
            switch (rsAttrValue.ptrAttrDef->eType)
            {
            case dbc::SAttributeDef::EType::integer:
                if (rsSig.dFactor == std::round(rsSig.dFactor) && rsSig.dOffset == std::round(rsSig.dOffset))
                    ssDefValue = to_string(rsAttrValue.iValue * static_cast<int32_t>(rsSig.dFactor) +
                        static_cast<int32_t>(rsSig.dOffset));
                else
                    ssDefValue = to_string(static_cast<double>(rsAttrValue.iValue) * rsSig.dFactor + rsSig.dOffset);
                break;
            case dbc::SAttributeDef::EType::hex_integer:
                if (rsSig.dFactor == std::round(rsSig.dFactor) && rsSig.dOffset == std::round(rsSig.dOffset))
                    ssDefValue = to_string(rsAttrValue.uiValue * static_cast<uint32_t>(rsSig.dFactor) +
                        static_cast<uint32_t>(rsSig.dOffset));
                else
                    ssDefValue = to_string(static_cast<double>(rsAttrValue.uiValue) * rsSig.dFactor + rsSig.dOffset);
                break;
            case dbc::SAttributeDef::EType::floating_point:
                ssDefValue = to_string(rsAttrValue.dValue * rsSig.dFactor + rsSig.dOffset);
                break;
            case dbc::SAttributeDef::EType::string:
                ssDefValue = std::string("\"") + rsAttrValue.ssValue + "\"";
                break;
            case dbc::SAttributeDef::EType::enumerator:
                for (size_t nIndex = 0; nIndex < rsAttrValue.ptrAttrDef->sEnumValues.vecEnumValues.size(); nIndex++)
                {
                    if (rsAttrValue.ptrAttrDef->sEnumValues.vecEnumValues[nIndex] == rsAttrValue.ssValue)
                    {
                        ssDefValue = to_string(nIndex);
                        break;
                    }
                    if (rsAttrValue.ptrAttrDef->sEnumValues.vecEnumValues[nIndex] == rsAttrValue.ptrAttrDef->sEnumValues.ssDefault)
                        ssDefValue = to_string(nIndex);
                }
                break;
            default:
                break;
            }
        });
    return ssDefValue.empty() ? "0" : ssDefValue;
}

std::string CSoftcarFMUGen::Code_AllSignalsIdentifierList(const std::vector<signalDefinition>& rvecSignalDefinitions) const
{
    CKeywordMap mapKeywords;
    std::stringstream sstreamSignals;
    for (const auto& signal : rvecSignalDefinitions)
    {
        sstreamSignals << Code_SignalIdentifierList(signal);
    }
    mapKeywords["signals_list"] = std::move(sstreamSignals.str());

    return ReplaceKeywords(R"code(%signals_list%)code", mapKeywords);
}

std::string CSoftcarFMUGen::Code_SignalIdentifierList(const signalDefinition& rSignal) const
{
    CKeywordMap mapKeywords;
    auto startWithUppercase = rSignal.name;
    if (!startWithUppercase.empty())
    {
        startWithUppercase[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(startWithUppercase[0])));
    }
    mapKeywords["uppercase_signal_name"] = startWithUppercase;
    mapKeywords["can_message_name"] = rSignal.message_name;
    mapKeywords["can_signale_name"] = rSignal.name;

    return ReplaceKeywords(R"code(    static std::string      ds%uppercase_signal_name% = "%can_message_name%.%can_signale_name%";
)code", mapKeywords);
}

