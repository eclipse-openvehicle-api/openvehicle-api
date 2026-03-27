/********************************************************************************
 * Copyright (c) 2025-2026 ZF Friedrichshafen AG
 *
 * This program and the accompanying materials are made available under the 
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Contributors: 
 *   Erik Verhoeven - initial API and implementation 
 ********************************************************************************/

#ifndef CMAKE_GENERATOR_H
#define CMAKE_GENERATOR_H

#include "../../../global/ipc_named_mutex.h"

/**
 * @brief CMake generator class.
 */
class CIdlCompilerCMakeGenerator : public CGenContext
{
public:
    /**
    * @brief Constructor
    * @param[in] pParser Pointer to the parser object.
    */
    CIdlCompilerCMakeGenerator(sdv::IInterfaceAccess* pParser);

    /**
    * @brief Destructor
    */
    virtual ~CIdlCompilerCMakeGenerator() override;

    /**
    * @brief Generate the definition.
    * @param[in] ssTargetLibName Library target name to add in the cmake file.
    * @return Returns whether the generation was successful.
    */
    bool Generate(const std::string& ssTargetLibName);

private:
    ipc::named_mutex        m_mtx;      ///< Guarantee exclusive access while writing the CMake file.
};

#endif // !defined CMAKE_GENERATOR_H