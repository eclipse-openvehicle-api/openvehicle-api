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
    DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::Utility)
    DECLARE_OBJECT_CLASS_NAME("TOMLParserUtility")

private:
    toml_parser::CParser m_parser;       ///< Configuration parser
};

DEFINE_SDV_OBJECT_NO_EXPORT(CTOMLParserUtility)

#endif // !defined CONFIG_UTILITY_H