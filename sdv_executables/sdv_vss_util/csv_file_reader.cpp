#include "csv_file_reader.h"

CCSVFileReader::CCSVFileReader(const std::string& ssFileName, const bool bSilent, const bool bVerbose)
{
    m_silent = bSilent;
    m_verbose = bVerbose;
    m_vdSignals.clear();
    m_bsSignals.clear();
    if (ssFileName.empty())
    {
        Help();
        return;
    }
    else if (!ReadCSVVSSFile(ssFileName))
    {
        std::cout << "\nError: '" << ssFileName.c_str() << "' file could not be oppened or has no vss definitions." << std::endl;
        Help();
        return;
    }
}

void CCSVFileReader::Help()
{
    std::cout << std::endl;
    std::cout << "-----------------------------------------------------------------------------------------------------------" << std::endl;
    std::cout << "Required: An excel text file in csv format, the separator should be ';'" << std::endl;
    std::cout << "    [required] Cells must not contain spaces." << std::endl;
    std::cout << "    [required] Column 'Device Type' must be either 'VD' or 'BS'." << std::endl;
    std::cout << "    [required] Column 'vss' must contain a character '.' or can be empty." << std::endl;
    std::cout << "    [required] Column signal direction, either 'RX' or 'TX' or empty." << std::endl;
    std::cout << "    [required] Column signal value type must not be empty." << std::endl;
    std::cout << "    [required] Column 'CAN name' must contain a character '.' (for vehicle device signal)." << std::endl;
    std::cout << "    [required] Last column: in case of vehicle device it is 'CAN name'." << std::endl;
    std::cout << "    [required] Last column: in case of basic service it is avss from vehicle device." << std::endl;
    std::cout << "    [optional] additonal column: c++ code to recalulate the signal value in the vehicle device." << std::endl;
    std::cout << "    [recommended] Cells may be empty. In that case either a default string is created or previous entry is used." << std::endl;
    std::cout << "    [recommended] Column Class name should start with uppercase character." << std::endl;
    std::cout << "    [recommended] Column Function name should start with uppercase character." << std::endl;
    std::cout << "    [recommended] Column Signal name should start with lowercase character." << std::endl;
    std::cout << "Summary can be found in file 'summary.txt'." << std::endl;
    std::cout << "Columns:" << std::endl;
    std::cout << "Device Type ';' Class name         ';' Function name   ';' Signal name  ';' Interface (vss)                             ';' Direction ';' Signal value type ';' 'CAN name' or Interface(vss); Formula (is optional, normally empty)" << std::endl;
    std::cout << std::endl;
    std::cout << "e.g." << std::endl;
    std::cout << "VD          ';' FrontWheel         ';' SetAverageAngle ';' averageAngle ';' Vehicle.Chassis.Axle.Row.Wheel.AverageAngle ';' RX        ';' float              ';' CANMsg.Angle" << std::endl;
    std::cout << "BS          ';' VehicleChassisAxle ';' SetAngleStatus  ';' angleStatus  ';' Vehicle.Chassis.Axle.Row.Wheel              ';' TX        ';' uint8_t            ';' Vehicle.Chassis.Axle.Row.Wheel" << std::endl;
    std::cout << "-----------------------------------------------------------------------------------------------------------" << std::endl;
}

bool CCSVFileReader::ReadCSVVSSFile(const std::string& ssFileName)
{
    std::ifstream file(ssFileName);
    if (!file.is_open()) 
    {
        std::cerr << "'" << ssFileName << "' could not be opened." << std::endl;
        return false;
    }
    std::string line;
    uint32_t index = 1;
    while (std::getline(file, line))
    {
        line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
        ReadLine(line, index++, vssVDColumns::column_end);
    }
    file.close();

    if ((m_vdSignals.size() == 0) && (m_bsSignals.size() == 0))
    {
        if (!m_silent)
        {
            std::cout << "No valid signal definitions found." << std::endl;
        }
    }
    return true;
}

