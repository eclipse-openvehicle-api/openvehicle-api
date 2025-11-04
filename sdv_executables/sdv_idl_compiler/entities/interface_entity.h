#ifndef INTERFACE_ENTITY_H
#define INTERFACE_ENTITY_H

#include "definition_entity.h"

/**
 * @brief The interface definition of an IDL file.
 * @details The interface section of the IDL file contains multiple declarations of attributes and operations, as well as the
 * definitions of enums, structs and unions.
 */
class CInterfaceEntity : public CDefinitionEntity, public sdv::idl::IInterfaceEntity
{
public:
    /**
     * @brief Default constructor
     * @param[in] rptrContext Reference to the smart pointer holding the parse context. Must not be NULL.
     * @param[in] ptrParent Pointer to the parent class holding this entity. This must not be NULL.
     * @param[in] bIsLocal When set, the interface is defined as a local interface not intended to be marshalled.
     */
    CInterfaceEntity(const CContextPtr& rptrContext, CEntityPtr ptrParent, bool bIsLocal);

    /**
     * @brief Destructor
     */
    virtual ~CInterfaceEntity() override = default;

    /**
     * @brief Get access to another interface. Overload of sdv::IInterfaceAccess::GetInterface.
     * @param[in] idInterface The interface id to get access to.
     * @return Returns a pointer to the interface or NULL when the interface is not supported.
     */
    virtual sdv::interface_t GetInterface(sdv::interface_id idInterface) override;

    /**
     * @brief Is this interface local? Overload of sdv::idl::IInterfaceEntity::IsLocal.
     * @return Returns whether the interface is defined as local.
     */
    virtual bool IsLocal() const override { return m_bIsLocal; }

    /**
     * @brief Get the type of the entity. Overload of CEntity::GetType.
     * @return Returns the interface entity type.
     */
    virtual sdv::idl::EEntityType GetType() const override { return sdv::idl::EEntityType::type_interface; }

    /**
     * @brief Get the declaration type of the entity as string. Overload of CEntity::GetDeclTypeStr.
     * @param[in] bResolveTypedef When set, resolve the typedef type into the base type.
     * @return Returns a string with interface type.
     */
    virtual std::string GetDeclTypeStr(bool bResolveTypedef) const override;

    /**
     * @brief Process the code. Overload of CEntity::Process.
     */
    virtual void Process() override;

    /**
     * @brief Create the content value node. Overload of CDefinitionEntity::CreateValueNode.
     * @details Create the value node and assign the value node to the ValueRef() reference..
     */
    virtual void CreateValueNode() override;

    /**
     * @brief Request whether the definition supports the content. Overload of CDefintionEntity::Supports.
     * @param[in] eSupport The type of support that is requested.
     * @return Returns 'true' when the definition supports the content; 'false' otherwise.
     */
    virtual bool Supports(EDefinitionSupport eSupport) const override;

    /**
     * @brief Does the entity support inheritance? Overload of CDefinitionEntity::SupportsInheritance.
     * @details Returns whether the entity supports inheritance.
     * @return Returns whether inheritance is supported (which is the case).
     */
    virtual bool SupportsInheritance() const override { return true; }

    /**
     * @brief Does the complex entity support attributes in its content? Overload of CDefinitionEntity::SupportContentAttributes.
     * @details The default implementation doesn't support attributes (they are specific to interfaces).
     * @return Returns whether the entity supports attributes (which is the case).
     */
    virtual bool SupportContentAttributes() const override { return true; }

    /**
     * @brief Does the complex entity support operations in its content? Overload of CDefinitionEntity::SupportContentOperations.
     * @details The default implementation doesn't support operations (they are specific to interfaces).
     * @return Returns whether the entity supports operations (which is the case).
     */
    virtual bool SupportContentOperations() const override { return true; }

private:
    bool    m_bIsLocal = false;         ///< Flag indicating that the interface is local and not intended to be marshalled.
};

#endif // !defined(INTERFACE_ENTITY_H)