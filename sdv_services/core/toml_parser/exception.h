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
 *   Martin Stimpfl - initial API and implementation
 *   Erik Verhoeven - writing TOML and whitespace preservation
 ********************************************************************************/

#ifndef CONFIG_EXCEPTION_H
#define CONFIG_EXCEPTION_H

#include <interfaces/toml.h>

/// The TOML parser namespace
namespace toml_parser
{
    /**
     * @brief Extended exception for the TOML parser.
     */
    except XTOMLParseException : public sdv::toml::XTOMLParseException
    {
        /**
         * @brief Constructor
         */
        XTOMLParseException(const std::string& rss)
        {
            ssMessage = rss;
        };

        /**
         * @brief Return the explanatory string.
         * @return The descriptive string.
         */
        virtual const char* what() const noexcept override
        {
            return ssMessage.c_str();
        }
    };
} // namespace toml_parser

#endif // !defined CONFIG_EXCEPTION_H