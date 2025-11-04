#include "vss_coding.h"


void CVSSCodingCommon::GetCommonKeyWordMap(const SSignalVDDefinition& signal, CKeywordMap& mapKeywords, const std::string& rsVersion) const
{
    CVSSCodingCommon coding(m_ssPrefix);
    //---------------------------------------------------------------------------------------------- Common file
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream sstreamDate;
    sstreamDate << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X");
    mapKeywords["creation_date"] = sstreamDate.str();
    mapKeywords["version"] = coding.Code_VSSFileVersion(rsVersion);

    //---------------------------------------------------------------------------------------------- Common
    // original vss example: Vehicle.Chassis.SteeringWheel.Angle
    // vssNoDot:           VehicleChassisSteeringWheelAngle 
    // vssWithUnderscores:   Vehicle_Chassis_SteeringWheel_Angle
    // vssUppercase:         VEHICLE_CHASSIS_STEERINGWHEEL_ANGLE
    // vssWithColons:        Vehicle::Chassis::SteeringWheel::Angle
    // vssShorten:           Vehicle.Chassis.SteeringWheel
    // vssShortenNoDot:    VehicleChassisSteeringWheel
    // vssShortenWithColons: Vehicle::Chassis::SteeringWheel

    mapKeywords["vss_original"] = signal.vssDefinition;
    std::string vssNoDot = ReplaceCharacters(signal.vssDefinition, ".", "");
    std::string vssWithUnderscores = ReplaceCharacters(signal.vssDefinition, ".", "_");
    std::string vssUppercase = ToUpper(vssWithUnderscores);
    std::string vssWithColons = ReplaceCharacters(signal.vssDefinition, ".", "::");
    std::string vssShorten = ShortenVSSString(signal.vssDefinition);
    std::string vssShortenNoDot = ReplaceCharacters(vssShorten, ".", "");
    std::string vssShortenWithColons = ReplaceCharacters(vssShorten, ".", "::");
    std::string class_name_lowercase = signal.className;
    std::transform(class_name_lowercase.begin(), class_name_lowercase.end(), class_name_lowercase.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

    mapKeywords["vss_original_no_dot"] = vssNoDot;
    mapKeywords["vss_uppercase"] = vssUppercase;
    mapKeywords["vss_with_colons"] = vssWithColons;
    mapKeywords["vss_shorten"] = vssShorten;
    mapKeywords["vss_shorten_no_dot"] = vssShortenNoDot;
    mapKeywords["vss_shorten_with_colons"] = vssShortenWithColons;
    mapKeywords["class_name"] = signal.className;
    mapKeywords["class_name_lowercase"] = class_name_lowercase;
}

void CVSSCodingCommon::GetCommonKeyWordMap(const SSignalBSDefinition& signal, CKeywordMap& mapKeywords, const std::string& rsVersion) const
{
    CVSSCodingCommon coding(m_ssPrefix);
    //---------------------------------------------------------------------------------------------- Common file
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream sstreamDate;
    sstreamDate << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X");
    mapKeywords["creation_date"] = sstreamDate.str();
    mapKeywords["version"] = coding.Code_VSSFileVersion(rsVersion);

    //---------------------------------------------------------------------------------------------- Common
    // original vss example: Vehicle.Chassis.SteeringWheel.Angle
    // vssNoDot:           VehicleChassisSteeringWheelAngle 
    // vssWithUnderscores:   Vehicle_Chassis_SteeringWheel_Angle
    // vssUppercase:         VEHICLE_CHASSIS_STEERINGWHEEL_ANGLE
    // vssWithColons:        Vehicle::Chassis::SteeringWheel::Angle
    // vssShorten:           Vehicle.Chassis.SteeringWheel
    // vssShortenNoDot:    VehicleChassisSteeringWheel
    // vssShortenWithColons: Vehicle::Chassis::SteeringWheel

    mapKeywords["vss_original"] = signal.vssDefinition;
    std::string vssNoDot = ReplaceCharacters(signal.vssDefinition, ".", "");
    std::string vssWithUnderscores = ReplaceCharacters(signal.vssDefinition, ".", "_");
    std::string vssUppercase = ToUpper(vssWithUnderscores);
    std::string vssWithColons = ReplaceCharacters(signal.vssDefinition, ".", "::");
    std::string vssShorten = ShortenVSSString(signal.vssDefinition);
    std::string vssShortenNoDot = ReplaceCharacters(vssShorten, ".", "");
    std::string vssShortenWithColons = ReplaceCharacters(vssShorten, ".", "::");
    std::string class_name_lowercase = signal.className;
    std::transform(class_name_lowercase.begin(), class_name_lowercase.end(), class_name_lowercase.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

    mapKeywords["vss_original_no_dot"] = vssNoDot;
    mapKeywords["vss_uppercase"] = vssUppercase;
    mapKeywords["vss_with_colons"] = vssWithColons;
    mapKeywords["vss_shorten"] = vssShorten;
    mapKeywords["vss_shorten_no_dot"] = vssShortenNoDot;
    mapKeywords["vss_shorten_with_colons"] = vssShortenWithColons;
    mapKeywords["class_name"] = signal.className;
    mapKeywords["class_name_lowercase"] = class_name_lowercase;
}

std::string CVSSCodingCommon::Code_VSSFileVersion(const std::string& ssVersion) const
{
    std::string version = ".";
    CKeywordMap mapKeywords;
    if (!ssVersion.empty())
    {
        version = "\n * VSS Version:" + ssVersion;
    }

    mapKeywords["version"] = version;
    return ReplaceKeywords(R"code(File is auto generated from VSS utility%version%)code", mapKeywords);
}

std::string CVSSCodingCommon::Code_SafeGuard(const std::string& ssFileName) const
{
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::string startWith = "__VSS_GENERATED__" + ssFileName + "_";

    std::stringstream sstreamSafeguard;
    sstreamSafeguard << ToUpper(ReplaceCharacters(startWith, ".", "_")); //"__DBC_GENERATED__CONFIG_H__";
    sstreamSafeguard << std::put_time(std::localtime(&in_time_t), "%Y%m%d_%H%M%S") << "_";
    sstreamSafeguard << std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count() % 1000;
    sstreamSafeguard << "__";
    return sstreamSafeguard.str();
}

std::string CVSSCodingCommon::Code_SignalIdentifier(const std::vector <SSignalVDDefinition>& allSignals, 
    const std::string& ssFileName, const std::string& ssVersion) const
{
    CKeywordMap mapKeywords;
    if (allSignals.size() > 0)
    {
        std::string signalTitle = "signal names";
        std::string canSignalTitle = "dbc variable names";
        std::string cTypeTitle = "C-type";
        std::string directionTitle = "RX/TX";
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream sstreamDate;
        sstreamDate << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X");
        mapKeywords["creation_date"] = sstreamDate.str();
        mapKeywords["version"] = Code_VSSFileVersion(ssVersion);
        mapKeywords["file_name"] = ssFileName;
        mapKeywords["safeguard"] = Code_SafeGuard(ssFileName);

        mapKeywords["object_prefix"] = m_ssPrefix;
        mapKeywords["signals"] = Code_AllSignalsIdentifierList(allSignals, signalTitle, canSignalTitle, cTypeTitle);
        mapKeywords["spaces01"] = Align(signalTitle, GetMaxSignalName(allSignals, signalTitle) + 1);
        mapKeywords["spaces02"] = Align(canSignalTitle, GetMaxCANSignalName(allSignals, canSignalTitle) + 1);
        mapKeywords["spaces03"] = Align(cTypeTitle, GetMaxCTypeFromIDLType(allSignals, cTypeTitle));
        mapKeywords["spaces04"] = Align(directionTitle, directionTitle.size() + 1);
    }

    return ReplaceKeywords(R"code(/**
 * @file %file_name%
 * @date %creation_date%
 * %version%
 */
#ifndef %safeguard%
#define %safeguard%

namespace %object_prefix%
{
    // Data Dispatch Service  %spaces01%to  %spaces02%      %spaces03% %spaces04% vss name space
%signals%} // %object_prefix%

#endif // %safeguard%
)code", mapKeywords);
}

std::string CVSSCodingCommon::Code_AllSignalsIdentifierList(const std::vector <SSignalVDDefinition>& allSignals,
    const std::string& signalTitle, const std::string& canSignalTitle, const std::string& cTypeTitle) const
{
    CKeywordMap mapKeywords;

    std::stringstream sstreamSignals;
    for (const auto& signal : allSignals)
    {
        for (auto func : signal.vecFunctions)
        {
            sstreamSignals << Code_SignalIdentifierList(allSignals, signal, func, signalTitle, canSignalTitle, cTypeTitle);
        }
    }
    mapKeywords["signals_list"] = std::move(sstreamSignals.str());

    return ReplaceKeywords(R"code(%signals_list%)code", mapKeywords);
}

std::string CVSSCodingCommon::Code_SignalIdentifierList(const std::vector <SSignalVDDefinition>& allSignals, const SSignalVDDefinition& signal,
    const SFunctionVDDefinition& func, const std::string& signalTitle, const std::string& canSignalTitle, const std::string& cTypeTitle) const
{
    CKeywordMap mapKeywords;
    auto startWithUppercase = Align(func.signalName, GetMaxSignalName(allSignals, "") + 1);
    if (!startWithUppercase.empty())
    {
        startWithUppercase[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(startWithUppercase[0])));
    }
    mapKeywords["start_with_uppercase"] = Align(startWithUppercase, GetMaxSignalName(allSignals, signalTitle) + 1);
    mapKeywords["can_signal_name"] = Align(AddQuotationMarks(func.canSignalName), GetMaxCANSignalName(allSignals, canSignalTitle) + 1);
    mapKeywords["value_ctype"] = Align(GetCTypeFromIDLType(func.idlType), GetMaxCTypeFromIDLType(allSignals, cTypeTitle));

    if (signal.signalDirection == sdv::core::ESignalDirection::sigdir_rx)
        mapKeywords["signal_dir"] = "RX   ";
    else
        mapKeywords["signal_dir"] = "TX   ";
    mapKeywords["vss"] = Align(signal.vssDefinition, GetMaxVSSDefinition(allSignals, ""));

    return ReplaceKeywords(R"code(    static std::string      ds%start_with_uppercase%=  %can_signal_name%; ///< %value_ctype% %signal_dir%  %vss%
)code", mapKeywords);
}

std::string CVSSCodingCommon::Code_CMakeProject(const std::string& targetLibName, const std::string& targetComponentName) const
{
    CKeywordMap mapKeywords;
    mapKeywords["target_lib_name"] = targetLibName;
    mapKeywords["component_file_name"] = targetComponentName;
    return ReplaceKeywords(R"code(# Enforce CMake version 3.20 or newer needed for path function
cmake_minimum_required (VERSION 3.20)

# Use new policy for project version settings and default warning level
cmake_policy(SET CMP0048 NEW)   # requires CMake 3.14
cmake_policy(SET CMP0092 NEW)   # requires CMake 3.15

# Define project
project(%target_lib_name% VERSION 1.0 LANGUAGES CXX)

# Use C++17 support
set(CMAKE_CXX_STANDARD 17)

# Library symbols are hidden by default
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

# Set target name.
set(TARGET_NAME %target_lib_name%)

# Set the SDV_FRAMEWORK_DEV_INCLUDE if not defined yet
if (NOT DEFINED SDV_FRAMEWORK_DEV_INCLUDE)
    if (NOT DEFINED ENV{SDV_FRAMEWORK_DEV_INCLUDE})
        message( FATAL_ERROR "The environment variable SDV_FRAMEWORK_DEV_INCLUDE needs to be pointing to the SDV V-API development include files location!")
    endif()
    set (SDV_FRAMEWORK_DEV_INCLUDE "$ENV{SDV_FRAMEWORK_DEV_INCLUDE}")
endif()

# Include link to export directory of SDV V-API development include files location
include_directories(${SDV_FRAMEWORK_DEV_INCLUDE})

# Set platform specific compile flags
if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_options(/W4 /WX /wd4996 /wd4100 /permissive- /Zc:rvalueCast)
else()
    add_compile_options(-Werror -Wall -Wextra -Wshadow -Wpedantic -Wunreachable-code -fno-common)
endif()

# Add the dynamic library
add_library(${TARGET_NAME} SHARED
    %component_file_name%.h
    %component_file_name%.cpp)

# Set extension to .sdv
set_target_properties(${TARGET_NAME} PROPERTIES PREFIX "")
set_target_properties(${TARGET_NAME} PROPERTIES SUFFIX ".sdv")

# TODO: set target name.
#add_dependencies(${TARGET_NAME} <add_cmake_target_this_depends_on>)
)code", mapKeywords);
}