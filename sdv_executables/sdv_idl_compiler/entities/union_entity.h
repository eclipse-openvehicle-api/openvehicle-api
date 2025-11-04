#ifndef UNION_ENTITY_H
#define UNION_ENTITY_H

#include "definition_entity.h"
#include "struct_entity.h"
#include "variable_entity.h"
#include "entity_value.h"
#include <set>

/**
 * @brief The enum entry declaration.
 */
class CCaseEntry : public CVariableEntity, public sdv::idl::ICaseEntity
{
public:
    /**
     * @brief Default constructor
     * @param[in] rptrContext Reference to the smart pointer holding the parse context. Must not be NULL.
     * @param[in] ptrParent Pointer to the parent class holding this entity. This must not be NULL.
     * @param[in] bDefault When set, the entry is the default case entry.
     */
    CCaseEntry(const CContextPtr& rptrContext, CEntityPtr ptrParent, bool bDefault);

    /**
     * @brief Destructor
     */
    virtual ~CCaseEntry() override = default;

    /**
     * @brief Get access to another interface. Overload of sdv::IInterfaceAccess::GetInterface.
     * @param[in] idInterface The interface id to get access to.
     * @return Returns a pointer to the interface or NULL when the interface is not supported.
     */
    virtual sdv::interface_t GetInterface(sdv::interface_id idInterface) override;

    /**
     * @brief Get the case label string. Overload of sdv::idl::ICaseEntity::GetLabel.
     * @return The label string.
     */
    virtual sdv::u8string GetLabel() const override;

    /**
     * @brief Is the case a default cae entry. Overload of sdv::idl::ICaseEntity::IsDefault.
     * @return Returns whether this is the default case entry.
     */
    virtual bool IsDefault() const override { return m_bDefault; }

    /**
     * @brief Get the label token (used for error reporting).
     * @return Returns a reference to the variable containing the label token.
     */
    const CToken& GetLabelToken() const;

    /**
     * @brief Get the type of the entity. Overload of CEntity::GetType.
     * @return Returns the union entity type.
     */
    virtual sdv::idl::EEntityType GetType() const override { return sdv::idl::EEntityType::type_case_entry; }

    /**
     * @brief Get the declaration type of the entity as string. Overload of CEntity::GetDeclTypeStr.
     * @param[in] bResolveTypedef When set, resolve the typedef type into the base type.
     * @return Returns a string with union type.
     */
    virtual std::string GetDeclTypeStr(bool bResolveTypedef) const override;

    /**
     * @brief Process the code. Overload of CEntity::Process.
     */
    virtual void Process() override;

    /**
     * @brief Post process the case entry.
     */
    void PostProcess();

    /**
     * @brief The entity doesn't support assignments. Overload of CDeclarationEntity::SupportAssignments.
     * @return Returns whether assignments are supported (which is not the case).
     */
    virtual bool SupportAssignments() const override { return false; }

private:
    bool                m_bDefault = false;     ///< When set, the case entry is the default case entry.
    CToken              m_tokenLabel;           ///< Label token.
    CEntityPtr          m_ptrLabel;             ///< The case label entity.
    CTokenList          m_lstCaseValue;         ///< Case value token list parsed during post processing.
};

/**
 * @brief The struct definition of an IDL file.
 * @details The struct section of the IDL file contains multiple declarations of members, as well as the definitions of structs
 * and unions.
 */
class CUnionEntity : public CStructEntity, public sdv::idl::IUnionEntity
{
public:
    /**
     * @brief Default constructor
     * @param[in] rptrContext Reference to the smart pointer holding the parse context. Must not be NULL.
     * @param[in] ptrParent Pointer to the parent class holding this entity. This must not be NULL.
     */
    CUnionEntity(const CContextPtr& rptrContext, CEntityPtr ptrParent);

    /**
     * @brief Destructor
     */
    virtual ~CUnionEntity() override = default;

    /**
     * @brief Get access to another interface. Overload of sdv::IInterfaceAccess::GetInterface.
     * @param[in] idInterface The interface id to get access to.
     * @return Returns a pointer to the interface or NULL when the interface is not supported.
     */
    virtual sdv::interface_t GetInterface(sdv::interface_id idInterface) override;

    /**
     * @brief Return the switch interpretation. Overload of sdv::idl::IUnionEntity::GetSwitchInterpretation.
     * @return The interpretation of the switch case of this union.
     */
    virtual sdv::idl::IUnionEntity::ESwitchInterpret GetSwitchInterpretation() const override;

    /**
     * @brief Return type information for the switch case. If the switch case is type base, this is the type information
     * that is used to select. If the switch case is variable based, this is the type of the variable. Overload of
     * sdv::idl::IUnionEntity::GetSwitchType.
     * @param[out] reType Reference to the declaration type (either enum or an integral type).
     * @param[out] rpType Reference to the type entity if existing.
     */
    virtual void GetSwitchType(/*out*/ sdv::idl::EDeclType& reType, /*out*/ sdv::IInterfaceAccess*& rpType) const;

