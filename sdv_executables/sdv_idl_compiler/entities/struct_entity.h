#ifndef STRUCT_ENTITY_H
#define STRUCT_ENTITY_H

#include "definition_entity.h"

/**
 * @brief The struct definition of an IDL file.
 * @details The struct section of the IDL file contains multiple declarations of members, as well as the definitions of structs
 * and unions.
 */
class CStructEntity : public CDefinitionEntity
{
public:
    /**
     * @brief Default constructor
     * @param[in] rptrContext Reference to the smart pointer holding the parse context. Must not be NULL.
     * @param[in] ptrParent Pointer to the parent class holding this entity. This must not be NULL.
     */
    CStructEntity(const CContextPtr& rptrContext, CEntityPtr ptrParent);

    /**
     * @brief Destructor
     */
    virtual ~CStructEntity() override = default;

    /**
     * @brief Get the type of the entity. Overload of CEntity::GetType.
     * @return Returns the struct type.
     */
    virtual sdv::idl::EEntityType GetType() const override { return sdv::idl::EEntityType::type_struct; }

    /**
     * @brief Get the declaration type of the entity as string. Overload of CEntity::GetDeclTypeStr.
     * @param[in] bResolveTypedef When set, resolve the typedef type into the base type.
     * @return Returns a string with struct type.
     */
    virtual std::string GetDeclTypeStr(bool bResolveTypedef) const override;

    // Suppress cppcheck warning of a useless override. The function is here for better understanding.
    // cppcheck-suppress uselessOverride
    /**
     * @brief Process the code. Overload of CEntity::Process.
     */
    virtual void Process() override;

    /**
     * @brief Request whether the definition supports the content. Overload of CDefintionEntity::Supports.
     * @param[in] eSupport The type of support that is requested.
     * @return Returns 'true' when the definition supports the content; 'false' otherwise.
     */
    virtual bool Supports(EDefinitionSupport eSupport) const override;

    /**
     * @brief Does the entity support inheritance? Overload of CDefinitionEntity::SupportsInheritance.
     * @return Returns whether the entity supports inheritance.
     */
    virtual bool SupportsInheritance() const override { return true; }

    // Suppress cppcheck warning of a useless override. The function is here for better understanding.
    // cppcheck-suppress uselessOverride
    /**
     * @brief Does the entity declaration support anonymous naming? Overload of CDefinitionEntity::SupportsAnonymous.
     * @remarks C11 supports anonymous structs. C++ not! Therefore, IDL does not support anonymous structs.
     * @return Returns whether the entity supports inheritance.
     */
    virtual bool SupportsAnonymous() const override { return false; }

    /**
     * @brief Create the content value node. Overload of CDefinitionEntity::CreateValueNode.
     * @details Create the value node and assign the value node to the ValueRef() reference..
     */
    virtual void CreateValueNode() override;
};

#endif // !defined(STRUCT_ENTITY_H)