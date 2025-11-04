#ifndef TEST_CAN_SILKIT_HELPER_H
#define TEST_CAN_SILKIT_HELPER_H

#include <iostream>
#include <mutex>
#include <deque>
#include <thread>
#include <gtest/gtest.h>
#include <interfaces/can.h>
#include <support/app_control.h>
#include <support/toml.h>
#include <support/component_impl.h>
#include "../sdv_services/core/toml_parser/parser_toml.h"
#include "../../../../sdv_services/can_communication_silkit/can_com_silkit.h"
#include "../../../../export/support/app_control.h"

class CTestCanSilKit : public CCANSilKit
{
    public:
        virtual void Initialize(const sdv::u8string& ssObjectConfig) override
        {
            return CCANSilKit::Initialize(ssObjectConfig);
        }

        virtual sdv::EObjectStatus GetStatus() const override
        {
            return CCANSilKit::GetStatus();
        }

        virtual void Shutdown() override
        {
            return CCANSilKit::Shutdown();
        }

        virtual void RegisterReceiver(/*in*/ sdv::can::IReceive* pReceiver) override
        {
            return CCANSilKit::RegisterReceiver(pReceiver);
        }

        virtual void UnregisterReceiver(/*in*/ sdv::can::IReceive* pReceiver) override
        {
            return CCANSilKit::UnregisterReceiver(pReceiver);
        }

        virtual void Send(/*in*/ const sdv::can::SMessage& sMsg, /*in*/ uint32_t uiIfcIndex) override
        {
            return CCANSilKit::Send(sMsg, uiIfcIndex);
        }

        virtual sdv::sequence<sdv::u8string> GetInterfaces() const override
        {
            return CCANSilKit::GetInterfaces();
        }

};

sdv::can::SMessage CreateCanMsgData(uint8_t init, uint8_t dataSize)
{
    uint8_t value = init;
    sdv::can::SMessage testMsg;
    testMsg.uiID = value;  
    testMsg.bCanFd = false;
    testMsg.bExtended = false;
    testMsg.seqData.resize(dataSize);
    for (uint8_t i = 0; i < dataSize; i++)
    {
        testMsg.seqData[i] = value;
        value += 2;
    }
    return testMsg;
}


class CanReceiver : public sdv::can::IReceive
{
public:
   
    void Receive(const sdv::can::SMessage& sMessage, uint32_t) override
    {
       vecReceivedMessages.push_back(sMessage);
    }
    void Error(const sdv::can::SErrorFrame&, uint32_t) override {}

    std::vector<sdv::can::SMessage> GetReceivedMessages() 
    {
        return vecReceivedMessages;
    }

    void ClearReceivedMessages()
    {
        vecReceivedMessages.clear();
    }

    private:
        std::vector<sdv::can::SMessage> vecReceivedMessages;
};

class CComTestHelper
{
public:
    struct PcloseDeleter 
    {
        void operator()(FILE* file) const 
        {
#ifdef _WIN32
            _pclose(file);
#else
            pclose(file);
#endif
        }
    };
    
    std::string exec(const char* cmd) 
    {
        std::array<char, 128> buffer;
        std::string result;
#ifdef _WIN32
        std::unique_ptr<FILE, PcloseDeleter> pipe(_popen(cmd, "r"));
#else
        std::unique_ptr<FILE, PcloseDeleter> pipe(popen(cmd, "r"));
#endif
        if (!pipe) 
        {
            return "";
        }
        while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe.get()) != nullptr) 
        {
            result += buffer.data();
        }
        return result;
    }    

    sdv::can::SMessage CreateTestData(uint8_t init, uint8_t dataSize)
    {
        uint8_t value = 65;
        // uint8_t value = init;
        sdv::can::SMessage testMsg;
        testMsg.uiID = init;  
        testMsg.bCanFd = false;
        testMsg.bExtended = false;
        testMsg.seqData.resize(dataSize);
        for (uint8_t i = 0; i < dataSize; i++)
        {
            testMsg.seqData[i] = value;
            value += 2;
        }
        return testMsg;
    }

    bool ValidateInterfaces(const sdv::sequence<sdv::u8string>& seqExpectedInterfaces, const sdv::sequence<sdv::u8string>& seqInterfaces)
    {
        bool success = true;
        if (seqExpectedInterfaces.size() != seqInterfaces.size())
        {
            std::cout << "Interfaces failure, expected: " << std::to_string(seqExpectedInterfaces.size()) << " got: " << std::to_string(seqInterfaces.size()) << std::endl;
            success = false;
        }
        else
        {            
            for (size_t nIndex = 0; nIndex < seqInterfaces.size(); nIndex++)
            {
                if (seqExpectedInterfaces[nIndex].compare(seqInterfaces[nIndex]) != 0)
                {
                    success = false,
                    std::cout << "Interface mismatch, expected: " << seqExpectedInterfaces[nIndex].c_str() << " got: " <<  seqInterfaces[nIndex].c_str() << std::endl;
                }
            }
        }

        return success;
    }

    bool ValidateReceivedMessages(const std::deque<std::pair<std::string, sdv::can::SMessage>>& received,
        const sdv::can::SMessage& testMessage, const std::string& interfaceName, size_t expectedSize)
    {
        uint32_t error = 0;
        if (expectedSize == 0)
            error++;

        if (received.size() != expectedSize)
            error++;  

        for (auto message : received)
        {
            if (interfaceName.compare(message.first) != 0)
                error++;

            if (message.second.uiID != testMessage.uiID)
                error++;
   
            if (message.second.seqData.size() != testMessage.seqData.size())
                error++;               
        }

        if (error)
        {
            std::cout << "Got " << std::to_string(error) << " errors from validation of " 
                      << interfaceName << ", expect " << std::to_string(expectedSize) 
                      << " messages, got " << std::to_string(received.size()) << std::endl;
            return false;
        }        
        return true;     
    } 
    
    std::string NormalizeWhitespace(const std::string& str)
    {
        std::string trimmed = str;
        trimmed.erase(trimmed.find_last_not_of(" \t\n\r") + 1); // Trim trailing whitespace
        trimmed.erase(0, trimmed.find_first_not_of(" \t\n\r")); // Trim leading whitespace
        std::string result;
        std::unique_copy(trimmed.begin(), trimmed.end(), std::back_inserter(result),
                            [](char a, char b) { return std::isspace(a) && std::isspace(b); });
        return result;
    }
};

#endif // ! defined TEST_CAN_SILKIT_HELPER_H
