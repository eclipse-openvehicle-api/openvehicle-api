#ifndef VSS_HELPER_H
#define VSS_HELPER_H

#include <iostream>
#include <vector>
#include <unordered_set>
#include <algorithm>
#include <string>
#include <sstream>
#include <fstream>
#include <thread>
#include <chrono>
//#include <algorithm>
#include <interfaces/dispatch.h>

namespace sdv
{
    /**
     * @brief vss namespace
     */
    namespace vss
    {
        /**
         * @brief The entity declaration type.
         */
        enum class EDeclType : uint32_t
        {
            /// Unknown type
            decltype_unknown,
            /// Integer type (int16_t)
            decltype_short,
            /// Integer type (int32_t)
            decltype_long,
            /// Integer type (int64_t)
            decltype_long_long,
            /// Octet type (uint8_t)
            decltype_octet,
            /// Integer type (uint16_t)
            decltype_unsigned_short,
            /// Integer type (uint32)
            decltype_unsigned_long,
            /// Integer type (uint64_t)
            decltype_unsigned_long_long,
            /// Floating point type
            decltype_float,
            /// Floating point type
            decltype_double,
            /// Floating point type
            decltype_long_double,
            /// Fixed point templated type
            decltype_fixed,
            /// Character type (int8_t)
            decltype_char,
            /// UTF-16 character
            decltype_char16,
            /// UTF-32 character
            decltype_char32,
            /// Character type
            decltype_wchar,
            /// Boolean type
            decltype_boolean,
            /// Native type
            decltype_native,
            /// ASCII templated string type
            decltype_string,
            /// UTF-8 templated string type
            decltype_u8string,
            /// UTF-16 templated string type
            decltype_u16string,
            /// UTF-32 templated string type
            decltype_u32string,
            /// Wide templated string type
            decltype_wstring,
            /// Enum type
            decltype_enum,
            /// Struct type
            decltype_struct,
            /// Union type
            decltype_union,
            /// Module type
            decltype_module,
            /// Interface type (not used in declarations)
            decltype_interface,
            /// Interface type (not used in declarations)
            decltype_exception,
            /// Attribute type (not used in declarations)
            decltype_attribute,
            /// Operation type (not used in declarations)
            decltype_operation,
            /// Parameter type (only for operations)
            decltype_parameter,
            /// Enum entry type (only for enums)
            decltype_enum_entry,
            /// Case entry type (only for unions)
            decltype_case_entry,
            /// Typedef type
            decltype_typedef,
            /// Void type (only for operations)
            decltype_void,
            /// Meta data type
            decltype_meta,
            /// Pointer templated data type
            decltype_pointer,
            /// Sequence templated data type
            decltype_sequence,
            /// Map templated data type
            decltype_map,
            /// Bitset data type
            decltype_bitset,
            /// Bitfield templated data type
            decltype_bitfield,
            /// Bitmask data type
            decltype_bitmask,
            /// Any composite data type
            decltype_any,
            /// Interface ID data type
            decltype_interface_id,
            /// Interface data type
            decltype_interface_type,
            /// Exception ID data type
            decltype_exception_id,
        };
    }
}

/**
 * @brief VSS code generator helper class.
*/
class CVSSHelper
{

public:

    /**
     * @brief Signal definition structure (vehicle device) with a single function/value combination
    */
    struct SFunctionVDDefinition
    {
        std::string idlType;                            ///< signal value type (IDL: boolean, float, uint8, ...)
        std::string functionName;                       ///< signal value set function name
        std::string signalName;                         ///< signal name
        std::string canSignalName;                      ///< can signal name (combination of signak messages and signal name)
    };

    /**
     * @brief Signal definition structure (basic service) with a single function/value combination
    */
    struct SFunctionBSDefinition
    {
        std::string idlType;                            ///< signal value type (IDL: boolean, float, uint8, ...)
        std::string functionName;                       ///< signal value set function name
        std::string signalName;                         ///< signal name
    };

    /**
     * @brief Signal definition structure (vehicle device) with a single function/value combination
    */
    struct SSignalVDDefinition
    {
        std::string vssDefinition;                        ///< vss interface string
        std::string className;                            ///< class name
        sdv::core::ESignalDirection signalDirection = sdv::core::ESignalDirection::sigdir_rx; ///< signal direction, either rx or tx
        std::vector<SFunctionVDDefinition> vecFunctions; ///< list of functions of the vss interface
    };

    /**
     * @brief Signal definition structure (basic service) with a single function/value combination
    */
    struct SSignalBSDefinition
    {
        std::string vssDefinition;                        ///< vss interface string
        std::string vssVDDefinition;                      ///< vss interface string
        std::string className;                            ///< class name
        sdv::core::ESignalDirection signalDirection = sdv::core::ESignalDirection::sigdir_rx; ///< signal direction, either rx or tx
        std::vector<SFunctionBSDefinition> vecFunctions; ///< list of functions of the vss interface
    };

