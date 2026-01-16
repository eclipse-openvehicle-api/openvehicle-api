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