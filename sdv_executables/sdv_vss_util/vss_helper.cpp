#include "vss_helper.h"

std::string CVSSHelper::MapDeclType2CType(const sdv::vss::EDeclType eDeclType) const
{
    switch (eDeclType)
    {
    case sdv::vss::EDeclType::decltype_short:               return "int16_t";
    case sdv::vss::EDeclType::decltype_long:                return "int32_t";
    case sdv::vss::EDeclType::decltype_long_long:           return "int64_t";
    case sdv::vss::EDeclType::decltype_unsigned_short:      return "uint16_t";
    case sdv::vss::EDeclType::decltype_unsigned_long:       return "uint32_t";
    case sdv::vss::EDeclType::decltype_unsigned_long_long:  return "uint64_t";
    case sdv::vss::EDeclType::decltype_float:               return "float";
    case sdv::vss::EDeclType::decltype_double:              return "double";
    case sdv::vss::EDeclType::decltype_long_double:         return "long double";
    case sdv::vss::EDeclType::decltype_fixed:               return "uint32_t";        // TODO: Not implemented!
    case sdv::vss::EDeclType::decltype_char:                return "char";
    case sdv::vss::EDeclType::decltype_char16:              return "char16_t";
    case sdv::vss::EDeclType::decltype_char32:              return "char32_t";
    case sdv::vss::EDeclType::decltype_wchar:               return "wchar_t";
    case sdv::vss::EDeclType::decltype_boolean:             return "bool";
    case sdv::vss::EDeclType::decltype_native:              return "size_t";
    case sdv::vss::EDeclType::decltype_octet:               return "uint8_t";
    case sdv::vss::EDeclType::decltype_string:              return "sdv::string";
    case sdv::vss::EDeclType::decltype_u8string:            return "sdv::u8string";
    case sdv::vss::EDeclType::decltype_u16string:           return "sdv::u16string";
    case sdv::vss::EDeclType::decltype_u32string:           return "sdv::u32string";
    case sdv::vss::EDeclType::decltype_wstring:             return "sdv::wstring";
    case sdv::vss::EDeclType::decltype_enum:                return "enum class";
    case sdv::vss::EDeclType::decltype_struct:              return "struct";
    case sdv::vss::EDeclType::decltype_union:               return "union";
    case sdv::vss::EDeclType::decltype_module:              return "namespace";
    case sdv::vss::EDeclType::decltype_interface:           return "interface";
    case sdv::vss::EDeclType::decltype_exception:           return "struct";
    case sdv::vss::EDeclType::decltype_attribute:           return "";
    case sdv::vss::EDeclType::decltype_operation:           return "";
    case sdv::vss::EDeclType::decltype_parameter:           return "";
    case sdv::vss::EDeclType::decltype_enum_entry:          return "";
    case sdv::vss::EDeclType::decltype_case_entry:          return "";
    case sdv::vss::EDeclType::decltype_typedef:             return "typedef";
    case sdv::vss::EDeclType::decltype_sequence:            return "sdv::sequence";
    case sdv::vss::EDeclType::decltype_pointer:             return "sdv::pointer";
    case sdv::vss::EDeclType::decltype_map:                 return "uint32_t";        // TODO: Not implemented!
    case sdv::vss::EDeclType::decltype_bitset:              return "uint32_t";        // TODO: Not implemented!
    case sdv::vss::EDeclType::decltype_bitfield:            return "uint32_t";        // TODO: Not implemented!
    case sdv::vss::EDeclType::decltype_bitmask:             return "uint32_t";        // TODO: Not implemented!
    case sdv::vss::EDeclType::decltype_any:                 return "sdv::any_t";
    case sdv::vss::EDeclType::decltype_interface_id:        return "sdv::interface_id";
    case sdv::vss::EDeclType::decltype_interface_type:      return "sdv::interface_t";
    case sdv::vss::EDeclType::decltype_exception_id:        return "sdv::exception_id";
    case sdv::vss::EDeclType::decltype_void:                return "void";
    case sdv::vss::EDeclType::decltype_unknown:
    default:
        return "";
    }
}