    /**
    * @brief Check if all required VD definitions are found required by the BS signals
    * @param[in] bSilent if true do not print error to console.
    * @param[in] vdSignals containing all signals definitions of the vehicle devices.
    * @param[in] bsSignals containing all signals definitions of the basic services.
    * @return Returns true on success when all definitions are found, otherwise false.
    */
   static bool VehicleDeviceVSSDefinitionExists(bool bSilent, const std::vector<SSignalVDDefinition>& vdSignals,
        const std::vector<SSignalBSDefinition>& bsSignals);

protected:

    /**
    * @brief Create the cmake if not exists and if content has changed
    * @param[in] filePath full path to the file
    * @param[in] cmakeContent content of the cmake file
    */
    void CreateCMakeFile(const std::filesystem::path& filePath, const std::string& cmakeContent) const;

    /**
    * @brief Converts the filename of a full path to lowercase
    * @param[in] fullPath full path 
    * @return Returns full path but the filename is lowercase
    */    
    std::filesystem::path MakeLowercaseFilename(std::filesystem::path& fullPath) const;

    /**
    * @brief Add multiple space characters to increase the input string to a certain size.
    * @param[in] ssInput Reference to the input string.
    * @param[in] size of the final string.
    * @return Returns the string increased by space characters.
    */
    std::string Align(const std::string& ssInput, size_t size) const;

    /**
    * @brief trim spaces from begin of the string and end of the string
    * @param[in] ssInput Reference to the input string to be checked.
    * @return Returns string without the spaces at front and at the end
    */
    std::string Trim(const std::string& ssInput) const;

    /**
    * @brief check for space characters in the string.
    * @param[in] ssInput Reference to the input string to be checked.
    * @return Returns true if string contains no space character, otherwise false.
    */
    bool MustNotContainSpaces(const std::string& ssInput) const;

    /**
    * @brief Get the maximal string size of all class names in the vector container.
    * @param[in] signals containing all signals definitions which have to be checked.
    * @param[in] ssTitle Reference to the title string. Is minimum of the return value.
    * @return Returns the string size of the largest class name string.
    */
    size_t GetMaxClassName(const std::vector<SSignalVDDefinition>& signals, const std::string& ssTitle) const;

    /**
    * @brief Get the maximal string size of all IDL type strings in the vector container.
    * @param[in] signals containing all signals definitions which have to be checked.
    * @param[in] ssTitle Reference to the title string. Is minimum of the return value.
    * @return Returns the string size of the largest IDL type string.
    */
    size_t GetMaxIDLType(const std::vector<SSignalVDDefinition>& signals, const std::string& ssTitle) const;

    /**
    * @brief Get the maximal string size of all ctype strings in the vector container.
    * @param[in] signals containing all signals definitions which have to be checked.
    * @param[in] ssTitle Reference to the title string. Is minimum of the return value.
    * @return Returns the string size of the largest ctype string.
    */
    size_t GetMaxCTypeFromIDLType(const std::vector<SSignalVDDefinition>& signals, const std::string& ssTitle) const;

    /**
    * @brief Get the maximal string size of all signal name strings in the vector container.
    * @param[in] signals containing all signals definitions which have to be checked.
    * @param[in] ssTitle Reference to the title string. Is minimum of the return value.
    * @return Returns the string size of the largest signal names string.
    */
    size_t GetMaxSignalName(const std::vector<SSignalVDDefinition>& signals, const std::string& ssTitle) const;

    /**
    * @brief Get the maximal string size of all CAN signal name strings in the vector container.
    * @param[in] signals containing all signals definitions which have to be checked.
    * @param[in] ssTitle Reference to the title string. Is minimum of the return value.
    * @return Returns the string size of the largest signal names string.
    */
    size_t GetMaxCANSignalName(const std::vector<SSignalVDDefinition>& signals, const std::string& ssTitle) const;

    /**
    * @brief Get the maximal string size of all function name strings in the vector container.
    * @param[in] signals containing all signals definitions which have to be checked.
    * @param[in] ssTitle Reference to the title string. Is minimum of the return value.
    * @return Returns the string size of the largest function names string.
    */
    size_t GetMaxFunctionName(const std::vector<SSignalVDDefinition>& signals, const std::string& ssTitle) const;

    /**
    * @brief Get the maximal string size of all vss strings in the vector container.
    * @param[in] signals containing all signals definitions which have to be checked.
    * @param[in] ssTitle Reference to the title string. Is minimum of the return value.
    * @return Returns the string size of the largest vss string.
    */
    size_t GetMaxVSSDefinition(const std::vector<SSignalVDDefinition>& signals, const std::string& ssTitle) const;

