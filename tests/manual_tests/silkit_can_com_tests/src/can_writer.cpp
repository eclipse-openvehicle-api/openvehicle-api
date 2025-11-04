#include "../include/can_com_test_helper.h"

int main()
{
    sdv::app::CAppControl appControl;
    appControl.Startup("");

    sdv::u8string config = R"(
        DebugInfo = true
        SyncMode = true
        SilKitParticipantName = "can_writer"
        CanSilKitNetwork = "PrivateCAN"
        RegistryURI = "silkit://localhost:8500"
        SilKitConfig = """{
                            "Logging": {
                                "Sinks": [ { "Type": "Stdout", "Level": "Info" } ]
                    },
        }"""                      
    )"; 

    CTestCanSilKit canComObj;
    canComObj.Initialize(config.c_str());
    canComObj.SetOperationMode(sdv::EOperationMode::running);

    uint8_t msgId = 65;
    uint8_t dataSize = 1;
    uint16_t i = 0;
    std::cout << "Can writes 30 different datas:\n";
    CComTestHelper testHelper;    
    while(i < 30) //Sending 30 messages
    {
        auto testData = testHelper.CreateTestData(msgId, dataSize);
        canComObj.Send(testData, 0);
        
        if(testData.seqData.size() != 0)
        {
            std::stringstream buffer;
            buffer << "\n[TX] CAN message being sent [CANID=";
            buffer << testData.uiID; buffer << ", Length=" ;
            buffer << unsigned(testData.seqData.size()); buffer << "]" ;

            buffer << " Data=";
            for(uint16_t j = 0; j < testData.seqData.size(); j++)
            {
                buffer << "[" ; buffer << std::hex << unsigned(testData.seqData[j]); buffer << "] ";
            }
            std::cout << buffer.str().c_str();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        msgId++;
        i++;
        dataSize++;
        if(dataSize > 8)
            dataSize = 1;
    }

    canComObj.Shutdown();
    return 0;
}