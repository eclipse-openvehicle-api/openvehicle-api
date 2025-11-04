#ifndef MODULE_ENTITY_H
#define MODULE_ENTITY_H

#include "definition_entity.h"
#include <map>
#include "../constvariant.h"

/**
 * @brief The module definition of an IDL file.
 * @details The module section of the IDL file contains multiple definitions of nested modules, const definitions and type
 * definitions.
 */
class CModuleEntity : public CDefinitionEntity
{
public:
    /**
     * @brief Default constructor
     * @param[in] rptrContext Reference to the smart pointer holding the parse context. Must not be NULL.
     * @param[in] ptrParent Pointer to the parent class holding this entity. This must not be NULL.
     */
    CModuleEntity(const CContextPtr& rptrContext, CEntityPtr ptrParent);

protected:
    /**
     * @brief Root entity constructor (name is 'root' and no parent).
     * @param[in] rParser Reference to the parser.
     * @param[in] rptrContext Reference to the smart pointer holding the parse context. Must not be NULL.
     */
    CModuleEntity(CParser& rParser, const CContextPtr& rptrContext);

public:
    /**
     * @brief Destructor
     */
    virtual ~CModuleEntity() override = default;

    /**
     * @brief Get the type of the entity. Overload of CEntity::GetType.
     * @return Returns the module entity type.
     */
    virtual sdv::idl::EEntityType GetType() const override { return sdv::idl::EEntityType::type_module; }

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

    // Suppress warning of cppcheck of a useless override. The function implementation improves readability.
    // cppcheck-suppress uselessOverride
    /**
     * @brief Is this definition a root entity? Overload of CDefinitionEntity::IsRootEntity.
     * @details The root entity is not expecting curly brackets '{...}'.
     * @return Returns whether this is a root entity (which is not the case).
     */
    virtual bool IsRootEntity() const override { return false; }

    /**
     * @brief Is the entity extendable? Overload of CEntity::IsExtendable.
     * @details Allow extendability of the module entity.
     * @return Returns whether the entity is extendable.
     */
    virtual bool IsExtendable() const override;

    // Suppress cppcheck warning of a useless override. The function is here for better understanding.
    // cppcheck-suppress uselessOverride
    /**
     * @brief Does the entity support children? Overload of CEntity::SupportsChildren.
     * @details The module supports children.
     * @return Return whether the entity support children.
     */
    virtual bool SupportsChildren() const override;

private:
};


#endif // !defined(MODULE_ENTITY_H)