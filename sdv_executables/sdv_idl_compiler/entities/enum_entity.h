#ifndef ENUM_ENTITY_H
#define ENUM_ENTITY_H

#include "definition_entity.h"
#include "declaration_entity.h"
#include "variable_entity.h"

/**
 * @brief The enum entry declaration.
 */
class CEnumEntry : public CDeclarationEntity
{
public:
    /**
     * @brief Default constructor
     * @param[in] rptrContext Reference to the smart pointer holding the parse context. Must not be NULL.
     * @param[in] ptrParent Pointer to the parent class holding this entity. This must not be NULL.
     */
    CEnumEntry(const CContextPtr& rptrContext, CEntityPtr ptrParent);

    /**
     * @brief Destructor
     */
    virtual ~CEnumEntry() override = default;

    /**
     * @brief Get the type of the entity. Overload of CEntity::GetType.
     * @return Returns enum entity type.
     */
    virtual sdv::idl::EEntityType GetType() const override { return sdv::idl::EEntityType::type_enum_entry; }

    /**
     * @brief Get the declaration type of the entity as string. Overload of CEntity::GetDeclTypeStr.
     * @param[in] bResolveTypedef When set, resolve the typedef type into the base type.
     * @return Returns a string with enum type.
     */
    virtual std::string GetDeclTypeStr(bool bResolveTypedef) const override;

    /**
     * @brief Process the code. Overload of CEntity::Process.
     */
    virtual void Process() override;

    /**
     * @brief Does the entity support assignments? Overload of CDeclarationEntity::SupportAssignments.
     * @return Returns 'true' when the entity supports assignments; 'false' otherwise.
     */
    virtual bool SupportAssignments() const override { return true; }

    // Suppress cppcheck warning of a useless override. The function is here for better understanding.
    // cppcheck-suppress uselessOverride
    /**
     * @brief Is the entity readonly? Overload of IEntityInfo::IsReadOnly.
     * @return Returns 'true' when the entity defined as readonly; 'false' otherwise.
     */
    virtual bool IsReadOnly() const override { return true; }

    /**
     * @brief Does the entity support multiple declarations on one line of code?  Overload of
     * CDeclarationEntity::SupportMultipleDeclarations.
     * @return Returns 'true' when the entity supports multiple declarations; 'false' otherwise.
     */
    virtual bool SupportMultipleDeclarations() const override { return true; }

    /**
     * @brief Do not enforce next declaration after comma (enums do)? Overload of
     * CDeclarationEntity::DoNotEnfoceNextDeclarationAfterComma.
     * @return Returns 'true' when not enforcing the next declaration; 'false' otherwise.
     */
    virtual bool DoNotEnfoceNextDeclarationAfterComma() const override { return true; }
};

/**
 * @brief The enum definition of an IDL file.
 * @details The enum section of the IDL file contains a list of enum value entries
 */
class CEnumEntity : public CDefinitionEntity, public sdv::idl::IEnumEntity
{
public:
    /**
     * @brief Default constructor
     * @param[in] rptrContext Reference to the smart pointer holding the parse context. Must not be NULL.
     * @param[in] ptrParent Pointer to the parent class holding this entity. This must not be NULL.
     */
    CEnumEntity(const CContextPtr& rptrContext, CEntityPtr ptrParent);

    /**
     * @brief Destructor
     */
    virtual ~CEnumEntity() override = default;

    /**
     * @brief Get access to another interface. Overload of sdv::IInterfaceAccess::GetInterface.
     * @param[in] idInterface The interface id to get access to.
     * @return Returns a pointer to the interface or NULL when the interface is not supported.
     */
    virtual sdv::interface_t GetInterface(sdv::interface_id idInterface) override;

    /**
     * @brief Get the type of the entity. Overload of CEntity::GetType.
     * @return Returns enum entity type.
     */
    virtual sdv::idl::EEntityType GetType() const override { return  sdv::idl::EEntityType::type_enum; }

    /**
     * @brief Get the declaration type of the entity as string. Overload of CEntity::GetDeclTypeStr.
     * @param[in] bResolveTypedef When set, resolve the typedef type into the base type.
     * @return Returns a string with enum type.
     */
    virtual std::string GetDeclTypeStr(bool bResolveTypedef) const override;

    /**
     * @brief Get the enumerator base type. Overload of sdv::idl::IEnumEntity.
     * @param[out] reType Reference to the declaration type. The type if EEntityType::type_unknown if not available.
     * @param[out] rpType Reference to the interface pointer if the type is a complex type. Otherwise is NULL.
     */
    virtual void GetBaseType(sdv::idl::EDeclType& reType, sdv::IInterfaceAccess*& rpType) const override;

    /**
     * @brief Process the code. Overload of CEntity::Process.
     */
    virtual void Process() override;

    /**
     * @brief Process the content of the definition.
     */
    virtual void ProcessContent() override;

    /**
     * @brief Process the definition addendum (following the definition statement before the content definition).
     */
    virtual void ProcessDefinitionAddendum() override;

    /**
     * @brief Get the enum type.
     * @return Returns the underlying enum type.
     */
    sdv::idl::EDeclType GetEnumType() const { return m_typedecl.GetBaseType(); }

    /**
     * @brief Request whether the definition supports the content. Overload of CDefintionEntity::Supports.
     * @param[in] eSupport The type of support that is requested.
     * @return Returns 'true' when the definition supports the content; 'false' otherwise.
     */
    virtual bool Supports(EDefinitionSupport eSupport) const override;

    /**
     * @brief Does the entity support children? Overload of CEntity::SupportsChildren.
     * @details The struct supports children.
     * @return Returns whether the entity supports children (which is supported).
     */
    virtual bool SupportsChildren() const override { return true; }

    /**
     * @brief Does the entity support anonymous naming?
     * @details The default implementation is that anonymous naming is not supported.
     * @return Returns whether the entity supports inheritance (which is supported).
     */
    virtual bool SupportsAnonymous() const override { return true; }

    /**
     * @brief Does the entity support inheritance?
     * @details The default implementation is that inheritance is not supported.
     * @return Returns whether the entity supports inheritance (which is supported).
     */
    virtual bool SupportsInheritance() const override { return true; }

    /**
     * @brief Create the content value node. Overload of CDefinitionEntity::CreateValueNode.
     * @details Create the value node and assign the value node to the ValueRef() reference..
     */
    virtual void CreateValueNode() override;

private:
    CTypeDeclaration    m_typedecl;        ///< The base type of the enum.
};

#endif // !defined(ENUM_ENTITY_H)
