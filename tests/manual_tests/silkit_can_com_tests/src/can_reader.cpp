#include "../include/can_com_test_helper.h"

void ReceiveData()
{
    sdv::app::CAppControl appControl;
    appControl.Startup("");

    sdv::u8string config = R"(
    DebugInfo = true
    SyncMode = true
    SilKitParticipantName = "can_reader"
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
    canComObj.SetOperationMode(sdv::EOperationMode::configuring);

    CanReceiver mockRcv; 
    canComObj.RegisterReceiver(&mockRcv);
    canComObj.SetOperationMode(sdv::EOperationMode::running);
    
    while(true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        auto receivedMessages = mockRcv.GetReceivedMessages();
        if(receivedMessages.size() != 0)
        {
            for(auto rcvdMsg : receivedMessages)
            {          
                std::stringstream buffer;
                buffer << "\n[RX] CAN message being received [CANID=";
                buffer << rcvdMsg.uiID; buffer << ", Length=" ;
                buffer << unsigned(rcvdMsg.seqData.size()); buffer << "]" ;

                buffer << " Data=";
                for(uint16_t i = 0; i < rcvdMsg.seqData.size(); i++)
                {
                    buffer << "[" ; buffer << std::hex << unsigned(rcvdMsg.seqData[i]); buffer << "] ";
                }
                std::cout << buffer.str().c_str();
                mockRcv.ClearReceivedMessages();
            }
        }
        else
        {
            std::cout << "[RX] Received Message size is " << receivedMessages.size() << std::endl;
        }
    }

    canComObj.UnregisterReceiver(&mockRcv);
    std::cout << "CAN has read all messages\n";

    canComObj.Shutdown();
}

int main()
{
    ReceiveData();
    return 0;
}