void CCSVFileReader::ReadLine(const std::string& ssLine, const uint32_t index, size_t endColumn)
{
    std::stringstream ss(ssLine);
    std::string item;
    std::vector<std::string> parts;
    while (std::getline(ss, item, ';'))
    {
        item = Trim(item);
        auto cType = GetCTypeFromIDLType(item); // maybe its a IDL type

        if ((parts.size() == (vssVDColumns::column_vdFormula)) && !item.empty())
        {
            // we have the formula column, if not empty, collect everything
            auto collectItem = item;
            while (std::getline(ss, item, ';'))
            {
                collectItem.append(";");
                collectItem.append(item);
            }

            // Remove " from front and end
            if (!collectItem.empty() && collectItem.front() == '"' && collectItem.back() == '"') {
                collectItem = collectItem.substr(1, collectItem.size() - 2);
            }

            // Replace all occurrences of "" with "
            collectItem = std::regex_replace(collectItem, std::regex("\"\""), "\"");

            parts.push_back(collectItem);
        }
        else if (MustNotContainSpaces(item) || (cType.size() != 0))
        {
            parts.push_back(item);
        }
        else
        {
            if ((m_vdSignals.size() != 0) && (m_bsSignals.size() && !m_silent))
            {
                std::cout << "Invalid, contains spaces: '" << item << "'" << std::endl;
            }
        }
    }
    if (parts.size() == (endColumn - 1))
    {
        parts.push_back("");
    }
    if (parts.size() >= endColumn)
    {
        ParseColumns(parts, index);
    }
}

    
void CCSVFileReader::ParseColumns(const std::vector<std::string>& parts, const uint32_t index)
{
    static std::string vss = "";
    static sdv::core::ESignalDirection signalDirection = sdv::core::ESignalDirection::sigdir_rx;

    if (parts.size() >= vssVDColumns::column_end)
    {
        if (parts[vssVDColumns::column_vssDefinition].size() != 0)  // if vss is empty use previous one
        {
            vss = parts[vssVDColumns::column_vssDefinition];
        }
        if (!MustContainDotOrIsEmpty(vss, false)) // vss can be empty
        {
            if ((m_vdSignals.size() != 0) && (m_bsSignals.size() && !m_silent))
            {
                std::cout << "Invalid, contains no charatcer '.': '" << vss << "', line number " << std::to_string(index)<< std::endl;
            }
            return;
        }
        vss = ValidateVSSFormatNumbers(vss);

        if (!parts[vssVDColumns::column_direction].compare("RX"))
            signalDirection = sdv::core::ESignalDirection::sigdir_rx;
        if (!parts[vssVDColumns::column_direction].compare("TX"))
            signalDirection = sdv::core::ESignalDirection::sigdir_tx;

        if (!parts[vssVDColumns::column_type].compare("VD"))
        {
            if (AddToExistingVDSignal(m_vdSignals, vss, signalDirection, parts, index))
            {
                return;
            }

            SSignalVDDefinition signal;
            signal.vssDefinition = vss;
            signal.signalDirection = signalDirection;
            signal.className = GenerateDefaultIfEmpty(parts[vssVDColumns::column_className], "ClassName", index);

            if (AddFunctionVDDefinition(signal, parts[vssVDColumns::column_functionName], parts[vssVDColumns::column_signalName],
                parts[vssVDColumns::column_canSignalName], parts[vssVDColumns::column_signalCType], parts[vssVDColumns::column_vdFormula], index))
            {
                ValidateVDCodeStyle(signal, m_verbose);
                m_vdSignals.push_back(signal);
            }
        }
        else
        {
            if (AddToExistingBSSignal(m_bsSignals, vss, signalDirection, parts, index))
            {
                return;
            }

            std::string vssOfVehicleDevice = parts[vssVDColumns::column_vdDefinition];
            vssOfVehicleDevice = ValidateVSSFormatNumbers(vssOfVehicleDevice);

            SSignalBSDefinition signal;
            signal.vssDefinition = vss;
            signal.signalDirection = signalDirection;
            signal.className = GenerateDefaultIfEmpty(parts[vssVDColumns::column_className], "ClassName", index);
            signal.vssVDDefinition = vssOfVehicleDevice;
            if (AddFunctionBSDefinition(signal, parts[vssVDColumns::column_functionName], parts[vssVDColumns::column_signalName],
                parts[vssVDColumns::column_canSignalName], parts[vssVDColumns::column_signalCType], index))
            {
                ValidateBSCodeStyle(signal, m_verbose);
                m_bsSignals.push_back(signal);
            }
        }
    }
}

