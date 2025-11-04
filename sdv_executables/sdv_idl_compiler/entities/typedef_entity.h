#ifndef TYPEDEF_ENTITY_H
#define TYPEDEF_ENTITY_H

#include "declaration_entity.h"

/**
 * @brief The const definition of an IDL file.
 * @details The const section of the IDL file defines const values.
 */
class CTypedefEntity : public CDeclarationEntity
{
public:
    /**
     * @brief Default constructor
     * @param[in] rptrContext Reference to the smart pointer holding the parse context. Must not be NULL.
     * @param[in] ptrParent Pointer to the parent class holding this entity. This must not be NULL.
     */
    CTypedefEntity(const CContextPtr& rptrContext, CEntityPtr ptrParent);

    /**
     * @brief Constructor for a local named type entity without parsing.
     * @attention This type entity is not attached as a child.
     * @param[in] rptrContext Reference to the smart pointer holding the parse context. Must not be NULL.
     * @param[in] ptrParent Pointer to the parent class holding this entity. This must not be NULL.
     * @param[in] rssName Reference to the name.
     */
    CTypedefEntity(const CContextPtr& rptrContext, CEntityPtr ptrParent, const std::string& rssName);

    /**
     * @brief Destructor
     */
    virtual ~CTypedefEntity() override = default;

    /**
     * @brief Get the type of the entity. Overload of CEntity::GetType.
     * @return The typedef entity type.
     */
    virtual sdv::idl::EEntityType GetType() const override { return sdv::idl::EEntityType::type_typedef; }

    /**
     * @brief Get the qualified type of the entity. Overload of CEntity::GetDeclTypeStr.
     * @attention To get the qualified type including array sizes, use the GetDeclTypeStr of the CEntityValueNode class.
     * @details The qualified type consists of "<base type> <type identifier>".
     * @param[in] bResolveTypedef When set, resolve the typedef type into the base type.
     * @return String with the typedef entity type.
     */
   virtual std::string GetDeclTypeStr(bool bResolveTypedef) const override;

    // Suppress cppcheck warning of a useless override. The function is here for better understanding.
    // cppcheck-suppress uselessOverride
    /**
     * @brief Process the code. Overload of CEntity::Process.
     */
    virtual void Process() override;

    /**
     * @brief Does the entity support arrays? Overload of CDeclarationEntity::SupportArrays.
     * @return Returns 'true' when the entity supports assignments; 'false' otherwise.
     */
    virtual bool SupportArrays() const override { return true; }

    /**
     * @brief Does the entity require an assignment? Overload of CDeclarationEntity::RequiresAssignment.
     * @return Returns 'true' when the entity requires an assignment; 'false' otherwise.
     */
    virtual bool RequiresAssignment() const override { return false; }

    /**
     * @brief Find the entity locally. Overload of CEntity::FindLocal.
     * @param[in] rssName Reference to the string object containing the name of the entity to search for.
     * @param[in] bDeclaration When set, the name belongs to a declaration; otherwise it belongs to a definition. Needed to allow
     * the reuse of names between declarations and definitions.
     * @return Returns a pair object containing an entity pointer if the entity exists or a NULL pointer if not as well as a
     * boolean that indicates that the entity was from an inherited entity.
     */
    virtual std::pair<CEntityPtr, bool> FindLocal(const std::string& rssName, bool bDeclaration) const override;
};

#endif // !defined(TYPEDEF_ENTITY_H)