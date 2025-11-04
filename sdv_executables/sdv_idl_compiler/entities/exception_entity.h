#ifndef EXCEPTION_ENTITY_H
#define EXCEPTION_ENTITY_H

#include "definition_entity.h"
#include "struct_entity.h"

/**
 * @brief The struct definition of an IDL file.
 * @details The struct section of the IDL file contains multiple declarations of members, as well as the definitions of structs
 * and unions.
 */
class CExceptionEntity : public CStructEntity
{
public:
    /**
     * @brief Default constructor
     * @param[in] rptrContext Reference to the smart pointer holding the parse context. Must not be NULL.
     * @param[in] ptrParent Pointer to the parent class holding this entity. This must not be NULL.
     */
    CExceptionEntity(const CContextPtr& rptrContext, CEntityPtr ptrParent);

    /**
     * @brief Destructor
     */
    virtual ~CExceptionEntity() override = default;

    /**
     * @brief Get the type of the entity. Overload of CEntity::GetType.
     * @return the exception entity type.
     */
    virtual  sdv::idl::EEntityType GetType() const override { return  sdv::idl::EEntityType::type_exception; }

    /**
     * @brief Get the declaration type of the entity as string. Overload of CEntity::GetDeclTypeStr.
     * @param[in] bResolveTypedef When set, resolve the typedef type into the base type.
     * @return Returns a string with exception type.
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
     * @brief Create the content value node. Overload of CDefinitionEntity::CreateValueNode.
     * @details Create the value node and assign the value node to the ValueRef() reference..
     */
    virtual void CreateValueNode() override;
};


#endif // !defined(EXCEPTION_ENTITY_H)