bool CCSVFileReader::AddToExistingVDSignal(std::vector <SSignalVDDefinition>& signals, const std::string& vssDefinition, 
    const sdv::core::ESignalDirection& directionX, const std::vector<std::string>& parts, const uint32_t index)
{
    for (auto& signal : signals)
    {
        if ((signal.vssDefinition.compare(vssDefinition) == 0) && (signal.signalDirection == directionX))
        {
            if ((signal.className != parts[vssVDColumns::column_className]) && !m_silent)
            {
                std::cout << "Signal already found, ignore class name '" << parts[vssVDColumns::column_className] << "', line number " << std::to_string(index) << std::endl;
            }


            AddFunctionVDDefinition(signal, parts[vssVDColumns::column_functionName], parts[vssVDColumns::column_signalName],
                parts[vssVDColumns::column_canSignalName], parts[vssVDColumns::column_signalCType], parts[vssVDColumns::column_vdFormula], index);
            return true;
        }
    }

    return false;
}

bool CCSVFileReader::AddToExistingBSSignal(std::vector <SSignalBSDefinition>& signals, const std::string& vssDefinition, 
    const sdv::core::ESignalDirection& directionX, const std::vector<std::string>& parts, const uint32_t index)
{
    for (auto& signal : signals)
    {
        if ((signal.vssDefinition.compare(vssDefinition) == 0) && (signal.signalDirection == directionX))
        {
            if ((signal.className != parts[vssVDColumns::column_className]) && !m_silent)
            {
                std::cout << "Signal already found, ignore class name '" << parts[vssVDColumns::column_className] << "', line number " << std::to_string(index) << std::endl;
            }

            AddFunctionBSDefinition(signal, parts[vssVDColumns::column_functionName], parts[vssVDColumns::column_signalName],
                parts[vssVDColumns::column_vdDefinition], parts[vssVDColumns::column_signalCType], index);
            return true;
        }
    }

    return false;
}

bool CCSVFileReader::AddFunctionVDDefinition(SSignalVDDefinition& signal, const std::string& functionName, const std::string& signalName, 
    const std::string& canSignalName, const std::string& idlType, const std::string& formula, const uint32_t index)
{
    if (!MustContainDotOrIsEmpty(canSignalName, true)) // can signal must must not be empty
    {
        if ((m_vdSignals.size() != 0) && (m_bsSignals.size() && !m_silent))
        {
            std::cout << "Invalid, contains no charatcer '.' or is empty: '" << canSignalName << "', line number " << std::to_string(index) << std::endl;
        }
        return false;
    }

    auto cType = GetCTypeFromIDLType(idlType);
    if ((cType.size() == 0) && !m_silent)
    {
        std::cout << "Invalid, idl type unknown: '" << idlType << "', line number " << std::to_string(index) << std::endl;
        return false;
    }

    SFunctionVDDefinition func;
    func.functionName = GenerateDefaultIfEmpty(functionName, "FunctionName", index);
    func.signalName = GenerateDefaultIfEmpty(signalName, "variableName", index);
    func.canSignalName = canSignalName;
    func.idlType = idlType;
    func.formula = formula;

    ValidateVDCodeStyle(func, m_verbose);
    signal.vecFunctions.push_back(func);

    return true;
}
bool CCSVFileReader::AddFunctionBSDefinition(SSignalBSDefinition& signal, const std::string& functionName, const std::string& signalName,
    const std::string& canSignalName, const std::string& idlType, const uint32_t index)
{
    if (!MustContainDotOrIsEmpty(canSignalName, true)) // can signal must must not be empty
    {
        if ((m_vdSignals.size() != 0) && (m_bsSignals.size() && !m_silent))
        {
            std::cout << "Invalid, contains no charatcer '.' or is empty: '" << canSignalName << "', line number " << std::to_string(index) << std::endl;
        }
        return false;
    }

    auto cType = GetCTypeFromIDLType(idlType);
    if ((cType.size() == 0) && !m_silent)
    {
        std::cout << "Invalid, idl type unknown: '" << idlType << "', line number " << std::to_string(index) << std::endl;
        return false;
    }

    SFunctionBSDefinition func;
    func.functionName = GenerateDefaultIfEmpty(functionName, "FunctionName", index);
    func.signalName = GenerateDefaultIfEmpty(signalName, "variableName", index);
    func.idlType = idlType;

    ValidateBSCodeStyle(func, m_verbose);
    signal.vecFunctions.push_back(func);

    return true;
}