    /**
     * @brief Get the switch variable information if the switch case is variable based. Will be empty/NULL when the switch
     * case is type based. Overload of sdv::idl::IUnionEntity::GetSwitchVar.
     * @param[out] rssVarStr Reference to the string receiving the exact scoped declaration name of the switch variable if
     * the interpretation is variable based. The variable name uses the scope separator '::' to define the common parent
     * definition and the member separator '.' to define the variable declaration as member from the common parent.
     * @param[out] rpVarEntity Reference to the variable entity if the interpretation is variable based.
     * @param[out] rpVarContainer Reference to the variable entity of the container of both the switch variable and the
     * union.
     */
    virtual void GetSwitchVar(/*out*/ sdv::u8string& rssVarStr, /*out*/ sdv::IInterfaceAccess*& rpVarEntity,
        /*out*/ sdv::IInterfaceAccess*& rpVarContainer) const;

    /**
     * @brief Get the type of the entity. Overload of CEntity::GetType.
     * @return Returns the union type.
     */
    virtual sdv::idl::EEntityType GetType() const override { return sdv::idl::EEntityType::type_union; }

    /**
     * @brief Get the declaration type of the entity as string. Overload of CEntity::GetDeclTypeStr.
     * @param[in] bResolveTypedef When set, resolve the typedef type into the base type.
     * @return Returns the declaration type string.
     */
    virtual std::string GetDeclTypeStr(bool bResolveTypedef) const override;

    /**
     * @brief Process the code. Overload of CEntity::Process.
     */
    virtual void Process() override;

    /**
     * @brief Process the definition addendum.
     * @details Process the definition addendum following the definition statement before the content definition. The default
     * implementation checks for an inheritance list.
     */
    virtual void ProcessDefinitionAddendum() override;

    /**
     * @brief Postprocess the switch/case assignments.
     */
    void PostProcess();

    /**
     * @brief Request whether the definition supports the content. Overload of CDefintionEntity::Supports.
     * @param[in] eSupport The type of support that is requested.
     * @return Returns 'true' when the definition supports the content; 'false' otherwise.
     */
    virtual bool Supports(EDefinitionSupport eSupport) const override;

    /**
     * @brief Does the entity support anonymous naming?
     * @details The default implementation is that anonymous naming is not supported.
     * @details Returns whether the entity supports inheritance.
     * @return Returns whether anonymous naming is supported.
     */
    virtual bool SupportsAnonymous() const override { return true; }

    /**
     * @brief Create the content value node. Overload of CDefinitionEntity::CreateValueNode.
     * @details Create the value node and assign the value node to the ValueRef() reference..
     */
    virtual void CreateValueNode() override;

    /**
     * @brief Does the definition require a declaration? Overload of CDefinitionEntity::RequireDeclaration.
     * @return Returns whether a declaration is required.
     */
    virtual bool RequireDeclaration() const override;

    /**
    * @brief Does the definition allow automatic transparent declaration if not present? Overload of
    * CDefinitionEntity::AllowAutoTransparentDeclaration.
    * @details When set an automatic transparent declaration is allowed without an explicit variable declaration. Currently this
    * is only the case with unions with a variable based switch type.
    * @return Returns whether the definition allows an automatic transparent declaration.
    */
    virtual bool AllowAutoTransparentDeclaration() const override;

    /**
     * @brief Get switch case type.
     * @param[out] reType Reference to the base type of the switch variable.
     * @param[out] rptrType Reference to the type definition of the switch variable. Can be null if the switch uses a basic type.
     * @param[out] rptrValue Reference to the value node of the switch variable. Can be null if the switch is determined by a type.
    */
    void GetSwitchCaseType(sdv::idl::EDeclType& reType, CEntityPtr& rptrType, CValueNodePtr& rptrValue);

private:
    /// Switch case interpretation.
    sdv::idl::IUnionEntity::ESwitchInterpret    m_eSwitchInterpret = sdv::idl::IUnionEntity::ESwitchInterpret::switch_type;
    std::string                 m_ssSwitchVar;          ///< Switch name
    CTypeDeclaration            m_typedeclSwitch;       ///< The identifier within the switch case.
    std::string                 m_ssValueNode;          ///< Value node of the variable. If not existing the switch could either be
                                                        ///< a type or a variable entity.
    CEntityPtr                  m_ptrContainer;         ///< The common container of both the switch variable and the union if the
                                                        ///< switch case is variable based.
    std::set<std::string>       m_setValues;            ///< Case entry values.
    CValueNodePtr               m_ptrSwitchValueNode;   ///< Value node of the switch entity of a variable based switch.

};

#endif // !defined(UNION_ENTITY_H)
