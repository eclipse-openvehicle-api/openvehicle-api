#ifndef META_ENTITY_H
#define META_ENTITY_H

#include "entity_base.h"

/**
 * @brief The meta data inserted into the code.
 */
class CMetaEntity : public CEntity, public sdv::idl::IMetaEntity
{
public:
    /**
     * @brief Constructor
     * @param[in] rptrContext Reference to the smart pointer holding the parse context. Must not be NULL.
     * @param[in] ptrParent Pointer to the parent class holding this entity. This must not be NULL.
     * @param[in] rtokenMeta The meta data content
     * @param[in] lstComments Any preceding comments
     */
    CMetaEntity(const CContextPtr& rptrContext, CEntityPtr ptrParent, const CToken& rtokenMeta,
        const CTokenList lstComments);

    /**
     * @brief Destructor
     */
    virtual ~CMetaEntity() override = default;

    /**
     * @brief Get the type of the entity. Overload of CEntity::GetType.
     * @return Returns the meta entity type.
     */
    virtual sdv::idl::EEntityType GetType() const override { return sdv::idl::EEntityType::type_meta; }

    /**
     * @brief Get the name of the entity. Overload of CEntity::GetName.
     * @return The entity name.
     */
    virtual sdv::u8string GetName() const override { return "meta"; }

    /**
     * @brief Get access to another interface. Overload of sdv::IInterfaceAccess::GetInterface.
     * @param[in] idInterface The interface id to get access to.
     * @return Returns a pointer to the interface or NULL when the interface is not supported.
     */
    virtual sdv::interface_t GetInterface(sdv::interface_id idInterface) override;

    /**
     * @brief Get the meta data type.
     * @return Returns meta entity type.
     */
    virtual sdv::idl::IMetaEntity::EType GetMetaType() const override;

    /**
     * @brief Get the meta data content.
     * @return Returns a string object.
     */
    virtual sdv::u8string GetContent() const override;

    /**
     * @brief Process the code. Since there is none... nothing to do. Overload of CEntity::Process.
     */
    virtual void Process() override {};

private:
    sdv::idl::IMetaEntity::EType    m_eType;        ///< Type of meta data
    std::string                     m_ssContent;    ///< The meta data string
};

#endif ///defined(META_ENTITY_H)