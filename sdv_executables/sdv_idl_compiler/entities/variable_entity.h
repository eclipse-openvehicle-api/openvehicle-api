#ifndef DECLARATOR_ENTITY_H
#define DECLARATOR_ENTITY_H

#include "declaration_entity.h"

/**
 * @brief The variable declaration.
 */
class CVariableEntity : public CDeclarationEntity
{
public:
    /**
     * @brief Default constructor
     * @param[in] rptrContext Reference to the smart pointer holding the parse context. Must not be NULL.
     * @param[in] ptrParent Pointer to the parent class holding this entity. This must not be NULL.
     * @param[in] bConst When set, the variable is defined as const.
     * @param[in] bAnonymous When set, the variable is part of a struct and anonymous (unnamed and not declared) so its members are
     * seen as members of the struct. For example, used with unions.
     */
    CVariableEntity(const CContextPtr& rptrContext, CEntityPtr ptrParent, bool bConst, bool bAnonymous);

    /**
     * @brief Constructor using the provided token-list to process the code.
     * @param[in] rptrContext Reference to the smart pointer holding the parse context. Must not be NULL.
     * @param[in] ptrParent Pointer to the parent class holding this entity. This must not be NULL.
     * @param[in] rlstTokenList Reference to the token list holding the tokens to process.
     * @param[in] bConst When set, the variable is defined as const.
     * @param[in] bAnonymous When set, the variable is part of a struct and anonymous (unnamed and not declared) so its members are
     * seen as members of the struct. For example, used with unions.
     */
    CVariableEntity(const CContextPtr& rptrContext, CEntityPtr ptrParent, const CTokenList& rlstTokenList, bool bConst,
        bool bAnonymous);

    /**
     * @brief Destructor
     */
    virtual ~CVariableEntity() override = default;

    /**
     * @brief Get the type of the entity. Overload of CEntity::GetType.
     * @return Returns the entity type.
     */
    virtual sdv::idl::EEntityType GetType() const override
    {
        return m_bPartOfSwitch ? sdv::idl::EEntityType::type_switch_variable : sdv::idl::EEntityType::type_variable;
    }

    /**
     * @brief Get the qualified type of the entity. Overload of CEntity::GetDeclTypeStr.
     * @attention To get the qualified type including array sizes, use the GetDeclTypeStr of the CEntityValueNode class.
     * @details The qualified type consists of "<base type> <type identifier>".
     * @param[in] bResolveTypedef When set, resolve the typedef type into the base type.
     * @return Returns the type string.
     */
   virtual std::string GetDeclTypeStr(bool bResolveTypedef) const override;

    // Suppress cppcheck warning of a useless override. The function is here for better understanding.
    // cppcheck-suppress uselessOverride
    /**
     * @brief Process the code. Overload of CEntity::Process.
     */
    virtual void Process() override;

    /**
     * @brief Does the entity support assignments? Overload of CDeclarationEntity::SupportAssignments.
     * @return Returns 'true' when the entity supports assignments; 'false' otherwise.
     */
    virtual bool SupportAssignments() const override { return true; }

    /**
     * @brief Can the entity be used for assignments of complex types? Overload of
     * CDeclarationEntity::CanSupportComplexTypeAssignments.
     * @return Returns 'true' when the entity defined as declaration; 'false' otherwise.
     */
    virtual bool CanSupportComplexTypeAssignments() const override { return true; }

    /**
     * @brief Does the entity support arrays? Overload of CDeclarationEntity::SupportArrays.
     * @return Returns 'true' when the entity supports assignments; 'false' otherwise.
     */
    virtual bool SupportArrays() const override { return true; }

    /**
     * @brief Is the entity readonly? Overload of IEntityInfo::IsReadOnly.
     * @return Returns 'true' when the entity defined as readonly; 'false' otherwise.
     */
    virtual bool IsReadOnly() const override { return m_bConst; }

    /**
     * @brief Does the entity require an assignment? Overload of CDeclarationEntity::RequiresAssignment.
     * @return Returns 'true' when the entity requires an assignment; 'false' otherwise.
     */
    virtual bool RequiresAssignment() const override { return m_bConst; }

    /**
     * @brief Does the entity support multiple declarations on one line of code?  Overload of
     * CDeclarationEntity::SupportMultipleDeclarations.
     * @return Returns 'true' when the entity supports multiple declarations; 'false' otherwise.
     */
    virtual bool SupportMultipleDeclarations() const override { return true; }

    /**
     * @brief Is the entity anonymous when used in a struct/union (unnamed and not declared)? Overload of
     * IDeclarationEntity::IsAnonymous.
     * @return Returns 'true' when the entity is anonymous; 'false' otherwise.
     */
    virtual bool IsAnonymous() const override { return m_bAnonymous; }

    /**
     * @brief Enable the variable to be used in a union switch.
     */
    void UseAsSwitchVariable() { m_bPartOfSwitch = true; }

protected:
    /**
     * @brief Set the variable as anonymous variable (unnamed and not declared).
     */
    void SetAnonymous() { m_bAnonymous = true; }

    /**
     * @brief Does the entity support an interface as base type (non-const variables, operations and parameters do)?
     * @details Returns whether the entity supports the an interface as base type base type. Default value is 'false'.
     * @return Returns 'true' when the entity supports interfaces as base type; 'false' otherwise.
     */
    virtual bool SupportInterface() const override { return !m_bConst; }

private:
    bool    m_bConst = false;           ///< When set, the variable is defined as const.
    bool    m_bAnonymous = false;       ///< When set, the variable declared anonymous (unnamed and not declared) so its members
                                        ///< are directly part of the parent struct.
    bool    m_bPartOfSwitch = false;    ///< When set, the variable is used in a union switch.
};


#endif // !defined(DECLARATOR_ENTITY_H)