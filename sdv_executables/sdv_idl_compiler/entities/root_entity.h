#ifndef ROOT_ENTITY_H
#define ROOT_ENTITY_H

#include "module_entity.h"
#include "meta_entity.h"

/**
 * @brief The root definition of an IDL file.
 * @details The root section of the IDL file contains multiple definitions of modules, constants and types.
 */
class CRootEntity : public CModuleEntity
{
public:
    /**
     * @brief Default constructor
     * @param[in] rParser Reference to the parser used to parse the code.
     * @param[in] rptrContext Reference to the smart pointer holding the parse context. Must not be NULL.
     */
    CRootEntity(CParser& rParser, const CContextPtr& rptrContext);

    /**
     * @brief Process the code. Overload of CModuleEntity::Process.
     */
    virtual void Process() override;

    /**
     * @brief Is this definition a root entity? Overload of CDefinitionEntity::IsRootEntity.
     * @details The root entity is not expecting curly brackets '{...}'.
     * @return Returns whether this entity is the root entity.
     */
    virtual bool IsRootEntity() const override { return true; }

    /**
     * @brief Is the entity extendable? Overload of CEntity::IsExtendable.
     * @details Prevents extending the root entity.
     * @return Returns whether the entity is extendable.
     */
    virtual bool IsExtendable() const override;

    /**
     * @brief Add meta data entity.
     * @param[in] ptrMeta Shared pointer to the meta data entity.
     */
    void AddMeta(const CEntityPtr& ptrMeta);

    /**
     * @brief Get the meta data entity list.
     * @return Reference to the meta data entity list.
    */
    const CEntityList& GetMeta() const;

private:
    CEntityList  m_lstMetaEntities;          ///< List of meta entities.

};

#endif // !defined(ROOT_ENTITY_H)