/**
* @file process_control.h
* @author Sudipta Babu Durjoy FRD DISS21 (mailto:sudipta.durjoy@zf.com)
* @brief
* @version 1.0
* @date 2023-10-23
*
* @copyright Copyright ZF Friedrichshafen AG (c) 2023
*
*/

#ifndef MANIFEST_UTIL_H
#define MANIFEST_UTIL_H

#include <interfaces/config.h>
#include <support/component_impl.h>

/**
* @brief Manifest helper utility class.
* @details The manifest helper utility component allows extracting module information from the module. To do this, it needs to
* load the module and access one of the functions. Since several modules are designed to run in an isolated context, loading
* the module and extracting the manifest should also occur in an isolated context (hence as a separate utility and not part of
* the core).
*/
class CManifestUtil : public sdv::CSdvObject, public sdv::helper::IModuleManifestHelper
{
public:

    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(sdv::helper::IModuleManifestHelper)
    END_SDV_INTERFACE_MAP()

    // Object declarations
    DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::Utility)
    DECLARE_OBJECT_CLASS_NAME("ManifestHelperUtility")

    /**
     * @brief Read the module manifest. Overload of sdv::helper::IModuleManifestHelper::ReadModuleManifest.
     * @param[in] ssModule Path to the module file.
     * @return The module manifest if available. Otherwise an empty string.
     */
    virtual sdv::u8string ReadModuleManifest(/*in*/ const sdv::u8string& ssModule) override;
};
DEFINE_SDV_OBJECT(CManifestUtil)

#endif // !define MANIFEST_UTIL_H
