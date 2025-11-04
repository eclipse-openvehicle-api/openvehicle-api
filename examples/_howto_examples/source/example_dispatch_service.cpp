#include <iostream>
#include <support/toml.h>
#include <support/component_impl.h>
#include <support/signal_support.h>

class CExampleDispatchService
{
public:
	CExampleDispatchService()
	{
		sdv::core::CDispatchService dispatch;
		m_dataLinkSignalRx = dispatch.RegisterRxSignal("Signal_DataLink_to_AbstractDevice");  ///< signals created by data link
		m_dataLinkSignalTx01 = dispatch.RegisterTxSignal("Signal_AbstractDevice_to_DataLink_01", 123);
		m_dataLinkSignalTx02 = dispatch.RegisterTxSignal("Signal_AbstractDevice_to_DataLink_02", 123);
		m_dataLinkSignalTx03 = dispatch.RegisterTxSignal("Signal_AbstractDevice_to_DataLink_03", 123);
	}

	~CExampleDispatchService()
	{
		if (m_dataLinkSignalRx)
			m_dataLinkSignalRx.Reset();
		if (m_dataLinkSignalTx01)
			m_dataLinkSignalTx01.Reset();
		if (m_dataLinkSignalTx02)
			m_dataLinkSignalTx02.Reset();
		if (m_dataLinkSignalTx03)
			m_dataLinkSignalTx03.Reset();
	}

	void SubscripSignals()
	{
		sdv::core::CDispatchService dispatch;
		m_abstractDeviceSubscriper = dispatch.Subscribe("Signal_DataLink_to_AbstractDevice", [&](sdv::any_t value) {  CallBackFunction(value); });
		if (!m_abstractDeviceSubscriper)
		{
			std::cout << "Could not subscrupe to 'Signal_DataLink_to_AbstractDevice'" << std::endl;
		}
		else
		{
			std::cout << "Subscript to 'Signal_DataLink_to_AbstractDevice'" << std::endl;
		}
	}

	void AddPublisherSignals()
	{
		sdv::core::CDispatchService dispatch;
		m_abstractDevicePublisher01 = dispatch.AddPublisher("Signal_AbstractDevice_to_DataLink_01");
		m_abstractDevicePublisher02 = dispatch.AddPublisher("Signal_AbstractDevice_to_DataLink_02");
		m_abstractDevicePublisher03 = dispatch.AddPublisher("Signal_AbstractDevice_to_DataLink_03");
		if (!m_abstractDevicePublisher01 || !m_abstractDevicePublisher02 || !m_abstractDevicePublisher03 )
		{
			std::cout << "Could not add publisher to 'Signal_AbstractDevice_to_DataLink'" << std::endl;
		}
		else
		{
			std::cout << "Publisher added to 'Signal_AbstractDevice_to_DataLink'" << std::endl;
		}
	}

	void DataLinkWriter(const uint32_t value)
	{
		m_dataLinkSignalRx.Write(value);
	}

	void WriteToSignalAsPublisher(const uint32_t value01, const uint32_t value02, const uint32_t value03)
	{
		sdv::core::CDispatchService dispatch;
		auto transaction = dispatch.CreateTransaction();
		m_abstractDevicePublisher01.Write(value01, transaction);
		m_abstractDevicePublisher02.Write(value02, transaction);
		m_abstractDevicePublisher03.Write(value03, transaction);
		transaction.Finish();
	}

	void GetTxSignalValue(const std::string& msg, uint32_t& value01, uint32_t& value02, uint32_t& value03)
	{
		sdv::core::CDispatchService dispatch;
		auto transaction = dispatch.CreateTransaction();
		value01 = m_dataLinkSignalTx01.Read(transaction).get<uint32_t>();
		value02 = m_dataLinkSignalTx02.Read(transaction).get<uint32_t>();
		value03 = m_dataLinkSignalTx03.Read(transaction).get<uint32_t>();
		transaction.Finish();
		std::cout << "Read Tx signals (" << msg.c_str() <<"): " << std::to_string(value01) << ", " << std::to_string(value02) << ", " << std::to_string(value03) << std::endl;
	}

private:

	/**
	* @brief CallBackFunction for the subscription
	*/
	void CallBackFunction(sdv::any_t value)
	{
		uint32_t sayHello = value.get<uint32_t>();
		std::cout << "This CallBackFunction is called on signal change: " << std::to_string(sayHello) << std::endl;
	}

	sdv::core::CSignal m_dataLinkSignalRx;          ///< signals created and used by data link
	sdv::core::CSignal m_dataLinkSignalTx01;        ///< signals created and used by data link
	sdv::core::CSignal m_dataLinkSignalTx02;        ///< signals created and used by data link
	sdv::core::CSignal m_dataLinkSignalTx03;        ///< signals created and used by data link

	sdv::core::CSignal m_abstractDeviceSubscriper;  ///< signals used by an abstract device
	sdv::core::CSignal m_abstractDevicePublisher01; ///< signals used by an abstract device
	sdv::core::CSignal m_abstractDevicePublisher02; ///< signals used by an abstract device
	sdv::core::CSignal m_abstractDevicePublisher03; ///< signals used by an abstract device
};

