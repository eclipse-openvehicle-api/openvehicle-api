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

#ifndef CONFIG_UTILITY_H
#define CONFIG_UTILITY_H

#include <support/component_impl.h>
#include "toml_parser/parser_toml.h"

/**
 * @brief Configuration utility component.
 */
class CTOMLParserUtility : public sdv::CSdvObject
{
public:
    /**
     * @brief Default constructor
     */
    CTOMLParserUtility() = default;

    // Interface map
    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_CHAIN_MEMBER(m_parser)
    END_SDV_INTERFACE_MAP()

    // Object declarations
    DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::utility)
    DECLARE_OBJECT_CLASS_NAME("TOMLParserUtility")

private:
    toml_parser::CParser m_parser;       ///< Configuration parser
};

DEFINE_SDV_OBJECT(CTOMLParserUtility)

#endif // !defined CONFIG_UTILITY_H