#include <iostream>
#include <support/component_impl.h>
#include <support/toml.h>

class DemoConfigurationComponent : public sdv::CSdvObject, public sdv::IObjectControl
{
  public:
    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(sdv::IObjectControl)
    END_SDV_INTERFACE_MAP()

	DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::Device)
	DECLARE_OBJECT_CLASS_NAME("Configuration_Example")

    /**
    * @brief initialize function to register, access the task timer interface from platform abstraction.
    * After initialization 'CreateTimer' function is called to execute the task periodically.
    * @param[in] rssObjectConfig An object configuration is currently not used by this demo component.
    */
    virtual void Initialize([[maybe_unused]] const sdv::u8string& rssObjectConfig) override
    {
        try
        {
            sdv::toml::CTOMLParser config(rssObjectConfig.c_str());

            sdv::toml::CNode messageNode = config.GetDirect("Message");
            if (messageNode.GetType() == sdv::toml::ENodeType::node_string)
            {
                m_Message = static_cast<std::string>(messageNode.GetValue());
            }
            
            sdv::toml::CNode jsonNode = config.GetDirect("JSONConfig");
            if (jsonNode.GetType() == sdv::toml::ENodeType::node_string)
            {
                m_JSONConfig = static_cast<std::string>(jsonNode.GetValue());
            }
            
            sdv::toml::CNode idNode = config.GetDirect("Id");
            if (idNode.GetType() == sdv::toml::ENodeType::node_integer)
            {
                m_Id = static_cast<int32_t>(idNode.GetValue());
            }

            sdv::toml::CNode floatNode = config.GetDirect("Pi");
            if (floatNode.GetType() == sdv::toml::ENodeType::node_floating_point)
            {
                m_Pi = static_cast<float>(floatNode.GetValue());
            }

            sdv::toml::CNode boolNode = config.GetDirect("Boolean");
            if (boolNode.GetType() == sdv::toml::ENodeType::node_boolean)
            {
                m_IsValid = static_cast<bool>(boolNode.GetValue());
            }

            sdv::toml::CNodeCollection arrayNodes = config.GetDirect("Array");
            if (arrayNodes.GetType() == sdv::toml::ENodeType::node_array)
            {
                for (size_t nIndex = 0; nIndex < arrayNodes.GetCount(); nIndex++)
                {
                    sdv::toml::CNode node = arrayNodes[nIndex];
                    if (node.GetType() == sdv::toml::ENodeType::node_integer)
                    {
                        m_Counters.push_back(static_cast<uint32_t>(node.GetValue()));
                    }
                }
            }

            sdv::toml::CNodeCollection tableNodes = config.GetDirect("Table");
            if (tableNodes.GetType() == sdv::toml::ENodeType::node_table)
            {
                if (tableNodes.GetCount() >= 3)
                {
                    m_TableA = static_cast<uint32_t>(tableNodes[0].GetValue());
                    m_TableB = static_cast<float>(tableNodes[1].GetValue());
                    m_TableC = static_cast<std::string>(tableNodes[2].GetValue());
                }
            }

            auto table_a = config.GetDirect("Table.a");
            auto table_b = config.GetDirect("Table.b");
            auto table_c = config.GetDirect("Table.c");
            m_DirectTableA = static_cast<uint32_t>(table_a.GetValue());
            m_DirectTableB = static_cast<float>(table_b.GetValue());
            m_DirectTableC = static_cast<std::string>(table_c.GetValue());

        }
        catch (const sdv::toml::XTOMLParseException& e)
        {
            SDV_LOG_ERROR("Parsing error: ", e.what());

            m_status = sdv::EObjectStatus::initialization_failure;
            return;
        }

        PrintAllVariables();

        m_status = sdv::EObjectStatus::initialized;
    };

    /**
    * @brief Gets the current status of the object
    * @return EObjectStatus The current status of the object
    */
    virtual sdv::EObjectStatus GetStatus() const override
    {
        return m_status;
    };

    /**
     * @brief Set the component operation mode. Overload of sdv::IObjectControl::SetOperationMode.
     * @param[in] eMode The operation mode, the component should run in.
     */
    void SetOperationMode(/*in*/ sdv::EOperationMode eMode)
    {
        switch (eMode)
        {
        case sdv::EOperationMode::configuring:
            if (m_status == sdv::EObjectStatus::running || m_status == sdv::EObjectStatus::initialized)
                m_status = sdv::EObjectStatus::configuring;
            break;
        case sdv::EOperationMode::running:
            if (m_status == sdv::EObjectStatus::configuring || m_status == sdv::EObjectStatus::initialized)
                m_status = sdv::EObjectStatus::running;
            break;
        default:
            break;
        }
    }

    /**
    * @brief Shutdown function is to shutdown the execution of periodic task.
    * Timer ID of the task is used to shutdown the specific task.
    */
    virtual void Shutdown() override
    {
        m_status = sdv::EObjectStatus::destruction_pending;
    }

    /**
    * @brief Print all global variables to console
    */
    void PrintAllVariables()  const
    {
        std::cout << "\nValues parsed during initialization:" << std::endl;
        std::cout << "string: " << m_Message.c_str() << std::endl;
        std::cout << "multiline string: " << m_JSONConfig.c_str() << std::endl;
        std::cout << "integer: " << std::to_string(m_Id) << std::endl;
        std::cout << "float: " << std::to_string(m_Pi) << std::endl;
        std::cout << "bool: " << std::to_string(m_IsValid) << std::endl;
        std::cout << "table column a: " << std::to_string(m_TableA) << "     " << std::to_string(m_DirectTableA) << std::endl;
        std::cout << "table column b: " << std::to_string(m_TableB) << "     " << std::to_string(m_DirectTableB) << std::endl;
        std::cout << "table column c: " << m_TableC.c_str() << "     " << m_DirectTableC.c_str() << std::endl;
        std::cout << "array: ";
        for (auto counter : m_Counters)
        {
            std::cout << std::to_string(counter) << ", ";
        }
        std::cout << std::endl;
    }

  private:
    std::atomic<sdv::EObjectStatus> m_status = {sdv::EObjectStatus::initialization_pending}; //!< To update the object status when it changes.

    std::string           m_Message { "" };
    std::string           m_JSONConfig { "" };
    int32_t               m_Id { -1 };
    float                 m_Pi { 0.0 };
    bool                  m_IsValid { false };
    std::vector<uint32_t> m_Counters{};
    uint32_t              m_TableA { 0 };
    float                 m_TableB { 0.0 };
    std::string           m_TableC { "" };
    uint32_t              m_DirectTableA { 0 };
    float                 m_DirectTableB { 0.0 };
    std::string           m_DirectTableC { "" };
};

DEFINE_SDV_OBJECT(DemoConfigurationComponent)