    /**
    * @brief check for '.' characters in the string.
    * @param[in] ssInput Reference to the input string to be checked.
    * @param[in] mustBeEmpty If true input string must not be empty
    * @return Returns true if string contains '.' or is empty depending on mustBeEmpty, otherwise false.
    */
    bool MustContainDotOrIsEmpty(const std::string& ssInput, bool mustBeEmpty) const;

    /**
    * @brief check if vss parts are string and no number. If number add '_'
    *   for vehicle.chassis.axel.01.left return vehicle.chassis.axel._01.left
    * @param[in] ssInput Reference to the input string to be checked.
    * @return Returns valid vss string.
    */
    std::string ValidateVSSFormatNumbers(const std::string& ssInput) const;

    /**
    * @brief check if string is a number
    * @param[in] ssInput Reference to the input string to be checked.
    * @return Returns false if it is a number, otherwise true.
    */
    bool IsNumber(const std::string& ssInput) const;

    /**
    * @brief Returns input string or if empty, returns default string.
    * @param[in] ssInput string
    * @param[in] ssDefaultString string used as default if input string is empty
    * @param[in] index number will be added to the default string.
    * @return Returns input string or if empty returns default string added by the number.
    */
    std::string GenerateDefaultIfEmpty(const std::string& ssInput, std::string ssDefaultString, const uint32_t& index) const;

    /**
    * @brief Returns input string but starts and ends with "'".
    * @param[in] ssInput string
    * @return Returns the input string with "'" added at the frond and the end.
    */
    std::string AddQuotationMarks(const std::string& ssInput) const;

    /**
    * @brief Replaces some characters of the input string.
    * @param[in] ssInput string.
    * @param[in] from character to be replaced
    * @param[in] to replacement character.
    * @return Returns the string with replacements.
    */
    std::string ReplaceCharacters(const std::string& ssInput, const std::string& from, const std::string& to) const;

    /**
    * @brief turn the input string to upper case
    * @param[in] ssInput string.
    * @return Returns the input string but all characters are upper case
    */
    std::string ToUpper(const std::string& ssInput) const;

    /**
    * @brief remove the last part of the vss string
    * @param[in] ssInput vss string
    * @return Returns the string but removed the last part after charatcer '.'.
    */
    std::string ShortenVSSString(const std::string& ssInput) const;

    /**
    * @brief add to the type a cast, for example for 'std::string' it adds "const std::string&'
    * @param[in]ssSignalType the string representing the type
    * @return Returns either original string or 'const' and '&' added.
    */
    std::string CastValueType(const std::string& ssSignalType) const;

    /**
    * @brief find IDL type and return CType
    * @param[in] ssIDLType string representing IDL type
    * @return CType string if IDL type found, otherwise empty string
    */
    std::string GetCTypeFromIDLType(const std::string& ssIDLType) const;

    /**
    * @brief validate if all IDL type are valid
    * @param[in] vecTypes container of the IDL types
    * @param[in] silent information to console if not silent
    * @return True if IDL types are valid, otherwise false
    */
    bool ValidateIDLTypes(const std::vector<std::string>& vecTypes, const bool silent) const;

    /**
    * @brief Create folder or sub folder
    * @param[in] rootPath complete root path
    * @param[in] subfolder in case this parameter is empty only root path is created otherwise rootPath + subfolder
    * @return true if directory exists or is created successfully, otherwise false
    */
    bool CreateFolder(const std::filesystem::path& rootPath, const std::string& subfolder) const;

    /**
    * @brief deletes a header file
    * @param[in] path full path to a folder
    * @param[in] fileNameNoExtension a file name without the extension
    */
    void DeleteHeaderFile(const std::filesystem::path& path, const std::string& fileNameNoExtension) const;

    /**
    * @brief Validates the code style (signal name starts with lower case, function name upper case ...
    * @param[in] function function definition structure
    * @param[in] verbose print information to console only if true
    */
    void ValidateVDCodeStyle(const SFunctionVDDefinition& function, const bool verbose) const;

    /**
    * @brief Validates the code style (signal name starts with lower case, function name upper case ...
    * @param[in] function function definition structure
    * @param[in] verbose print information to console only if true
    */
    void ValidateBSCodeStyle(const SFunctionBSDefinition& function, const bool verbose) const;

    /**
    * @brief Validates the code style (signal name starts with lower case, function name upper case ...
    * @param[in] signal signal definition structure 
    * @param[in] verbose print information to console only if true
    */
    void ValidateVDCodeStyle(const SSignalVDDefinition& signal, const bool verbose) const;

    /**
    * @brief Validates the code style (signal name starts with lower case, function name upper case ...
    * @param[in] signal signal definition structure
    * @param[in] verbose print information to console only if true
    */
    void ValidateBSCodeStyle(const SSignalBSDefinition& signal, const bool verbose) const; 

private:

