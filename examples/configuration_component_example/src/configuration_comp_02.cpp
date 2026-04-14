 /********************************************************************************
 * Copyright (c) 2025-2026 ZF Friedrichshafen AG
 *
 * This program and the accompanying materials are made available under the 
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0 
 ********************************************************************************/

#include <iostream>
#include <support/component_impl.h>
#include <support/toml.h>

class DemoConfigurationComponent02 : public sdv::CSdvObject
{
public:
	DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::device)
	DECLARE_OBJECT_CLASS_NAME("Configuration_Example_02")

    // Parameter map
    BEGIN_SDV_PARAM_MAP()
        SDV_PARAM_ENABLE_LOCKING() // Parameters will be protected against writing when locked (e.g. after initialization).
        SDV_PARAM_ENTRY(m_InitializedValue, "initializedValue", 7, "km/h", "Description for an initialized parameter.")
        SDV_PARAM_ENTRY(m_UpdatableValue,   "updatableValue",   7, "m/s", "Description for an updatable parameter.")
        SDV_PARAM_ENTRY(m_Message,   "Message",   "", "", "Example of type string.")
        SDV_PARAM_ENTRY(m_JSONConfig,   "JSONConfig",   "", "", "Example of type string.")
        SDV_PARAM_ENTRY(m_Id,   "Id",   0, "", "Example of type unsigned int.")
        SDV_PARAM_ENTRY(m_Pi,   "Pi",   0.0f, "", "Example of type float.")
        SDV_PARAM_ENTRY(m_IsValid,   "Boolean",   0, "", "Example of type bool.")
        SDV_PARAM_ENTRY(m_DirectTableA,   "Table.a",   0, "", "Example of type unsigned int within a table.")
        SDV_PARAM_ENTRY(m_DirectTableB,   "Table.b",   0.0, "", "Example of type float within a table.")
        SDV_PARAM_ENTRY(m_DirectTableC,   "Table.c",   "", "", "Example of type string within a table.")
    END_SDV_PARAM_MAP()

    /**
     * @brief Initialization event, called after object configuration was loaded. Overload of sdv::CSdvObject::OnInitialize.
     * @return Returns 'true' when the initialization was successful, 'false' when not.
     */
    virtual bool OnInitialize() override
    {
        PrintAllVariables();

        return true;
    };

    /**
     * @brief Shutdown the object. Overload of sdv::CSdvObject::OnShutdown.
     */
    virtual void OnShutdown() override
    {        
    }

    /**
    * @brief Print all global variables to console
    */
    void PrintAllVariables()  const
    {
        std::cout << "\n****************************************" << std::endl;            
        std::cout << "***** Component 2, without parsing *****" << std::endl;   
        std::cout << "****************************************" << std::endl;   
        std::cout << "\n----------\nValues from the parameter map:" << std::endl;        
        std::cout << "Expect  7,                 got " << "initialized value - not changed because not in configuration file: " << std::to_string(m_InitializedValue) << std::endl;          
        std::cout << "Expect 13,                 got " << "updated value - changed, found in configuration file: " << std::to_string(m_UpdatableValue) << std::endl;
        std::cout << "Expect 'It's me',          got " << "string: " << m_Message.c_str() << std::endl;
        std::cout << "Expect multiline string: " << m_JSONConfig.c_str() << std::endl;
        std::cout << "Expect 42,                 got " << "integer: " << std::to_string(m_Id) << std::endl;
        std::cout << "Expect  3.141593,          got " << "float: " << std::to_string(m_Pi) << std::endl;
        std::cout << "Expect  1,                 got " << "bool: " << std::to_string(m_IsValid) << std::endl;
        std::cout << "Expect 77,                 got " << "table column a: " << std::to_string(m_DirectTableA) << std::endl;
        std::cout << "Expect  1.200000,          got " << "table column b: " << std::to_string(m_DirectTableB) << std::endl;
        std::cout << "Expect 'this is a string', got " << "table column c: " << m_DirectTableC.c_str() << std::endl;
        std::cout << std::endl;
    }

  private:
    std::string           m_Message { "" };
    std::string           m_JSONConfig { "" };
    int32_t               m_Id { -1 };
    float                 m_Pi { 0.0 };
    bool                  m_IsValid { false };
    uint32_t              m_DirectTableA { 0 };
    float                 m_DirectTableB { 0.0 };
    std::string           m_DirectTableC { "" };
    uint32_t              m_InitializedValue { 0 };
    uint32_t              m_UpdatableValue { 0 }; 
};

DEFINE_SDV_OBJECT(DemoConfigurationComponent02)