void CVSSHelper::CreateCMakeFile(const std::filesystem::path& filePath, const std::string& cmakeContent) const
{
    try
    {
        // create/write CMakeLists.txt only if content has changed
        std::string cmakeExisitingContent = "";

        if (std::filesystem::exists(filePath))
        {
            std::ifstream stream;
            stream.open(filePath);
            if (stream.is_open())
            {
                std::stringstream sstream;
                sstream << stream.rdbuf();
                cmakeExisitingContent = std::move(sstream.str());
            }
            stream.close();
        }

        if (cmakeContent.compare(cmakeExisitingContent) != 0)
        {
            std::ofstream fstreamCMakeFile;
            fstreamCMakeFile.open(filePath, std::ios::out | std::ios::trunc);
            fstreamCMakeFile << cmakeContent;
            fstreamCMakeFile.close();
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
}

std::filesystem::path CVSSHelper::MakeLowercaseFilename(std::filesystem::path & fullPath) const
{
    std::filesystem::path parentPath = fullPath.parent_path();
    std::string filename = fullPath.filename().string();

    std::transform(filename.begin(), filename.end(), filename.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

    return parentPath / filename;
}

std::string CVSSHelper::Align(const std::string& ssInput, size_t size) const
{
    auto alignedString = ssInput;
    while (alignedString.size() < size)
    {
        alignedString.append(" ");
    }
    return alignedString.c_str();
}

size_t CVSSHelper::GetMaxClassName(const std::vector <SSignalVDDefinition>& signals, const std::string& ssTitle) const
{
    size_t max = ssTitle.size();
    for (const auto& signal : signals)
    {
        if (max < signal.className.size())
        {
            max = signal.className.size();
        }
    }

    return max + 1;
}

size_t CVSSHelper::GetMaxIDLType(const std::vector <SSignalVDDefinition>& signals, const std::string& ssTitle) const
{
    size_t max = ssTitle.size();
    for (const auto& signal : signals)
    {
        for (auto func : signal.vecFunctions)
        {
            if (max < func.idlType.size())
            {
                max = func.idlType.size();
            }
        }
    }

    return max + 1;
}

size_t CVSSHelper::GetMaxCTypeFromIDLType(const std::vector <SSignalVDDefinition>& signals, const std::string& ssTitle) const
{
    size_t max = ssTitle.size();
    for (const auto& signal : signals)
    {
        for (auto func : signal.vecFunctions)
        {
            if (max < GetCTypeFromIDLType(func.idlType).size())
            {
                max = GetCTypeFromIDLType(func.idlType).size();
            }
        }
    }
    return max + 1;
}

size_t CVSSHelper::GetMaxSignalName(const std::vector <SSignalVDDefinition>& signals, const std::string& ssTitle) const
{
    size_t max = ssTitle.size();
    for (const auto& signal : signals)
    {
        for (auto func : signal.vecFunctions)
        {
            if (max < func.signalName.size())
            {
                max = func.signalName.size();
            }
        }
    }

    return max + 1;
}

size_t CVSSHelper::GetMaxCANSignalName(const std::vector <SSignalVDDefinition>& signals, const std::string& ssTitle) const
{
    size_t max = ssTitle.size();
    for (const auto& signal : signals)
    {
        for (auto func : signal.vecFunctions)
        {
            if (max < func.canSignalName.size())
            {
                max = func.canSignalName.size();
            }
        }
    }

    return max + 1;
}

size_t CVSSHelper::GetMaxFunctionName(const std::vector <SSignalVDDefinition>& signals, const std::string& ssTitle) const
{
    size_t max = ssTitle.size();
    for (const auto& signal : signals)
    {
        for (auto func : signal.vecFunctions)
        {
            if (max < func.functionName.size())
            {
                max = func.functionName.size();
            }
        }
    }
    return max + 1;
}

size_t CVSSHelper::GetMaxVSSDefinition(const std::vector <SSignalVDDefinition>& signals, const std::string& ssTitle) const
{
    size_t max = ssTitle.size();
    for (const auto& signal : signals)
    {
        if (max < signal.vssDefinition.size())
        {
            max = signal.vssDefinition.size();
        }
    }
    return max + 1;
}

std::string CVSSHelper::GenerateDefaultIfEmpty(const std::string& ssInput, std::string ssDefaultString, const uint32_t& index) const
{
    if (ssInput.size() > 0)
    {
        return ssInput;
    }
    if (index != 0)
    {
        ssDefaultString.append("_");
        ssDefaultString.append(std::to_string(index));
    }
    return ssDefaultString;
}

std::string CVSSHelper::Trim(const std::string& ssInput) const
{
    size_t first = ssInput.find_first_not_of(' ');
    if (first == std::string::npos) 
    {
        return ""; // No content
    }
    size_t last = ssInput.find_last_not_of(' ');
    return ssInput.substr(first, (last - first + 1));
}

bool CVSSHelper::MustNotContainSpaces(const std::string& ssInput) const
{
    if (ssInput.find(' ') != std::string::npos)
    {
        return false;
    }

    return true;
}

bool CVSSHelper::IsNumber(const std::string& ssInput) const
{
    if (!ssInput.empty())
    {
        auto character = ssInput[0];
        return std::isdigit(character);
    }

    return false;
}

std::string CVSSHelper::ValidateVSSFormatNumbers(const std::string& ssInput) const
{
    std::stringstream ss(ssInput);
    std::string part;
    std::vector<std::string> parts;

    while (std::getline(ss, part, '.')) 
    {
        if (IsNumber(part)) 
        {
            parts.push_back("_" + part);
        }
        else {
            parts.push_back(part);
        }
    }

    if (!ssInput.empty() && ssInput.back() == '.')
    {
        parts.push_back("");
    }

    std::ostringstream result;
    for (size_t i = 0; i < parts.size(); ++i) 
    {
        if (i > 0)
        {
            result << ".";
        }
        result << parts[i];
    }

    return result.str();
}

bool CVSSHelper::MustContainDotOrIsEmpty(const std::string& ssInput, bool mustBeEmpty) const
{
    if ((ssInput.size() == 0) && (mustBeEmpty)) // empty is not allowed
    {
        return false;
    }
    if (ssInput.size() == 0) // empty is allowed
    {
        return false;
    }
    if (ssInput.find('.') != std::string::npos)
    {
        return true;
    }

    return false;
}

std::string CVSSHelper::AddQuotationMarks(const std::string& value) const
{
    std::string returnString = "\"";
    returnString.append(value);
    returnString.append("\"");
    return returnString;
}

std::string CVSSHelper::ReplaceCharacters(const std::string& ssInput, const std::string& from, const std::string& to) const
{
    std::string str = ssInput;

    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos)
    {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Move past the replacement
    }
    return str;
}

std::string CVSSHelper::ToUpper(const std::string& ssInput) const
{
    std::stringstream sstreamToUpper;
    for (char c : ssInput)
    {
        sstreamToUpper << static_cast<char>(std::toupper(c));
    }

    return sstreamToUpper.str();
}

std::string CVSSHelper::ShortenVSSString(const std::string& ssInput) const
{
    size_t start_pos = ssInput.size();
    start_pos = ssInput.rfind(".", start_pos);
    if (start_pos != std::string::npos)
    {
        return ssInput.substr(0, start_pos);
    }

    return ssInput;
}

std::string CVSSHelper::CastValueType(const std::string& ssSignalType) const
{
    std::vector<std::string> stringTypes = { "sdv::string", "sdv::u8string", "sdv::u16string", "sdv::u32string", "sdv::wstring" };
    if (std::find(stringTypes.begin(), stringTypes.end(), ssSignalType) != stringTypes.end())
    {
        std::string type = "const " + ssSignalType + "&";
        return type;
    }

    return ssSignalType;
}

std::string CVSSHelper::GetCTypeFromIDLType(const std::string& ssIDLType) const

{
    for (const auto& type : m_vecDeclTypes)
    {
        if (ssIDLType == type.first)
        {
            auto s = type.second;
            return MapDeclType2CType(s);
        }
    }
    return "";
}

bool CVSSHelper::ValidateIDLTypes(const std::vector<std::string>& vecTypes, const bool silent) const
{
    for (const auto& type : vecTypes)
    {
        if (GetCTypeFromIDLType(type).empty())
        {
            if (!silent)
            {
                std::cout << "ERROR! Invalid IDL type: '" << type << "'" << std::endl;
            }
            return false; 
        }
    }

    return true;
}

void CVSSHelper::ValidateVDCodeStyle(const SSignalVDDefinition& signal, const bool verbose) const
{
    if (verbose)
    {
        if (!std::isupper(static_cast<unsigned char>(signal.className[0])))
        {
            std::cout << "Class name should start with upper case: '" << signal.className << "'" <<std::endl;
        }
    }
}
void CVSSHelper::ValidateBSCodeStyle(const SSignalBSDefinition& signal, const bool verbose) const
{
    if (verbose)
    {
        if (!std::isupper(static_cast<unsigned char>(signal.className[0])))
        {
            std::cout << "Class name should start with upper case: '" << signal.className << "'" << std::endl;
        }
    }
}

void CVSSHelper::ValidateVDCodeStyle(const SFunctionVDDefinition& function, const bool verbose) const
{
    if (verbose)
    {
        if (std::isupper(static_cast<unsigned char>(function.signalName[0])))
        {
            std::cout << "Signal name should start with lower case: '" << function.signalName << "'" << std::endl;
        }
        if (!std::isupper(static_cast<unsigned char>(function.functionName[0])))
        {
            std::cout << "Function name should start with upper case: '" << function.functionName << "'" << std::endl;
        }
    }
}
void CVSSHelper::ValidateBSCodeStyle(const SFunctionBSDefinition& function, const bool verbose) const
{
    if (verbose)
    {
        if (std::isupper(static_cast<unsigned char>(function.signalName[0])))
        {
            std::cout << "Signal name should start with lower case: '" << function.signalName << "'" << std::endl;
        }
        if (!std::isupper(static_cast<unsigned char>(function.functionName[0])))
        {
            std::cout << "Function name should start with upper case: '" << function.functionName << "'" << std::endl;
        }
    }
}

bool CVSSHelper::CreateFolder(const std::filesystem::path& rootPath, const std::string& subfolder) const
{
    auto path = rootPath;
    if (!subfolder.empty())
    {
        path = rootPath / subfolder;
    }
    try
    {
        for (size_t nCnt = 0; nCnt < 5; nCnt++)
        {
            if (!std::filesystem::exists(path))
            {
                std::filesystem::create_directories(path);
            }
            else
                break;
            std::this_thread::sleep_for(std::chrono::milliseconds(250));
        }
    }
    catch (const std::filesystem::filesystem_error& e)
    {
        std::cout << "catch (const std::filesystem::filesystem_error& e)" << std::endl;
        std::cout << "Error removing directory: " << e.what() << '\n';
    }
    return std::filesystem::exists(path);
}

void CVSSHelper::DeleteHeaderFile(const std::filesystem::path& path, const std::string& fileNameNoExtension) const
{
    auto headerFile = fileNameNoExtension;
    headerFile.append(".h");
    auto file = path / headerFile;
    try
    {
        if (!std::filesystem::is_directory(file))
        {
            std::filesystem::remove(file);
        }
    }
    catch (const std::filesystem::filesystem_error& e)
    {
        std::cout << "catch (const std::filesystem::filesystem_error& e)" << std::endl;
        std::cout << "Error removing directory: " << e.what() << '\n';
    }
}

bool CVSSHelper::VehicleDeviceVSSDefinitionExists(bool bSilent, const std::vector <SSignalVDDefinition>& vdSignals,
    const std::vector <SSignalBSDefinition>& bsSignals)
{
    if (bsSignals.size())
    {
        std::unordered_set<std::string> vssVDDefinitions;
        for (const auto& vssDefinition : vdSignals)
        {
            vssVDDefinitions.insert(vssDefinition.vssDefinition);
        }

        for (const auto& vssDefinition : bsSignals)
        {
            if (vssVDDefinitions.find(vssDefinition.vssVDDefinition) == vssVDDefinitions.end())
            {
                if (!bSilent)
                {
                    std::cout << vssDefinition.vssVDDefinition << " cannot be found, requested by " << vssDefinition.vssDefinition << std::endl;
                }
                return false;
            }
        }
    }
    return true;
}