    /**
* @brief Declaration type name association.
*/
    using TDeclTypeAssoc = std::pair<std::string, sdv::vss::EDeclType>;

    /**
    * @brief change EDeclType to cType string
    * @param[in] eDeclType EDeclTyp type
    * @return Returns cType stringif available, otherwise empty string
    */
    std::string MapDeclType2CType(const sdv::vss::EDeclType eDeclType) const;

    /**
    * @brief Mapping between declaration type and string name.
    * @attention Some types have multiple string representatives (e.g. sdv::vss::EDeclType::decltype_short vs. 'short' and
    * 'int16').
    * @attention Based on the provided extensions on the command line, the types might be extended by: char16, char32, u8string,
    * u16string, u32string, pointer, interface_id, interface_t and exception_id
    */
    const std::vector<TDeclTypeAssoc> m_vecDeclTypes = {
        {"short", sdv::vss::EDeclType::decltype_short},
        {"unsigned short", sdv::vss::EDeclType::decltype_unsigned_short},
        {"long", sdv::vss::EDeclType::decltype_long},
        {"unsigned long", sdv::vss::EDeclType::decltype_unsigned_long},
        {"long long", sdv::vss::EDeclType::decltype_long_long},
        {"unsigned long long", sdv::vss::EDeclType::decltype_unsigned_long_long},
        {"fixed", sdv::vss::EDeclType::decltype_fixed},
        {"float", sdv::vss::EDeclType::decltype_float},
        {"double", sdv::vss::EDeclType::decltype_double},
        {"long double", sdv::vss::EDeclType::decltype_long_double},
        {"char", sdv::vss::EDeclType::decltype_char},
        {"wchar", sdv::vss::EDeclType::decltype_wchar},
        {"int8", sdv::vss::EDeclType::decltype_char},
        {"int16", sdv::vss::EDeclType::decltype_short},
        {"int32", sdv::vss::EDeclType::decltype_long},
        {"int64", sdv::vss::EDeclType::decltype_long_long},
        {"int", sdv::vss::EDeclType::decltype_long},
        {"uint8", sdv::vss::EDeclType::decltype_octet},
        {"uint16", sdv::vss::EDeclType::decltype_unsigned_short},
        {"uint32", sdv::vss::EDeclType::decltype_unsigned_long},
        {"uint64", sdv::vss::EDeclType::decltype_unsigned_long_long},
        {"uint", sdv::vss::EDeclType::decltype_unsigned_long},
        {"boolean", sdv::vss::EDeclType::decltype_boolean},
        {"native", sdv::vss::EDeclType::decltype_native},
        {"octet", sdv::vss::EDeclType::decltype_octet},
        {"byte", sdv::vss::EDeclType::decltype_octet},
        {"string", sdv::vss::EDeclType::decltype_string},
        {"wstring", sdv::vss::EDeclType::decltype_wstring},
        {"enum", sdv::vss::EDeclType::decltype_enum},
        {"struct", sdv::vss::EDeclType::decltype_struct},
        {"union", sdv::vss::EDeclType::decltype_union},
        {"operation", sdv::vss::EDeclType::decltype_operation},
        {"attribute", sdv::vss::EDeclType::decltype_parameter},
        {"enum_entry", sdv::vss::EDeclType::decltype_enum_entry},
        {"case_entry", sdv::vss::EDeclType::decltype_case_entry},
        {"typedef", sdv::vss::EDeclType::decltype_typedef},
        {"sequence", sdv::vss::EDeclType::decltype_sequence},
        {"map", sdv::vss::EDeclType::decltype_map},
        {"bitset", sdv::vss::EDeclType::decltype_bitset},
        {"bitfield", sdv::vss::EDeclType::decltype_bitfield},
        {"bitmask", sdv::vss::EDeclType::decltype_bitmask},
        {"any", sdv::vss::EDeclType::decltype_any},
        {"void", sdv::vss::EDeclType::decltype_void},
        // Add additional types based on enabled extensions
        {"interface_id", sdv::vss::EDeclType::decltype_interface_id},
        {"interface_t", sdv::vss::EDeclType::decltype_interface_type},
        {"exception_id", sdv::vss::EDeclType::decltype_exception_id},
        {"pointer", sdv::vss::EDeclType::decltype_pointer},
        {"char16", sdv::vss::EDeclType::decltype_char16},
        {"char32", sdv::vss::EDeclType::decltype_char32},
        {"u8string", sdv::vss::EDeclType::decltype_u8string},
        {"u16string", sdv::vss::EDeclType::decltype_u16string},
        {"u32string", sdv::vss::EDeclType::decltype_u32string}
    };
};

#endif // !defined VSS_HELPER_H
