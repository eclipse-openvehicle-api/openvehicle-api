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

#ifndef PS_CPP_GENERATOR_H
#define PS_CPP_GENERATOR_H

#include "../../../global/ipc_named_mutex.h"

/**
 * @brief Prox/stub CPP file generator class.
 */
class CPsCppGenerator : public CGenContext
{
public:
    /**
    * @brief Constructor
    * @param[in] pParser Pointer to the parser object.
    */
    CPsCppGenerator(sdv::IInterfaceAccess* pParser);

    /**
    * @brief Destructor
    */
    virtual ~CPsCppGenerator() override;

    /**
    * @brief Generate the definition.
    * @return Returns whether the generation was successful.
    */
    bool Generate();

private:
    ipc::named_mutex        m_mtx;      ///< Guarantee exclusive access while writing the PS file.
};

#endif // !defined PS_CPP_GENERATOR_H